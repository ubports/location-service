/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include "provider.h"

#include <location/logging.h>
#include <location/runtime.h>
#include <location/events/reference_position_updated.h>
#include <location/glib/runtime.h>

#include <location/providers/ubx/_8/cfg/gnss.h>
#include <location/providers/ubx/_8/cfg/msg.h>
#include <location/providers/ubx/_8/nav/pvt.h>
#include <location/providers/ubx/_8/nav/sat.h>

#include <core/net/http/client.h>
#include <core/posix/this_process.h>

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>

namespace env = core::posix::this_process::env;
namespace ubx = location::providers::ubx;

namespace
{

struct SettingsHelper
{
    template<typename T>
    static T get_value(std::string key, T&& default_value)
    {
        static const std::string snap_path = env::get("SNAP_DATA");

        boost::filesystem::path path{snap_path};
        std::replace(key.begin(), key.end(), '.', '/');
        path /= key;

        LOG(INFO) << "Reading setting from " << path.string();

        std::ifstream in{path.string().c_str()};
        T value{default_value}; in >> value;

        return value;
    }
};

}

std::string ubx::Provider::class_name()
{
    return "ubx::Provider";
}

void ubx::Provider::Monitor::on_new_ubx_message(const _8::Message& message)
{
    VLOG(1) << message;
    if (auto sp = provider.lock())
        if (sp->configuration.protocol == Provider::Protocol::ubx)
            boost::apply_visitor(*this, message);
}


void ubx::Provider::Monitor::on_new_nmea_sentence(const nmea::Sentence& sentence)
{
    VLOG(1) << sentence;
    if (auto sp = provider.lock())
        if (sp->configuration.protocol == Provider::Protocol::nmea)
            boost::apply_visitor(*this, sentence);
}

void ubx::Provider::Monitor::operator()(const nmea::Gga& gga)
{   
    if (gga.latitude && gga.longitude)
    {
        auto lat = gga.latitude.get();
        auto lon = gga.longitude.get();

        Position position
        {
            (lat.degrees + lat.minutes / 60.f) * units::degrees,
            (lon.degrees + lon.minutes / 60.f) * units::degrees
        };

        if (gga.altitude)
            position.altitude(units::Meters::from_value(*gga.altitude));

        // TODO(tvoss): Maximum accuracy should be reported by the receiver
        // implementation rather than hardcoding 3 [m] here.
        if (gga.hdop)
            position.accuracy().horizontal(gga.hdop.get() * 3. * units::meters);

        auto thiz = shared_from_this();
        std::weak_ptr<ubx::Provider::Monitor> wp{thiz};

        glib::Runtime::instance()->dispatch([position, wp]()
        {
            if (auto sp = wp.lock())
                if (auto spp = sp->provider.lock())
                    spp->updates.position(location::Update<location::Position>{position});
        });
    }
}

void ubx::Provider::Monitor::operator()(const _8::nav::Pvt& pvt)
{
    if (pvt.fix_type == _8::nav::Pvt::FixType::no_fix)
        return;

    Position position
    {
        pvt.latitude * units::degrees,
        pvt.longitude * units::degrees
    };
    position.accuracy().horizontal(pvt.accuracy.horizontal * 1e-3 * units::meters);

    if (pvt.fix_type == _8::nav::Pvt::FixType::fix_3d)
    {
        position.altitude(pvt.height.above_msl * 1e-3 * units::meters);
        position.accuracy().vertical(pvt.accuracy.vertical * 1e-3 * units::meters);
    }

    units::Degrees heading = pvt.heading.vehicle * units::degrees;
    units::MetersPerSecond speed = pvt.speed_over_ground * 1e-3 * units::meters_per_second;

    auto thiz = shared_from_this();
    std::weak_ptr<ubx::Provider::Monitor> wp{thiz};

    glib::Runtime::instance()->dispatch([position, heading, speed, wp]()
    {
        if (auto sp = wp.lock())
        {
            if (auto spp = sp->provider.lock())
            {
                spp->updates.position(location::Update<location::Position>{position});
                spp->updates.heading(location::Update<units::Degrees>{heading});
                spp->updates.velocity(location::Update<units::MetersPerSecond>{speed});
            }
        }
    });
}

void ubx::Provider::Monitor::operator()(const nmea::Gsa&)
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const nmea::Gll&)
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const nmea::Gsv&)
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const nmea::Rmc&)
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const nmea::Txt&)
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const nmea::Vtg& vtg)
{
    auto thiz = shared_from_this();
    std::weak_ptr<ubx::Provider::Monitor> wp{thiz};

    glib::Runtime::instance()->dispatch([vtg, wp]()
    {
        if (auto sp = wp.lock())
        {
            if (vtg.cog_true)
                if (auto spp = sp->provider.lock())
                    spp->updates.heading(
                                Update<units::Degrees>(
                                    vtg.cog_true.get() * units::degrees));
            if (vtg.sog_kmh)
                if (auto spp = sp->provider.lock())
                    spp->updates.velocity(
                                Update<units::MetersPerSecond>(
                                    vtg.sog_kmh.get() * 1000./3600. * units::meters_per_second));
        }
    });
}

location::Provider::Ptr ubx::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    Configuration configuration
    {
        SettingsHelper::get_value<Protocol>(
            "ubx.provider.protocol",
            Protocol::ubx),
        config.get<std::string>(
            "device", SettingsHelper::get_value<std::string>(
                    "ubx.provider.path",
                    "/dev/ttyACM1"
            )
        ),
        {
            SettingsHelper::get_value<std::string>(
                "ubx.provider.assist_now.enable",
                "false"
            ) == "true",
            SettingsHelper::get_value<std::string>(
                "ubx.provider.assist_now.token",
                ""
            ),
            boost::posix_time::seconds(
                boost::lexical_cast<std::uint64_t>(
                    SettingsHelper::get_value<std::string>(
                        "ubx.provider.assist_now.acquisition_timeout",
                        "5"
                    )
                )
            )
        }
    };

    return ubx::Provider::create(configuration);
}

// Create a new instance with configuration.
std::shared_ptr<ubx::Provider> ubx::Provider::create(const Configuration& configuration)
{
    auto sp = std::shared_ptr<Provider>{new Provider{configuration}};
    return sp->finalize_construction();
}


ubx::Provider::Provider(const Configuration& configuration)
    : configuration{configuration},
      runtime{location::Runtime::create(1)},
      monitor{std::make_shared<Monitor>()},
      receiver{_8::SerialPortReceiver::create(runtime->service(), configuration.device, monitor)},
      assist_now_online_client{std::make_shared<_8::AssistNowOnlineClient>(core::net::http::make_client())},
      acquisition_timer{runtime->service()}
{
    runtime->start();

    configure_gnss();
    configure_protocol();
}

ubx::Provider::~Provider() noexcept
{
    deactivate();
    runtime->stop();
}

void ubx::Provider::reset()
{
    receiver->send_message(_8::cfg::Rst{_8::cfg::Rst::Bits::cold_start, _8::cfg::Rst::Mode::controlled_software_reset_gnss});
}

void ubx::Provider::on_new_event(const Event&)
{
    // TODO(tvoss): Use incoming reference position updates
    // to query assistance data.
}

location::Provider::Requirements ubx::Provider::requirements() const
{
    return Requirements::none;
}

bool ubx::Provider::satisfies(const location::Criteria&)
{
    return true;
}

void ubx::Provider::enable()
{
}

void ubx::Provider::disable()
{
}

void ubx::Provider::activate()
{
    receiver->start();

    if (configuration.assist_now.enable)
    {
        auto thiz = shared_from_this();
        std::weak_ptr<Provider> wp{thiz};

        acquisition_timer.expires_from_now(configuration.assist_now.acquisition_timeout);
        acquisition_timer.async_wait([this, wp](boost::system::error_code ec)
        {
            if (auto sp = wp.lock())
                if (!ec) request_assist_now_online_data(Optional<Position>{});
        });
    }
}

void ubx::Provider::deactivate()
{
    receiver->stop();
    acquisition_timer.cancel();
}

const core::Signal<location::Update<location::Position>>& ubx::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& ubx::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& ubx::Provider::velocity_updates() const
{
    return updates.velocity;
}

std::shared_ptr<ubx::Provider> ubx::Provider::finalize_construction()
{
    auto thiz = shared_from_this();
    std::weak_ptr<ubx::Provider> wp{thiz};

    updates.position.connect([wp](const Update<Position>&)
    {
        if (auto sp = wp.lock())
            sp->acquisition_timer.cancel();
    });

    monitor->provider = wp;
    return thiz;
}

void ubx::Provider::configure_gnss()
{
    _8::cfg::Gnss::Gps gps;
    gps.l1ca = true;
    gps.enable = true;
    gps.min_tracking_channels = 4;
    gps.max_tracking_channels = 8;

    _8::cfg::Gnss::Galileo galileo;
    galileo.e1os = true;
    galileo.enable = true;
    galileo.min_tracking_channels = 4;
    galileo.max_tracking_channels = 8;

    _8::cfg::Gnss::Glonass glonass;
    glonass.l1of = true;
    glonass.enable = true;
    glonass.min_tracking_channels = 4;
    glonass.max_tracking_channels = 8;

    _8::cfg::Gnss::Sbas sbas;
    sbas.l1ca = true;
    sbas.enable = true;
    sbas.min_tracking_channels = 4;
    sbas.max_tracking_channels = 8;

    _8::cfg::Gnss gnss;
    gnss.gps = gps;
    gnss.galileo = galileo;
    gnss.glonass = glonass;
    gnss.sbas = sbas;

    receiver->send_message(gnss);
}

void ubx::Provider::configure_protocol()
{
    if (configuration.protocol == Protocol::ubx)
    {
        _8::cfg::Msg cfg_msg{ubx::_8::nav::Pvt::class_id, ubx::_8::nav::Pvt::message_id, { 0 }};
        cfg_msg.rate[_8::cfg::Msg::Port::usb] = 1;
        cfg_msg.rate[_8::cfg::Msg::Port::uart1] = 1;
        receiver->send_message(cfg_msg);

        cfg_msg.configured_class_id = _8::nav::Sat::class_id;
        cfg_msg.configured_message_id = _8::nav::Sat::message_id;
        receiver->send_message(cfg_msg);
    }
}

void ubx::Provider::request_assist_now_online_data(const Optional<Position>& position)
{
    auto thiz = shared_from_this();
    std::weak_ptr<Provider> wp{thiz};

    _8::AssistNowOnlineClient::Parameters params;
    params.token = configuration.assist_now.token;
    params.gnss = {_8::GnssId::gps, _8::GnssId::glonass, _8::GnssId::galileo};
    params.data_types =
    {
        _8::AssistNowOnlineClient::DataType::almanac,
        _8::AssistNowOnlineClient::DataType::ephemeris
    };
    params.position = position;

    assist_now_online_client->request_assistance_data(params, [this, wp](const Result<std::string>& result)
    {
        if (result)
        {
            if (auto sp = wp.lock())
            {
                LOG(INFO) << "Successfully queried assistance data, injecting into chipset now.";
                receiver->send_encoded_message(
                        std::vector<std::uint8_t>(
                            result.value().begin(), result.value().end()));
            }
        }
        else
        {
            try
            {
                result.rethrow();
            }
            catch (const std::exception& e)
            {
                LOG(WARNING) << "Failed to query AssistNow for aiding data: " << e.what() << std::endl;
            }
            catch (...)
            {
                LOG(WARNING) << "Failed to query AssistNow for aiding data." << std::endl;
            }
        }
    });
}

std::istream& ubx::operator>>(std::istream& in, Provider::Protocol& protocol)
{
    std::string value;
    in >> value;

    if (value == "ubx")
        protocol = Provider::Protocol::ubx;
    if (value == "nmea")
        protocol = Provider::Protocol::nmea;

    return in;
}

std::ostream& ubx::operator<<(std::ostream& out, Provider::Protocol protocol)
{
    switch (protocol)
    {
    case Provider::Protocol::ubx:
        out << "ubx";
        break;
    case Provider::Protocol::nmea:
        out << "nmea";
        break;
    }

    return out;
}

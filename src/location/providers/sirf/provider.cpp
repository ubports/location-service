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

#include <location/providers/sirf/provider.h>

#include <location/logging.h>
#include <location/runtime.h>
#include <location/events/reference_position_updated.h>
#include <location/glib/runtime.h>
#include <location/util/settings.h>

#include <core/net/http/client.h>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iterator>
#include <thread>

namespace sirf = location::providers::sirf;

namespace
{

namespace options
{

constexpr const char* protocol{"sirf.provider.protocol"};
constexpr const char* device{"sirf.provider.device"};

}  // namespace options
}  // namespace

void sirf::Provider::Monitor::on_new_sirf_message(const Message& message)
{
    VLOG(1) << message;
    if (auto sp = provider.lock())
        if (sp->configuration.protocol == Provider::Protocol::sirf)
            boost::apply_visitor(*this, message);
}


void sirf::Provider::Monitor::on_new_nmea_sentence(const nmea::Sentence& sentence)
{
    VLOG(1) << sentence;
    if (auto sp = provider.lock())
        if (sp->configuration.protocol == Provider::Protocol::nmea)
            boost::apply_visitor(*this, sentence);
}

void sirf::Provider::Monitor::operator()(const sirf::GeodeticNavigationData& gnd)
{
    auto thiz = shared_from_this();
    std::weak_ptr<sirf::Provider::Monitor> wp{thiz};

    glib::Runtime::instance()->dispatch([gnd, wp]()
    {
        if (auto sp = wp.lock())
        {
            if (auto spp = sp->provider.lock())
            {
                if (gnd.nav_valid == GeodeticNavigationData::valid_navigation)
                {
                    Position position
                    {
                        gnd.latitude * units::degrees,
                        gnd.longitude * units::degrees
                    };
                    position.accuracy().horizontal(gnd.error.horizontal_position * units::meters);

                    position.altitude(gnd.altitude.above_mean_sea_level * units::meters);
                    position.accuracy().vertical(gnd.error.vertical_position * units::meters);

                    spp->updates.position(location::Update<location::Position>{position});
                    if (!(gnd.nav_type & GeodeticNavigationData::invalid_heading))
                        spp->updates.heading(location::Update<units::Degrees>{gnd.over_ground.course * units::degrees});
                    if (!(gnd.nav_type & GeodeticNavigationData::invalid_velocity))
                        spp->updates.velocity(location::Update<units::MetersPerSecond>{gnd.over_ground.speed * units::meters_per_second});
                }
            }
        }
    });
}

void sirf::Provider::Monitor::operator()(const nmea::Gga& gga)
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
        std::weak_ptr<sirf::Provider::Monitor> wp{thiz};

        glib::Runtime::instance()->dispatch([position, wp]()
        {
            if (auto sp = wp.lock())
                if (auto spp = sp->provider.lock())
                    spp->updates.position(location::Update<location::Position>{position});
        });
    }
}

void sirf::Provider::Monitor::operator()(const nmea::Gsa&)
{
    // Empty on purpose
}

void sirf::Provider::Monitor::operator()(const nmea::Gll&)
{
    // Empty on purpose
}

void sirf::Provider::Monitor::operator()(const nmea::Gsv&)
{
    // Empty on purpose
}

void sirf::Provider::Monitor::operator()(const nmea::Rmc&)
{
    // Empty on purpose
}

void sirf::Provider::Monitor::operator()(const nmea::Txt&)
{
    // Empty on purpose
}

void sirf::Provider::Monitor::operator()(const nmea::Vtg& vtg)
{
    auto thiz = shared_from_this();
    std::weak_ptr<sirf::Provider::Monitor> wp{thiz};

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

void sirf::Provider::add_to_registry()
{
    ProviderRegistry::instance().add_provider_for_name("sirf::Provider", [](const util::settings::Source& settings)
    {
        return sirf::Provider::create_instance(settings);
    },
    {
        {options::protocol, "switch between binary SiRF or textual NMEA protocol"},
        {options::device, "read data from this device"}
    });
}

location::Provider::Ptr sirf::Provider::create_instance(const util::settings::Source& settings)
{
    Configuration configuration
    {
        settings.get_value<Protocol>(options::protocol, Protocol::sirf),
        settings.get_value<std::string>(options::device, "/dev/ttyUSB0")
    };

    return sirf::Provider::create(configuration);
}

// Create a new instance with configuration.
std::shared_ptr<sirf::Provider> sirf::Provider::create(const Configuration& configuration)
{
    auto sp = std::shared_ptr<Provider>{new Provider{configuration}};
    return sp->finalize_construction();
}


sirf::Provider::Provider(const Configuration& configuration)
    : configuration{configuration},
      runtime{location::Runtime::create(1)},
      monitor{std::make_shared<Monitor>()},
      receiver{SerialPortReceiver::create(runtime->service(), configuration.device, monitor)}
{
    runtime->start();
    configure_protocol();
}

sirf::Provider::~Provider() noexcept
{
    deactivate();
    runtime->stop();
}

void sirf::Provider::reset()
{
    InitializeDataSource ids;
    ids.channels = InitializeDataSource::max_number_channels;
    ids.reset_configuration = InitializeDataSource::clear_ephemeris_data | InitializeDataSource::clear_all_history | InitializeDataSource::reset;
    receiver->send_message(Message{ids});
}

void sirf::Provider::on_new_event(const Event&)
{
    // TODO(tvoss): Use incoming reference position updates
    // to query assistance data.
}

location::Provider::Requirements sirf::Provider::requirements() const
{
    return Requirements::none;
}

bool sirf::Provider::satisfies(const location::Criteria&)
{
    return true;
}

void sirf::Provider::enable()
{
}

void sirf::Provider::disable()
{
}

void sirf::Provider::activate()
{
    receiver->start();
}

void sirf::Provider::deactivate()
{
    receiver->stop();
}

const core::Signal<location::Update<location::Position>>& sirf::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& sirf::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& sirf::Provider::velocity_updates() const
{
    return updates.velocity;
}

std::shared_ptr<sirf::Provider> sirf::Provider::finalize_construction()
{
    auto thiz = shared_from_this();
    std::weak_ptr<sirf::Provider> wp{thiz};

    monitor->provider = wp;
    return thiz;
}

void sirf::Provider::configure_protocol()
{
    switch(configuration.protocol)
    {
    case Protocol::sirf:
    {
        static const std::string set_serial_port{"$PSRF100,0,4800,8,1,0*0F\r\n"};
        receiver->send_encoded_message(std::vector<std::uint8_t>{set_serial_port.begin(), set_serial_port.end()});
        receiver->send_message(Message{SetMessageRate{SetMessageRate::one_message, GeodeticNavigationData::id, 1}});
        break;
    }
    case Protocol::nmea:
        receiver->send_message(Message{SetProtocol{SetProtocol::nmea}});
        break;
    default:
        break;
    }
}

std::istream& sirf::operator>>(std::istream& in, Provider::Protocol& protocol)
{
    std::string value;
    in >> value;

    if (value == "sirf")
        protocol = Provider::Protocol::sirf;
    if (value == "nmea")
        protocol = Provider::Protocol::nmea;

    return in;
}

std::ostream& sirf::operator<<(std::ostream& out, Provider::Protocol protocol)
{
    switch (protocol)
    {
    case Provider::Protocol::sirf:
        out << "sirf";
        break;
    case Provider::Protocol::nmea:
        out << "nmea";
        break;
    }

    return out;
}

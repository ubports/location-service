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
#include <location/glib/runtime.h>

#include <location/providers/ubx/_8/cfg/gnss.h>
#include <location/providers/ubx/_8/cfg/msg.h>
#include <location/providers/ubx/_8/nav/pvt.h>
#include <location/providers/ubx/_8/nav/sat.h>

#include <core/posix/this_process.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>

namespace env = core::posix::this_process::env;
namespace ubx = location::providers::ubx;

std::string ubx::Provider::class_name()
{
    return "ubx::Provider";
}

ubx::Provider::Monitor::Monitor(Provider* provider) : provider{provider}
{
}

void ubx::Provider::Monitor::on_new_ubx_message(const _8::Message& message)
{
    VLOG(1) << message;
    if (provider->protocol == Provider::Protocol::ubx)
        boost::apply_visitor(*this, message);
}


void ubx::Provider::Monitor::on_new_nmea_sentence(const _8::nmea::Sentence& sentence)
{
    VLOG(1) << sentence;
    if (provider->protocol == Provider::Protocol::nmea)
        boost::apply_visitor(*this, sentence);
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Gga& gga) const
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

        glib::Runtime::instance()->dispatch([this, position]()
        {
            provider->updates.position(location::Update<location::Position>{position});
        });
    }
}

void ubx::Provider::Monitor::operator()(const _8::nav::Pvt& pvt) const
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

    glib::Runtime::instance()->dispatch([this, position, heading, speed]()
    {
        provider->updates.position(location::Update<location::Position>{position});
        provider->updates.heading(location::Update<units::Degrees>{heading});
        provider->updates.velocity(location::Update<units::MetersPerSecond>{speed});
    });
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Gsa&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Gll&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Gsv&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Rmc&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Txt&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Vtg& vtg) const
{
    glib::Runtime::instance()->dispatch([this, vtg]()
    {
        if (vtg.cog_true)
            provider->updates.heading(
                        Update<units::Degrees>(
                            vtg.cog_true.get() * units::degrees));
        if (vtg.sog_kmh)
            provider->updates.velocity(
                        Update<units::MetersPerSecond>(
                            vtg.sog_kmh.get() * 1000./3600. * units::meters_per_second));
    });
}

location::Provider::Ptr ubx::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    std::string device_path = "/dev/ttyACM0";
    std::ifstream in(env::get("SNAP_DATA") + "/ubx/provider/path");
    in >> device_path;

    return location::Provider::Ptr{new ubx::Provider{
            Protocol::ubx, config.get<std::string>("device", device_path)}};
}

ubx::Provider::Provider(Protocol protocol, const boost::filesystem::path& device)
    : protocol{protocol},
      runtime{location::Runtime::create(1)},
      monitor{std::make_shared<Monitor>(this)},
      receiver{_8::SerialPortReceiver::create(runtime->service(), device, monitor)}
{   
    runtime->start();

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

    if (protocol == Protocol::ubx)
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

ubx::Provider::~Provider() noexcept
{
    deactivate();
    runtime->stop();
}

void ubx::Provider::on_new_event(const Event&)
{
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
}

void ubx::Provider::deactivate()
{
    receiver->stop();
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

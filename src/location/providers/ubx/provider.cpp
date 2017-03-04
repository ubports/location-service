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

#include <core/posix/this_process.h>

#include <fstream>
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

void ubx::Provider::Monitor::on_new_chunk(_8::Receiver::Buffer::iterator, _8::Receiver::Buffer::iterator)
{
    // We drop the chunk on purpose.
}

void ubx::Provider::Monitor::on_new_nmea_sentence(const _8::nmea::Sentence& sentence)
{
    // TODO(tvoss): This is a little verbose and we should remove it
    // for production scenarios.
    LOG(INFO) << sentence;
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

        provider->updates.position(location::Update<location::Position>{position});
    }
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
    if (vtg.cog_true)
        provider->updates.heading(
                    Update<units::Degrees>(
                        vtg.cog_true.get() * units::degrees));
    if (vtg.sog_kmh)
        provider->updates.velocity(
                    Update<units::MetersPerSecond>(
                        vtg.sog_kmh.get() * 1000./3600. * units::meters_per_second));
}

location::Provider::Ptr ubx::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    std::string device_path;
    std::ifstream in(env::get("SNAP_DATA") + "/ubx/provider/path");

    in >> device_path;

    return location::Provider::Ptr{new ubx::Provider{config.get<std::string>("device", device_path.empty() ? "/dev/ttyACM0" : device_path)}};
}

ubx::Provider::Provider(const boost::filesystem::path& device)
    : runtime{location::Runtime::create(1)},
      monitor{std::make_shared<Monitor>(this)},
      receiver{_8::SerialPortReceiver::create(runtime->service(), device, monitor)}
{
    runtime->start();
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

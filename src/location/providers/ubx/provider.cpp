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

#include <thread>

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
    // We are dropping raw chunks on purpose here.
}

void ubx::Provider::Monitor::on_new_nmea_sentence(const _8::nmea::Sentence& sentence)
{
    boost::apply_visitor(*this, sentence);
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Gga&) const
{
    // Empty on purpose
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

void ubx::Provider::Monitor::operator()(const _8::nmea::Rmc& rmc) const
{
    if (rmc.latitude && rmc.longitude)
    {
        auto lat = rmc.latitude.get();
        auto lon = rmc.longitude.get();

        auto pos = location::Position{
            (lat.degrees + lat.minutes) * units::degrees,
            (lon.degrees + lon.minutes) * units::degrees};

        provider->updates.position(location::Update<location::Position>{pos});
    }

    if (rmc.course_over_ground)
        provider->updates.heading(location::Update<units::Degrees>{rmc.course_over_ground.get() * units::degrees});

    if (rmc.speed_over_ground)
        provider->updates.velocity(location::Update<units::MetersPerSecond>{rmc.speed_over_ground.get() * units::meters_per_second});
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Txt&) const
{
    // Empty on purpose
}

void ubx::Provider::Monitor::operator()(const _8::nmea::Vtg&) const
{
    // Empty on purpose
}

location::Provider::Ptr ubx::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    return location::Provider::Ptr{new ubx::Provider{config.get<std::string>("device", "/dev/ttyS5")}};
}

ubx::Provider::Provider(const boost::filesystem::path& device)
    : runtime{location::Runtime::create(1)},
      monitor{std::make_shared<Monitor>(this)},
      receiver{_8::SerialPortReceiver::create(runtime->service(), device, monitor)}
{
}

ubx::Provider::~Provider() noexcept
{
    deactivate();
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
    receiver->start();
}

void ubx::Provider::disable()
{
    receiver->stop();
}

void ubx::Provider::activate()
{
}

void ubx::Provider::deactivate()
{
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

/*
 * Copyright © 2012-2013 Canonical Ltd.
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

#include "hardware_abstraction_layer.h"

#include <location/logging.h>
#include <location/connectivity/manager.h>

#include <ubuntu/hardware/gps.h>


std::string location::providers::gps::Provider::class_name()
{
    return "gps::Provider";
}

location::Provider::Ptr location::providers::gps::Provider::create_instance(const location::ProviderRegistry::Configuration&)
{
    return location::Provider::Ptr{new location::providers::gps::Provider{location::providers::gps::HardwareAbstractionLayer::create_default_instance()}};
}

location::providers::gps::Provider::Provider(const std::shared_ptr<HardwareAbstractionLayer>& hal)
    : hal(hal)
{

    hal->position_updates().connect([this](const location::Position& pos)
    {
        updates.position(Update<Position>(pos));
    });

    hal->heading_updates().connect([this](const units::Degrees& heading)
    {
        updates.heading(Update<units::Degrees>(heading));
    });

    hal->velocity_updates().connect([this](const units::MetersPerSecond& velocity)
    {
        updates.velocity(Update<units::MetersPerSecond>(velocity));
    });

    hal->space_vehicle_updates().connect([this](const std::set<location::SpaceVehicle>& svs)
    {
        updates.svs(Update<std::set<location::SpaceVehicle>>(svs));
    });
}

location::providers::gps::Provider::~Provider() noexcept
{
}

void location::providers::gps::Provider::on_new_event(const Event&)
{
}

location::Provider::Requirements location::providers::gps::Provider::requirements() const
{
    return Requirements::satellites;
}

bool location::providers::gps::Provider::satisfies(const location::Criteria&)
{
    return true;
}

void location::providers::gps::Provider::enable()
{
}

void location::providers::gps::Provider::disable()
{
}

void location::providers::gps::Provider::activate()
{
    hal->start_positioning();
}

void location::providers::gps::Provider::deactivate()
{
    hal->stop_positioning();
}

const core::Signal<location::Update<location::Position>>& location::providers::gps::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& location::providers::gps::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& location::providers::gps::Provider::velocity_updates() const
{
    return updates.velocity;
}

const core::Signal<location::Update<std::set<location::SpaceVehicle>>>& location::providers::gps::Provider::svs_updates() const
{
    return updates.svs;
}

void location::providers::gps::Provider::on_reference_location_updated(const location::Update<location::Position>& position)
{
    hal->inject_reference_position(position.value);
}

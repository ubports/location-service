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

location::Provider::Ptr location::providers::gps::Provider::create_instance(const location::ProviderFactory::Configuration&)
{
    return location::Provider::Ptr{new location::providers::gps::Provider{location::providers::gps::HardwareAbstractionLayer::create_default_instance()}};
}

location::providers::gps::Provider::Provider(const std::shared_ptr<HardwareAbstractionLayer>& hal)
    : location::Provider(
          location::Provider::Features::position | location::Provider::Features::velocity | location::Provider::Features::heading,
          location::Provider::Requirements::satellites),
          hal(hal)
{

    hal->position_updates().connect([this](const location::Position& pos)
    {
        mutable_updates().position(Update<Position>(pos));
    });

    hal->heading_updates().connect([this](const location::Heading& heading)
    {
        mutable_updates().heading(Update<Heading>(heading));
    });

    hal->velocity_updates().connect([this](const location::Velocity& velocity)
    {
        mutable_updates().velocity(Update<Velocity>(velocity));
    });

    hal->space_vehicle_updates().connect([this](const std::set<location::SpaceVehicle>& svs)
    {
        mutable_updates().svs(Update<std::set<location::SpaceVehicle>>(svs));
    });
}

location::providers::gps::Provider::~Provider() noexcept
{
}

bool location::providers::gps::Provider::matches_criteria(const location::Criteria&)
{
    return true;
}

void location::providers::gps::Provider::start_position_updates()
{
    hal->start_positioning();
}

void location::providers::gps::Provider::stop_position_updates()
{
    hal->stop_positioning();
}

void location::providers::gps::Provider::start_velocity_updates()
{   
}

void location::providers::gps::Provider::stop_velocity_updates()
{
}    

void location::providers::gps::Provider::start_heading_updates()
{
}

void location::providers::gps::Provider::stop_heading_updates()
{
}

void location::providers::gps::Provider::on_reference_location_updated(const location::Update<location::Position>& position)
{
    hal->inject_reference_position(position.value);
}


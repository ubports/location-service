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

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/connectivity/manager.h>

#include <ubuntu/hardware/gps.h>

namespace cul = com::ubuntu::location;
namespace culg = com::ubuntu::location::providers::gps;

struct culg::Provider::Private
{
    std::shared_ptr<HardwareAbstractionLayer> hal;
};

std::string culg::Provider::class_name()
{
    return "gps::Provider";
}

cul::Provider::Ptr culg::Provider::create_instance(const cul::ProviderFactory::Configuration&)
{
    return cul::Provider::Ptr{new culg::Provider{culg::HardwareAbstractionLayer::create_default_instance()}};
}

culg::Provider::Provider(const std::shared_ptr<HardwareAbstractionLayer>& hal)
    : cul::Provider(
          cul::Provider::Features::position | cul::Provider::Features::velocity | cul::Provider::Features::heading,
          cul::Provider::Requirements::satellites),
      d(new Private())
{
    d->hal = hal;

    d->hal->position_updates().connect([this](const location::Position& pos)
    {
        mutable_updates().position(Update<Position>(pos));
    });

    d->hal->heading_updates().connect([this](const location::Heading& heading)
    {
        mutable_updates().heading(Update<Heading>(heading));
    });

    d->hal->velocity_updates().connect([this](const location::Velocity& velocity)
    {
        mutable_updates().velocity(Update<Velocity>(velocity));
    });

    d->hal->space_vehicle_updates().connect([this](const std::set<location::SpaceVehicle>& svs)
    {
        mutable_updates().svs(Update<std::set<location::SpaceVehicle>>(svs));
    });
}

culg::Provider::~Provider() noexcept
{
    d->hal->stop_positioning();
}

bool culg::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culg::Provider::start_position_updates()
{
    d->hal->start_positioning();
}

void culg::Provider::stop_position_updates()
{
    d->hal->stop_positioning();
}

void culg::Provider::start_velocity_updates()
{
    d->hal->start_positioning();
}

void culg::Provider::stop_velocity_updates()
{
    d->hal->stop_positioning();
}    

void culg::Provider::start_heading_updates()
{
    d->hal->start_positioning();
}

void culg::Provider::stop_heading_updates()
{
    d->hal->stop_positioning();
}

void culg::Provider::on_reference_location_updated(const cul::Update<cul::Position>& position)
{
    d->hal->inject_reference_position(position.value);
}


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
#include <location/provider.h>

#include <atomic>
#include <bitset>
#include <memory>

namespace
{
static const int our_magic_disabling_value = -1;
}

void location::Provider::Controller::enable()
{
    position_updates_counter = 0;
    heading_updates_counter = 0;
    velocity_updates_counter = 0;
}

void location::Provider::Controller::disable()
{    
    if (position_updates_counter > 0)
        instance.stop_position_updates();
    if (heading_updates_counter > 0)
        instance.stop_heading_updates();
    if (velocity_updates_counter > 0)
        instance.stop_velocity_updates();

    position_updates_counter = our_magic_disabling_value;
    heading_updates_counter = our_magic_disabling_value;
    velocity_updates_counter = our_magic_disabling_value;
}

void location::Provider::Controller::start_position_updates()
{
    if (position_updates_counter < 0)
        return;

    if (++position_updates_counter == 1)
    {
        instance.start_position_updates();
    }
}

void location::Provider::Controller::stop_position_updates()
{
    if (position_updates_counter < 0)
        return;

    if (--position_updates_counter == 0)
    {
        instance.stop_position_updates();
    }
}

bool location::Provider::Controller::are_position_updates_running() const
{
    return position_updates_counter > 0;
}

void location::Provider::Controller::start_heading_updates()
{
    if (heading_updates_counter < 0)
        return;

    if (++heading_updates_counter == 1)
    {
        instance.start_heading_updates();
    }
}

void location::Provider::Controller::stop_heading_updates()
{
    if (heading_updates_counter < 0)
        return;

    if (--heading_updates_counter == 0)
    {
        instance.stop_heading_updates();
    }
}

bool location::Provider::Controller::are_heading_updates_running() const
{
    return heading_updates_counter > 0;
}

void location::Provider::Controller::start_velocity_updates()
{
    if (velocity_updates_counter < 0)
        return;

    if (++velocity_updates_counter == 1)
    {
        instance.start_velocity_updates();
    }
}

void location::Provider::Controller::stop_velocity_updates()
{
    if (velocity_updates_counter < 0)
        return;

    if (--velocity_updates_counter == 0)
    {
        instance.stop_velocity_updates();
    }
}

bool location::Provider::Controller::are_velocity_updates_running() const
{
    return velocity_updates_counter > 0;
}

location::Provider::Controller::Controller(location::Provider& instance)
    : instance(instance),
      position_updates_counter(0),
      heading_updates_counter(0),
      velocity_updates_counter(0)
{  
}

const location::Provider::Controller::Ptr& location::Provider::state_controller() const
{
    return d.controller;
}

bool location::Provider::supports(const location::Provider::Features& f) const
{
    return (d.features & f) != Features::none;
}

bool location::Provider::requires(const location::Provider::Requirements& r) const
{
    return (d.requirements & r) != Requirements::none;
}

bool location::Provider::matches_criteria(const location::Criteria&)
{
    return false;
}

const location::Provider::Updates& location::Provider::updates() const
{
    return d.updates;
}

location::Provider::Provider(
    const location::Provider::Features& features,
    const location::Provider::Requirements& requirements)
{
    d.features = features;
    d.requirements = requirements;
    d.controller = std::shared_ptr<Provider::Controller>(new Provider::Controller(*this));
}

location::Provider::Updates& location::Provider::mutable_updates()
{
    return d.updates;
}

void location::Provider::on_wifi_and_cell_reporting_state_changed(location::WifiAndCellIdReportingState)
{
}

void location::Provider::on_reference_location_updated(const location::Update<location::Position>&)
{
}

void location::Provider::on_reference_velocity_updated(const location::Update<location::Velocity>&)
{
}

void location::Provider::on_reference_heading_updated(const location::Update<location::Heading>&)
{
}

void location::Provider::start_position_updates() {}
void location::Provider::stop_position_updates() {}
void location::Provider::start_heading_updates() {}
void location::Provider::stop_heading_updates() {}
void location::Provider::start_velocity_updates() {}
void location::Provider::stop_velocity_updates() {}

location::Provider::Features location::operator|(location::Provider::Features lhs, location::Provider::Features rhs)
{
    return static_cast<location::Provider::Features>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

location::Provider::Features location::operator&(location::Provider::Features lhs, location::Provider::Features rhs)
{
    return static_cast<location::Provider::Features>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

location::Provider::Requirements location::operator|(location::Provider::Requirements lhs, location::Provider::Requirements rhs)
{
    return static_cast<location::Provider::Requirements>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

location::Provider::Requirements location::operator&(location::Provider::Requirements lhs, location::Provider::Requirements rhs)
{
    return static_cast<location::Provider::Requirements>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

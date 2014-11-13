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
#include <com/ubuntu/location/provider.h>

#include <atomic>
#include <bitset>
#include <memory>

namespace cul = com::ubuntu::location;

core::Property<cul::Provider::Controller::State>& cul::Provider::Controller::state()
{
    return current_state;
}

void cul::Provider::Controller::start_position_updates()
{
    if (current_state == cul::Provider::Controller::State::disabled)
        return;

    if (++position_updates_counter == 1)
    {
        instance.start_position_updates();
    }
}

void cul::Provider::Controller::stop_position_updates()
{
    if (--position_updates_counter == 0)
    {
        instance.stop_position_updates();
    }
}

bool cul::Provider::Controller::are_position_updates_running() const
{
    return position_updates_counter > 0;
}

void cul::Provider::Controller::start_heading_updates()
{
    if (current_state == cul::Provider::Controller::State::disabled)
        return;

    if (++heading_updates_counter == 1)
    {
        instance.start_heading_updates();
    }
}

void cul::Provider::Controller::stop_heading_updates()
{
    if (--heading_updates_counter == 0)
    {
        instance.stop_heading_updates();
    }
}

bool cul::Provider::Controller::are_heading_updates_running() const
{
    return heading_updates_counter > 0;
}

void cul::Provider::Controller::start_velocity_updates()
{
    if (current_state == cul::Provider::Controller::State::disabled)
        return;

    if (++velocity_updates_counter == 1)
    {
        instance.start_velocity_updates();
    }
}

void cul::Provider::Controller::stop_velocity_updates()
{
    if (--velocity_updates_counter == 0)
    {
        instance.stop_velocity_updates();
    }
}

bool cul::Provider::Controller::are_velocity_updates_running() const
{
    return velocity_updates_counter > 0;
}

cul::Provider::Controller::Controller(cul::Provider& instance)
    : instance(instance),
      current_state(cul::Provider::Controller::State::enabled),
      position_updates_counter(0),
      heading_updates_counter(0),
      velocity_updates_counter(0)
{
    current_state.changed().connect([this](cul::Provider::Controller::State state)
    {
        switch (state)
        {
        case cul::Provider::Controller::State::disabled:
            Controller::instance.disable();
            stop_position_updates();
            stop_heading_updates();
            stop_velocity_updates();
            break;
        case cul::Provider::Controller::State::enabled:
            Controller::instance.enable();
            break;
        }
    });
}

const cul::Provider::Controller::Ptr& cul::Provider::state_controller() const
{
    return d.controller;
}

bool cul::Provider::supports(const cul::Provider::Features& f) const
{
    return (d.features & f) != Features::none;
}

bool cul::Provider::requires(const cul::Provider::Requirements& r) const
{
    return (d.requirements & r) != Requirements::none;
}

bool cul::Provider::matches_criteria(const cul::Criteria&)
{
    return false;
}

const cul::Provider::Updates& cul::Provider::updates() const
{
    return d.updates;
}

cul::Provider::Provider(
    const cul::Provider::Features& features,
    const cul::Provider::Requirements& requirements)
{
    d.features = features;
    d.requirements = requirements;
    d.controller = std::shared_ptr<Provider::Controller>(new Provider::Controller(*this));
}

cul::Provider::Updates& cul::Provider::mutable_updates()
{
    return d.updates;
}

void cul::Provider::on_wifi_and_cell_reporting_state_changed(cul::WifiAndCellIdReportingState)
{
}

void cul::Provider::on_reference_location_updated(const cul::Update<cul::Position>&)
{
}

void cul::Provider::on_reference_velocity_updated(const cul::Update<cul::Velocity>&)
{
}

void cul::Provider::on_reference_heading_updated(const cul::Update<cul::Heading>&)
{
}

void cul::Provider::disable() {}
void cul::Provider::enable() {}
void cul::Provider::start_position_updates() {}
void cul::Provider::stop_position_updates() {}
void cul::Provider::start_heading_updates() {}
void cul::Provider::stop_heading_updates() {}
void cul::Provider::start_velocity_updates() {}
void cul::Provider::stop_velocity_updates() {}

cul::Provider::Features cul::operator|(cul::Provider::Features lhs, cul::Provider::Features rhs)
{
    return static_cast<cul::Provider::Features>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

cul::Provider::Features cul::operator&(cul::Provider::Features lhs, cul::Provider::Features rhs)
{
    return static_cast<cul::Provider::Features>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

cul::Provider::Requirements cul::operator|(cul::Provider::Requirements lhs, cul::Provider::Requirements rhs)
{
    return static_cast<cul::Provider::Requirements>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

cul::Provider::Requirements cul::operator&(cul::Provider::Requirements lhs, cul::Provider::Requirements rhs)
{
    return static_cast<cul::Provider::Requirements>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

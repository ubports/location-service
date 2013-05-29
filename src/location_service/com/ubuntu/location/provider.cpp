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
#include "com/ubuntu/location/provider.h"

#include <atomic>
#include <bitset>
#include <memory>

namespace cul = com::ubuntu::location;

void cul::Provider::Controller::start_position_updates()
{
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

const cul::Provider::Controller::Cache<cul::Update<cul::Position>>& cul::Provider::Controller::cached_position_update() const
{
    return cached.position;
}

const cul::Provider::Controller::Cache<cul::Update<cul::Heading>>& cul::Provider::Controller::cached_heading_update() const
{
    return cached.heading;
}

const cul::Provider::Controller::Cache<cul::Update<cul::Velocity>>& cul::Provider::Controller::cached_velocity_update() const
{
    return cached.velocity;
}

cul::Provider::Controller::Controller(cul::Provider& instance)
    : instance(instance),
      position_updates_counter(0),
      heading_updates_counter(0),
      velocity_updates_counter(0),
      cached
      {
          Cache<Update<Position>>{},
          Cache<Update<Velocity>>{},
          Cache<Update<Heading>>{}
      }
{
    position_update_connection =
            instance.subscribe_to_position_updates(
                std::bind(&Controller::on_position_updated,
                          this,
                          std::placeholders::_1));

    velocity_update_connection =
            instance.subscribe_to_velocity_updates(
                std::bind(&Controller::on_velocity_updated,
                          this,
                          std::placeholders::_1));

    heading_update_connection =
            instance.subscribe_to_heading_updates(
                std::bind(&Controller::on_heading_updated,
                          this,
                          std::placeholders::_1));
}

void cul::Provider::Controller::on_position_updated(const cul::Update<cul::Position>& position)
{
    cached.position.update(position);
}

void cul::Provider::Controller::on_velocity_updated(const cul::Update<cul::Velocity>& velocity)
{
    cached.velocity.update(velocity);
}

void cul::Provider::Controller::on_heading_updated(const cul::Update<cul::Heading>& heading)
{
    cached.heading.update(heading);
}

const cul::Provider::Controller::Ptr& cul::Provider::state_controller() const
{
    return controller;
}

cul::ChannelConnection cul::Provider::subscribe_to_position_updates(std::function<void(const cul::Update<cul::Position>&)> f)
{
    return position_updates_channel.connect(f);
}

cul::ChannelConnection cul::Provider::subscribe_to_heading_updates(std::function<void(const cul::Update<cul::Heading>&)> f)
{
    return heading_updates_channel.connect(f);
}

cul::ChannelConnection cul::Provider::subscribe_to_velocity_updates(std::function<void(const cul::Update<cul::Velocity>&)> f)
{
    return velocity_updates_channel.connect(f);
}

bool cul::Provider::supports(const cul::Provider::Feature& f) const
{
    return feature_flags.test(static_cast<std::size_t>(f));
}

bool cul::Provider::requires(const cul::Provider::Requirement& r) const
{
    return requirement_flags.test(static_cast<std::size_t>(r));
}

bool cul::Provider::matches_criteria(const cul::Criteria&)
{
    return false;
}

cul::Provider::Provider(
    const cul::Provider::FeatureFlags& feature_flags,
    const cul::Provider::RequirementFlags& requirement_flags)
    : feature_flags(feature_flags),
      requirement_flags(requirement_flags),
      controller(new Controller(*this))
{
}

void cul::Provider::deliver_position_updates(const cul::Update<cul::Position>& update)
{
    position_updates_channel(update);
}

void cul::Provider::deliver_heading_updates(const cul::Update<cul::Heading>& update)
{
    heading_updates_channel(update);
}

void cul::Provider::deliver_velocity_updates(const cul::Update<cul::Velocity>& update)
{
    velocity_updates_channel(update);
}

void cul::Provider::start_position_updates()
{
}

void cul::Provider::stop_position_updates()
{
}

void cul::Provider::start_heading_updates()
{
}

void cul::Provider::stop_heading_updates()
{
}

void cul::Provider::start_velocity_updates()
{
}

void cul::Provider::stop_velocity_updates()
{
}

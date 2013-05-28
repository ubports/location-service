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
#include "com/ubuntu/location/proxy_provider.h"

#include <bitset>
#include <memory>

namespace cul = com::ubuntu::location;

cul::ProxyProvider::ProxyProvider(const cul::ProviderSelection& selection)
    : Provider(selection.to_feature_flags()),
      position_updates_provider(selection.position_updates_provider),
      heading_updates_provider(selection.heading_updates_provider),
      velocity_updates_provider(selection.velocity_updates_provider)
{
}

cul::ProxyProvider::~ProxyProvider() noexcept
{
}

cul::ChannelConnection cul::ProxyProvider::subscribe_to_position_updates(std::function<void(const cul::Update<cul::Position>&)> f)
{
    if (position_updates_provider)
        return position_updates_provider->subscribe_to_position_updates(f);
    return ChannelConnection {};
}

cul::ChannelConnection cul::ProxyProvider::subscribe_to_heading_updates(std::function<void(const cul::Update<cul::Heading>&)> f)
{
    if (heading_updates_provider)
        return heading_updates_provider->subscribe_to_heading_updates(f);
    return ChannelConnection {};
}

cul::ChannelConnection cul::ProxyProvider::subscribe_to_velocity_updates(std::function<void(const cul::Update<cul::Velocity>&)> f)
{
    if (velocity_updates_provider)
        return velocity_updates_provider->subscribe_to_velocity_updates(f);
    return ChannelConnection {};
}

void cul::ProxyProvider::start_position_updates()
{
    if (position_updates_provider)
        position_updates_provider->state_controller()->start_position_updates();
}

void cul::ProxyProvider::stop_position_updates()
{
    if (position_updates_provider)
        position_updates_provider->state_controller()->stop_position_updates();
}

void cul::ProxyProvider::start_velocity_updates()
{
    if (velocity_updates_provider)
        position_updates_provider->state_controller()->start_velocity_updates();
}

void cul::ProxyProvider::stop_velocity_updates()
{
    if (velocity_updates_provider)
        position_updates_provider->state_controller()->stop_velocity_updates();
}    

void cul::ProxyProvider::start_heading_updates()
{
    if (heading_updates_provider)
        position_updates_provider->state_controller()->start_heading_updates();
}

void cul::ProxyProvider::stop_heading_updates()
{
    if (heading_updates_provider)
        position_updates_provider->state_controller()->stop_heading_updates();
}

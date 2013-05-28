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
#include "com/ubuntu/location/service/session/interface.h"

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/traits/service.h>
#include <org/freedesktop/dbus/types/object_path.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

culss::Interface::~Interface() noexcept
{
}

cul::ChannelConnection culss::Interface::install_position_updates_handler(std::function<void(const cul::Update<cul::Position>&)> handler)
{
    return position_updates_channel.connect(handler);
}

cul::ChannelConnection culss::Interface::install_velocity_updates_handler(std::function<void(const cul::Update<cul::Velocity>&)> handler)
{
    return velocity_updates_channel.connect(handler);
}

cul::ChannelConnection culss::Interface::install_heading_updates_handler(std::function<void(const cul::Update<cul::Heading>&)> handler)
{
    return heading_updates_channel.connect(handler);
}

cul::Channel<cul::Update<cul::Position>>& culss::Interface::access_position_updates_channel()
{
    return position_updates_channel;
}

cul::Channel<cul::Update<cul::Heading>>& culss::Interface::access_heading_updates_channel()
{
    return heading_updates_channel;
}

cul::Channel<cul::Update<cul::Velocity>>& culss::Interface::access_velocity_updates_channel()
{
    return velocity_updates_channel;
}

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

#include <core/dbus/codec.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culss::Interface::Private
{
    cul::Channel<cul::Update<cul::Position>> position_updates_channel;
    cul::Channel<cul::Update<cul::Heading>> heading_updates_channel;
    cul::Channel<cul::Update<cul::Velocity>> velocity_updates_channel;
};

culss::Interface::Interface() : d{new Private{}}
{
}

culss::Interface::~Interface() noexcept
{
}

cul::ChannelConnection culss::Interface::install_position_updates_handler(std::function<void(const cul::Update<cul::Position>&)> handler)
{
    return d->position_updates_channel.connect(handler);
}

cul::ChannelConnection culss::Interface::install_velocity_updates_handler(std::function<void(const cul::Update<cul::Velocity>&)> handler)
{
    return d->velocity_updates_channel.connect(handler);
}

cul::ChannelConnection culss::Interface::install_heading_updates_handler(std::function<void(const cul::Update<cul::Heading>&)> handler)
{
    return d->heading_updates_channel.connect(handler);
}

cul::Channel<cul::Update<cul::Position>>& culss::Interface::access_position_updates_channel()
{
    return d->position_updates_channel;
}

cul::Channel<cul::Update<cul::Heading>>& culss::Interface::access_heading_updates_channel()
{
    return d->heading_updates_channel;
}

cul::Channel<cul::Update<cul::Velocity>>& culss::Interface::access_velocity_updates_channel()
{
    return d->velocity_updates_channel;
}

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
#include <location/service/session/interface.h>

#include <core/dbus/codec.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

namespace dbus = core::dbus;

struct location::service::session::Interface::Private
{
    location::service::session::Interface::Updates updates;
};

location::service::session::Interface::Interface() : d{new Private{}}
{
}

location::service::session::Interface::~Interface() noexcept
{
}

location::service::session::Interface::Updates& location::service::session::Interface::updates()
{
    return d->updates;
}

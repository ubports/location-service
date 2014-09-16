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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DBUS_CONNECTION_FACTORY_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DBUS_CONNECTION_FACTORY_H_

#include <core/dbus/bus.h>

#include <functional>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
/** @brief Function signature for creating DBus connections. */
typedef std::function<core::dbus::Bus::Ptr(core::dbus::WellKnownBus)> DBusConnectionFactory;

/** @brief Returns the default connection factory. */
static DBusConnectionFactory default_dbus_connection_factory()
{
    return [](core::dbus::WellKnownBus bus)
    {
        return core::dbus::Bus::Ptr
        {
            new core::dbus::Bus(bus)
        };
    };
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DBUS_CONNECTION_FACTORY_H_

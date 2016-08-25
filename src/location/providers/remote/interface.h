/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Manuel de la Pena <manuel.delapena@canonical.com>
 */

#ifndef LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_
#define LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_

#include <core/dbus/macros.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/signal.h>

#include <core/dbus/traits/service.h>

#include <location/dbus/codec.h>
#include <location/update.h>

#include <location/heading.h>
#include <location/position.h>
#include <location/velocity.h>

namespace location
{
namespace providers
{
namespace remote
{
struct Interface
{
    static const std::string& name()
    {
        static const std::string s{"com.ubuntu.remote.Service.Provider"};
        return s;
    }

    struct Observer
    {
        static const std::string& name()
        {
            static const std::string s{"com.ubuntu.remote.Service.Provider.Observer"};
            return s;
        }

        DBUS_CPP_METHOD_DEF(UpdatePosition, remote::Interface::Observer)
        DBUS_CPP_METHOD_DEF(UpdateHeading, remote::Interface::Observer)
        DBUS_CPP_METHOD_DEF(UpdateVelocity, remote::Interface::Observer)
    };

    DBUS_CPP_METHOD_DEF(OnNewEvent, remote::Interface)
    DBUS_CPP_METHOD_DEF(AddObserver, remote::Interface)
    DBUS_CPP_METHOD_DEF(Satisfies, remote::Interface)
    DBUS_CPP_METHOD_DEF(Requirements, remote::Interface)
    DBUS_CPP_METHOD_DEF(Enable, remote::Interface)
    DBUS_CPP_METHOD_DEF(Disable, remote::Interface)
    DBUS_CPP_METHOD_DEF(Activate, remote::Interface)
    DBUS_CPP_METHOD_DEF(Deactivate, remote::Interface)

    struct Skeleton
    {
        // Creates a new skeleton instance and installs the interface
        // remote::Interface on it.
        Skeleton(const core::dbus::Object::Ptr& object)
            : object{object}
        {
        }

        // The object that the interface is installed on.
        core::dbus::Object::Ptr object;
    };

    struct Stub
    {
        // Creates a new skeleton instance and installs the interface
        // remote::Interface on it.
        Stub(const core::dbus::Object::Ptr& object)
            : object{object}
        {
        }

        // The object that the interface is installed on.
        core::dbus::Object::Ptr object;   
    };

};
}
}
}

#endif // LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_

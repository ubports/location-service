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

#ifndef CORE_UBUNTU_ESPOO_PROVIDER_P_H_
#define CORE_UBUNTU_ESPOO_PROVIDER_P_H_

#include <core/dbus/macros.h>
#include <core/dbus/traits/service.h>

namespace com
{
namespace ubuntu
{
namespace remote
{
struct RemoteInterface
{

    static const std::string& name()
    {
        static const std::string s{"com.ubuntu.remote.Service.Provider"};
        return s;
    }

    DBUS_CPP_METHOD_DEF(StartPositionUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopPositionUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StartHeadingUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopHeadingUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StartVelocityUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopvVelocityUpdates, RemoteInterface)

    struct Signals
    {
        struct PositionChanged
        {
            inline static std::string name()
            {
                return "PositionChanged";
            };
            typedef RemoteInterface Interface;
            typedef std::tuple<double, double, double, double, uint32_t> ArgumentType;
        };

        DBUS_CPP_SIGNAL_DEF(HeadingChanged, RemoteInterface, double)
        DBUS_CPP_SIGNAL_DEF(VelocityChanged, RemoteInterface, double)
    };

    struct Properties
    {
        DBUS_CPP_READABLE_PROPERTY_DEF(HasPosition, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasVelocity, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasHeading, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresSatellites, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresCellNetwork, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresDataNetwork, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresMonetarySpending, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(ArePositionUpdatesRunning, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreHeadingUpdatesRunning, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreVelocityUpdatesRunning, RemoteInterface, bool)
    };

};
} // remote
} // ubuntu
}  // core

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<com::ubuntu::remote::RemoteInterface>
{
    static const std::string& interface_name()
    {
        static const std::string s{"com.ubuntu.espoo.Service.Provider"};
        return s;
    }

    inline static const std::string& object_path()
    {
        static const std::string s{"/com/ubuntu/espoo/Service/Provider"};
        return s;
    }

};
}
}
}

#endif

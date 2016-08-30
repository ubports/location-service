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
#ifndef LOCATION_DBUS_SERVICE_H_
#define LOCATION_DBUS_SERVICE_H_

#include <location/service.h>
#include <location/space_vehicle.h>

#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

namespace location
{
namespace dbus
{
struct Service
{
    inline static const std::string& path()
    {
        static const std::string s{"/com/ubuntu/location/Service"};
        return s;
    }

    struct Errors
    {
        struct InsufficientPermissions
        {
            inline static std::string name() { return "com.ubuntu.location.Service.Error.InsufficientPermissions";}
        };

        struct AddingProvider
        {
            inline static std::string name() { return "com.ubuntu.location.Service.Error.AddingProvider";}
        };

        struct CreatingSession
        {
            inline static std::string name() { return "com.ubuntu.location.Service.Error.CreatingSession"; }
        };
    };

    struct CreateSessionForCriteria
    {
        typedef dbus::Service Interface;

        inline static const std::string& name()
        {
            static const std::string s{"CreateSessionForCriteria"};
            return s;
        }

        typedef core::dbus::types::ObjectPath ResultType;

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{25};
        }
    };

    struct AddProvider
    {
        typedef dbus::Service Interface;

        inline static const std::string& name()
        {
            static const std::string s{"AddProvider"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{25};
        }
    };

    struct Properties
    {
        struct State
        {
            inline static const std::string& name()
            {
                static const std::string s{"State"};
                return s;
            }

            typedef location::dbus::Service Interface;
            typedef location::Service::State ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct DoesSatelliteBasedPositioning
        {
            inline static const std::string& name()
            {
                static const std::string s{"DoesSatelliteBasedPositioning"};
                return s;
            }

            typedef location::dbus::Service Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct DoesReportCellAndWifiIds
        {
            inline static const std::string& name()
            {
                static const std::string s{"DoesReportCellAndWifiIds"};
                return s;
            }

            typedef location::dbus::Service Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct IsOnline
        {
            inline static const std::string& name()
            {
                static const std::string s{"IsOnline"};
                return s;
            }

            typedef location::dbus::Service Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct VisibleSpaceVehicles
        {
            inline static const std::string& name()
            {
                static const std::string s{"VisibleSpaceVehicles"};
                return s;
            }

            typedef location::dbus::Service Interface;
            typedef std::map<SpaceVehicle::Key, SpaceVehicle> ValueType;

            static const bool readable = true;
            static const bool writable = false;
        };
    };

    Service() = delete;

  };
}
}

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<location::dbus::Service>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            "com.ubuntu.location.Service"
        };
        return s;
    }
};
}
}
}

#endif // LOCATION_DBUS_SERVICE_H_

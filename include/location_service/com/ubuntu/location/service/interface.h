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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_

#include <com/ubuntu/location/service/session/interface.h>

#include <com/ubuntu/location/space_vehicle.h>

#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/traits/service.h>
#include <org/freedesktop/dbus/types/object_path.h>

#include <chrono>
#include <functional>

namespace dbus = org::freedesktop::dbus;

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria;
namespace service
{
/**
 * @brief The Interface class models the primary interface to the location service.
 */
class Interface
{
  protected:
    struct Errors
    {
        struct InsufficientPermissions
        {
            inline static std::string name()
            {
                return "com.ubuntu.location.Service.Error.InsufficientPermissions";
            }
        };
        struct CreatingSession
        {
            inline static std::string name()
            {
                return "com.ubuntu.location.Service.Error.CreatingSession";
            }
        };
    };

    struct CreateSessionForCriteria
    {
        typedef com::ubuntu::location::service::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "CreateSessionForCriteria"
            };
            return s;
        }

        typedef dbus::types::ObjectPath ResultType;

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
        }
    };

    struct Properties
    {
        struct DoesSatelliteBasedPositioning
        {
            inline static const std::string& name()
            {
                static const std::string s
                {
                    "DoesSatelliteBasedPositioning"
                };
                return s;
            }

            typedef com::ubuntu::location::service::Interface Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct DoesReportCellAndWifiIds
        {
            inline static const std::string& name()
            {
                static const std::string s
                {
                    "DoesReportCellAndWifiIds"
                };
                return s;
            }

            typedef com::ubuntu::location::service::Interface Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct IsOnline
        {
            inline static const std::string& name()
            {
                static const std::string s
                {
                    "IsOnline"
                };
                return s;
            }

            typedef com::ubuntu::location::service::Interface Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct VisibleSpaceVehicles
        {
            inline static const std::string& name()
            {
                static const std::string s
                {
                    "VisibleSpaceVehicles"
                };
                return s;
            }

            typedef com::ubuntu::location::service::Interface Interface;
            typedef std::vector<com::ubuntu::location::SpaceVehicle> ValueType;

            static const bool readable = true;
            static const bool writable = false;
        };
    };

    Interface() = default;

  public:
    typedef std::shared_ptr<Interface> Ptr;

    /**
     * @brief Queries the path that this object is known under.
     */
    inline static const std::string& path()
    {
        static const std::string s{"/com/ubuntu/location/Service"};
        return s;
    }

    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;
    virtual ~Interface() = default;

    /**
     * @brief Whether the service uses satellite-based positioning.
     * @return  A setable/getable/observable property.
     */
    virtual core::Property<bool>& does_satellite_based_positioning() = 0;

    /**
     * @brief Whether the overall service and its positioning engine is online or not.
     * @return  A setable/getable/observable property.
     */
    virtual core::Property<bool>& is_online() = 0;

    /**
     * @brief Whether the engine and its providers/reporters do call home to
     * report reference locations together with wifi and cell ids.
     *
     * We consider this feature privacy sensitive and it defaults to false. The
     * user has to explicitly opt-in into this feature.
     *
     * @return A setable/getable/observable property.
     */
    virtual core::Property<bool>& does_report_cell_and_wifi_ids() = 0;

    /**
      * @brief All space vehicles currently visible.
      */
    virtual core::Property<std::vector<SpaceVehicle>>& visible_space_vehicles() = 0;

    /**
     * @brief Starts a new session for the given criteria
     * @throw std::runtime_error in case of errors.
     * @param criteria The client's requirements in terms of accuraccy and functionality
     * @return A session instance.
     */
    virtual session::Interface::Ptr create_session_for_criteria(const Criteria& criteria) = 0;
};
}
}
}
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace traits
{
template<>
struct Service<com::ubuntu::location::service::Interface>
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
}

#include <com/ubuntu/location/codec.h>

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_

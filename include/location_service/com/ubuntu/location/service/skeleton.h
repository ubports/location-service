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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_

#include <com/ubuntu/location/service/interface.h>
#include <com/ubuntu/location/service/permission_manager.h>
#include <com/ubuntu/location/service/session/interface.h>

#include <core/dbus/dbus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/skeleton.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Skeleton
        : public core::dbus::Skeleton<com::ubuntu::location::service::Interface>,
          public std::enable_shared_from_this<Skeleton>
{
public:
    typedef std::shared_ptr<Skeleton> Ptr;

    struct Configuration
    {
        // DBus connection set up for handling requests to the service.
        core::dbus::Bus::Ptr incoming;
        // DBus connection for reaching out to other services in a non-blocking way.
        core::dbus::Bus::Ptr outgoing;
        // Permission manager implementation for verifying incoming requests.
        PermissionManager::Ptr permission_manager;
    };

    Skeleton(const Configuration& configuration);
    ~Skeleton() noexcept;

    // From com::ubuntu::location::service::Interface
    core::Property<bool>& does_satellite_based_positioning();
    core::Property<bool>& does_report_cell_and_wifi_ids();
    core::Property<bool>& is_online();
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles();

private:
    // Handles incoming message calls for create_session_for_criteria.
    // Dispatches to the actual implementation, and manages object lifetimes.
    void handle_create_session_for_criteria(const core::dbus::Message::Ptr& msg);

    // Tries to register the given session under the given path in the session store.
    // Returns true iff the session has been added to the store.
    bool add_to_session_store_for_path(
            const core::dbus::types::ObjectPath& path,
            const session::Interface::Ptr& session);

    // Stores the configuration passed in at creation time.
    Configuration configuration;
    // DBus-daemon stub for resolving credentials (pid, uid) for incoming message calls.
    core::dbus::DBus daemon;
    // The skeleton object representing com.ubuntu.location.service.Interface on the bus.
    core::dbus::Object::Ptr object;
    // DBus properties as exposed on the bus for com.ubuntu.location.service.Interface
    struct
    {
        std::shared_ptr< core::dbus::Property<Interface::Properties::DoesSatelliteBasedPositioning> > does_satellite_based_positioning;
        std::shared_ptr< core::dbus::Property<Interface::Properties::DoesReportCellAndWifiIds> > does_report_cell_and_wifi_ids;
        std::shared_ptr< core::dbus::Property<Interface::Properties::IsOnline> > is_online;
        std::shared_ptr< dbus::Property<Interface::Properties::VisibleSpaceVehicles> > visible_space_vehicles;
    } properties;
    // Guards the session store.
    std::mutex guard;
    // Keeps track of running sessions, keying them by their unique object path.
    std::map<dbus::types::ObjectPath, std::shared_ptr<session::Interface>> session_store;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_

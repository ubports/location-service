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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_

#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/connectivity/manager.h>
#include <com/ubuntu/location/service/harvester.h>
#include <com/ubuntu/location/service/skeleton.h>

#include <memory>

namespace dbus = core::dbus;

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Implementation : public Skeleton
{
public:
    typedef std::shared_ptr<Implementation> Ptr;

    // Summarizes configuration options for the implementation.
    struct Configuration
    {
        // The bus connection to expose the service upon.
        core::dbus::Bus::Ptr incoming;
        // The bus connection for querying other services.
        core::dbus::Bus::Ptr outgoing;
        // The positioning Engine that the service should use.
        Engine::Ptr engine;
        // The permission manager that the service should use.
        PermissionManager::Ptr permission_manager;
        // All harvesting specific options.
        Harvester::Configuration harvester;
    };

    // Creates a new instance of the service with the given configuration.
    // Throws std::runtime_error in case of issues.
    Implementation(const Configuration& configuration);

    // Creates a new session for the given criteria.
    session::Interface::Ptr create_session_for_criteria(const Criteria& criteria);

  private:
    // The service configuration.
    Configuration configuration;
    // The harvester instance.
    Harvester harvester;
    // All event connections are automatically cut on destruction.
    struct
    {
        core::ScopedConnection is_online;
        core::ScopedConnection does_report_cell_and_wifi_ids;
        core::ScopedConnection does_satellite_based_positioning;
        core::ScopedConnection engine_state;
        core::ScopedConnection satellite_based_positioning_state;
        core::ScopedConnection visible_space_vehicles;
        core::ScopedConnection reference_position;
    } connections;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_

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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_TRUST_STORE_PERMISSION_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_TRUST_STORE_PERMISSION_MANAGER_H_

#include <com/ubuntu/location/service/permission_manager.h>

#include <core/trust/agent.h>

#include <functional>

namespace core
{
namespace dbus
{
class Bus;
}
}

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
// A PermmissionManager implementation leveraging the trust-store
// infrastructure to cache a user's answer and to dispatch
// to user-specific trust-store instances.
class TrustStorePermissionManager : public PermissionManager
{
public:
    // Just a convenience typedef.
    typedef std::shared_ptr<TrustStorePermissionManager> Ptr;

    // Functor for resolving a process id to an app-armor profile name.
    typedef std::function<std::string(const core::trust::Pid&)> AppArmorProfileResolver;

    // Returns an AppArmorProfileResolver leveraging libapparmor.
    static AppArmorProfileResolver libapparmor_profile_resolver();

    // The default feature tag we use when calling out to the agent.
    static core::trust::Feature default_feature();

    // Creates a default instance, initializing the agent and resolver
    // fields to sensible default choices.
    static Ptr create_default_instance_with_bus(const std::shared_ptr<core::dbus::Bus>& bus);

    // Sets up the manager for operation and stores the agent and resolver
    // instances given to the ctor.
    TrustStorePermissionManager(
            const std::shared_ptr<core::trust::Agent>& agent,
            AppArmorProfileResolver app_armor_profile_resolver);

    // From PermissionManager
    Result check_permission_for_credentials(const Criteria&, const Credentials& credentials) override;

private:
    // The agent instance we leverage to authenticate
    // permission requests.
    std::shared_ptr<core::trust::Agent> agent;

    // Helper to resolve an application's pid to an app-armor profile name.
    AppArmorProfileResolver app_armor_profile_resolver;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_TRUST_STORE_PERMISSION_MANAGER_H_

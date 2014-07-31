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

#include <com/ubuntu/location/service/trust_store_permission_manager.h>

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/service/config.h>

#include <core/trust/dbus_agent.h>

#include <core/dbus/bus.h>
#include <core/dbus/asio/executor.h>

#include <sys/apparmor.h>

namespace location = com::ubuntu::location;
namespace service = com::ubuntu::location::service;

service::TrustStorePermissionManager::AppArmorProfileResolver service::TrustStorePermissionManager::libapparmor_profile_resolver()
{
    return [](core::trust::Pid pid)
    {
        static const int app_armor_error{-1};

        // We make sure to clean up the returned string.
        struct Scope
        {
            ~Scope()
            {
                if (con) ::free(con);
            }

            char* con{nullptr};
            char* mode{nullptr};
        } scope;

        // Reach out to apparmor
        auto rc = aa_gettaskcon(pid.value, &scope.con, &scope.mode);

        // From man aa_gettaskcon:
        // On success size of data placed in the buffer is returned, this includes the mode if
        //present and any terminating characters. On error, -1 is returned, and errno(3) is
        //set appropriately.
        if (rc == app_armor_error) throw std::system_error
        {
            errno,
            std::system_category()
        };

        // Safely construct the string
        return std::string
        {
            scope.con ? scope.con : ""
        };
    };
}

core::trust::Feature service::TrustStorePermissionManager::default_feature()
{
    return core::trust::Feature{0};
}

service::TrustStorePermissionManager::Ptr service::TrustStorePermissionManager::create_default_instance_with_bus(const std::shared_ptr<core::dbus::Bus>& bus)
{
    return Ptr
    {
        new TrustStorePermissionManager
        {
            core::trust::dbus::create_multi_user_agent_for_bus_connection(
                        bus,
                        com::ubuntu::location::service::trust_store_service_name),
            TrustStorePermissionManager::libapparmor_profile_resolver()
        }
    };
}

service::TrustStorePermissionManager::TrustStorePermissionManager(
        const std::shared_ptr<core::trust::Agent>& agent,
        service::TrustStorePermissionManager::AppArmorProfileResolver app_armor_profile_resolver)
    : agent{agent},
      app_armor_profile_resolver{app_armor_profile_resolver}
{
}

service::PermissionManager::Result service::TrustStorePermissionManager::check_permission_for_credentials(
        const location::Criteria&,
        const service::Credentials& credentials)
{
    std::string profile;
    try
    {
        profile = app_armor_profile_resolver(core::trust::Pid{credentials.pid});
    } catch(const std::exception& e)
    {
        LOG(ERROR) << "Could not resolve PID " << credentials.pid << " to apparmor profile: " << e.what();
        return service::PermissionManager::Result::rejected;
    } catch(...)
    {
        LOG(ERROR) << "Could not resolve PID " << credentials.pid << " to apparmor profile.";
        return service::PermissionManager::Result::rejected;
    }

    std::string description;

    if (profile == "unconfined")
    {
        description = "An unconfined application is trying to access the location service.";
    } else
    {
        description = profile + " is trying to access the location service.";
    }

    core::trust::Agent::RequestParameters params
    {
        core::trust::Uid{credentials.uid},
        core::trust::Pid{credentials.pid},
        profile,
        TrustStorePermissionManager::default_feature(),
        description
    };

    Result result{Result::rejected};

    try
    {
        auto answer = agent->authenticate_request_with_parameters(params);
        switch(answer)
        {
        case core::trust::Request::Answer::granted:
            result = Result::granted;
            break;
        case core::trust::Request::Answer::denied:
            result = Result::rejected;
            break;
        }
    } catch(...)
    {
        // We silently drop all issues here and return rejected.
        result = Result::rejected;
    }

    return result;
}

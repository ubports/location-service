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

#include <location/trust_store_permission_manager.h>
#include <location/logging.h>

#include <core/trust/dbus_agent.h>

#include <core/dbus/bus.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/this_process.h>

#include <boost/format.hpp>

#include <sys/apparmor.h>

namespace
{
bool is_running_under_testing()
{
    return core::posix::this_process::env::get(
                "TRUST_STORE_PERMISSION_MANAGER_IS_RUNNING_UNDER_TESTING",
                "0") == "1";

}

namespace i18n
{
// We only tag strings that should be translated but do not do the actual translation.
// Point is: The service might run in a system context, without correct locale information.
// We leave the translation to in-session trust-store instances.
std::string tr(const std::string& msg)
{
    return msg;
}
}

// At least make the name a constant, no need to expose it, though.
static constexpr const char* trust_store_service_name{"UbuntuLocationService"};
}

location::TrustStorePermissionManager::AppArmorProfileResolver location::TrustStorePermissionManager::libapparmor_profile_resolver()
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

core::trust::Feature location::TrustStorePermissionManager::default_feature()
{
    return core::trust::Feature{0};
}

location::TrustStorePermissionManager::Ptr location::TrustStorePermissionManager::create_default_instance_with_bus(const std::shared_ptr<core::dbus::Bus>& bus)
{
    return Ptr
    {
        new TrustStorePermissionManager
        {
            core::trust::dbus::create_multi_user_agent_for_bus_connection(
                        bus,
                        trust_store_service_name),
            TrustStorePermissionManager::libapparmor_profile_resolver()
        }
    };
}

location::TrustStorePermissionManager::TrustStorePermissionManager(
        const std::shared_ptr<core::trust::Agent>& agent,
        location::TrustStorePermissionManager::AppArmorProfileResolver app_armor_profile_resolver)
    : agent{agent},
      app_armor_profile_resolver{app_armor_profile_resolver}
{
}

location::PermissionManager::Result location::TrustStorePermissionManager::check_permission_for_credentials(
        const location::Criteria&,
        const location::Credentials& credentials)
{
    // This is ugly and we should get rid of it. Ideally, we would be able
    // inject a mocked trust-store into our acceptance testing.
    if (is_running_under_testing())
        return Result::granted;

    std::string profile;
    try
    {
        profile = app_armor_profile_resolver(core::trust::Pid{credentials.pid});
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << "Could not resolve PID " << credentials.pid << " to apparmor profile: " << e.what();
        return location::PermissionManager::Result::rejected;
    } catch(...)
    {
        SYSLOG(ERROR) << "Could not resolve PID " << credentials.pid << " to apparmor profile.";
        return location::PermissionManager::Result::rejected;
    }

    std::string description = i18n::tr("wants to access your current location.");

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

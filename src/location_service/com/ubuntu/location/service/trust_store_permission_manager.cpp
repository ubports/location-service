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

#include <core/posix/this_process.h>

#include <boost/format.hpp>

namespace location = com::ubuntu::location;
namespace service = com::ubuntu::location::service;

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
        }
    };
}

service::TrustStorePermissionManager::TrustStorePermissionManager(
        const std::shared_ptr<core::trust::Agent>& agent)
    : agent{agent}
{
}

service::PermissionManager::Result service::TrustStorePermissionManager::check_permission_for_credentials(
        const location::Criteria&,
        const service::Credentials& credentials)
{
    // This is ugly and we should get rid of it. Ideally, we would be able
    // inject a mocked trust-store into our acceptance testing.
    if (is_running_under_testing())
        return Result::granted;

    if (credentials.profile.empty()) {
        SYSLOG(ERROR) << "Could not resolve PID " << credentials.pid << " to apparmor profile";
        return service::PermissionManager::Result::rejected;
    }

    std::string description = i18n::tr("wants to access your current location.");

    core::trust::Agent::RequestParameters params
    {
        core::trust::Uid{credentials.uid},
        core::trust::Pid{credentials.pid},
        credentials.profile,
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

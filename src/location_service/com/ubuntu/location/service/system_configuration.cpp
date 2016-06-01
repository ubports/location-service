/*
 * Copyright © 2016 Canonical Ltd.
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
 *              Scott Sweeny <scott.sweeny@canonical.com>
 */
#include <com/ubuntu/location/service/system_configuration.h>

#include <com/ubuntu/location/service/always_granting_permission_manager.h>
#include <com/ubuntu/location/service/trust_store_permission_manager.h>

#include <com/ubuntu/location/logging.h>

#include <core/posix/this_process.h>

namespace culs = com::ubuntu::location::service;
namespace env = core::posix::this_process::env;

#if defined(SNAPPY_UBUNTU_CORE)
namespace
{
struct SnappySystemConfiguration : public culs::SystemConfiguration
{
    fs::path runtime_persistent_data_dir() const
    {
        if (std::string data_dir = env::get("SNAP_DATA")) {
            return data_dir;
        } else {
            LOG(WARNING) << "SNAP_DATA environment variable is not defined.";

            // If SNAP_DATA isn't set it's almost certain we're not running
            // from a snap, so return the conventional path
            return "/var/lib/ubuntu-location-service";
        }
    }
    
    culs::PermissionManager::Ptr create_permission_manager(const std::shared_ptr<core::dbus::Bus>&) const
    {
        return std::make_shared<culs::AlwaysGrantingPermissionManager>();
    }
};
}

culs::SystemConfiguration& culs::SystemConfiguration::instance()
{
    static SnappySystemConfiguration config;
    return config;
}
#else
namespace
{
struct UbuntuSystemConfiguration : public culs::SystemConfiguration
{
    fs::path runtime_persistent_data_dir() const
    {
        return "/var/lib/ubuntu-location-service";
    }
    
    culs::PermissionManager::Ptr create_permission_manager(const std::shared_ptr<core::dbus::Bus>& bus) const
    {
        return culs::TrustStorePermissionManager::create_default_instance_with_bus(bus);
    }    
};
}

culs::SystemConfiguration& culs::SystemConfiguration::instance()
{
    static UbuntuSystemConfiguration config;
    return config;
}
#endif // SNAPPY_UBUNTU_CORE

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
#ifndef LOCATION_SYSTEM_CONFIGURATION_H_
#define LOCATION_SYSTEM_CONFIGURATION_H_

#include <location/permission_manager.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace location
{
class SystemConfiguration
{
public:
    // A singleton is ugly, but it's the easier way forward
    // if we want to contain snappy-specific setup for now.
    static SystemConfiguration& instance();

    virtual ~SystemConfiguration() = default;
    
    // runtime_data_dir returns the path to the directory the service should
    // use to store runtime persistent data.
    virtual fs::path runtime_persistent_data_dir() const = 0;
    
    // create_permission_manager returns an instance of PermissionManager that 
    // is meant to be used by the service to verify incoming connection requests.
    virtual PermissionManager::Ptr create_permission_manager() const = 0;
    
protected:
    SystemConfiguration() = default;
};
}

#endif // LOCATION_SYSTEM_CONFIGURATION_H_

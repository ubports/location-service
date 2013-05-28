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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/provider_selection_policy.h"

#include <functional>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
struct Credentials
{
    pid_t pid;
    uid_t uid;
};

class PermissionManager
{
public:
    typedef std::shared_ptr<PermissionManager> Ptr;

    enum class Result
    {
        granted,
        rejected
    };

    virtual ~PermissionManager() = default;
    PermissionManager(const PermissionManager&) = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;

    virtual Result check_permission_for_credentials(
        const Criteria& criteria,
        const Credentials& credentials) = 0;
    
protected:
    PermissionManager() = default;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_

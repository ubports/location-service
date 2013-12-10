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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_

#include <com/ubuntu/location/service/permission_manager.h>

#include <sys/types.h>
#include <unistd.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class DefaultPermissionManager : public PermissionManager
{
  public:
    typedef std::shared_ptr<DefaultPermissionManager> Ptr;

    DefaultPermissionManager();
    ~DefaultPermissionManager() noexcept;
    
    Result check_permission_for_credentials(
    	const Criteria&,
        const Credentials& credentials);

  private:
    pid_t pid;
    uid_t uid;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_

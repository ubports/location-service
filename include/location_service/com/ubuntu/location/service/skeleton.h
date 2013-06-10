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

#include "com/ubuntu/location/service/interface.h"
#include "com/ubuntu/location/service/permission_manager.h"
#include "com/ubuntu/location/service/session/interface.h"

#include <org/freedesktop/dbus/skeleton.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Skeleton : public org::freedesktop::dbus::Skeleton<com::ubuntu::location::service::Interface>,
                 public std::enable_shared_from_this<Skeleton>
{
  public:
    typedef std::shared_ptr<Skeleton> Ptr;
    
    Skeleton(const dbus::Bus::Ptr& connection, const PermissionManager::Ptr& permission_manager);
    Skeleton(const Skeleton&) = delete;
    Skeleton& operator=(const Skeleton&) = delete;
    ~Skeleton() noexcept;

  private:
    struct Private;
    std::shared_ptr<Private> d;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_

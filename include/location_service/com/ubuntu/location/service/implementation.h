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

    Implementation(
        const core::dbus::Bus::Ptr& bus,
        const Engine::Ptr& engine,
        const PermissionManager::Ptr& permission_manager);
    Implementation(const Implementation&) = delete;
    virtual ~Implementation() noexcept;
    Implementation& operator=(const Implementation&) = delete;

    session::Interface::Ptr create_session_for_criteria(const Criteria& criteria);

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_

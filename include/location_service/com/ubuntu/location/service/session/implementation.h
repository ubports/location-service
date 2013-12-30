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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_

#include <com/ubuntu/location/service/session/skeleton.h>

#include <com/ubuntu/location/provider.h>

#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
namespace session
{
class Implementation : public Interface
{
  public:
    Implementation(const Provider::Ptr& provider);
    Implementation(const Implementation&) = delete;
    virtual ~Implementation() noexcept;
    Implementation& operator=(const Implementation&) = delete;

    virtual void start_position_updates();
    virtual void stop_position_updates() noexcept;

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates() noexcept;

    virtual void start_heading_updates();
    virtual void stop_heading_updates() noexcept;

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_

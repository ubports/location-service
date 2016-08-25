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

#ifndef LOCATION_DBUS_STUB_SESSION_H_
#define LOCATION_DBUS_STUB_SESSION_H_

#include <location/service.h>

#include <location/position.h>
#include <location/provider.h>
#include <location/update.h>

#include <core/dbus/stub.h>

#include <memory>

namespace location
{
namespace dbus
{
namespace stub
{
class Session : public location::Service::Session
{
  public:
    Session(const core::dbus::Bus::Ptr& connection,
            const core::dbus::Object::Ptr& object);
    ~Session();

    void start_position_updates();
    void stop_position_updates();

    void start_velocity_updates();
    void stop_velocity_updates();

    void start_heading_updates();
    void stop_heading_updates();

    Updates& updates() override;

  private:
    core::dbus::Bus::Ptr connection_;
    core::dbus::Object::Ptr object_;

    Updates updates_;

    core::ScopedConnection position_;
    core::ScopedConnection velocity_;
    core::ScopedConnection heading_;
};
}
}
}

#endif // LOCATION_DBUS_STUB_SESSION_H_

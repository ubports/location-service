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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_

#include <com/ubuntu/location/service/session/interface.h>

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>

#include <memory>
#include <string>

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
class Skeleton : public core::dbus::Skeleton<Interface>
{
public:
    // All local, i.e., in-process creation-time properties of the Skeleton.
    struct Local
    {
        // The actual implementation of com::ubuntu::location::service::session::Interface.
        Interface::Ptr impl;
        // The bus connection that the object is exposed upon.
        core::dbus::Bus::Ptr bus;
    };

    // We communicate position, heading and velocity updates to the client
    // via an explicit function call. The reason is simple: We want to know
    // whether the client is still alive and responding as expected to make sure
    // that we stop positioning as early as possible.
    struct Remote
    {
        // The remote object corresponding to the client, implementing
        // com.ubuntu.location.service.session.Interface
        core::dbus::Object::Ptr object;
        // The application ID of the client
        std::string app_id;
    };

    struct Configuration
    {
        // The object path of the session object, shared between clients and service.
        core::dbus::types::ObjectPath path;
        // Local attributes
        Local local;
        // Remote attributes
        Remote remote;
    };

    Skeleton(const Configuration& configuration);
    virtual ~Skeleton() noexcept;

    virtual const core::dbus::types::ObjectPath& path() const;
    const std::string& remote_app_id() const { return configuration.remote.app_id; }

private:
    // Handle incoming requests for Start/StopPositionUpdates
    virtual void on_start_position_updates(const core::dbus::Message::Ptr&);
    virtual void on_stop_position_updates(const core::dbus::Message::Ptr&);
    // Handles incoming requests for Start/StopHeadingUpdates
    virtual void on_start_heading_updates(const core::dbus::Message::Ptr&);
    virtual void on_stop_heading_updates(const core::dbus::Message::Ptr&);
    // Handles incoming requests for Start/StopVelocityUpdates
    virtual void on_start_velocity_updates(const core::dbus::Message::Ptr&);
    virtual void on_stop_velocity_updates(const core::dbus::Message::Ptr&);

    // Invoked whenever the actual session impl. for the session reports a position update.
    virtual void on_position_changed(const Update<Position>& position);
    // Invoked whenever the actual session impl. reports a heading update.
    virtual void on_heading_changed(const Update<Heading>& heading);
    // Invoked whenever the actual session impl. reports a velocity update.
    virtual void on_velocity_changed(const Update<Velocity>& velocity);

    // Stores all attributes passed at creation time.
    Configuration configuration;
    // The DBus object corresponding to the session.
    core::dbus::Object::Ptr object;
    // Scoped connections for automatically disconnecting on destruction
    struct
    {
        // Corresponds to position updates coming in from the actual implementation instance.
        core::ScopedConnection position_changed;
        // Corresponds to heading updates coming in from the actual implementation instance.
        core::ScopedConnection heading_changed;
        // Corresponds to velocity updates coming in from the actual implementation instance.
        core::ScopedConnection velocity_changed;
    } connections;
};
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_

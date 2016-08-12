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
#ifndef LOCATION_DBUS_SKELETON_SESSION_H_
#define LOCATION_DBUS_SKELETON_SESSION_H_

#include <location/service.h>

#include <location/heading.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/update.h>
#include <location/velocity.h>

#include <core/dbus/message.h>
#include <core/dbus/object.h>

#include <memory>

namespace location
{
namespace dbus
{
namespace skeleton
{
class Session : public location::Service::Session
{
public:
    // All local, i.e., in-process creation-time properties of the Skeleton.
    struct Local
    {
        // The actual implementation of location::service::session::Interface.
        Session::Ptr impl;
        // The bus connection that the object is exposed upon.
        core::dbus::Bus::Ptr bus;
        // The object that the skeleton is mounted upon.
        core::dbus::Object::Ptr object;
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

    Session(const Configuration& configuration);
    virtual ~Session() noexcept;

    virtual const core::dbus::types::ObjectPath& path() const;

    // From Service::Session
    Updates& updates() override;

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

#endif // LOCATION_DBUS_SKELETON_SESSION_H_

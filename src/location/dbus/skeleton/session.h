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

#include <location/position.h>
#include <location/provider.h>
#include <location/update.h>

#include <location/dbus/session_gen.h>
#include <location/glib/shared_object.h>

#include <gio/gio.h>

#include <memory>

namespace location
{
namespace dbus
{
namespace skeleton
{
class Session : public location::Service::Session,
                public std::enable_shared_from_this<Session>
{
public:
    using Ptr = std::shared_ptr<Session>;

    struct Configuration
    {
        glib::SharedObject<ComUbuntuLocationServiceSession> skeleton;
        location::Service::Session::Ptr impl;
    };

    static std::shared_ptr<Session> create(const Configuration& configuration);

    virtual ~Session() noexcept;

    // From Service::Session
    Updates& updates() override;

private:
    static gboolean handle_start_position_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);
    static gboolean handle_stop_position_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);
    static gboolean handle_start_heading_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);
    static gboolean handle_stop_heading_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);
    static gboolean handle_start_velocity_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);
    static gboolean handle_stop_velocity_updates(
            ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data);

    Session(const Configuration& configuration);

    std::shared_ptr<Session> finalize_construction();

    // Handle incoming requests for Start/StopPositionUpdates
    virtual void on_start_position_updates();
    virtual void on_stop_position_updates();
    // Handles incoming requests for Start/StopHeadingUpdates
    virtual void on_start_heading_updates();
    virtual void on_stop_heading_updates();
    // Handles incoming requests for Start/StopVelocityUpdates
    virtual void on_start_velocity_updates();
    virtual void on_stop_velocity_updates();

    // Invoked whenever the actual session impl. for the session reports a position update.
    virtual void on_position_changed(const Update<Position>& position);
    // Invoked whenever the actual session impl. reports a heading update.
    virtual void on_heading_changed(const Update<units::Degrees>& heading);
    // Invoked whenever the actual session impl. reports a velocity update.
    virtual void on_velocity_changed(const Update<units::MetersPerSecond>& velocity);

    glib::SharedObject<ComUbuntuLocationServiceSession> skeleton;
    location::Service::Session::Ptr impl;
};
}
}
}

#endif // LOCATION_DBUS_SKELETON_SESSION_H_

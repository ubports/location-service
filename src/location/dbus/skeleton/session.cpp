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
#include <location/dbus/skeleton/session.h>

#include <location/dbus/codec.h>
#include <location/dbus/util.h>
#include <location/glib/holder.h>

#include <location/logging.h>

#include <boost/core/ignore_unused.hpp>

#include <functional>

namespace
{

using Holder = location::glib::Holder<std::weak_ptr<location::dbus::skeleton::Session>>;

}  // namespace

gboolean location::dbus::skeleton::Session::handle_start_position_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_start_position_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

gboolean location::dbus::skeleton::Session::handle_stop_position_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_stop_position_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

gboolean location::dbus::skeleton::Session::handle_start_heading_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_start_heading_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

gboolean location::dbus::skeleton::Session::handle_stop_heading_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_stop_heading_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

gboolean location::dbus::skeleton::Session::handle_start_velocity_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_start_velocity_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

gboolean location::dbus::skeleton::Session::handle_stop_velocity_updates(
        ComUbuntuLocationServiceSession* session, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(session);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->on_stop_velocity_updates();
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return true;
        }
    }
    return false;
}

std::shared_ptr<location::dbus::skeleton::Session> location::dbus::skeleton::Session::create(const Configuration& configuration)
{
    auto session = std::shared_ptr<Session>{new Session{configuration}};
    return session->finalize_construction();
}

location::dbus::skeleton::Session::Session(const location::dbus::skeleton::Session::Configuration& config)
        : skeleton{config.skeleton},
          impl{config.impl}
{
}

location::dbus::skeleton::Session::~Session() noexcept
{
}

std::shared_ptr<location::dbus::skeleton::Session> location::dbus::skeleton::Session::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Session> wp{sp};

    impl->updates().position.changed().connect([wp](const location::Update<location::Position>& position)
    {
        if (auto sp = wp.lock())
            sp->on_position_changed(position);
    });

    impl->updates().heading.changed().connect([wp](const location::Update<location::units::Degrees>& heading)
    {
        if (auto sp = wp.lock())
            sp->on_heading_changed(heading);
    });

    impl->updates().velocity.changed().connect([wp](const location::Update<location::units::MetersPerSecond>& velocity)
    {
        if (auto sp = wp.lock())
            sp->on_velocity_changed(velocity);
    });

    g_signal_connect_data(skeleton.get(), "handle-start-position-updates",
                          G_CALLBACK(Session::handle_start_position_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));
    g_signal_connect_data(skeleton.get(), "handle-stop-position-updates",
                          G_CALLBACK(Session::handle_stop_position_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));
    g_signal_connect_data(skeleton.get(), "handle-start-heading-updates",
                          G_CALLBACK(Session::handle_start_heading_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));
    g_signal_connect_data(skeleton.get(), "handle-stop-heading-updates",
                          G_CALLBACK(Session::handle_stop_heading_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));
    g_signal_connect_data(skeleton.get(), "handle-start-velocity-updates",
                          G_CALLBACK(Session::handle_start_velocity_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));
    g_signal_connect_data(skeleton.get(), "handle-stop-velocity-updates",
                          G_CALLBACK(Session::handle_stop_velocity_updates),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    return sp;
}

void location::dbus::skeleton::Session::on_start_position_updates()
{
    impl->updates().position_status = Updates::Status::enabled;
}

void location::dbus::skeleton::Session::on_stop_position_updates()
{
    impl->updates().position_status = Updates::Status::disabled;
}

void location::dbus::skeleton::Session::on_start_heading_updates()
{
    impl->updates().heading_status = Updates::Status::enabled;
}

void location::dbus::skeleton::Session::on_stop_heading_updates()
{
    impl->updates().heading_status = Updates::Status::disabled;
}

void location::dbus::skeleton::Session::on_start_velocity_updates()
{
    impl->updates().velocity_status = Updates::Status::enabled;
}

void location::dbus::skeleton::Session::on_stop_velocity_updates()
{
    impl->updates().velocity_status = Updates::Status::disabled;
}

// Invoked whenever the actual session impl. for the session reports a position update.
void location::dbus::skeleton::Session::on_position_changed(const location::Update<location::Position>& position)
{
    com_ubuntu_location_service_session_set_position(
                COM_UBUNTU_LOCATION_SERVICE_SESSION(skeleton.get()), dbus::encode(position));
}

// Invoked whenever the actual session impl. reports a heading update.
void location::dbus::skeleton::Session::on_heading_changed(const location::Update<location::units::Degrees>& heading)
{
    com_ubuntu_location_service_session_set_heading(
                COM_UBUNTU_LOCATION_SERVICE_SESSION(skeleton.get()), dbus::encode(heading));
}

// Invoked whenever the actual session impl. reports a velocity update.
void location::dbus::skeleton::Session::on_velocity_changed(const location::Update<location::units::MetersPerSecond>& velocity)
{
    com_ubuntu_location_service_session_set_velocity(
                COM_UBUNTU_LOCATION_SERVICE_SESSION(skeleton.get()), dbus::encode(velocity));

}

location::Service::Session::Updates& location::dbus::skeleton::Session::updates()
{
    return impl->updates();
}

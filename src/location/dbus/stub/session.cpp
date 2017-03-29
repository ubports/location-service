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

#include <location/dbus/stub/session.h>
#include <location/dbus/codec.h>
#include <location/dbus/service.h>
#include <location/dbus/util.h>
#include <location/glib/holder.h>

#include <location/logging.h>
#include <location/result.h>

#include <boost/core/ignore_unused.hpp>

namespace
{

using Holder = location::glib::Holder<std::weak_ptr<location::dbus::stub::Session>>;

}  // namespace

void location::dbus::stub::Session::create(
        const glib::SharedObject<GDBusConnection>& connection, const std::string& path, std::function<void (const Result<Session::Ptr>&)> cb)
{
    com_ubuntu_location_service_session_proxy_new(
                connection.get(), G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START, location::dbus::Service::name(), path.c_str(),
                nullptr, on_proxy_ready, new ProxyCreationContext{std::move(cb)});
}

location::dbus::stub::Session::Session(const glib::SharedObject<ComUbuntuLocationServiceSession>& session)
        : session_{session}
{
}

location::dbus::stub::Session::Ptr location::dbus::stub::Session::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Session> wp{sp};

    updates_.position_status.changed().connect([this, wp](const Updates::Status& status)
    {
        if (auto sp = wp.lock())
        {
            switch(status)
            {
            case Updates::Status::enabled: start_position_updates(); break;
            case Updates::Status::disabled: stop_position_updates(); break;
            }
        }
    });

    updates_.velocity_status.changed().connect([this, wp](const Updates::Status& status)
    {
        if (auto sp = wp.lock())
        {
            switch(status)
            {
            case Updates::Status::enabled: start_velocity_updates(); break;
            case Updates::Status::disabled: stop_velocity_updates(); break;
            }
        }
    });

    updates_.heading_status.changed().connect([this, wp](const Updates::Status& status)
    {
        if (auto sp = wp.lock())
        {
            switch(status)
            {
            case Updates::Status::enabled: start_heading_updates(); break;
            case Updates::Status::disabled: stop_heading_updates(); break;
            }
        }
    });

    signal_handler_ids_.insert(g_signal_connect_data(
                                   session_.get(), "notify::position",
                                   G_CALLBACK(on_position_changed),
                                   new Holder{wp}, Holder::closure_notify, GConnectFlags(0)));

    signal_handler_ids_.insert(g_signal_connect_data(
                                   session_.get(), "notify::heading",
                                   G_CALLBACK(on_heading_changed),
                                   new Holder{wp}, Holder::closure_notify, GConnectFlags(0)));

    signal_handler_ids_.insert(g_signal_connect_data(
                                   session_.get(), "notify::velocity",
                                   G_CALLBACK(on_velocity_changed),
                                   new Holder{wp}, Holder::closure_notify, GConnectFlags(0)));

    return sp;
}

location::dbus::stub::Session::~Session() noexcept
{
    for (auto id : signal_handler_ids_)
        g_signal_handler_disconnect(session_.get(), id);
}

void location::dbus::stub::Session::start_position_updates()
{
    com_ubuntu_location_service_session_call_start_position_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

void location::dbus::stub::Session::stop_position_updates()
{
    com_ubuntu_location_service_session_call_stop_position_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

void location::dbus::stub::Session::start_velocity_updates()
{
    com_ubuntu_location_service_session_call_start_velocity_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

void location::dbus::stub::Session::stop_velocity_updates()
{
    com_ubuntu_location_service_session_call_stop_velocity_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

void location::dbus::stub::Session::start_heading_updates()
{
    com_ubuntu_location_service_session_call_start_heading_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

void location::dbus::stub::Session::stop_heading_updates()
{
    com_ubuntu_location_service_session_call_stop_heading_updates(
                session_.get(), nullptr, nullptr, nullptr);
}

location::Service::Session::Updates& location::dbus::stub::Session::updates()
{
    return updates_;
}

void location::dbus::stub::Session::on_proxy_ready(GObject* source, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    boost::ignore_unused(source);

    if (auto context = static_cast<ProxyCreationContext*>(user_data))
    {
        GError* error{nullptr};
        location::glib::SharedObject<ComUbuntuLocationServiceSession> session =
                location::glib::make_shared_object(com_ubuntu_location_service_session_proxy_new_finish(res, &error));

        if (error)
        {
            context->cb(location::make_error_result<location::dbus::stub::Session::Ptr>(
                            std::make_exception_ptr(
                                std::runtime_error{error->message})));
        }
        else
        {
            location::dbus::stub::Session::Ptr ptr{new location::dbus::stub::Session{session}};
            context->cb(location::make_result(ptr->finalize_construction()));
        }

        delete context;
    }
}

void location::dbus::stub::Session::on_position_changed(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto thiz = holder->value.lock())
        {
            if (auto variant = com_ubuntu_location_service_session_get_position(thiz->session_.get()))
            {
                auto update =
                    location::dbus::decode<
                        location::Update<location::Position>
                    >(variant);

                if (update)
                    thiz->updates().position = *update;
            }
        }
    }
}

void location::dbus::stub::Session::on_heading_changed(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto thiz = holder->value.lock())
        {
            if (auto variant = com_ubuntu_location_service_session_get_heading(thiz->session_.get()))
            {
                auto update =
                        location::dbus::decode<
                            location::Update<units::Degrees>
                        >(variant);

                if (update)
                    thiz->updates().heading = *update;
            }
        }
    }
}

void location::dbus::stub::Session::on_velocity_changed(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto thiz = holder->value.lock())
        {
            if (auto variant = com_ubuntu_location_service_session_get_velocity(thiz->session_.get()))
            {
                auto update =
                        location::dbus::decode<
                            location::Update<units::MetersPerSecond>
                        >(variant);

                if (update)
                    thiz->updates().velocity = *update;
            }
        }
    }
}

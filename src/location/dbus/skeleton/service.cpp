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

#include <location/dbus/codec.h>
#include <location/dbus/util.h>
#include <location/dbus/skeleton/service.h>
#include <location/dbus/skeleton/session.h>

#include <location/glib/holder.h>
#include <location/glib/util.h>
#include <location/providers/remote/provider.h>

#include <location/criteria.h>
#include <location/logging.h>

#include <boost/core/ignore_unused.hpp>
#include <boost/lexical_cast.hpp>

namespace
{

using Holder = location::glib::Holder<std::weak_ptr<location::dbus::skeleton::Service>>;

}  // namespace

location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::DBusDaemonCredentialsResolver(const location::glib::SharedObject<GDBusConnection>& connection)
{
    g_dbus_proxy_new(
                connection.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES, nullptr,
                "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                nullptr, DBusDaemonCredentialsResolver::handle_daemon_proxy_ready, this);
}

void location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::resolve_credentials_for_sender(
        const std::string& sender, std::function<void(const Result<Credentials>&)> cb)
{
    if (!daemon) throw std::runtime_error{"Cannot access dbus daemon"};

    g_dbus_proxy_call(
                daemon.get(), "GetConnectionUnixProcessID", g_variant_new("(s)", sender.c_str()),
                G_DBUS_CALL_FLAGS_NONE, -1, nullptr, DBusDaemonCredentialsResolver::handle_unix_process_id_ready,
                new ProcessIdQueryContext{daemon, sender, std::move(cb)});
}

void location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::handle_unix_process_id_ready(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source);

    if (auto context = static_cast<ProcessIdQueryContext*>(user_data))
    {
        GError* error{nullptr};
        std::shared_ptr<GVariant> variant(
                    g_dbus_proxy_call_finish(context->daemon.get(), result, &error),
                    [](GVariant* variant) { if (variant) g_variant_unref(variant); });

        if (!error)
        {
            std::uint32_t pid{0}; g_variant_get(variant.get(), "(u)", &pid);
            if (pid != 0)
            {
                g_dbus_proxy_call(
                            context->daemon.get(), "GetConnectionUnixUser", g_variant_new("(s)", context->sender.c_str()),
                            G_DBUS_CALL_FLAGS_NONE, -1, nullptr, DBusDaemonCredentialsResolver::handle_unix_user_id_ready,
                            new UserIdQueryContext{context->daemon, context->sender, pid, std::move(context->cb)});
            }
            else
            {
                auto exception = std::make_exception_ptr(std::runtime_error{"Failed to decode result value."});
                context->cb(make_error_result<Credentials>(exception));
            }
        }
        else
        {
            context->cb(make_error_result<Credentials>(glib::wrap_error_as_exception(error)));
        }

        delete context;
    }
}

void location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::handle_unix_user_id_ready(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source);

    if (auto context = static_cast<UserIdQueryContext*>(user_data))
    {
        GError* error{nullptr};
        std::shared_ptr<GVariant> variant(
                    g_dbus_proxy_call_finish(context->daemon.get(), result, &error),
                    [](GVariant* variant) { if (variant) g_variant_unref(variant); });

        if (!error)
        {
            std::uint32_t uid{std::numeric_limits<std::uint32_t>::max()}; g_variant_get(variant.get(), "(u)", &uid);
            if (uid != std::numeric_limits<std::uint32_t>::max())
            {
                context->cb(make_result(Credentials{context->process_id, uid}));
            }
            else
            {
                context->cb(make_error_result<Credentials>(
                                std::make_exception_ptr(
                                    std::runtime_error("Failed to query user id."))));
            }
        }
        else
        {
            context->cb(make_error_result<Credentials>(glib::wrap_error_as_exception(error)));
        }

        delete context;
    }
}

void location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::handle_daemon_proxy_ready(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source);

    if (auto thiz = static_cast<DBusDaemonCredentialsResolver*>(user_data))
    {
        GError* error = nullptr;
        auto proxy = g_dbus_proxy_new_finish(result, &error);

        if (error)
        {
            thiz->daemon.reset();

            try
            {
                std::rethrow_exception(glib::wrap_error_as_exception(error));
            }
            catch(const std::exception& e)
            {
                LOG(WARNING) << e.what();
            }
        }
        else
        {
            thiz->daemon = glib::make_shared_object(proxy);
        }
    }
}

void location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::handle_credentials_query_finished(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source);

    if (auto p = static_cast<CredentialsQueryContext*>(user_data)) {
        std::unique_ptr<CredentialsQueryContext> context{p};

        GError* error{nullptr};

        std::shared_ptr<GVariant> variant(
                    g_dbus_proxy_call_finish(context->daemon.get(), result, &error),
                    [](GVariant* variant) { if (variant) g_variant_unref(variant); });
        if (error)
            context->callback(Result<Credentials>{glib::wrap_error_as_exception(error)});
        else if (auto credentials = dbus::decode<Credentials>(variant.get()))
            context->callback(Result<Credentials>{*credentials});
    }
}

std::string location::dbus::skeleton::Service::ObjectPathGenerator::object_path_for_caller_credentials(const location::Credentials&)
{
    static std::uint32_t index{0};
    std::stringstream ss;
    ss << location::dbus::Service::path() << "/sessions/" << index++;

    return ss.str();
}

void location::dbus::skeleton::Service::on_bus_acquired(GDBusConnection* connection, const std::string& name) 
{
    boost::ignore_unused(name);

    this->connection = glib::make_shared_object<GDBusConnection>(glib::ref_object(connection));
    this->credentials_resolver.reset(new DBusDaemonCredentialsResolver(this->connection));
}

void location::dbus::skeleton::Service::on_name_acquired(GDBusConnection*, const std::string& name)
{
    boost::ignore_unused(name);

    g_dbus_object_manager_server_set_connection(object_manager_server.get(), connection.get());
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton.get()), connection.get(), location::dbus::Service::path(), nullptr);
}

void location::dbus::skeleton::Service::on_name_lost(GDBusConnection* connection, const std::string& name) 
{
    boost::ignore_unused(connection, name);
}

void location::dbus::skeleton::Service::on_name_vanished(
        GDBusConnection* connection, const std::string& name)
{
    auto it = bus_name_to_session_path_map.find(name);
    if (it != bus_name_to_session_path_map.end())
    {
        session_store.erase(it->second);
        bus_name_to_session_path_map.erase(it);
    }
}

location::dbus::skeleton::Service::Ptr location::dbus::skeleton::Service::create(const Configuration& configuration)
{
    Ptr service{new Service(configuration)};
    return service->finalize_construction();
}

location::dbus::skeleton::Service::Service(const location::dbus::skeleton::Service::Configuration& configuration)
    : configuration(configuration),
      skeleton{glib::make_shared_object(com_ubuntu_location_service_skeleton_new())},
      object_manager_server(glib::make_shared_object<GDBusObjectManagerServer>(g_dbus_object_manager_server_new(location::dbus::Service::path())))
{
    com_ubuntu_location_service_set_state(skeleton.get(), boost::lexical_cast<std::string>(Service::configuration.impl->state().get()).c_str());
    com_ubuntu_location_service_set_does_satellite_based_positioning(skeleton.get(), Service::configuration.impl->does_satellite_based_positioning().get());
    com_ubuntu_location_service_set_does_report_cell_and_wifi_ids(skeleton.get(), Service::configuration.impl->does_report_cell_and_wifi_ids().get());
    com_ubuntu_location_service_set_is_online(skeleton.get(), Service::configuration.impl->is_online().get());
}

std::shared_ptr<location::dbus::skeleton::Service> location::dbus::skeleton::Service::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Service> wp{sp};

    configuration.impl->state().changed().connect([wp](State state) mutable {
        if (auto sp = wp.lock())
        {
            std::ostringstream ss; ss << state;
            com_ubuntu_location_service_set_state(COM_UBUNTU_LOCATION_SERVICE(sp->skeleton.get()), ss.str().c_str());
        }
    });

    configuration.impl->does_satellite_based_positioning().changed().connect([wp](bool value) mutable {
        if (auto sp = wp.lock())
        {
            com_ubuntu_location_service_set_does_satellite_based_positioning(
                        COM_UBUNTU_LOCATION_SERVICE(sp->skeleton.get()), value);
        }
    });

    configuration.impl->does_report_cell_and_wifi_ids().changed().connect([wp](bool value) mutable {
        if (auto sp = wp.lock())
        {
            com_ubuntu_location_service_set_does_report_cell_and_wifi_ids(
                        COM_UBUNTU_LOCATION_SERVICE(sp->skeleton.get()), value);
        }
    });

    configuration.impl->is_online().changed().connect([wp](bool value) mutable {
        if (auto sp = wp.lock())
        {
            com_ubuntu_location_service_set_is_online(
                        COM_UBUNTU_LOCATION_SERVICE(sp->skeleton.get()), value);
        }
    });

    g_bus_own_name(
                G_BUS_TYPE_SESSION, location::dbus::Service::name(), G_BUS_NAME_OWNER_FLAGS_NONE,
                Service::on_bus_acquired, Service::on_name_acquired, Service::on_name_lost,
                new Holder{wp}, Holder::destroy_notify);

    g_signal_connect_data(G_OBJECT(skeleton.get()), "handle-create-session-for-criteria",
                          G_CALLBACK(Service::handle_create_session_for_criteria),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(skeleton.get()), "handle-add-provider",
                          G_CALLBACK(Service::handle_add_provider),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(skeleton.get()), "handle-remove-provider",
                          G_CALLBACK(Service::handle_remove_provider),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(skeleton.get()), "notify::does-satellite-based-positioning",
                          G_CALLBACK(Service::on_does_satellite_based_positioning_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(skeleton.get()), "notify::does-report-cell-and-wifi-ids",
                          G_CALLBACK(Service::on_does_report_cell_and_wifi_ids_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(skeleton.get()), "notify::is-online",
                          G_CALLBACK(Service::on_is_online_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    return sp;
}

location::dbus::skeleton::Service::~Service() noexcept
{
}

void location::dbus::skeleton::Service::on_bus_acquired(
        GDBusConnection* connection, const gchar* name, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    if (auto holder = reinterpret_cast<Holder*>(user_data))
        if (auto thiz = holder->value.lock())
            thiz->on_bus_acquired(connection, name);
}

void location::dbus::skeleton::Service::on_name_acquired(
        GDBusConnection* connection, const gchar* name, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    if (auto holder = reinterpret_cast<Holder*>(user_data))
        if (auto thiz = holder->value.lock())
            thiz->on_name_acquired(connection, name);
}

void location::dbus::skeleton::Service::on_name_lost(
        GDBusConnection* connection, const gchar* name, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;

    if (auto holder = reinterpret_cast<Holder*>(user_data))
        if (auto thiz = holder->value.lock())
            thiz->on_name_lost(connection, name);
}

void location::dbus::skeleton::Service::on_name_vanished(
        GDBusConnection* connection, const gchar* name, gpointer user_data)
{
    if (auto holder = static_cast<glib::Holder<std::weak_ptr<Service>>*>(user_data))
        if (auto sp = holder->value.lock())
            sp->on_name_vanished(connection, name);
}

gboolean location::dbus::skeleton::Service::handle_create_session_for_criteria(ComUbuntuLocationService* service,
                                                                               GDBusMethodInvocation* invocation,
                                                                               GVariant* parameters,
                                                                               gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(service);

    auto criteria = dbus::decode<Criteria>(parameters);

    if (not criteria)
        return false;

    if (auto holder = reinterpret_cast<Holder*>(user_data))
        if (auto thiz = holder->value.lock())
            thiz->create_session(glib::make_shared_object(glib::ref_object(invocation)), *criteria);

    return true;
}

gboolean location::dbus::skeleton::Service::handle_add_provider(ComUbuntuLocationService* service,
                                                                GDBusMethodInvocation* invocation,
                                                                const gchar* path,
                                                                gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(service);

    if (auto holder = reinterpret_cast<Holder*>(user_data))
    {
        if (auto thiz = holder->value.lock())
        {
            location::providers::remote::Provider::Stub::create(
                        thiz->connection, g_dbus_method_invocation_get_sender(invocation),
                        path, [thiz, invocation](const Result<Provider::Ptr>& result)
            {
                if (!result)
                {
                    g_dbus_method_invocation_return_error_literal(
                                invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Could not create provider stub.");
                }
                else
                {
                    thiz->add_provider(result.value());
                    com_ubuntu_location_service_complete_add_provider(
                                thiz->skeleton.get(), invocation);
                }
            });

            return true;
        }
    }

    return false;
}

gboolean location::dbus::skeleton::Service::handle_remove_provider(ComUbuntuLocationService* service,
                                                                   GDBusMethodInvocation* invocation,
                                                                   const gchar* path,
                                                                   gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(service, invocation, path, user_data);
    return false;
}

void location::dbus::skeleton::Service::on_does_satellite_based_positioning_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
        if (auto sp = holder->value.lock())
            sp->does_satellite_based_positioning() =
                    com_ubuntu_location_service_get_does_satellite_based_positioning(
                        sp->skeleton.get());
}

void location::dbus::skeleton::Service::on_does_report_cell_and_wifi_ids_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
        if (auto sp = holder->value.lock())
            sp->does_report_cell_and_wifi_ids() =
                    com_ubuntu_location_service_get_does_report_cell_and_wifi_ids(
                        sp->skeleton.get());
}

void location::dbus::skeleton::Service::on_is_online_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
        if (auto sp = holder->value.lock())
            sp->is_online() =
                    com_ubuntu_location_service_get_is_online(
                        sp->skeleton.get());
}

const core::Property<location::Service::State>& location::dbus::skeleton::Service::state() const
{
    return configuration.impl->state();
}

core::Property<bool>& location::dbus::skeleton::Service::does_satellite_based_positioning()
{
    return configuration.impl->does_satellite_based_positioning();
}

core::Property<bool>& location::dbus::skeleton::Service::does_report_cell_and_wifi_ids()
{
    return configuration.impl->does_report_cell_and_wifi_ids();
}

core::Property<bool>& location::dbus::skeleton::Service::is_online()
{
    return configuration.impl->is_online();
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::dbus::skeleton::Service::visible_space_vehicles()
{
    return configuration.impl->visible_space_vehicles();
}

void location::dbus::skeleton::Service::create_session_for_criteria(const Criteria& criteria, const std::function<void(const Session::Ptr&)>& cb)
{
    configuration.impl->create_session_for_criteria(criteria, cb);
}

void location::dbus::skeleton::Service::add_provider(const Provider::Ptr& provider)
{
    configuration.impl->add_provider(provider);
}

void location::dbus::skeleton::Service::create_session(
        const glib::SharedObject<GDBusMethodInvocation>& invocation, const Criteria& criteria)
{
    if (!credentials_resolver) {
        g_dbus_method_invocation_return_error_literal(
                    invocation.get(), G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "Dependencies not yet available, try again later.");
        return;
    }

    credentials_resolver->resolve_credentials_for_sender(
                g_dbus_method_invocation_get_sender(invocation.get()),
                [this, criteria, invocation](const Result<Credentials>& credentials)
    {
        if (!credentials)
        {
            try
            {
                credentials.rethrow();
            }
            catch (const std::exception& e)
            {
                g_dbus_method_invocation_return_error_literal(
                            invocation.get(), G_DBUS_ERROR, G_DBUS_ERROR_FAILED, e.what());
            }
            catch (...)
            {
                g_dbus_method_invocation_return_error_literal(
                            invocation.get(), G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Could not resolve credentials.");
            }

            return;
        }

        auto response = configuration.permission_manager->check_permission_for_credentials(
                    criteria, credentials.value());

        if (response != PermissionManager::Result::granted)
        {
            g_dbus_method_invocation_return_error_literal(
                        invocation.get(), G_DBUS_ERROR, G_DBUS_ERROR_AUTH_FAILED, "");
            return;
        }

        create_session(invocation, criteria, credentials.value());
    });
    return;
}

void location::dbus::skeleton::Service::create_session(
        const glib::SharedObject<GDBusMethodInvocation>& invocation, const Criteria& criteria, const Credentials& credentials)
{
    auto sp = shared_from_this();
    std::weak_ptr<Service> wp{sp};

    create_session_for_criteria(criteria, [this, wp, invocation, credentials](const Session::Ptr& impl)
    {
        if (auto sp = wp.lock())
        {
            auto path = object_path_generator.object_path_for_caller_credentials(credentials);
            auto session = glib::make_shared_object(com_ubuntu_location_service_session_skeleton_new());
            auto skeleton = location::dbus::skeleton::Session::create(
                        location::dbus::skeleton::Session::Configuration{session, impl});
            auto skeleton_object = glib::make_shared_object(object_skeleton_new(path.c_str()));

            session_store[path] = skeleton;
            bus_name_to_session_path_map[g_dbus_method_invocation_get_sender(invocation.get())] = path;

            g_bus_watch_name_on_connection(
                        g_dbus_method_invocation_get_connection(invocation.get()),
                        g_dbus_method_invocation_get_sender(invocation.get()),
                        G_BUS_NAME_WATCHER_FLAGS_NONE,
                        nullptr, Service::on_name_vanished,
                        new glib::Holder<std::weak_ptr<Service>>{wp},
                        glib::Holder<std::weak_ptr<Service>>::destroy_notify);

            g_dbus_interface_skeleton_export(
                        G_DBUS_INTERFACE_SKELETON(session.get()), connection.get(),
                        path.c_str(), nullptr);

            g_dbus_method_invocation_return_value(
                        invocation.get(), g_variant_new("(o)", path.c_str()));
        }
    });
}

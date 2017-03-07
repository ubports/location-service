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

#ifndef LOCATION_DBUS_SKELETON_SERVICE_H_
#define LOCATION_DBUS_SKELETON_SERVICE_H_

#include <location/service.h>
#include <location/permission_manager.h>
#include <location/result.h>

#include <location/dbus/bus.h>
#include <location/dbus/service.h>
#include <location/dbus/service_gen.h>
#include <location/dbus/skeleton/session.h>
#include <location/glib/shared_object.h>

#include <unordered_map>

#include <gio/gio.h>

namespace location
{
namespace dbus
{
namespace skeleton
{
class Service : public location::Service,
                public std::enable_shared_from_this<Service>
{
public:
    typedef std::shared_ptr<Service> Ptr;    

    struct Configuration
    {
        Bus bus;                                        // Connect to this bus instance.
        location::Service::Ptr impl;                    // The actual service implementation.
        PermissionManager::Ptr permission_manager;      // A permission manager implementation.
    };

    static Ptr create(const Configuration& configuration);

    Service(const Configuration& configuration);
    ~Service() noexcept;

    // From location::service::Interface
    const core::Property<State>& state() const override;
    core::Property<bool>& does_satellite_based_positioning() override;
    core::Property<bool>& does_report_cell_and_wifi_ids() override;
    core::Property<bool>& is_online() override;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() override;
    void create_session_for_criteria(const Criteria& criteria, const std::function<void(const Session::Ptr&)>& cb) override;
    void add_provider(const Provider::Ptr& provider) override;

private:
    // Models the generation of stable and unique object paths for client-specific sessions.
    // The requirements for the resulting object path are:
    //   * Unique for the entire system over its complete lifetime
    //   * Stable with respect to an app. That is, one app is always assigned the same object path.
    struct ObjectPathGenerator
    {
        typedef std::shared_ptr<ObjectPathGenerator> Ptr;

        ObjectPathGenerator() = default;
        virtual ~ObjectPathGenerator() = default;

        // Calculates an object path from pid and uid. The default implementation
        // creates the path according to the following steps:
        //    [1.] Query the AppArmor profile name for pid in credentials.
        //    [1.1] If the process is running unconfined, rely on a counter to assemble the session name.
        //    [1.2] If the process is confined, use the AppArmor profile name to generate the path.
        virtual std::string object_path_for_caller_credentials(const Credentials& credentials);
    };

    // Resolves the credentials of a caller:
    //   * GetConnectionUnixProcessID
    //   * GetConnectionUnixUser
    class DBusDaemonCredentialsResolver
    {
    public:
        // Sets up a new instance for the given bus connection.
        DBusDaemonCredentialsResolver(const glib::SharedObject<GDBusConnection>& connection);

        // Resolves the caller to pid, uid by calling out to the dbus daemon.
        void resolve_credentials_for_sender(const std::string& sender, std::function<void(const Result<Credentials>&)> cb);

    private:
        struct CredentialsQueryContext
        {
            glib::SharedObject<GDBusProxy> daemon;
            std::function<void(const Result<Credentials>&)> callback;
        };

        struct ProcessIdQueryContext
        {
            glib::SharedObject<GDBusProxy> daemon;
            std::string sender;
            std::function<void(const Result<Credentials>&)> cb;
        };

        struct UserIdQueryContext
        {
            glib::SharedObject<GDBusProxy> daemon;
            std::string sender;
            std::uint32_t process_id;
            std::function<void(const Result<Credentials>&)> cb;
        };

        static void handle_daemon_proxy_ready(
                GObject* source, GAsyncResult* result, gpointer user_data);
        static void handle_unix_process_id_ready(
                GObject* source, GAsyncResult* result, gpointer user_data);
        static void handle_unix_user_id_ready(
                GObject* source, GAsyncResult* result, gpointer user_data);
        static void handle_credentials_query_finished(
                GObject* source, GAsyncResult* result, gpointer user_data);

        glib::SharedObject<GDBusProxy> daemon{nullptr};
    };

    static void on_bus_acquired(
            GDBusConnection* connection, const gchar* name, gpointer user_data);
    static void on_name_acquired(
            GDBusConnection* connection, const gchar* name, gpointer user_data);
    static void on_name_lost(
            GDBusConnection* connection, const gchar* name, gpointer user_data);
    static void on_name_vanished(
            GDBusConnection* connection, const gchar* name, gpointer user_data);

    static gboolean handle_create_session_for_criteria(
            ComUbuntuLocationService* service, GDBusMethodInvocation* invocation, GVariant* criteria, gpointer user_data);
    static gboolean handle_add_provider(
            ComUbuntuLocationService* service, GDBusMethodInvocation* invocation, const gchar* path, gpointer user_data);
    static gboolean handle_remove_provider(
            ComUbuntuLocationService* service, GDBusMethodInvocation* invocation, const gchar* path, gpointer user_data);

    static void on_does_satellite_based_positioning_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_does_report_cell_and_wifi_ids_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_is_online_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);

    std::shared_ptr<Service> finalize_construction();

    void on_bus_acquired(GDBusConnection* connection, const std::string& name);
    void on_name_acquired(GDBusConnection* connection, const std::string& name);
    void on_name_lost(GDBusConnection* connection, const std::string& name);
    void on_name_vanished(GDBusConnection* connection, const std::string& name);

    void create_session(const glib::SharedObject<GDBusMethodInvocation>& invocation, const Criteria& criteria);
    void create_session(const glib::SharedObject<GDBusMethodInvocation>& invocation, const Criteria& criteria, const Credentials& credentials);

    // Stores the configuration passed in at creation time.
    Configuration configuration;

    // Keeps track of running sessions, keying them by their unique object path.
    std::unordered_map<std::string, skeleton::Session::Ptr> session_store;
    std::unordered_map<std::string, std::string> bus_name_to_session_path_map;
    // Keeps track of providers
    std::unordered_map<std::string, location::Provider::Ptr> provider_store;

    ObjectPathGenerator object_path_generator;

    glib::SharedObject<GDBusConnection> connection;
    glib::SharedObject<ComUbuntuLocationService> skeleton;
    glib::SharedObject<GDBusObjectManagerServer> object_manager_server;
    std::shared_ptr<DBusDaemonCredentialsResolver> credentials_resolver;
};

}  // namespace skeleton
}  // namespace dbus
}  // namespace location

#endif // LOCATION_DBUS_SKELETON_SERVICE_H_

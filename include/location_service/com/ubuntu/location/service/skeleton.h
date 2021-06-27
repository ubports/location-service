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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_

#include <com/ubuntu/location/service/interface.h>
#include <com/ubuntu/location/service/permission_manager.h>
#include <com/ubuntu/location/service/session/interface.h>

#include <core/dbus/dbus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/skeleton.h>

#include <core/dbus/interfaces/properties.h>

#include <functional>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Skeleton
        : public core::dbus::Skeleton<com::ubuntu::location::service::Interface>,
          public std::enable_shared_from_this<Skeleton>
{
public:
    typedef std::shared_ptr<Skeleton> Ptr;

    // Models resolution of an incoming dbus message to the credentials of the message sender.
    struct CredentialsResolver
    {
        typedef std::shared_ptr<CredentialsResolver> Ptr;

        CredentialsResolver() = default;
        virtual ~CredentialsResolver() = default;

        // Resolves the sender of msg to the respective credentials.
        virtual Credentials resolve_credentials_for_incoming_message(const core::dbus::Message::Ptr& msg) = 0;
    };

    // Implements CredentialsResolver by reaching out to the dbus daemon and
    // invoking:
    //   * GetConnectionUnixProcessID
    //   * GetConnectionUnixUser
    struct DBusDaemonCredentialsResolver : public CredentialsResolver
    {
        // Functor for resolving a process id to an app-armor profile name.
        typedef std::function<std::string(pid_t)> AppArmorProfileResolver;

        // Returns an AppArmorProfileResolver leveraging libapparmor.
        static AppArmorProfileResolver libapparmor_profile_resolver();

        // Sets up a new instance for the given bus connection.
        DBusDaemonCredentialsResolver(const core::dbus::Bus::Ptr& bus,
                                      AppArmorProfileResolver app_armor_profile_resolver);

        // Resolves the sender of msg to pid, uid by calling out to the dbus daemon.
        Credentials resolve_credentials_for_incoming_message(const core::dbus::Message::Ptr& msg);

        // Stub for accessing the dbus daemon.
        core::dbus::DBus daemon;

        // Helper to resolve an application's pid to an app-armor profile name.
        AppArmorProfileResolver app_armor_profile_resolver;
    };

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
        virtual core::dbus::types::ObjectPath object_path_for_caller_credentials(const Credentials& credentials);
    };

    struct Configuration
    {
        // DBus connection set up for handling requests to the service.
        core::dbus::Bus::Ptr incoming;
        // DBus connection for reaching out to other services in a non-blocking way.
        core::dbus::Bus::Ptr outgoing;
        // An implementation of CredentialsResolver for resolving incoming message sender
        // to Credentials = uid, pid.
        CredentialsResolver::Ptr credentials_resolver;
        // An implementation of ObjectPathGenerator for generating session names.
        ObjectPathGenerator::Ptr object_path_generator;
        // Permission manager implementation for verifying incoming requests.
        PermissionManager::Ptr permission_manager;
    };

    Skeleton(const Configuration& configuration);
    ~Skeleton() noexcept;

    // From com::ubuntu::location::service::Interface
    const core::Property<State>& state() const;
    core::Property<bool>& does_satellite_based_positioning();
    core::Property<bool>& does_report_cell_and_wifi_ids();
    core::Property<bool>& is_online();
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles();
    core::Property<std::vector<std::string>>& client_applications();

protected:
    // Enable subclasses to alter the state.
    core::Property<State>& mutable_state();
private:
    // Handles incoming message calls for create_session_for_criteria.
    // Dispatches to the actual implementation, and manages object lifetimes.
    void handle_create_session_for_criteria(const core::dbus::Message::Ptr& msg);

    // Tries to register the given session under the given path in the session store.
    // Returns true iff the session has been added to the store.
    bool add_to_session_store_for_path(
            const core::dbus::types::ObjectPath& path,
            std::unique_ptr<core::dbus::ServiceWatcher> watcher,
            const session::Interface::Ptr& session);

    // Removes the session with the given path from the session store.
    void remove_from_session_store_for_path(const core::dbus::types::ObjectPath& path);

    void add_client_application(const std::string& app_id);
    void remove_client_application(const std::string& app_id);

    // Called whenever the overall state of the service changes.
    void on_state_changed(State state);
    // Called whenever the value of the respective property changes.
    void on_does_satellite_based_positioning_changed(bool value);
    // Called whenever the value of the respective property changes.
    void on_does_report_cell_and_wifi_ids_changed(bool value);
    // Called whenever the value of the respective property changes.
    void on_is_online_changed(bool value);
    void on_client_applications_changed(const std::vector<std::string>& value);

    // Stores the configuration passed in at creation time.
    Configuration configuration;
    // We observe sessions if they have died and resigned from the bus.
    core::dbus::DBus daemon;
    // The skeleton object representing com.ubuntu.location.service.Interface on the bus.
    core::dbus::Object::Ptr object;
    // We emit property changes manually.
    core::dbus::Signal
    <
        core::dbus::interfaces::Properties::Signals::PropertiesChanged,
        core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType
    >::Ptr properties_changed;

    // DBus properties as exposed on the bus for com.ubuntu.location.service.Interface
    struct
    {
        std::shared_ptr< core::dbus::Property<Interface::Properties::State> > state;
        std::shared_ptr< core::dbus::Property<Interface::Properties::DoesSatelliteBasedPositioning> > does_satellite_based_positioning;
        std::shared_ptr< core::dbus::Property<Interface::Properties::DoesReportCellAndWifiIds> > does_report_cell_and_wifi_ids;
        std::shared_ptr< core::dbus::Property<Interface::Properties::IsOnline> > is_online;
        std::shared_ptr< core::dbus::Property<Interface::Properties::VisibleSpaceVehicles> > visible_space_vehicles;
        std::shared_ptr< core::dbus::Property<Interface::Properties::ClientApplications> > client_applications;
    } properties;
    // We sign up to property changes here, to be able to report them to the bus
    struct
    {
        core::ScopedConnection state;
        core::ScopedConnection does_satellite_based_positioning;
        core::ScopedConnection does_report_cell_and_wifi_ids;
        core::ScopedConnection is_online;
        core::ScopedConnection client_applications;
    } connections;
    // Guards the session store.
    std::mutex guard;
    // We track sessions and their respective watchers.
    struct Element
    {
        std::unique_ptr<core::dbus::ServiceWatcher> watcher;
        std::shared_ptr<session::Interface> session;
    };
    // Keeps track of running sessions, keying them by their unique object path.
    std::map<dbus::types::ObjectPath, Element> session_store;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SKELETON_H_

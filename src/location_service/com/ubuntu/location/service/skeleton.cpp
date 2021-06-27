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
#include <com/ubuntu/location/service/skeleton.h>
#include <com/ubuntu/location/service/session/skeleton.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/types/object_path.h>

#include <algorithm>

#include <sys/apparmor.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

namespace
{
const std::vector<std::string>& the_empty_array_of_invalidated_properties()
{
    static const std::vector<std::string> v; return v;
}

dbus::Message::Ptr the_empty_reply()
{
    return dbus::Message::Ptr{};
}
}

culs::Skeleton::DBusDaemonCredentialsResolver::DBusDaemonCredentialsResolver(
    const dbus::Bus::Ptr& bus,
    AppArmorProfileResolver app_armor_profile_resolver)
    : daemon(bus),
      app_armor_profile_resolver{app_armor_profile_resolver}
{
}

culs::Skeleton::DBusDaemonCredentialsResolver::AppArmorProfileResolver
culs::Skeleton::DBusDaemonCredentialsResolver::libapparmor_profile_resolver()
{
    return [](pid_t pid)
    {
        static const int app_armor_error{-1};

        // We make sure to clean up the returned string.
        struct Scope
        {
            ~Scope()
            {
                if (con) ::free(con);
            }

            char* con{nullptr};
            char* mode{nullptr};
        } scope;

        // Reach out to apparmor
        auto rc = aa_gettaskcon(pid, &scope.con, &scope.mode);

        // From man aa_gettaskcon:
        // On success size of data placed in the buffer is returned, this includes the mode if
        //present and any terminating characters. On error, -1 is returned, and errno(3) is
        //set appropriately.
        if (rc == app_armor_error) throw std::system_error
        {
            errno,
            std::system_category()
        };

        // Safely construct the string
        return std::string
        {
            scope.con ? scope.con : ""
        };
    };
}

culs::Credentials
culs::Skeleton::DBusDaemonCredentialsResolver::resolve_credentials_for_incoming_message(const dbus::Message::Ptr& msg)
{
    /* We should really use the GetConnectionCredentials method here, but
     * dbus-cpp does not support it. So, get the PID and use the apparmor
     * client library to obtain the profile label.
     */
    std::string profile;
    auto pid = static_cast<pid_t>(daemon.get_connection_unix_process_id(msg->sender()));
    try {
        profile = app_armor_profile_resolver(pid);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << "Could not resolve PID " << pid << " to apparmor profile: " << e.what();
    }

    return culs::Credentials
    {
        pid,
        static_cast<uid_t>(daemon.get_connection_unix_user(msg->sender())),
        profile,
    };
}

core::dbus::types::ObjectPath culs::Skeleton::ObjectPathGenerator::object_path_for_caller_credentials(const culs::Credentials&)
{
    static std::uint32_t index{0};
    std::stringstream ss; ss << "/sessions/" << index++;

    return core::dbus::types::ObjectPath{ss.str()};
}


culs::Skeleton::Skeleton(const culs::Skeleton::Configuration& configuration)
    : dbus::Skeleton<culs::Interface>(configuration.incoming),
      configuration(configuration),
      daemon(configuration.incoming),
      object(access_service()->add_object_for_path(culs::Interface::path())),
      properties_changed(object->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>()),
      properties
      {
          object->get_property<culs::Interface::Properties::State>(),
          object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>(),
          object->get_property<culs::Interface::Properties::DoesReportCellAndWifiIds>(),
          object->get_property<culs::Interface::Properties::IsOnline>(),
          object->get_property<culs::Interface::Properties::VisibleSpaceVehicles>(),
          object->get_property<culs::Interface::Properties::ClientApplications>(),
      },
      connections
      {
          properties.state->changed().connect([this](State state)
          {
              on_state_changed(state);
          }),
          properties.does_satellite_based_positioning->changed().connect([this](bool value)
          {
              on_does_satellite_based_positioning_changed(value);
          }),
          properties.does_report_cell_and_wifi_ids->changed().connect([this](bool value)
          {
              on_does_report_cell_and_wifi_ids_changed(value);
          }),
          properties.is_online->changed().connect([this](bool value)
          {
              on_is_online_changed(value);
          }),
          properties.client_applications->changed().connect([this](const std::vector<std::string>& value)
          {
              on_client_applications_changed(value);
          }),
      }
{
    object->install_method_handler<culs::Interface::CreateSessionForCriteria>([this](const dbus::Message::Ptr& msg)
    {
        handle_create_session_for_criteria(msg);
    });
}

culs::Skeleton::~Skeleton() noexcept
{
    object->uninstall_method_handler<culs::Interface::CreateSessionForCriteria>();
}

core::Property<culs::State>& culs::Skeleton::mutable_state()
{
    return *properties.state;
}

void culs::Skeleton::handle_create_session_for_criteria(const dbus::Message::Ptr& in)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto sender = in->sender();
    auto reply = the_empty_reply();
    auto thiz = shared_from_this();

    std::string apparmor_profile;

    try
    {
        Criteria criteria;
        in->reader() >> criteria;

        auto credentials =
            configuration.credentials_resolver->resolve_credentials_for_incoming_message(in);

        auto result =
            configuration.permission_manager->check_permission_for_credentials(criteria, credentials);

        if (PermissionManager::Result::rejected == result) throw std::runtime_error
        {
            "Client lacks permissions to access the service with the given criteria"
        };

        auto path =
            configuration.object_path_generator->object_path_for_caller_credentials(credentials);

        auto stub =
            dbus::Service::use_service(configuration.outgoing, sender);

        culss::Skeleton::Configuration config
        {
            path,
            culss::Skeleton::Local
            {
                create_session_for_criteria(criteria),
                configuration.incoming
            },
            culss::Skeleton::Remote
            {
                stub->object_for_path(path),
                credentials.profile,
            }
        };

        auto watcher = daemon.make_service_watcher(sender);
        watcher->owner_changed().connect([thiz, path](const std::string&, const std::string&)
        {
            thiz->remove_from_session_store_for_path(path);
        });

        if (not add_to_session_store_for_path(path, std::move(watcher), culss::Interface::Ptr{new culss::Skeleton{config}}))
        {
            reply = dbus::Message::make_error(
                        in,
                        culs::Interface::Errors::CreatingSession::name(),
                        "Refused to create second session for same process");
        } else
        {
            reply = dbus::Message::make_method_return(in);
            reply->writer() << path;
            add_client_application(credentials.profile);
        }
    } catch(const std::exception& e)
    {
        // We only send a very generic error message to the client to avoid
        // leaking any sort of internal error handling details to untrusted
        // apps.
        reply = dbus::Message::make_error(
                    in,
                    culs::Interface::Errors::CreatingSession::name(),
                    "Error creating session");
        // We log the error for debugging purposes.
        SYSLOG(ERROR) << "Error creating session: " << e.what();
    }

    // We are done processing the request and try to send out the result to the client.
    try
    {
        configuration.incoming->send(reply);
    } catch(const std::exception& e)
    {
        // We log the error for debugging purposes.
        SYSLOG(ERROR) << "Error sending reply to session creation request: " << e.what();
    }
}

bool culs::Skeleton::add_to_session_store_for_path(
        const core::dbus::types::ObjectPath& path,
        std::unique_ptr<core::dbus::ServiceWatcher> watcher,
        const culss::Interface::Ptr& session)
{
    std::lock_guard<std::mutex> lg(guard);
    bool inserted = false;
    std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(path, Element{std::move(watcher), session}));
    return inserted;
}

void culs::Skeleton::remove_from_session_store_for_path(const core::dbus::types::ObjectPath& path)
{
    std::lock_guard<std::mutex> lg(guard);
    auto i = session_store.find(path);
    if (i != session_store.end()) {
        auto session = i->second.session;
        remove_client_application(static_cast<culss::Skeleton*>(session.get())->remote_app_id());
        session_store.erase(i);
    }
}

void culs::Skeleton::add_client_application(const std::string& app_id)
{
    std::vector<std::string> apps = client_applications().get();
    apps.push_back(app_id);
    client_applications() = apps;
}

void culs::Skeleton::remove_client_application(const std::string& app_id)
{
    std::vector<std::string> apps = client_applications().get();
    apps.erase(std::remove(apps.begin(), apps.end(), app_id), apps.end());
    client_applications() = apps;
}

void culs::Skeleton::on_state_changed(culs::State state)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            culs::Interface::Properties::State::name(),
            core::dbus::types::Variant::encode(state)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<culs::Interface>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}


void culs::Skeleton::on_does_satellite_based_positioning_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            culs::Interface::Properties::DoesSatelliteBasedPositioning::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<culs::Interface>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}

void culs::Skeleton::on_does_report_cell_and_wifi_ids_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            culs::Interface::Properties::DoesReportCellAndWifiIds::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<culs::Interface>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

void culs::Skeleton::on_is_online_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            culs::Interface::Properties::IsOnline::name(),
            core::dbus::types::Variant::encode(value)
        }
    };
    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<culs::Interface>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

void culs::Skeleton::on_client_applications_changed(const std::vector<std::string>& value)
{
    VLOG(1) << __PRETTY_FUNCTION__;
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            culs::Interface::Properties::ClientApplications::name(),
            core::dbus::types::Variant::encode(value)
        }
    };
    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<culs::Interface>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

const core::Property<culs::State>& culs::Skeleton::state() const
{
    return *properties.state;
}

core::Property<bool>& culs::Skeleton::does_satellite_based_positioning()
{
    return *properties.does_satellite_based_positioning;
}

core::Property<bool>& culs::Skeleton::does_report_cell_and_wifi_ids()
{
    return *properties.does_report_cell_and_wifi_ids;
}

core::Property<bool>& culs::Skeleton::is_online()
{
    return *properties.is_online;
}

core::Property<std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>>& culs::Skeleton::visible_space_vehicles()
{
    return *properties.visible_space_vehicles;
}

core::Property<std::vector<std::string>>& culs::Skeleton::client_applications()
{
    VLOG(1) << __PRETTY_FUNCTION__;
    return *properties.client_applications;
}

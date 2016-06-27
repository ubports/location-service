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
#include <location/service/skeleton.h>
#include <location/service/session/skeleton.h>

#include <location/logging.h>

#include <core/dbus/types/object_path.h>

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

location::service::Skeleton::DBusDaemonCredentialsResolver::DBusDaemonCredentialsResolver(const dbus::Bus::Ptr& bus)
    : daemon(bus)
{
}

location::service::Credentials
location::service::Skeleton::DBusDaemonCredentialsResolver::resolve_credentials_for_incoming_message(const dbus::Message::Ptr& msg)
{
    return location::service::Credentials
    {
        static_cast<pid_t>(daemon.get_connection_unix_process_id(msg->sender())),
        static_cast<uid_t>(daemon.get_connection_unix_user(msg->sender()))
    };
}

core::dbus::types::ObjectPath location::service::Skeleton::ObjectPathGenerator::object_path_for_caller_credentials(const location::service::Credentials&)
{
    static std::uint32_t index{0};
    std::stringstream ss; ss << "/sessions/" << index++;

    return core::dbus::types::ObjectPath{ss.str()};
}


location::service::Skeleton::Skeleton(const location::service::Skeleton::Configuration& configuration)
    : dbus::Skeleton<location::service::Interface>(configuration.incoming),
      configuration(configuration),
      daemon(configuration.incoming),
      object(access_service()->add_object_for_path(location::service::Interface::path())),
      properties_changed(object->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>()),
      properties
      {
          object->get_property<location::service::Interface::Properties::State>(),
          object->get_property<location::service::Interface::Properties::DoesSatelliteBasedPositioning>(),
          object->get_property<location::service::Interface::Properties::DoesReportCellAndWifiIds>(),
          object->get_property<location::service::Interface::Properties::IsOnline>(),
          object->get_property<location::service::Interface::Properties::VisibleSpaceVehicles>()
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
          })
      }
{
    object->install_method_handler<location::service::Interface::CreateSessionForCriteria>([this](const dbus::Message::Ptr& msg)
    {
        handle_create_session_for_criteria(msg);
    });
}

location::service::Skeleton::~Skeleton() noexcept
{
    object->uninstall_method_handler<location::service::Interface::CreateSessionForCriteria>();
}

core::Property<location::service::State>& location::service::Skeleton::mutable_state()
{
    return *properties.state;
}

void location::service::Skeleton::handle_create_session_for_criteria(const dbus::Message::Ptr& in)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto sender = in->sender();
    auto reply = the_empty_reply();
    auto thiz = shared_from_this();

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

        location::service::session::Skeleton::Configuration config
        {
            path,
            location::service::session::Skeleton::Local
            {
                create_session_for_criteria(criteria),
                configuration.incoming
            },
            location::service::session::Skeleton::Remote
            {
                stub->object_for_path(path)
            }
        };

        auto watcher = daemon.make_service_watcher(sender);
        watcher->owner_changed().connect([thiz, path](const std::string&, const std::string&)
        {
            thiz->remove_from_session_store_for_path(path);
        });

        if (not add_to_session_store_for_path(path, std::move(watcher), location::service::session::Interface::Ptr{new location::service::session::Skeleton{config}}))
        {
            reply = dbus::Message::make_error(
                        in,
                        location::service::Interface::Errors::CreatingSession::name(),
                        "Refused to create second session for same process");
        } else
        {
            reply = dbus::Message::make_method_return(in);
            reply->writer() << path;
        }

    } catch(const std::exception& e)
    {
        // We only send a very generic error message to the client to avoid
        // leaking any sort of internal error handling details to untrusted
        // apps.
        reply = dbus::Message::make_error(
                    in,
                    location::service::Interface::Errors::CreatingSession::name(),
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

bool location::service::Skeleton::add_to_session_store_for_path(
        const core::dbus::types::ObjectPath& path,
        std::unique_ptr<core::dbus::ServiceWatcher> watcher,
        const location::service::session::Interface::Ptr& session)
{
    std::lock_guard<std::mutex> lg(guard);
    bool inserted = false;
    std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(path, Element{std::move(watcher), session}));
    return inserted;
}

void location::service::Skeleton::remove_from_session_store_for_path(const core::dbus::types::ObjectPath& path)
{
    std::lock_guard<std::mutex> lg(guard);
    session_store.erase(path);
}

void location::service::Skeleton::on_state_changed(location::service::State state)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::service::Interface::Properties::State::name(),
            core::dbus::types::Variant::encode(state)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<location::service::Interface>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}


void location::service::Skeleton::on_does_satellite_based_positioning_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::service::Interface::Properties::DoesSatelliteBasedPositioning::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<location::service::Interface>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}

void location::service::Skeleton::on_does_report_cell_and_wifi_ids_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::service::Interface::Properties::DoesReportCellAndWifiIds::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<location::service::Interface>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

void location::service::Skeleton::on_is_online_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::service::Interface::Properties::IsOnline::name(),
            core::dbus::types::Variant::encode(value)
        }
    };
    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<location::service::Interface>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

const core::Property<location::service::State>& location::service::Skeleton::state() const
{
    return *properties.state;
}

core::Property<bool>& location::service::Skeleton::does_satellite_based_positioning()
{
    return *properties.does_satellite_based_positioning;
}

core::Property<bool>& location::service::Skeleton::does_report_cell_and_wifi_ids()
{
    return *properties.does_report_cell_and_wifi_ids;
}

core::Property<bool>& location::service::Skeleton::is_online()
{
    return *properties.is_online;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::service::Skeleton::visible_space_vehicles()
{
    return *properties.visible_space_vehicles;
}

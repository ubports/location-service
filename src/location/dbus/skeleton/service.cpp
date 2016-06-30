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

#include <location/dbus/skeleton/service.h>
#include <location/dbus/skeleton/session.h>

#include <location/dbus/codec.h>

#include <location/criteria.h>
#include <location/logging.h>

namespace
{
const std::vector<std::string>& the_empty_array_of_invalidated_properties()
{
    static const std::vector<std::string> v; return v;
}

core::dbus::Message::Ptr the_empty_reply()
{
    return core::dbus::Message::Ptr{};
}
}

location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::DBusDaemonCredentialsResolver(const core::dbus::Bus::Ptr& bus)
    : daemon(bus)
{
}

location::service::Credentials
location::dbus::skeleton::Service::DBusDaemonCredentialsResolver::resolve_credentials_for_incoming_message(const core::dbus::Message::Ptr& msg)
{
    return location::service::Credentials
    {
        static_cast<pid_t>(daemon.get_connection_unix_process_id(msg->sender())),
        static_cast<uid_t>(daemon.get_connection_unix_user(msg->sender()))
    };
}

core::dbus::types::ObjectPath location::dbus::skeleton::Service::ObjectPathGenerator::object_path_for_caller_credentials(const location::service::Credentials&)
{
    static std::uint32_t index{0};
    std::stringstream ss; ss << "/sessions/" << index++;

    return core::dbus::types::ObjectPath{ss.str()};
}


location::dbus::skeleton::Service::Service(const location::dbus::skeleton::Service::Configuration& configuration)
    : configuration(configuration),
      daemon(configuration.incoming),
      object(configuration.service->add_object_for_path(location::dbus::Service::path())),
      properties_changed(object->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>()),
      properties
      {
          object->get_property<location::dbus::Service::Properties::State>(),
          object->get_property<location::dbus::Service::Properties::DoesSatelliteBasedPositioning>(),
          object->get_property<location::dbus::Service::Properties::DoesReportCellAndWifiIds>(),
          object->get_property<location::dbus::Service::Properties::IsOnline>(),
          object->get_property<location::dbus::Service::Properties::VisibleSpaceVehicles>()
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
    object->install_method_handler<location::dbus::Service::CreateSessionForCriteria>([this](const core::dbus::Message::Ptr& msg)
    {
        handle_create_session_for_criteria(msg);
    });
}

location::dbus::skeleton::Service::~Service() noexcept
{
    object->uninstall_method_handler<location::dbus::Service::CreateSessionForCriteria>();
}

core::Property<location::Service::State>& location::dbus::skeleton::Service::mutable_state()
{
    return *properties.state;
}

void location::dbus::skeleton::Service::handle_create_session_for_criteria(const core::dbus::Message::Ptr& in)
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

        if (service::PermissionManager::Result::rejected == result) throw std::runtime_error
        {
            "Client lacks permissions to access the service with the given criteria"
        };

        auto path =
            configuration.object_path_generator->object_path_for_caller_credentials(credentials);

        auto stub =
            core::dbus::Service::use_service(configuration.outgoing, sender);

        location::dbus::skeleton::Session::Configuration config
        {
            path,
            location::dbus::skeleton::Session::Local
            {
                create_session_for_criteria(criteria),
                configuration.incoming,
                configuration.service->add_object_for_path(path)
            },
            location::dbus::skeleton::Session::Remote
            {
                stub->object_for_path(path)
            }
        };

        auto watcher = daemon.make_service_watcher(sender);
        watcher->owner_changed().connect([thiz, path](const std::string&, const std::string&)
        {
            thiz->remove_from_session_store_for_path(path);
        });

        if (not add_to_session_store_for_path(path, std::move(watcher), std::make_shared<location::dbus::skeleton::Session>(config)))
        {
            reply = core::dbus::Message::make_error(
                        in,
                        location::dbus::Service::Errors::CreatingSession::name(),
                        "Refused to create second session for same process");
        } else
        {
            reply = core::dbus::Message::make_method_return(in);
            reply->writer() << path;
        }

    } catch(const std::exception& e)
    {
        // We only send a very generic error message to the client to avoid
        // leaking any sort of internal error handling details to untrusted
        // apps.
        reply = core::dbus::Message::make_error(
                    in,
                    location::dbus::Service::Errors::CreatingSession::name(),
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

bool location::dbus::skeleton::Service::add_to_session_store_for_path(
        const core::dbus::types::ObjectPath& path,
        std::unique_ptr<core::dbus::ServiceWatcher> watcher,
        const Session::Ptr& session)
{
    std::lock_guard<std::mutex> lg(guard);
    bool inserted = false;
    std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(path, Element{std::move(watcher), session}));
    return inserted;
}

void location::dbus::skeleton::Service::remove_from_session_store_for_path(const core::dbus::types::ObjectPath& path)
{
    std::lock_guard<std::mutex> lg(guard);
    session_store.erase(path);
}

void location::dbus::skeleton::Service::on_state_changed(location::Service::State state)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::dbus::Service::Properties::State::name(),
            core::dbus::types::Variant::encode(state)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<location::dbus::Service>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}


void location::dbus::skeleton::Service::on_does_satellite_based_positioning_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::dbus::Service::Properties::DoesSatelliteBasedPositioning::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
                std::tie(
                    core::dbus::traits::Service<location::dbus::Service>::interface_name(),
                    dict,
                    the_empty_array_of_invalidated_properties()));
}

void location::dbus::skeleton::Service::on_does_report_cell_and_wifi_ids_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::dbus::Service::Properties::DoesReportCellAndWifiIds::name(),
            core::dbus::types::Variant::encode(value)
        }
    };

    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<location::dbus::Service>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

void location::dbus::skeleton::Service::on_is_online_changed(bool value)
{
    std::map<std::string, core::dbus::types::Variant> dict
    {
        {
            location::dbus::Service::Properties::IsOnline::name(),
            core::dbus::types::Variant::encode(value)
        }
    };
    properties_changed->emit(
            std::tie(
                core::dbus::traits::Service<location::dbus::Service>::interface_name(),
                dict,
                the_empty_array_of_invalidated_properties()));
}

const core::Property<location::Service::State>& location::dbus::skeleton::Service::state() const
{
    return *properties.state;
}

core::Property<bool>& location::dbus::skeleton::Service::does_satellite_based_positioning()
{
    return *properties.does_satellite_based_positioning;
}

core::Property<bool>& location::dbus::skeleton::Service::does_report_cell_and_wifi_ids()
{
    return *properties.does_report_cell_and_wifi_ids;
}

core::Property<bool>& location::dbus::skeleton::Service::is_online()
{
    return *properties.is_online;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::dbus::skeleton::Service::visible_space_vehicles()
{
    return *properties.visible_space_vehicles;
}

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

culs::Skeleton::DBusDaemonCredentialsResolver::DBusDaemonCredentialsResolver(const dbus::Bus::Ptr& bus)
    : daemon(bus)
{
}

culs::Credentials
culs::Skeleton::DBusDaemonCredentialsResolver::resolve_credentials_for_incoming_message(const dbus::Message::Ptr& msg)
{
    return culs::Credentials
    {
        static_cast<pid_t>(daemon.get_connection_unix_process_id(msg->sender())),
        static_cast<uid_t>(daemon.get_connection_unix_user(msg->sender()))
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
      object(access_service()->add_object_for_path(culs::Interface::path())),
      properties_changed(object->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>()),
      properties
      {
          object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>(),
          object->get_property<culs::Interface::Properties::DoesReportCellAndWifiIds>(),
          object->get_property<culs::Interface::Properties::IsOnline>(),
          object->get_property<culs::Interface::Properties::VisibleSpaceVehicles>()
      },
      connections
      {
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
    object->install_method_handler<culs::Interface::CreateSessionForCriteria>([this](const dbus::Message::Ptr& msg)
    {
        handle_create_session_for_criteria(msg);
    });
}

culs::Skeleton::~Skeleton() noexcept
{
    object->uninstall_method_handler<culs::Interface::CreateSessionForCriteria>();
}

void culs::Skeleton::handle_create_session_for_criteria(const dbus::Message::Ptr& in)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto sender = in->sender();
    auto reply = the_empty_reply();

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
                stub->object_for_path(path)
            }
        };

        culss::Interface::Ptr session
        {
            new culss::Skeleton{config}
        };

        if (not add_to_session_store_for_path(path, session))
        {
            reply = dbus::Message::make_error(
                        in,
                        culs::Interface::Errors::CreatingSession::name(),
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
        const culss::Interface::Ptr& session)
{
    std::lock_guard<std::mutex> lg(guard);
    bool inserted = false;
    std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(path, session));
    return inserted;
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

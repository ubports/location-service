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

#include <core/dbus/dbus.h>
#include <core/dbus/property.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/types/object_path.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culs::Skeleton::Private : public std::enable_shared_from_this<culs::Skeleton::Private>
{
    Private(Skeleton* parent, const dbus::Bus::Ptr& connection, const culs::PermissionManager::Ptr& permission_manager)
        : parent(parent),
          permission_manager(permission_manager),
          daemon(connection),
          object(parent->access_service()->add_object_for_path(culs::Interface::path())),
          does_satellite_based_positioning(object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>()),
          does_report_cell_and_wifi_ids(object->get_property<culs::Interface::Properties::DoesReportCellAndWifiIds>()),
          is_online(object->get_property<culs::Interface::Properties::IsOnline>()),
          visible_space_vehicles(object->get_property<culs::Interface::Properties::VisibleSpaceVehicles>())
    {
        object->install_method_handler<culs::Interface::CreateSessionForCriteria>([this](const dbus::Message::Ptr& msg)
        {
            handle_create_session_for_criteria(msg);
        });
    }

    ~Private() noexcept
    {
        object->uninstall_method_handler<culs::Interface::CreateSessionForCriteria>();
    }

    void handle_create_session_for_criteria(const dbus::Message::Ptr& msg);

    Skeleton* parent;
    PermissionManager::Ptr permission_manager;
    dbus::DBus daemon;
    dbus::Object::Ptr object;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesReportCellAndWifiIds>> does_report_cell_and_wifi_ids;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::IsOnline>> is_online;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::VisibleSpaceVehicles>> visible_space_vehicles;
    std::mutex guard;
    std::map<dbus::types::ObjectPath, std::shared_ptr<culss::Interface>> session_store;
};

culs::Skeleton::Skeleton(
        const dbus::Bus::Ptr& connection,
        const culs::PermissionManager::Ptr& permission_manager)
    : dbus::Skeleton<culs::Interface>(connection),
      d{new Private{this, connection, permission_manager}}
{
}

culs::Skeleton::~Skeleton() noexcept
{
}

void culs::Skeleton::Private::handle_create_session_for_criteria(const dbus::Message::Ptr& in)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto sender = in->sender();

    try
    {
        Criteria criteria;
        in->reader() >> criteria;

        Credentials credentials
        {
            static_cast<pid_t>(daemon.get_connection_unix_process_id(sender)),
            static_cast<uid_t>(daemon.get_connection_unix_user(sender))
        };

        if (PermissionManager::Result::rejected == permission_manager->check_permission_for_credentials(criteria, credentials))
            throw std::runtime_error("Client lacks permissions to access the service with the given criteria");

        // TODO(tvoss): Factor session path creation out into its own interface.
        std::stringstream ss; ss << "/sessions/" << credentials.pid;
        dbus::types::ObjectPath path{ss.str()};

        auto session = parent->create_session_for_criteria(criteria);

        auto service = dbus::Service::use_service(
                    parent->access_bus(),
                    in->sender());

        auto object = service->object_for_path(path);

        {
            std::lock_guard<std::mutex> lg(guard);

            auto skeleton = culss::Interface::Ptr{new culss::Skeleton(
                        session,
                        parent->access_bus(),
                        service,
                        object,
                        path)};

            bool inserted = false;
            std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(path, skeleton));

            if (!inserted)
                throw std::runtime_error("Could not insert duplicate session into store.");

            auto reply = dbus::Message::make_method_return(in);
            reply->writer() << path;
            parent->access_bus()->send(reply);
        }

    } catch(const std::runtime_error& e)
    {
        parent->access_bus()->send(
                    dbus::Message::make_error(
                        in,
                        culs::Interface::Errors::CreatingSession::name(),
                        e.what()));

        LOG(ERROR) << "Error creating session: " << e.what();
    }
}

core::Property<bool>& culs::Skeleton::does_satellite_based_positioning()
{
    return *d->does_satellite_based_positioning;
}

core::Property<bool>& culs::Skeleton::does_report_cell_and_wifi_ids()
{
    return *d->does_report_cell_and_wifi_ids;
}

core::Property<bool>& culs::Skeleton::is_online()
{
    return *d->is_online;
}

core::Property<std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>>& culs::Skeleton::visible_space_vehicles()
{
    return *d->visible_space_vehicles;
}

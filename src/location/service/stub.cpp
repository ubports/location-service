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
#include <location/service/stub.h>
#include <location/service/session/stub.h>

#include <location/logging.h>

#include <core/dbus/property.h>

namespace dbus = core::dbus;

struct location::service::Stub::Private
{
    Private(const dbus::Bus::Ptr& connection,
            const dbus::Object::Ptr& object)
        : bus(connection),
          object(object),
          state(object->get_property<location::service::Interface::Properties::State>()),
          does_satellite_based_positioning(object->get_property<location::service::Interface::Properties::DoesSatelliteBasedPositioning>()),
          does_report_cell_and_wifi_ids(object->get_property<location::service::Interface::Properties::DoesReportCellAndWifiIds>()),
          is_online(object->get_property<location::service::Interface::Properties::IsOnline>()),
          visible_space_vehicles(object->get_property<location::service::Interface::Properties::VisibleSpaceVehicles>())
    {
    }

    dbus::Bus::Ptr bus;
    dbus::Object::Ptr object;
    std::shared_ptr<dbus::Property<location::service::Interface::Properties::State>> state;
    std::shared_ptr<dbus::Property<location::service::Interface::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning;
    std::shared_ptr<dbus::Property<location::service::Interface::Properties::DoesReportCellAndWifiIds>> does_report_cell_and_wifi_ids;
    std::shared_ptr<dbus::Property<location::service::Interface::Properties::IsOnline>> is_online;
    std::shared_ptr<dbus::Property<location::service::Interface::Properties::VisibleSpaceVehicles>> visible_space_vehicles;
};

location::service::Stub::Stub(const dbus::Bus::Ptr& connection) : dbus::Stub<location::service::Interface>(connection),
    d(new Private{connection, access_service()->object_for_path(location::service::Interface::path())})
{
}

location::service::Stub::~Stub() noexcept
{
}

location::service::session::Interface::Ptr location::service::Stub::create_session_for_criteria(const location::Criteria& criteria)
{
    auto op = d->object->transact_method<
            location::service::Interface::CreateSessionForCriteria,
            location::service::Interface::CreateSessionForCriteria::ResultType
            >(criteria);

    if (op.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << op.error().print();
        throw std::runtime_error(ss.str());
    }

    return location::service::session::Interface::Ptr(new location::service::session::Stub{d->bus, op.value()});
}

const core::Property<location::service::State>& location::service::Stub::state() const
{
    return *d->state;
}

core::Property<bool>& location::service::Stub::does_satellite_based_positioning()
{
    return *d->does_satellite_based_positioning;
}

core::Property<bool>& location::service::Stub::does_report_cell_and_wifi_ids()
{
    return *d->does_report_cell_and_wifi_ids;
}

core::Property<bool>& location::service::Stub::is_online()
{
    return *d->is_online;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::service::Stub::visible_space_vehicles()
{
    return *d->visible_space_vehicles;
}

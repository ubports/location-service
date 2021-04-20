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
#include <com/ubuntu/location/service/stub.h>
#include <com/ubuntu/location/service/session/stub.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/property.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culs::Stub::Private
{
    Private(const dbus::Bus::Ptr& connection,
            const dbus::Object::Ptr& object)
        : bus(connection),
          object(object),
          state(object->get_property<culs::Interface::Properties::State>()),
          does_satellite_based_positioning(object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>()),
          does_report_cell_and_wifi_ids(object->get_property<culs::Interface::Properties::DoesReportCellAndWifiIds>()),
          is_online(object->get_property<culs::Interface::Properties::IsOnline>()),
          visible_space_vehicles(object->get_property<culs::Interface::Properties::VisibleSpaceVehicles>()),
          client_applications(object->get_property<culs::Interface::Properties::ClientApplications>())
    {
    }

    dbus::Bus::Ptr bus;
    dbus::Object::Ptr object;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::State>> state;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesReportCellAndWifiIds>> does_report_cell_and_wifi_ids;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::IsOnline>> is_online;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::VisibleSpaceVehicles>> visible_space_vehicles;
    std::shared_ptr<dbus::Property<culs::Interface::Properties::ClientApplications>> client_applications;
};

culs::Stub::Stub(const dbus::Bus::Ptr& connection) : dbus::Stub<culs::Interface>(connection),
    d(new Private{connection, access_service()->object_for_path(culs::Interface::path())})
{
}

culs::Stub::~Stub() noexcept
{
}

culss::Interface::Ptr culs::Stub::create_session_for_criteria(const cul::Criteria& criteria)
{
    auto op = d->object->transact_method<
            culs::Interface::CreateSessionForCriteria,
            culs::Interface::CreateSessionForCriteria::ResultType
            >(criteria);

    if (op.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << op.error().print();
        throw std::runtime_error(ss.str());
    }

    return culss::Interface::Ptr(new culss::Stub{d->bus, op.value()});
}

const core::Property<culs::State>& culs::Stub::state() const
{
    return *d->state;
}

core::Property<bool>& culs::Stub::does_satellite_based_positioning()
{
    return *d->does_satellite_based_positioning;
}

core::Property<bool>& culs::Stub::does_report_cell_and_wifi_ids()
{
    return *d->does_report_cell_and_wifi_ids;
}

core::Property<bool>& culs::Stub::is_online()
{
    return *d->is_online;
}

core::Property<std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>>& culs::Stub::visible_space_vehicles()
{
    return *d->visible_space_vehicles;
}

core::Property<std::vector<std::string>>& culs::Stub::client_applications()
{
    return *d->client_applications;
}

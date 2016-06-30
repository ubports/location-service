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
#ifndef LOCATION_DBUS_STUB_SERVICE_H_
#define LOCATION_DBUS_STUB_SERVICE_H_

#include <location/service.h>

#include <location/dbus/service.h>

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>

namespace location
{
namespace dbus
{
namespace stub
{
class Service : public location::Service
{
  public:
    Service(const core::dbus::Bus::Ptr& connection,
            const core::dbus::Service::Ptr& service,
            const core::dbus::Object::Ptr& object);

    Session::Ptr create_session_for_criteria(const Criteria& criteria) override;
    const core::Property<State>& state() const override;
    core::Property<bool>& does_satellite_based_positioning() override;
    core::Property<bool>& does_report_cell_and_wifi_ids() override;
    core::Property<bool>& is_online() override;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() override;

  private:
    core::dbus::Bus::Ptr connection;
    core::dbus::Service::Ptr service;
    core::dbus::Object::Ptr object;

    std::shared_ptr<core::dbus::Property<location::dbus::Service::Properties::State>> state_;
    std::shared_ptr<core::dbus::Property<location::dbus::Service::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning_;
    std::shared_ptr<core::dbus::Property<location::dbus::Service::Properties::DoesReportCellAndWifiIds>> does_report_cell_and_wifi_ids_;
    std::shared_ptr<core::dbus::Property<location::dbus::Service::Properties::IsOnline>> is_online_;
    std::shared_ptr<core::dbus::Property<location::dbus::Service::Properties::VisibleSpaceVehicles>> visible_space_vehicles_;
};
}
}
}

#endif // LOCATION_DBUS_STUB_SERVICE_H_

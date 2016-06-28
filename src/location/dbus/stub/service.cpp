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

#include <location/dbus/stub/service.h>
#include <location/dbus/stub/session.h>

#include <location/dbus/codec.h>
#include <location/dbus/service.h>

#include <location/logging.h>

#include <core/dbus/property.h>

location::dbus::stub::Service::Service(const core::dbus::Bus::Ptr& connection,
                                       const core::dbus::Service::Ptr& service,
                                       const core::dbus::Object::Ptr& object) :
    connection{connection},
    service{service},
    object{object},
    state_{object->get_property<location::dbus::Service::Properties::State>()},
    does_satellite_based_positioning_{object->get_property<location::dbus::Service::Properties::DoesSatelliteBasedPositioning>()},
    does_report_cell_and_wifi_ids_{object->get_property<location::dbus::Service::Properties::DoesReportCellAndWifiIds>()},
    is_online_{object->get_property<location::dbus::Service::Properties::IsOnline>()},
    visible_space_vehicles_{object->get_property<location::dbus::Service::Properties::VisibleSpaceVehicles>()}
{
}

location::Service::Session::Ptr location::dbus::stub::Service::create_session_for_criteria(const location::Criteria& criteria)
{
    auto op = object->transact_method<
            location::dbus::Service::CreateSessionForCriteria,
            location::dbus::Service::CreateSessionForCriteria::ResultType
            >(criteria);

    if (op.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << op.error().print();
        throw std::runtime_error(ss.str());
    }

    return std::make_shared<location::dbus::stub::Session>(connection, service->add_object_for_path(op.value()));
}

const core::Property<location::Service::State>& location::dbus::stub::Service::state() const
{
    return *state_;
}

core::Property<bool>& location::dbus::stub::Service::does_satellite_based_positioning()
{
    return *does_satellite_based_positioning_;
}

core::Property<bool>& location::dbus::stub::Service::does_report_cell_and_wifi_ids()
{
    return *does_report_cell_and_wifi_ids_;
}

core::Property<bool>& location::dbus::stub::Service::is_online()
{
    return *is_online_;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::dbus::stub::Service::visible_space_vehicles()
{
    return *visible_space_vehicles_;
}

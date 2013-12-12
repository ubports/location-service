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

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

namespace
{
template<typename PropertyType>
struct MappedProperty : public core::Property<typename PropertyType::ValueType>
{
    typedef core::Property<typename PropertyType::ValueType> Super;

    MappedProperty(const std::shared_ptr<dbus::Property<PropertyType>>& dbus_property)
        : dbus_property(dbus_property)
    {

    }

    void set(const typename PropertyType::ValueType& value)
    {
        dbus_property->value(value);
        Super::set(value);
    }

    const typename PropertyType::ValueType& get() const
    {
        return Super::mutable_get() = dbus_property->value();
    }

    std::shared_ptr<dbus::Property<PropertyType>> dbus_property;
};
}

struct culs::Stub::Private
{
    Private(const dbus::Bus::Ptr& connection,
            const dbus::Object::Ptr& object)
        : bus(connection),
          remote
          {
              object,
              object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>(),
              object->get_property<culs::Interface::Properties::DoesReportCellAndWifiIds>(),
              object->get_property<culs::Interface::Properties::IsOnline>(),
              object->get_property<culs::Interface::Properties::VisibleSpaceVehicles>()
          },
          does_satellite_based_positioning(remote.does_satellite_based_positioning),
          does_report_cell_and_wifi_ids(remote.does_report_cell_and_wifi_ids),
          is_online(remote.is_online),
          visible_space_vehicles(remote.visible_space_vehicles)
    {
        does_satellite_based_positioning.changed().connect(
                    [this](bool value)
                    {
                       remote.does_satellite_based_positioning->value(value);
                    });
        does_report_cell_and_wifi_ids.changed().connect(
                    [this](bool value)
                    {
                        remote.does_report_cell_and_wifi_ids->value(value);
                    });
        is_online.changed().connect(
                    [this](bool value)
                    {
                        remote.is_online->value(value);
                    });
    }

    dbus::Bus::Ptr bus;
    struct
    {
        dbus::Object::Ptr object;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesReportCellAndWifiIds>> does_report_cell_and_wifi_ids;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::IsOnline>> is_online;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::VisibleSpaceVehicles>> visible_space_vehicles;
    } remote;
    MappedProperty<culs::Interface::Properties::DoesSatelliteBasedPositioning> does_satellite_based_positioning;
    MappedProperty<culs::Interface::Properties::DoesReportCellAndWifiIds> does_report_cell_and_wifi_ids;
    MappedProperty<culs::Interface::Properties::IsOnline> is_online;
    MappedProperty<culs::Interface::Properties::VisibleSpaceVehicles> visible_space_vehicles;
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
    auto op = d->remote.object->invoke_method_synchronously<
            culs::Interface::CreateSessionForCriteria,
            culs::Interface::CreateSessionForCriteria::ResultType
            >(criteria);

    if (op.is_error())
        throw std::runtime_error(op.error());

    return culss::Interface::Ptr(new culss::Stub{d->bus, op.value()});
}

core::Property<bool>& culs::Stub::does_satellite_based_positioning()
{
    return d->does_satellite_based_positioning;
}

core::Property<bool>& culs::Stub::does_report_cell_and_wifi_ids()
{
    return d->does_report_cell_and_wifi_ids;
}

core::Property<bool>& culs::Stub::is_online()
{
    return d->is_online;
}

core::Property<std::set<cul::SpaceVehicle>>& culs::Stub::visible_space_vehicles()
{
    return d->visible_space_vehicles;
}

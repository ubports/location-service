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
#include "com/ubuntu/location/service/stub.h"
#include "com/ubuntu/location/service/session/stub.h"

#include "com/ubuntu/location/logging.h"

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

struct culs::Stub::Private
{
    Private(const dbus::Bus::Ptr& connection,
            const dbus::Object::Ptr& object)
        : bus(connection),
          remote
          {
              object,
              object->get_property<culs::Interface::Properties::DoesSatelliteBasedPositioning>(),
              object->get_property<culs::Interface::Properties::IsOnline>()
          }
    {
        void (dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>::*vs1)(const bool&)
                = &dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>::value;

        does_satellite_based_positioning.changed().connect(
                std::bind(
                    vs1,
                    remote.does_satellite_based_positioning,
                    std::placeholders::_1));

        void (dbus::Property<culs::Interface::Properties::IsOnline>::*vs2)(const bool&)
                = &dbus::Property<culs::Interface::Properties::IsOnline>::value;

        is_online.changed().connect(
                std::bind(
                    vs2,
                    remote.is_online,
                    std::placeholders::_1));

    }

    dbus::Bus::Ptr bus;
    com::ubuntu::Property<bool> does_satellite_based_positioning;
    com::ubuntu::Property<bool> is_online;
    struct
    {
        dbus::Object::Ptr object;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::DoesSatelliteBasedPositioning>> does_satellite_based_positioning;
        std::shared_ptr<dbus::Property<culs::Interface::Properties::IsOnline>> is_online;
    } remote;
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

com::ubuntu::Property<bool>& culs::Stub::does_satellite_based_positioning()
{
    return d->does_satellite_based_positioning;
}

com::ubuntu::Property<bool>& culs::Stub::is_online()
{
    return d->is_online;
}

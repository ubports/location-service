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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_

#include <com/ubuntu/location/service/interface.h>
#include <com/ubuntu/location/service/session/interface.h>

#include <core/dbus/stub.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Stub : public core::dbus::Stub<Interface>
{
  public:
    Stub(const dbus::Bus::Ptr& connection);
    Stub(const Stub&) = delete;
    Stub& operator=(const Stub&) = delete;
    ~Stub() noexcept;

    session::Interface::Ptr create_session_for_criteria(const Criteria& criteria);

    const core::Property<State>& state() const;
    core::Property<bool>& does_satellite_based_positioning();
    core::Property<bool>& does_report_cell_and_wifi_ids();
    core::Property<bool>& is_online();
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles();
    core::Property<std::vector<std::string>>& client_applications();

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_

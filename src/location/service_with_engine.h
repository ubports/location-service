/*
 * Copyright © 2016 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_WITH_ENGINE_H_
#define LOCATION_SERVICE_WITH_ENGINE_H_

#include <location/engine.h>
#include <location/service.h>

namespace location
{
class ServiceWithEngine : public Service
{
public:
    ServiceWithEngine(const Engine::Ptr& engine);

    // From Service
    const core::Property<State>& state() const override;
    core::Property<bool>& does_satellite_based_positioning() override;
    core::Property<bool>& is_online() override;
    core::Property<bool>& does_report_cell_and_wifi_ids() override;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() override;
    Session::Ptr create_session_for_criteria(const Criteria& criteria) override;

private:
    Engine::Ptr engine; // The engine instance we use for positioning.

    // Properties of the service implementation.
    core::Property<State> state_;
    core::Property<bool> does_satellite_based_positioning_;
    core::Property<bool> is_online_;
    core::Property<bool> does_report_cell_and_wifi_ids_;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>> visible_space_vehicles_;

    struct
    {
        core::ScopedConnection is_online;
        core::ScopedConnection does_report_cell_and_wifi_ids;
        core::ScopedConnection does_satellite_based_positioning;
        core::ScopedConnection engine_state;
        core::ScopedConnection satellite_based_positioning_state;
        core::ScopedConnection visible_space_vehicles;
    } connections;      // All event connections are automatically cut on destruction.
};
}

#endif // LOCATION_SERVICE_WITH_ENGINE_H_

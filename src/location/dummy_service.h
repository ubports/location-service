/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
 *
 */

#ifndef LOCATION_DUMMY_SERVICE_H_
#define LOCATION_DUMMY_SERVICE_H_

#include <location/service.h>

namespace location
{
class DummyService : public Service
{
public:
    class Session : public Service::Session
    {
    public:
        Updates& updates() override;

    private:
        Updates updates_;
    };

    const core::Property<State>& state() const override;
    core::Property<bool>& does_satellite_based_positioning() override;
    core::Property<bool>& is_online() override;
    core::Property<bool>& does_report_cell_and_wifi_ids() override;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() override;
    Session::Ptr create_session_for_criteria(const Criteria& criteria) override;

private:
    core::Property<State> state_;
    core::Property<bool> does_satellite_based_positioning_;
    core::Property<bool> is_online_;
    core::Property<bool> does_report_cell_and_wifi_ids_;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>> visible_space_vehicles_;
};
}

#endif // LOCATION_DUMMY_SERVICE_H_

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

#include <location/dummy_service.h>

location::Service::Session::Updates& location::DummyService::Session::updates()
{
    return updates_;
}

const core::Property<location::Service::State>& location::DummyService::state() const
{
    return state_;
}

core::Property<bool>& location::DummyService::does_satellite_based_positioning()
{
    return does_satellite_based_positioning_;
}

core::Property<bool>& location::DummyService::is_online()
{
    return is_online_;
}

core::Property<bool>& location::DummyService::does_report_cell_and_wifi_ids()
{
    return does_report_cell_and_wifi_ids_;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::DummyService::visible_space_vehicles()
{
    return visible_space_vehicles_;
}

location::Service::Session::Ptr location::DummyService::create_session_for_criteria(const Criteria&)
{
    return std::make_shared<DummyService::Session>();
}

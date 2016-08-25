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

#include <location/events/wifi_and_cell_id_reporting_state_changed.h>

#include <gtest/gtest.h>

TEST(WifiAndCellIdReportingStateChanged, ctor_and_dtor_work)
{
    location::events::WifiAndCellIdReportingStateChanged event{location::WifiAndCellIdReportingState::on};
}

TEST(WifiAndCellIdReportingStateChanged, ctor_initiales_state_value)
{
    location::events::WifiAndCellIdReportingStateChanged event{location::WifiAndCellIdReportingState::off};
    EXPECT_EQ(location::WifiAndCellIdReportingState::off, event.new_state());
}

TEST(WifiAndCellIdReportingStateChanged, returns_correct_type_from_instance)
{
    location::events::WifiAndCellIdReportingStateChanged event{location::WifiAndCellIdReportingState::on};
    EXPECT_EQ(location::TypeOf<location::events::WifiAndCellIdReportingStateChanged>::query(), event.type());
}

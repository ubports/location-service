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

#include <com/ubuntu/location/connectivity/manager.h>

#include <gtest/gtest.h>

namespace location = com::ubuntu::location;

TEST(ConnectivityManager, default_implementation_is_queryable_for_wifis_and_radio_cells_requires_hardware)
{
    try
    {
        auto manager = location::connectivity::platform_default_manager();

        for (const auto& cell : manager->visible_radio_cells().get())
            std::cout << cell << std::endl;

        EXPECT_NO_THROW(
        {
            for (const auto& wifi: manager->visible_wireless_networks().get())
                std::cout << wifi << std::endl;
        });
    } catch(...)
    {
    }
}

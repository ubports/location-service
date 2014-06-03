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

TEST(RadioCell, default_construction_yields_a_gsm_cell)
{
    location::connectivity::RadioCell cell;

    EXPECT_EQ(location::connectivity::RadioCell::Type::gsm, cell.type());
}

TEST(RadioCell, explicit_construction_yields_correct_type)
{
    {
        location::connectivity::RadioCell::Gsm gsm
        {
            location::connectivity::RadioCell::Gsm::MCC{42},
            location::connectivity::RadioCell::Gsm::MNC{42},
            location::connectivity::RadioCell::Gsm::LAC{42},
            location::connectivity::RadioCell::Gsm::ID{42},
            location::connectivity::RadioCell::Gsm::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{gsm};

        EXPECT_EQ(location::connectivity::RadioCell::Type::gsm, cell.type());
    }

    {
        location::connectivity::RadioCell::Umts umts
        {
            location::connectivity::RadioCell::Umts::MCC{42},
            location::connectivity::RadioCell::Umts::MNC{42},
            location::connectivity::RadioCell::Umts::LAC{42},
            location::connectivity::RadioCell::Umts::ID{42},
            location::connectivity::RadioCell::Umts::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{umts};

        EXPECT_EQ(location::connectivity::RadioCell::Type::umts, cell.type());
    }

    {
        location::connectivity::RadioCell::Lte lte
        {
            location::connectivity::RadioCell::Lte::MCC{42},
            location::connectivity::RadioCell::Lte::MNC{42},
            location::connectivity::RadioCell::Lte::TAC{42},
            location::connectivity::RadioCell::Lte::ID{42},
            location::connectivity::RadioCell::Lte::PID{42},
            location::connectivity::RadioCell::Lte::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{lte};

        EXPECT_EQ(location::connectivity::RadioCell::Type::lte, cell.type());
    }
}

TEST(ConnectivityManager, repeatedly_requesting_network_scans_works)
{
    auto manager = location::connectivity::platform_default_manager();

    for (unsigned int i = 0; i < 100; i++)
        manager->request_scan_for_wireless_networks();
}

TEST(ConnectivityManager, repeatedly_querying_the_connected_cell_works)
{
    auto manager = location::connectivity::platform_default_manager();

    for (unsigned int i = 0; i < 100; i++)
    {
        auto cells = manager->connected_radio_cells().get();

        for (const auto& cell : cells)
            std::cout << cell << std::endl;
    }
}

TEST(ConnectivityManager, default_implementation_is_queryable_for_wifis_and_radio_cells_requires_hardware)
{
    auto manager = location::connectivity::platform_default_manager();

    for (const auto& cell : manager->connected_radio_cells().get())
        std::cout << cell << std::endl;

    manager->enumerate_visible_wireless_networks([](location::connectivity::WirelessNetwork::Ptr wifi)
    {
        std::cout << *wifi << std::endl;
    });
}

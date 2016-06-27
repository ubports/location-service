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

#include <location/service/harvester.h>

#include "mock_connectivity_manager.h"
#include "mock_reporter.h"
#include "null_provider_selection_policy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
location::Update<location::Position> reference_position_update
{
    {
        location::wgs84::Latitude{9. * location::units::Degrees},
        location::wgs84::Longitude{53. * location::units::Degrees},
        location::wgs84::Altitude{-2. * location::units::Meters}
    },
    location::Clock::now()
};
}

TEST(Harvester, calls_start_and_stop_on_reporter)
{
    using namespace ::testing;

    auto reporter = std::make_shared<MockReporter>();

    EXPECT_CALL(*reporter, start()).Times(1);
    EXPECT_CALL(*reporter, stop()).Times(1);

    location::service::Harvester::Configuration config
    {
        com::ubuntu::location::connectivity::platform_default_manager(),
        reporter
    };

    location::service::Harvester harvester(config);
    harvester.start();
    harvester.stop();
}

TEST(Harvester, invokes_reporter_on_location_update_only_if_started)
{
    using namespace ::testing;

    auto reporter = std::make_shared<NiceMock<MockReporter>>();

    EXPECT_CALL(*reporter, report(reference_position_update,_,_)).Times(1);

    location::service::Harvester::Configuration config
    {
        com::ubuntu::location::connectivity::platform_default_manager(),
        reporter
    };

    location::service::Harvester harvester(config);

    harvester.start();
    harvester.report_position_update(reference_position_update);

    harvester.stop();
    harvester.report_position_update(reference_position_update);
}

TEST(Harvester, queries_wifis_and_cells_on_location_update)
{
    using namespace ::testing;
    using namespace location;

    auto conn_man = std::make_shared<MockConnectivityManager>();

    EXPECT_CALL(*conn_man, enumerate_connected_radio_cells(_)).Times(1);
    EXPECT_CALL(*conn_man, enumerate_visible_wireless_networks(_)).Times(1);

    service::Harvester::Configuration config
    {
        conn_man,
        std::make_shared<NiceMock<MockReporter>>()
    };

    service::Harvester harvester(config);

    harvester.start();
    harvester.report_position_update(reference_position_update);
}

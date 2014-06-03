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

#include <com/ubuntu/location/service/harvester.h>

#include "mock_connectivity_manager.h"
#include "null_provider_selection_policy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace location = com::ubuntu::location;

namespace
{
struct MockReporter : public location::service::Harvester::Reporter
{
    MockReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    MOCK_METHOD0(start, void());

    /** @brief Tell the reporter to shut down its operation. */
    MOCK_METHOD0(stop, void());

    /**
     * @brief Triggers the reporter to send off the information.
     */
    MOCK_METHOD3(report,
                 void(
                     const location::Update<location::Position>&,
                     const std::vector<location::connectivity::WirelessNetwork::Ptr>&,
                     const std::vector<location::connectivity::RadioCell>&));
};

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
        std::make_shared<location::Engine>(std::make_shared<NullProviderSelectionPolicy>()),
        location::connectivity::platform_default_manager(),
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
        std::make_shared<location::Engine>(std::make_shared<NullProviderSelectionPolicy>()),
        location::connectivity::platform_default_manager(),
        reporter
    };

    location::service::Harvester harvester(config);

    harvester.start();
    config.engine->updates.reference_location = reference_position_update;

    harvester.stop();
    config.engine->updates.reference_location = reference_position_update;
}

TEST(Harvester, queries_wifis_and_cells_on_location_update)
{
    using namespace ::testing;
    using namespace location;

    core::Property<std::vector<connectivity::WirelessNetwork::Ptr> > wifis;
    core::Property<std::vector<connectivity::RadioCell> > cells;

    auto conn_man = std::make_shared<MockConnectivityManager>();

    ON_CALL(*conn_man, connected_radio_cells()).WillByDefault(ReturnRef(cells));

    EXPECT_CALL(*conn_man, enumerate_visible_wireless_networks(_)).Times(1);
    EXPECT_CALL(*conn_man, connected_radio_cells()).Times(1);

    service::Harvester::Configuration config
    {
        std::make_shared<Engine>(std::make_shared<NullProviderSelectionPolicy>()),
        conn_man,
        std::make_shared<NiceMock<MockReporter>>()
    };

    service::Harvester harvester(config);

    harvester.start();
    config.engine->updates.reference_location = reference_position_update;
}

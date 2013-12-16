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

#include <com/ubuntu/location/service/daemon.h>

#include "cross_process_sync.h"
#include "fork_and_run.h"

#include <gtest/gtest.h>

#include <ctime>

#include <thread>

namespace location = com::ubuntu::location;

namespace
{
auto testing_daemon = []()
{
    char const* argv[] =
    {
        "Daemon",
        "--bus=session",
        "--testing",
        nullptr
    };

    auto result = location::service::Daemon::main(3, argv);

    EXPECT_EQ(EXIT_SUCCESS, result);
};

std::function<void()> querying_cli_for_property(const std::string& property)
{
    return [property]()
    {
        // We need to wait some time to make sure that the service is up and running
        timespec ts = { 0, 500 * 1000 * 1000 };
        ::nanosleep(&ts, nullptr);

        char const* argv[] =
        {
            "Cli",
            "--bus=session",
            nullptr,
            nullptr
        };

        argv[2] = property.c_str();

        auto result = location::service::Daemon::Cli::main(3, argv);

        EXPECT_EQ(EXIT_SUCCESS, result);
    };
}
}

TEST(DaemonAndCli, QueryingIsOnlinePropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                test::fork_and_run(
                    testing_daemon,
                    querying_cli_for_property("is_online")));
}

TEST(DaemonAndCli, QueryingDoesSatelliteBasedPositionPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                test::fork_and_run(
                    testing_daemon,
                    querying_cli_for_property("does_satellite_based_positioning")));
}

TEST(DaemonAndCli, QueryingDoesReportWifiAndCellIdsPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                test::fork_and_run(
                    testing_daemon,
                    querying_cli_for_property("does_report_wifi_and_cell_ids")));
}

TEST(DaemonAndCli, QueryingVisibleSpaceVehiclesPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                test::fork_and_run(
                    testing_daemon,
                    querying_cli_for_property("visible_space_vehicles")));
}

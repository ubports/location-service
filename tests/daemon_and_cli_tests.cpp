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

#include <com/ubuntu/location/space_vehicle.h>

#include <core/dbus/dbus.h>
#include <core/dbus/message.h>

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

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

    return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
};

std::function<core::posix::exit::Status ()> querying_cli_for_property(const std::string& property)
{
    return [property]()
    {
        // We need to wait some time to make sure that the service is up and running
        timespec ts = { 1, 0 };
        ::nanosleep(&ts, nullptr);

        char const* argv[] =
        {
            "Cli",
            "--bus=session",
            nullptr,
            "--get",
            nullptr
        };

        auto arg = std::string("--property=") + property;
        argv[2] = arg.c_str();

        auto result = location::service::Daemon::Cli::main(4, argv);

        EXPECT_EQ(EXIT_SUCCESS, result);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}

template<typename T>
std::function<core::posix::exit::Status ()> adjusting_cli_for_property(const std::string& property, const T& value)
{
    return [property, value]()
    {
        // We need to wait some time to make sure that the service is up and running
        timespec ts = { 1, 0 };
        ::nanosleep(&ts, nullptr);

        char const* argv[] =
        {
            "Cli",
            "--bus=session",
            nullptr,
            nullptr,
            nullptr
        };

        auto property_arg = std::string("--property=") + property;
        argv[2] = property_arg.c_str();

        std::stringstream ss; ss << "--set=" << std::boolalpha << value;
        auto set_arg = ss.str();
        argv[3] = set_arg.c_str();

        auto result = location::service::Daemon::Cli::main(4, argv);

        EXPECT_EQ(EXIT_SUCCESS, result);
        EXPECT_EQ(core::posix::exit::Status::success, querying_cli_for_property(property)());


        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}
}

TEST(DaemonAndCli, QueryingIsOnlinePropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon,
                  querying_cli_for_property("is_online")));
}

TEST(DaemonAndCli, AdjustingIsOnlinePropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon,
                  adjusting_cli_for_property("is_online", false)));
}

TEST(DaemonAndCli, QueryingDoesSatelliteBasedPositionPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon,
                  querying_cli_for_property("does_satellite_based_positioning")));
}

TEST(DaemonAndCli, AdjustingDoesSatelliteBasedPositionPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon,
                  adjusting_cli_for_property("does_satellite_based_positioning", false)));
}

TEST(DaemonAndCli, QueryingDoesReportWifiAndCellIdsPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                core::testing::fork_and_run(
                    testing_daemon,
                    querying_cli_for_property("does_report_wifi_and_cell_ids")));
}

TEST(DaemonAndCli, AdjustingDoesReportWifiAndCellIdsPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                core::testing::fork_and_run(
                    testing_daemon,
                    adjusting_cli_for_property("does_report_wifi_and_cell_ids", true)));
}

TEST(DaemonAndCli, QueryingVisibleSpaceVehiclesPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon,
                  querying_cli_for_property("visible_space_vehicles")));
}

TEST(DaemonAndCli, AdjustingVisibleSpaceVehiclesPropertyDoesNotWork)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::client_failed,
              core::testing::fork_and_run(
                  testing_daemon,
                  adjusting_cli_for_property("visible_space_vehicles", 0)));
}

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

#include <location/service/daemon.h>
#include <location/service/default_configuration.h>

#include <location/boost_ptree_settings.h>
#include <location/space_vehicle.h>

#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/message.h>

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_engine.h"

#include <ctime>

#include <thread>

namespace
{
struct DaemonAndCli : public core::dbus::testing::Fixture
{
};

std::function<core::posix::exit::Status()> testing_daemon(DaemonAndCli& fixture)
{
    return [&fixture]()
    {
        static constexpr const char* fn{"/tmp/settings.ini"};

        // Clean up prior to running the test case.
        std::remove(fn);

        location::service::Daemon::Configuration config;

        config.incoming = fixture.session_bus();
        config.outgoing = fixture.session_bus();
        config.is_testing_enabled = true;
        config.settings = std::make_shared<location::BoostPtreeSettings>(fn);

        auto result = location::service::Daemon::main(config);

        EXPECT_EQ(EXIT_SUCCESS, result);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}

std::function<core::posix::exit::Status ()> querying_cli_for_property(
        location::service::Daemon::Cli::Property property,
        DaemonAndCli& fixture)
{
    return [property, &fixture]()
    {
        // We need to wait some time to make sure that the service is up and running
        std::this_thread::sleep_for(std::chrono::seconds{1});

        location::service::Daemon::Cli::Configuration config;

        config.bus = fixture.session_bus();
        config.command = location::service::Daemon::Cli::Command::get;
        config.property = property;

        auto result = location::service::Daemon::Cli::main(config);

        EXPECT_EQ(EXIT_SUCCESS, result);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}

template<typename T>
std::function<core::posix::exit::Status ()> adjusting_cli_for_property(
        location::service::Daemon::Cli::Property property,
        const T& value,
        DaemonAndCli& fixture)
{
    return [property, value, &fixture]()
    {
        // We need to wait some time to make sure that the service is up and running
        std::this_thread::sleep_for(std::chrono::seconds{1});

        std::stringstream ss; ss << value;

        location::service::Daemon::Cli::Configuration config;

        config.bus = fixture.session_bus();
        config.command = location::service::Daemon::Cli::Command::set;
        config.property = property;
        config.new_value = ss.str();

        auto result = location::service::Daemon::Cli::main(config);

        EXPECT_EQ(EXIT_SUCCESS, result);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}
}

TEST_F(DaemonAndCli, QueryingStatePropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  querying_cli_for_property(
                      location::service::Daemon::Cli::Property::state,
                      *this)));
}

TEST_F(DaemonAndCli, QueryingIsOnlinePropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  querying_cli_for_property(
                      location::service::Daemon::Cli::Property::is_online,
                      *this)));
}

TEST_F(DaemonAndCli, AdjustingIsOnlinePropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  adjusting_cli_for_property(
                      location::service::Daemon::Cli::Property::is_online, false, *this)));
}

TEST_F(DaemonAndCli, QueryingDoesSatelliteBasedPositionPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  querying_cli_for_property(
                      location::service::Daemon::Cli::Property::does_satellite_based_positioning,
                      *this)));
}

TEST_F(DaemonAndCli, AdjustingDoesSatelliteBasedPositionPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  adjusting_cli_for_property(
                      location::service::Daemon::Cli::Property::does_satellite_based_positioning,
                      false,
                      *this)));
}

TEST_F(DaemonAndCli, QueryingDoesReportWifiAndCellIdsPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                core::testing::fork_and_run(
                    testing_daemon(*this),
                    querying_cli_for_property(
                        location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids,
                        *this)));
}

TEST_F(DaemonAndCli, AdjustingDoesReportWifiAndCellIdsPropertyWorks)
{
    EXPECT_NO_FATAL_FAILURE(
                core::testing::fork_and_run(
                    testing_daemon(*this),
                    adjusting_cli_for_property(
                        location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids,
                        true,
                        *this)));
}

TEST_F(DaemonAndCli, QueryingVisibleSpaceVehiclesPropertyWorks)
{
    EXPECT_EQ(core::testing::ForkAndRunResult::empty,
              core::testing::fork_and_run(
                  testing_daemon(*this),
                  querying_cli_for_property(
                      location::service::Daemon::Cli::Property::visible_space_vehicles,
                      *this)));
}

namespace
{
auto null_dbus_connection_factory = [](core::dbus::WellKnownBus)
{
    return core::dbus::Bus::Ptr{};
};
}
TEST(DaemonCli, CommandLineArgsParsingWorksForCorrectArguments)
{
    const char* args[] =
    {
        "--bus", "session",
        "--get",
        "--property", "is_online"
    };

    auto config = location::service::Daemon::Cli::Configuration::from_command_line_args(5, args, null_dbus_connection_factory);

    EXPECT_EQ(location::service::Daemon::Cli::Command::get, config.command);
    EXPECT_EQ(location::service::Daemon::Cli::Property::is_online, config.property);
}

TEST(DaemonCli, CommandLineArgsParsingThrowsForInvalidArguments)
{
    const char* args[] =
    {
        "--bus", "session",
        "--get", "--set", // Both get and set specificed, expected to throw
        "--property", "is_online"
    };

    EXPECT_ANY_THROW(location::service::Daemon::Cli::Configuration::from_command_line_args(5, args, null_dbus_connection_factory));
}

TEST(Daemon, CommandLineParsingThrowsForEmptyProviders)
{
    const char* args[] =
    {
        "--bus", "session"
    };

    EXPECT_ANY_THROW(location::service::Daemon::Configuration::from_command_line_args(2, args, null_dbus_connection_factory));
}

TEST(Daemon, CommandLineParsingDoesNotThrowForEmptyProvidersInTesting)
{
    const char* args[] =
    {
        "--bus", "session",
        "--testing"
    };

    EXPECT_ANY_THROW(location::service::Daemon::Configuration::from_command_line_args(2, args, null_dbus_connection_factory));
}

TEST(Daemon, CommandLineParsingWorksForProvidersAndProviderOptions)
{
    const char* args[] =
    {
        "--bus", "session",
        "--provider", "does::not::exist::Provider",
        "--does::not::exist::Provider::option1=test1",
        "--does::not::exist::Provider::option2=test2",
        "--does::not::exist::Provider::option3=test3"
    };

    auto config = location::service::Daemon::Configuration::from_command_line_args(7, args, null_dbus_connection_factory);

    EXPECT_EQ(1u, config.providers.size());
    EXPECT_EQ("does::not::exist::Provider", config.providers[0]);
    EXPECT_EQ("test1", config.provider_options.at(config.providers[0]).get<std::string>("option1"));
    EXPECT_EQ("test2", config.provider_options.at(config.providers[0]).get<std::string>("option2"));
    EXPECT_EQ("test3", config.provider_options.at(config.providers[0]).get<std::string>("option3"));
}

TEST(Daemon, ProviderLoadingWorks)
{
    const char* args[] =
    {
        "--bus", "session",
        "--provider", "dummy::Provider",
    };

    auto config = location::service::Daemon::Configuration::from_command_line_args(4, args, null_dbus_connection_factory);
    location::service::DefaultConfiguration dc;
    auto engine = std::make_shared<MockEngine>(dc.the_provider_selection_policy(), config.settings);

    EXPECT_CALL(*engine, add_provider(::testing::_));

    location::service::Daemon::load_providers(config, engine);
}

TEST(Daemon, MultipleProviderLoadingWorks)
{
    const char* args[] =
    {
        "--bus", "session",
        "--provider", "dummy::Provider",
        "--provider", "dummy::DelayedProvider",
        "--dummy::DelayedProvider::DelayInMs=250"
    };

    auto config = location::service::Daemon::Configuration::from_command_line_args(7, args, null_dbus_connection_factory);
    location::service::DefaultConfiguration dc;
    auto engine = std::make_shared<MockEngine>(dc.the_provider_selection_policy(), config.settings);

    EXPECT_CALL(*engine, add_provider(::testing::_)).Times(2);

    location::service::Daemon::load_providers(config, engine);
}

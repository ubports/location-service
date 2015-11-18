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
 * Authored by: Manuel de la Peña <manuel.delapena@canonical.com>
 */

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/provider.h>

#include <com/ubuntu/location/providers/remote/interface.h>
#include <com/ubuntu/location/providers/remote/skeleton.h>
#include <com/ubuntu/location/providers/remote/stub.h>

#include <com/ubuntu/location/providers/remote/provider.h>

#include "mock_event_consumer.h"
#include "mock_provider.h"

#include <core/dbus/fixture.h>
#include <core/dbus/service.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>
#include <core/posix/signal.h>

#include <core/testing/fork_and_run.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <condition_variable>

namespace cul = com::ubuntu::location;
namespace dbus = core::dbus;
namespace remote = com::ubuntu::location::providers::remote;

using namespace ::testing;


MATCHER_P(PositionUpdatesAreEqualExceptForTiming, value, "Returns true if the positions in both updates are equal.")
{
    return arg.value == value;
}

MATCHER_P(HeadingUpdatesAreEqualExceptForTiming, value, "Returns true if the heading in both updates are equal.")
{
    auto heading = arg.value;
    return heading == value;
}

MATCHER_P(VelocityUpdatesAreEqualExceptForTiming, value, "Returns true if the velocity in both updates are equal.")
{
    auto velocity = arg.value;
    return velocity == value;
}

namespace
{
::testing::AssertionResult did_finish_successfully(const core::posix::wait::Result& result)
{
    if (result.status != core::posix::wait::Result::Status::exited)
        return ::testing::AssertionFailure() << "Process did not exit, but: " << (int)result.status;
    if (result.detail.if_exited.status != core::posix::exit::Status::success)
        return ::testing::AssertionFailure() << "Process did exit with failure.";

    return ::testing::AssertionSuccess();
}

struct RemoteProvider : public core::dbus::testing::Fixture
{
    static constexpr const char* stub_remote_provider_service_name
    {
        "does.not.exist.remote.Provider"
    };

    static constexpr const char* stub_remote_provider_path
    {
        "/com/ubuntu/remote/Provider"
    };
};
}

TEST_F(RemoteProvider, DISABLED_updates_are_fwd)
{
    using namespace ::testing;

    static const cul::Position position
    {
        cul::wgs84::Latitude{2* cul::units::Degrees},
        cul::wgs84::Longitude{3* cul::units::Degrees},
        cul::wgs84::Altitude{4* cul::units::Meters},
        cul::Position::Accuracy::Horizontal(5* cul::units::Meters),
        cul::Position::Accuracy::Vertical(6* cul::units::Meters)
    };

    static const cul::Heading heading
    {
        120. * cul::units::Degrees
    };

    static const cul::Velocity velocity
    {
        5. * cul::units::MetersPerSecond
    };

    auto skeleton = core::posix::fork([this]()
    {
        bool running{true};
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});

        trap->signal_raised().connect([trap, &running](core::posix::Signal)
        {
            trap->stop();
            running = false;
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));

        std::thread worker([bus]()
        {
            bus->run();
        });

        auto object = dbus::Service::add_service(
                    bus,
                    RemoteProvider::stub_remote_provider_service_name)
                        ->add_object_for_path(
                            dbus::types::ObjectPath{RemoteProvider::stub_remote_provider_path});

        // We use this instance to capture incoming requests.
        auto mock_provider = std::make_shared<NiceMock<MockProvider>>();

        EXPECT_CALL(*mock_provider, matches_criteria(_)).Times(1).WillRepeatedly(Return(false));

        EXPECT_CALL(*mock_provider, supports(_)).Times(4).WillRepeatedly(Return(true));
        EXPECT_CALL(*mock_provider, requires(_)).Times(5).WillRepeatedly(Return(true));

        EXPECT_CALL(*mock_provider, on_wifi_and_cell_reporting_state_changed(_)).Times(1);
        EXPECT_CALL(*mock_provider, on_reference_location_updated(_)).Times(1);
        EXPECT_CALL(*mock_provider, on_reference_velocity_updated(_)).Times(1);
        EXPECT_CALL(*mock_provider, on_reference_heading_updated(_)).Times(1);

        EXPECT_CALL(*mock_provider, start_position_updates()).Times(1);
        EXPECT_CALL(*mock_provider, start_heading_updates()).Times(1);
        EXPECT_CALL(*mock_provider, start_velocity_updates()).Times(1);
        EXPECT_CALL(*mock_provider, stop_position_updates()).Times(1);
        EXPECT_CALL(*mock_provider, stop_heading_updates()).Times(1);
        EXPECT_CALL(*mock_provider, stop_velocity_updates()).Times(1);

        auto provider = remote::skeleton::create_with_configuration(remote::skeleton::Configuration
        {
            object,
            bus,
            mock_provider
        });

        std::thread position_updates_injector{[mock_provider, &running]()
        {
            while (running)
            {
                mock_provider->inject_update(cul::Update<cul::Position>{position});
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }};

        std::thread heading_updates_injector{[mock_provider, &running]()
        {
            while (running)
            {
                mock_provider->inject_update(cul::Update<cul::Heading>{heading});
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }};

        std::thread velocity_updates_injector{[mock_provider, &running]()
        {
            while (running)
            {
                mock_provider->inject_update(cul::Update<cul::Velocity>{velocity});
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }};

        trap->run();

        if (position_updates_injector.joinable())
            position_updates_injector.join();

        if (heading_updates_injector.joinable())
            heading_updates_injector.join();

        if (velocity_updates_injector.joinable())
            velocity_updates_injector.join();

        bus->stop();

        if (worker.joinable())
            worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::milliseconds{500});

    auto stub = core::posix::fork([this]()
    {
        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));

        std::thread worker([bus]()
        {
            bus->run();
        });

        remote::stub::Configuration conf
        {
            core::dbus::Service::use_service(
                bus,
                RemoteProvider::stub_remote_provider_service_name)->object_for_path(
                    core::dbus::types::ObjectPath{RemoteProvider::stub_remote_provider_path})
        };

        auto provider = remote::stub::create_with_configuration(conf);

        EXPECT_FALSE(provider->matches_criteria(cul::Criteria{}));

        EXPECT_TRUE(provider->supports(cul::Provider::Features::none));
        EXPECT_TRUE(provider->supports(cul::Provider::Features::position));
        EXPECT_TRUE(provider->supports(cul::Provider::Features::heading));
        EXPECT_TRUE(provider->supports(cul::Provider::Features::velocity));

        EXPECT_TRUE(provider->requires(cul::Provider::Requirements::cell_network));
        EXPECT_TRUE(provider->requires(cul::Provider::Requirements::data_network));
        EXPECT_TRUE(provider->requires(cul::Provider::Requirements::monetary_spending));
        EXPECT_TRUE(provider->requires(cul::Provider::Requirements::none));
        EXPECT_TRUE(provider->requires(cul::Provider::Requirements::satellites));

        provider->on_wifi_and_cell_reporting_state_changed(cul::WifiAndCellIdReportingState::on);
        provider->on_reference_location_updated(cul::Update<cul::Position>{});
        provider->on_reference_heading_updated(cul::Update<cul::Heading>{});
        provider->on_reference_velocity_updated(cul::Update<cul::Velocity>{});
        provider->state_controller()->start_position_updates();
        provider->state_controller()->start_heading_updates();
        provider->state_controller()->start_velocity_updates();

        MockEventConsumer mec;
        EXPECT_CALL(mec, on_new_position(PositionUpdatesAreEqualExceptForTiming(position))).Times(AtLeast(1));
        EXPECT_CALL(mec, on_new_heading(HeadingUpdatesAreEqualExceptForTiming(heading))).Times(AtLeast(1));
        EXPECT_CALL(mec, on_new_velocity(VelocityUpdatesAreEqualExceptForTiming(velocity))).Times(AtLeast(1));

        core::ScopedConnection sc1
        {
            provider->updates().position.connect([&mec](const cul::Update<cul::Position>& p)
            {
                mec.on_new_position(p);
            })
        };

        core::ScopedConnection sc2
        {
            provider->updates().heading.connect([&mec](const cul::Update<cul::Heading>& h)
            {
                mec.on_new_heading(h);
            })
        };

        core::ScopedConnection sc3
        {
            provider->updates().velocity.connect([&mec](const cul::Update<cul::Velocity>& v)
            {
                mec.on_new_velocity(v);
            })
        };

        EXPECT_TRUE(mec.wait_for_position_update_for(std::chrono::milliseconds{1000}));
        EXPECT_TRUE(mec.wait_for_heading_update_for(std::chrono::milliseconds{1000}));
        EXPECT_TRUE(mec.wait_for_velocity_update_for(std::chrono::milliseconds{1000}));

        provider->state_controller()->stop_position_updates();
        provider->state_controller()->stop_heading_updates();
        provider->state_controller()->stop_velocity_updates();

        bus->stop();

        std::cout << "Stopped the bus" << std::endl;

        if (worker.joinable())
            worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    std::cout << "Waiting for stub to finish." << std::endl;
    EXPECT_TRUE(did_finish_successfully(stub.wait_for(core::posix::wait::Flags::untraced)));
    std::cout << "Shutting down skeleton." << std::endl;
    skeleton.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(skeleton.wait_for(core::posix::wait::Flags::untraced)));
}

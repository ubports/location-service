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
#include <com/ubuntu/location/providers/remote/provider.h>

#include "mock_provider.h"

#include <core/dbus/fixture.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>
#include <core/posix/signal.h>

#include <core/testing/fork_and_run.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;
namespace cur = com::ubuntu::remote;
namespace dbus = core::dbus;
namespace remote = com::ubuntu::location::providers::remote;

using namespace ::testing;


MATCHER_P(PositionUpdatesAreEqualExceptForTiming, value, "Returns true if the positions in both updates are equal.")
{
    auto pos = arg.value;

    return value.longitude == pos.longitude && value.latitude == pos.latitude && value.altitude == pos.altitude
        && pos.accuracy.horizontal == value.accuracy.horizontal
        && pos.accuracy.vertical == value.accuracy.vertical;
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

class MockEventConsumer
{
 public:
    MockEventConsumer()
    {
        using namespace ::testing;

        ON_CALL(*this, on_new_position(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_position_update_arrived));
        ON_CALL(*this, on_new_heading(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_heading_update_arrived));
        ON_CALL(*this, on_new_velocity(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_velocity_update_arrived));
    }

    bool wait_for_position_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{position.guard};
        return position.wait_condition.wait_for(ul, timeout, [this] { return position.update_arrived; });
    }

    bool wait_for_heading_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{heading.guard};
        return heading.wait_condition.wait_for(ul, timeout, [this] { return heading.update_arrived; });
    }

    bool wait_for_velocity_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{velocity.guard};
        return velocity.wait_condition.wait_for(ul, timeout, [this] { return velocity.update_arrived; });
    }

    MOCK_METHOD1(on_new_position, void(const cul::Update<cul::Position>&));
    MOCK_METHOD1(on_new_heading, void(const cul::Update<cul::Heading>&));
    MOCK_METHOD1(on_new_velocity, void(const cul::Update<cul::Velocity>&));

private:
    // Notes down the arrival of a position update
    // and notifies any waiting threads about the event.
    void notify_position_update_arrived()
    {
        position.update_arrived = true;
        position.wait_condition.notify_all();
    }

    // Notes down the arrival of a heading update
    // and notifies any waiting threads about the event.
    void notify_heading_update_arrived()
    {
        heading.update_arrived = true;
        heading.wait_condition.notify_all();
    }

    // Notes down the arrival of a heading update
    // and notifies any waiting threads about the event.
    void notify_velocity_update_arrived()
    {
        velocity.update_arrived = true;
        velocity.wait_condition.notify_all();
    }

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } position;

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } heading;

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } velocity;
};
}

TEST_F(RemoteProvider, updates_are_fwd)
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
        NiceMock<MockProvider> mock_provider;

        EXPECT_CALL(mock_provider, start_position_updates()).Times(1);
        EXPECT_CALL(mock_provider, start_heading_updates()).Times(1);
        EXPECT_CALL(mock_provider, start_velocity_updates()).Times(1);
        EXPECT_CALL(mock_provider, stop_position_updates()).Times(1);
        EXPECT_CALL(mock_provider, stop_heading_updates()).Times(1);
        EXPECT_CALL(mock_provider, stop_velocity_updates()).Times(1);

        cur::RemoteInterface::Skeleton remote_provider{object};

        remote_provider.object->install_method_handler<cur::RemoteInterface::StartPositionUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StartPositionUpdates";
            mock_provider.start_position_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        remote_provider.object->install_method_handler<cur::RemoteInterface::StopPositionUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StopPositionUpdates";
            mock_provider.stop_position_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        remote_provider.object->install_method_handler<cur::RemoteInterface::StartHeadingUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StartHeadingUpdates";
            mock_provider.start_heading_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        remote_provider.object->install_method_handler<cur::RemoteInterface::StopHeadingUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StopHeadingUpdates";
            mock_provider.stop_heading_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        remote_provider.object->install_method_handler<cur::RemoteInterface::StartVelocityUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StartVelocityUpdates";
            mock_provider.start_velocity_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        remote_provider.object->install_method_handler<cur::RemoteInterface::StopVelocityUpdates>([bus, &mock_provider](const dbus::Message::Ptr & msg)
        {
            VLOG(1) << "StartVelocityUpdates";
            mock_provider.stop_velocity_updates();
            bus->send(dbus::Message::make_method_return(msg));
        });

        std::thread position_updates_injector{[&remote_provider, &running]()
        {
            while (running)
            {
                remote_provider.signals.position_changed->emit(position);
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }};

        std::thread heading_updates_injector{[&remote_provider, &running]()
        {
            while (running)
            {
                remote_provider.signals.heading_changed->emit(heading);
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }};

        std::thread velocity_updates_injector{[&remote_provider, &running]()
        {
            while (running)
            {
                remote_provider.signals.velocity_changed->emit(velocity);
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
        auto conf = remote::Provider::Configuration{};
        conf.name = RemoteProvider::stub_remote_provider_service_name;
        conf.path = RemoteProvider::stub_remote_provider_path;
        conf.connection = session_bus();
        conf.connection->install_executor(dbus::asio::make_executor(conf.connection));

        remote::Provider provider{conf};
        provider.start_position_updates();
        provider.start_heading_updates();
        provider.start_velocity_updates();

        MockEventConsumer mec;
        EXPECT_CALL(mec, on_new_position(PositionUpdatesAreEqualExceptForTiming(position))).Times(AtLeast(1));
        EXPECT_CALL(mec, on_new_heading(HeadingUpdatesAreEqualExceptForTiming(heading))).Times(AtLeast(1));
        EXPECT_CALL(mec, on_new_velocity(VelocityUpdatesAreEqualExceptForTiming(velocity))).Times(AtLeast(1));

        core::ScopedConnection sc1
        {
            provider.updates().position.connect([&mec](const cul::Update<cul::Position>& p)
            {
                mec.on_new_position(p);
            })
        };

        core::ScopedConnection sc2
        {
            provider.updates().heading.connect([&mec](const cul::Update<cul::Heading>& h)
            {
                mec.on_new_heading(h);
            })
        };

        core::ScopedConnection sc3
        {
            provider.updates().velocity.connect([&mec](const cul::Update<cul::Velocity>& v)
            {
                mec.on_new_velocity(v);
            })
        };

        EXPECT_TRUE(mec.wait_for_position_update_for(std::chrono::milliseconds{1000}));
        EXPECT_TRUE(mec.wait_for_heading_update_for(std::chrono::milliseconds{1000}));
        EXPECT_TRUE(mec.wait_for_velocity_update_for(std::chrono::milliseconds{1000}));

        provider.stop_position_updates();
        provider.stop_heading_updates();
        provider.stop_velocity_updates();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    EXPECT_TRUE(did_finish_successfully(stub.wait_for(core::posix::wait::Flags::untraced)));
    skeleton.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(skeleton.wait_for(core::posix::wait::Flags::untraced)));
}

TESTP_F(RemoteProvider, matches_criteria,
{
    auto conf = remote::Provider::Configuration{};
    conf.name = RemoteProvider::stub_remote_provider_service_name;
    conf.path = RemoteProvider::stub_remote_provider_path;
    conf.connection = session_bus();
    conf.connection->install_executor(dbus::asio::make_executor(conf.connection));
    remote::Provider provider(conf);

    EXPECT_FALSE(provider.requires(com::ubuntu::location::Provider::Requirements::satellites));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::cell_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::data_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::monetary_spending));
})

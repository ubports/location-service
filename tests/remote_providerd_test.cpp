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
#include <com/ubuntu/location/service/provider_daemon.h>

#include <com/ubuntu/location/service/stub.h>

#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/update.h>

#include <core/dbus/fixture.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>

#include <gmock/gmock.h>

namespace location = com::ubuntu::location;

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

struct MockEventReceiver
{
    MOCK_METHOD1(position_update_received, void(const location::Update<location::Position>&));
    MOCK_METHOD1(heading_update_received, void(const location::Update<location::Heading>&));
    MOCK_METHOD1(velocity_update_received, void(const location::Update<location::Velocity>&));
};

struct RemoteProviderdTest : public core::dbus::testing::Fixture
{

};
}

TEST_F(RemoteProviderdTest, AClientReceivesUpdatesFromAnOutOfProcessProvider)
{
    auto oopp = core::posix::fork([this]()
    {
        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));

        const char* argv[] =
        {
            "--bus", "session",                                         // 2
            "--service-name", "com.ubuntu.location.providers.Dummy",    // 4
            "--service-path", "/com/ubuntu/location/providers/Dummy",   // 6
            "--provider", "dummy::Provider"                             // 8
        };

        auto dbus_connection_factory = [bus](core::dbus::WellKnownBus)
        {
            return bus;
        };

        return static_cast<core::posix::exit::Status>(
                    location::service::ProviderDaemon::main(
                        location::service::ProviderDaemon::Configuration::from_command_line_args(
                            8, argv, dbus_connection_factory)));
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::milliseconds{250});

    auto service = core::posix::fork([this]()
    {
        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));

        const char* argv[] =
        {
            "--bus", "session",                                                 // 2
            "--provider", "remote::Provider",                                   // 4
            "--remote::Provider::name=com.ubuntu.location.providers.Dummy",     // 5
            "--remote::Provider::path=/com/ubuntu/location/providers/Dummy"     // 6
        };

        auto dbus_connection_factory = [bus](core::dbus::WellKnownBus)
        {
            return bus;
        };

        return static_cast<core::posix::exit::Status>(
                    location::service::Daemon::main(
                        location::service::Daemon::Configuration::from_command_line_args(
                            6, argv, dbus_connection_factory)));
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::milliseconds{250});

    auto client = core::posix::fork([this]()
    {
        using namespace ::testing;

        auto trap = core::posix::trap_signals_for_all_subsequent_threads(
        {
            core::posix::Signal::sig_term,
            core::posix::Signal::sig_int
        });

        trap->signal_raised().connect([trap](const core::posix::Signal&)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));

        std::thread worker
        {
            [bus]() { bus->run(); }
        };

        MockEventReceiver receiver;
        EXPECT_CALL(receiver, position_update_received(_)).Times(1);
        EXPECT_CALL(receiver, heading_update_received(_)).Times(1);
        EXPECT_CALL(receiver, velocity_update_received(_)).Times(1);

        com::ubuntu::location::service::Stub service{bus};

        auto session = service.create_session_for_criteria(location::Criteria{});

        session->updates().position.changed().connect([&receiver](location::Update<location::Position> pos)
        {
             receiver.position_update_received(pos);
        });

        session->updates().heading.changed().connect([&receiver](location::Update<location::Heading> heading)
        {
             receiver.heading_update_received(heading);
        });

        session->updates().velocity.changed().connect([&receiver](location::Update<location::Velocity> velocity)
        {
             receiver.velocity_update_received(velocity);
        });

        session->updates().position_status = location::service::session::Interface::Updates::Status::enabled;
        session->updates().velocity_status = location::service::session::Interface::Updates::Status::enabled;
        session->updates().heading_status = location::service::session::Interface::Updates::Status::enabled;

        trap->run();

        bus->stop();

        if (worker.joinable())
            worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::milliseconds{1000});

    client.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(client.wait_for(core::posix::wait::Flags::untraced)));

    service.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(service.wait_for(core::posix::wait::Flags::untraced)));

    oopp.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(oopp.wait_for(core::posix::wait::Flags::untraced)));
}

/*
 * Copyright © 2014 Canonical Ltd.
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
#include <location/service/provider_daemon.h>

#include <location/service/stub.h>

#include <location/position.h>
#include <location/update.h>

#include <core/dbus/fixture.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>

#include "did_finish_successfully.h"
#include "mock_event_receiver.h"

#include <gmock/gmock.h>

namespace
{
struct RemoteProviderdTest : public core::dbus::testing::Fixture
{

};
}

TEST_F(RemoteProviderdTest, AClientReceivesUpdatesFromAnOutOfProcessProvider)
{
    auto oopp = core::posix::fork([this]()
    {
        auto bus = session_bus();

        const char* argv[] =
        {
            "--bus", "session",                                         // 2
            "--service-name", "com.ubuntu.location.providers.Dummy",    // 4
            "--service-path", "/location/providers/Dummy",   // 6
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
        core::posix::this_process::env::set_or_throw(
                    "TRUST_STORE_PERMISSION_MANAGER_IS_RUNNING_UNDER_TESTING",
                    "1");

        const char* argv[] =
        {
            "--bus", "session",                                                 // 2
            "--provider", "remote::Provider",                                   // 4
            "--remote::Provider::bus=session_with_address_from_env",            // 5
            "--remote::Provider::name=com.ubuntu.location.providers.Dummy",     // 6
            "--remote::Provider::path=/location/providers/Dummy"     // 7
        };

        // The daemon instance requires two bus instances.
        auto dbus_connection_factory = [this](core::dbus::WellKnownBus)
        {
            return session_bus();
        };

        return static_cast<core::posix::exit::Status>(
                    location::service::Daemon::main(
                        location::service::Daemon::Configuration::from_command_line_args(
                            7, argv, dbus_connection_factory)));
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
        EXPECT_CALL(receiver, position_update_received(_)).Times(AtLeast(1));
        EXPECT_CALL(receiver, heading_update_received(_)).Times(AtLeast(1));
        EXPECT_CALL(receiver, velocity_update_received(_)).Times(AtLeast(1));

        location::service::Stub service{bus};

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

    std::this_thread::sleep_for(std::chrono::milliseconds{4000});

    std::cout << "Shutting down client" << std::endl;
    client.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(client.wait_for(core::posix::wait::Flags::untraced)));

    std::cout << "Shutting down service" << std::endl;
    service.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(service.wait_for(core::posix::wait::Flags::untraced)));

    std::cout << "Shutting down oopp" << std::endl;
    oopp.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(oopp.wait_for(core::posix::wait::Flags::untraced)));
}

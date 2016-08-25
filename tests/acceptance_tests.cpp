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

#include <location/criteria.h>
#include <location/clock.h>
#include <location/engine.h>
#include <location/heading.h>
#include <location/logging.h>
#include <location/permission_manager.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/update.h>
#include <location/velocity.h>

#include <location/cmds/monitor.h>
#include <location/cmds/provider.h>
#include <location/cmds/run.h>
#include <location/cmds/status.h>

#include <location/wgs84/altitude.h>
#include <location/wgs84/latitude.h>
#include <location/wgs84/longitude.h>

#include <location/providers/dummy/provider.h>

#include <core/dbus/announcer.h>
#include <core/dbus/bus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/resolver.h>
#include <core/dbus/service_watcher.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>
#include <core/posix/signal.h>
#include <core/posix/this_process.h>

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <thread>

namespace dbus = core::dbus;

namespace
{
bool setup_trust_store_permission_manager_for_testing()
{
    core::posix::this_process::env::set_or_throw("TRUST_STORE_PERMISSION_MANAGER_IS_RUNNING_UNDER_TESTING", "1");
    return true;
}

static const bool trust_store_is_set_up_for_testing = setup_trust_store_permission_manager_for_testing();

struct LocationServiceStandalone : public core::dbus::testing::Fixture
{
    struct SignalingStatefulMonitorDelegate : public location::cmds::Monitor::Delegate
    {
        void on_new_position(const location::Update<location::Position>& pos)
        {
            position_ = pos; signal_if_updates_complete();
        }

        void on_new_heading(const location::Update<location::Heading>& heading)
        {
            heading_ = heading; signal_if_updates_complete();
        }

        void on_new_velocity(const location::Update<location::Velocity>& velocity)
        {
            velocity_ = velocity; signal_if_updates_complete();
        }

        void signal_if_updates_complete()
        {
            if (position_ && heading_ && velocity_)
                core::posix::this_process::instance().send_signal_or_throw(core::posix::Signal::sig_term);
        }

        location::Optional<location::Update<location::Position>> position_;
        location::Optional<location::Update<location::Heading>> heading_;
        location::Optional<location::Update<location::Velocity>> velocity_;
    };

    struct StatefulStatusDelegate : public location::cmds::Status::Delegate
    {
        void on_summary(const location::cmds::Status::Summary& summary)
        {
            summary_ = summary;
        }

        location::cmds::Status::Summary summary_;
    };
};

struct AlwaysGrantingPermissionManager : public location::PermissionManager
{
    PermissionManager::Result
    check_permission_for_credentials(const location::Criteria&,
                                     const location::Credentials&)
    {
        return PermissionManager::Result::granted;
    }
};
} // namespace

TEST_F(LocationServiceStandalone, SessionsReceiveUpdatesViaDBus)
{
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    auto server = [this]()
    {
        SCOPED_TRACE("Server");
        location::cmds::Run run;
        return static_cast<core::posix::exit::Status>(run.run(location::util::cli::Command::Context{std::cin, std::cout, {"--testing", "1"}}));
    };

    auto client = [this]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        SCOPED_TRACE("Client");

        location::providers::dummy::Configuration config;

        auto delegate = std::make_shared<SignalingStatefulMonitorDelegate>();
        location::cmds::Monitor monitor{delegate};

        monitor.run(location::util::cli::Command::Context{std::cin, std::cout, {}});

        EXPECT_EQ(config.reference_position, delegate->position_.get().value);
        EXPECT_EQ(config.reference_velocity, delegate->velocity_.get().value);
        EXPECT_EQ(config.reference_heading, delegate->heading_.get().value);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

TEST_F(LocationServiceStandalone, StatusCanBeQueried)
{
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    auto server = [this]()
    {
        SCOPED_TRACE("Server");
        location::cmds::Run run;
        return static_cast<core::posix::exit::Status>(run.run(location::util::cli::Command::Context{std::cin, std::cout, {"--testing", "1"}}));
    };

    auto client = [this]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        SCOPED_TRACE("Client");

        location::providers::dummy::Configuration config;

        auto delegate = std::make_shared<StatefulStatusDelegate>();
        location::cmds::Status status{delegate};

        status.run(location::util::cli::Command::Context{std::cin, std::cout, {}});

        EXPECT_TRUE(delegate->summary_.is_online);
        EXPECT_EQ(location::Service::State::enabled, delegate->summary_.state);
        EXPECT_TRUE(delegate->summary_.does_satellite_based_positioning);
        EXPECT_FALSE(delegate->summary_.does_report_cell_and_wifi_ids);
        EXPECT_TRUE(delegate->summary_.svs.empty());

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

namespace
{
struct LocationServiceStandaloneLoad : public LocationServiceStandalone
{
    unsigned int client_count{100};     // We start up this many client processes in parallel.
    std::chrono::seconds duration{15};  // We keep the test running for this long.
};
}

#include "did_finish_successfully.h"

TEST_F(LocationServiceStandaloneLoad, MultipleClientsConnectingAndDisconnectingWorks)
{
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    auto server = core::posix::fork([this]()
    {
        location::cmds::Run run;
        return static_cast<core::posix::exit::Status>(run.run(location::util::cli::Command::Context{std::cin, std::cout, {}}));
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::seconds{1});

    auto provider = [this]()
    {
        location::cmds::Provider provider;
        return static_cast<core::posix::exit::Status>(provider.run(location::util::cli::Command::Context{std::cin, std::cout, {"--id=dummy::Provider"}}));
    };

    auto p1 = core::posix::fork(provider, core::posix::StandardStream::empty);
    auto p2 = core::posix::fork(provider, core::posix::StandardStream::empty);    

    auto client = []()
    {
        SCOPED_TRACE("Client");

        location::providers::dummy::Configuration config;

        auto delegate = std::make_shared<LocationServiceStandalone::SignalingStatefulMonitorDelegate>();
        location::cmds::Monitor monitor{delegate};

        monitor.run(location::util::cli::Command::Context{std::cin, std::cout, {}});

        EXPECT_EQ(config.reference_position, delegate->position_.get().value);
        EXPECT_EQ(config.reference_velocity, delegate->velocity_.get().value);
        EXPECT_EQ(config.reference_heading, delegate->heading_.get().value);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    std::vector<core::posix::ChildProcess> clients;

    for (unsigned int i = 0; i < client_count; i++)
        clients.push_back(core::posix::fork(client, core::posix::StandardStream::empty));

    bool running = true;

    std::thread chaos
    {
        [&clients, client, &running]()
        {
            std::this_thread::sleep_for(std::chrono::seconds{2});

            std::default_random_engine generator;
            std::uniform_int_distribution<int> dice(1, 6);

            while (running && clients.size() > 1)
            {
                std::uniform_int_distribution<int> client_dist(0,clients.size()-1);

                // Sample a client index
                auto idx = client_dist(generator);
                // Pick a client at random
                auto& c = clients.at(idx);
                // Kill that client
                c.send_signal_or_throw(core::posix::Signal::sig_kill);
                // Wait for the child to avoid zombies
                c.wait_for(core::posix::wait::Flags::untraced);

                clients.erase(clients.begin() + idx);

                // And we pause for 500ms
                std::this_thread::sleep_for(std::chrono::milliseconds{50});
            }
        }
    };

    // We let the setup spin for 30 seconds.
    std::this_thread::sleep_for(duration);

    running = false;

    if (chaos.joinable())
        chaos.join();

    VLOG(1) << "Stopping clients: " << clients.size();
    for (auto& client : clients)
    {
        VLOG(1) << "  Stopping client: " << client.pid();
        EXPECT_TRUE(did_finish_successfully(client.wait_for(core::posix::wait::Flags::untraced)));
    }

    VLOG(1) << "Stopping providers: ";
    VLOG(1) << "  " << p1.pid(); p1.send_signal_or_throw(core::posix::Signal::sig_term); EXPECT_TRUE(did_finish_successfully(p1.wait_for(core::posix::wait::Flags::untraced)));
    VLOG(1) << "  " << p2.pid(); p2.send_signal_or_throw(core::posix::Signal::sig_term); EXPECT_TRUE(did_finish_successfully(p2.wait_for(core::posix::wait::Flags::untraced)));

    VLOG(1) << "Cleaned up clients, shutting down the service...";

    server.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(server.wait_for(core::posix::wait::Flags::untraced)));
}

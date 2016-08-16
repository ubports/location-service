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

#include <location/daemon.h>

#include "did_finish_successfully.h"

#include <core/dbus/fixture.h>
#include <core/posix/fork.h>

#include <gtest/gtest.h>

#include <thread>

namespace
{
bool setup_trust_store_permission_manager_for_testing()
{
    core::posix::this_process::env::set_or_throw("TRUST_STORE_PERMISSION_MANAGER_IS_RUNNING_UNDER_TESTING", "1");
    return true;
}

static const bool trust_store_is_set_up_for_testing = setup_trust_store_permission_manager_for_testing();

struct DaemonAndCli : public core::dbus::testing::Fixture
{
};
}

TEST_F(DaemonAndCli, QueryingStatusWorks)
{
    auto server = [this]()
    {
        location::Daemon daemon;
        return static_cast<core::posix::exit::Status>(daemon.run({"run", "--testing=1"}));
    };

    auto client = [this]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        location::Daemon daemon;
        return static_cast<core::posix::exit::Status>(daemon.run({"status"}));
    };

    auto d = core::posix::fork(server, core::posix::StandardStream::empty);
    auto c = core::posix::fork(client, core::posix::StandardStream::empty);

    EXPECT_TRUE(did_finish_successfully(c.wait_for(core::posix::wait::Flags::untraced)));
    EXPECT_NO_THROW(d.send_signal_or_throw(core::posix::Signal::sig_term));
    EXPECT_TRUE(did_finish_successfully(d.wait_for(core::posix::wait::Flags::untraced)));
}

TEST_F(DaemonAndCli, MonitoringWorks)
{
    auto server = [this]()
    {
        location::Daemon daemon;
        return static_cast<core::posix::exit::Status>(daemon.run({"run", "--testing=1"}));
    };

    auto client = [this]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        location::Daemon daemon;
        return static_cast<core::posix::exit::Status>(daemon.run({"monitor"}));
    };

    auto d = core::posix::fork(server, core::posix::StandardStream::empty);
    auto c = core::posix::fork(client, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::seconds{5});

    EXPECT_NO_THROW(c.send_signal_or_throw(core::posix::Signal::sig_term));
    EXPECT_TRUE(did_finish_successfully(c.wait_for(core::posix::wait::Flags::untraced)));
    EXPECT_NO_THROW(d.send_signal_or_throw(core::posix::Signal::sig_term));
    EXPECT_TRUE(did_finish_successfully(d.wait_for(core::posix::wait::Flags::untraced)));
}

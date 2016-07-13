/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/cmds/run.h>

#include <core/dbus/fixture.h>
#include <core/posix/fork.h>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include <thread>

namespace
{
struct Bug1447110 : public core::dbus::testing::Fixture
{

};
}

TEST_F(Bug1447110, obsolete_log_directory_is_removed_on_startup)
{
    // We are trying to establish the pre-condition that a non-empty
    // directory /var/log/ubuntu-location-service exists. If we fail to do
    // so, we leave a message and bail out, not marking the test as failure.
    {
        boost::system::error_code ec;
        boost::filesystem::remove_all("/var/log/ubuntu-location-service/not_empty", ec);
        if (not boost::filesystem::create_directories("/var/log/ubuntu-location-service/not_empty", ec) || ec)
        {
            std::cerr << "Unable to create directory for testing purposes." << std::endl;
            return;
        }
    }

    auto cp = core::posix::fork([]()
    {
        SCOPED_TRACE("Server");
        location::cmds::Run run;
        return static_cast<core::posix::exit::Status>(run.run(location::util::cli::Command::Context{std::cin, std::cout, {"--testing", "1"}}));
    }, core::posix::StandardStream::stderr);

    std::this_thread::sleep_for(std::chrono::seconds{1});
    EXPECT_FALSE(boost::filesystem::exists("/var/log/ubuntu-location-service"));
}

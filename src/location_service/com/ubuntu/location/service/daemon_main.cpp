/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTIlocationAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include "daemon.h"

#include <com/ubuntu/location/logging.h>

#include <boost/filesystem.hpp>

namespace location = com::ubuntu::location;

int main(int argc, const char** argv)
{
    // Ensure that log files dating back to before the fix
    // for lp:1447110 are removed and do not waste space.
    {
        static const boost::filesystem::path old_log_dir{"/var/log/ubuntu-location-service"};
        boost::system::error_code ec;
        boost::filesystem::remove(old_log_dir, ec);
    }
    // Setup logging for the daemon.
    FLAGS_logtostderr = true;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_max_log_size = 5;

    google::InitGoogleLogging("com.ubuntu.location");

    location::service::Daemon::Configuration config;
    try
    {
        config = location::service::Daemon::Configuration::from_command_line_args(argc, argv);
    } catch(const std::runtime_error& e)
    {
        std::cerr << "Problem parsing command line: " << e.what();
        location::service::Daemon::print_help(std::cerr);
        return EXIT_FAILURE;
    }

    try
    {
        location::service::Daemon::main(config);
        config.settings->sync();
    } catch(const std::exception& e)
    {
        std::cout << "Problem executing the daemon: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

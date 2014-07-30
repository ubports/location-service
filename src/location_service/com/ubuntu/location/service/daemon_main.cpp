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

namespace
{
// The directory we put log files in.
constexpr const char* log_dir{"/var/log/ubuntu-location-service"};
}

int main(int argc, char** argv)
{
    // Setup logging for the daemon.
    boost::system::error_code ec;
    boost::filesystem::create_directories(boost::filesystem::path{log_dir}, ec);

    // According to http://www.boost.org/doc/libs/1_55_0/libs/filesystem/doc/reference.html#create_directories
    //   Creation failure because p resolves to an existing directory shall not be treated as an error.
    // With that, we are free to check the error condition and adjust to stderr
    // logging accordingly.
    if (ec)
    {
        FLAGS_logtostderr = true;
        LOG(WARNING) << "Problem creating directory for log files: " << ec << "."
                     << "Falling back to stderr logging.";
    }

    FLAGS_log_dir = log_dir;
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
    } catch(const std::exception& e)
    {
        std::cout << "Problem executing the daemon: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

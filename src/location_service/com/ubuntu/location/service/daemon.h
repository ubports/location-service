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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DAEMON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DAEMON_H_

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
/**
 * @brief The Daemon struct encapsulates main functions for the location service and its cli.
 */
struct Daemon
{
    struct Cli
    {
        Cli() = delete;

        /**
         * @brief main of the command-line interface to the location service.
         *
         * The cli supports the following arguments
         *   --bus arg (=session)  The well-known bus to connect to the service upon
         *   --help                Produces this help message
         *   --command             Command to execute against a running service, known
         *                         commands are:
         *                              is_online
         *                              does_satellite_based_positioning
         *                              does_report_wifi_and_cell_ids
         *                              visible_space_vehicles
         *
         * @param argc Size of the arguments array.
         * @param argv Array of arguments.
         * @return EXIT_SUCCESS or EXIT_FAILURE.
         */
        static int main(int argc, char const** argv);
    };

    Daemon() = delete;

    /**
     * @brief main of the location service daemon.
     *
     *   --bus arg (=session)  The well-known bus to connect to the service upon
     *   --help                Produces this help message
     *   --provider arg        The providers that should be added to the engine
     *
     * @param argc Size of the arguments array.
     * @param argv Array of arguments.
     * @return EXIT_SUCCESS or EXIT_FAILURE.
     */
    static int main(int argc, char const** argv);
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DAEMON_H_

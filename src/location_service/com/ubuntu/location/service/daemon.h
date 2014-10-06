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

#include <com/ubuntu/location/configuration.h>

#include <com/ubuntu/location/service/dbus_connection_factory.h>

#include <iosfwd>
#include <string>

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
    /** @brief Describes the command-line interface to the daemon. */
    struct Cli
    {
        Cli() = delete;

        /** @brief Enumerates all commands known to the cli. */
        enum class Command
        {
            /** @brief Marks the unknown command. */
            unknown,
            /** @brief Request to query a property value of the running service. */
            get,
            /** @brief Request to adjust a property value of the running service. */
            set
        };

        /** @brief Enumerates all properties known to the cli. */
        enum class Property
        {
            /** @brief Marks the unknown property. */
            unknown,
            /** @brief Indicates whether the positioning engine is online. */
            is_online,
            /** @brief Indicates whether the positioning engine uses satellite-based positioning. */
            does_satellite_based_positioning,
            /** @brief Indicates whether the positioning engine leverages wifi and cell ids for positioning. */
            does_report_wifi_and_cell_ids,
            /** @brief The list of currently visible space-vehicles. */
            visible_space_vehicles
        };

        /** @brief Parameters for an invocation of the CLI. */
        struct Configuration
        {
            /** @brief Parses a configuration from the command line.
             *
             * --bus arg (=session)      The well-known bus to connect to the service upon
             * --help                    Produces this help message
             * --property arg (=unknown) Property to set/get from a running service, known
                                         properties are:
                                            is_online [get/set]
                                            does_satellite_based_positioning [get/set]
                                            does_report_wifi_and_cell_ids [get/set]
                                            visible_space_vehicles [get]
             * --set arg                 Adjust the value of the property.
             * --get                     Query the value of the property.
             */
            static Configuration from_command_line_args(
                    int argc,
                    const char** argv,
                    DBusConnectionFactory factory = default_dbus_connection_factory());

            /** @brief The bus to connect to. */
            core::dbus::Bus::Ptr bus;

            /** @brief The command to execute against a running daemon. */
            Command command
            {
                Command::unknown
            };

            /** @brief If command is get/set/monitor, the property to act upon. */
            Property property
            {
                Property::unknown
            };

            /** @brief The new, string-based value for a property. */
            std::string new_value;
        };

        /** @brief Pretty-prints the CLI's help text to the given output stream. */
        static void print_help(std::ostream& out);

        /**
         * @brief main of the command-line interface to the location service.
         * @return EXIT_SUCCESS or EXIT_FAILURE.
         */
        static int main(const Configuration& configuration);
    };

    Daemon() = delete;

    /** @brief Parameters for an invocation of the daemon. */
    struct Configuration
    {
        /** @brief Parses a configuration from the command line.
         *
         *   --bus arg (=session)  The well-known bus to connect to the service upon
         *   --help                Produces this help message
         *   --testing             Enables executing the service without selected providers
         *   --provider arg        The providers that should be added to the engine
         */
        static Configuration from_command_line_args(
                int argc,
                const char** argv,
                DBusConnectionFactory factory = default_dbus_connection_factory());

        /** @brief The bus to expose the service upon. */
        core::dbus::Bus::Ptr incoming;

        /** @brief The bus to use for querying other services. */
        core::dbus::Bus::Ptr outgoing;

        /** @brief Configures the daemon for testing mode. */
        bool is_testing_enabled
        {
            false
        };
        /** @brief Providers that have been requested on the command line. */
        std::vector<std::string> providers;
        /** @brief Provider-specific options keyed on the provider name. */
        std::map< std::string, location::Configuration > provider_options;
    };

    /** @brief Pretty-prints the CLI's help text to the given output stream. */
    static void print_help(std::ostream& out);

    /**
     * @brief Executes the daemon with the given configuration.
     * @return EXIT_SUCCESS or EXIT_FAILURE.
     */
    static int main(const Configuration& config);
};

/** @brief Parses a Cli property from the given input stream, throws std::runtime_error. */
std::istream& operator>>(std::istream& in, Daemon::Cli::Property& property);

/** @brief Pretty-prints a property value */
std::ostream& operator<<(std::ostream& out, Daemon::Cli::Property property);
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DAEMON_H_

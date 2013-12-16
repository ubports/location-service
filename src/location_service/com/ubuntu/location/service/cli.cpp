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
#include "program_options.h"

#include <com/ubuntu/location/service/stub.h>

#include <org/freedesktop/dbus/resolver.h>
#include <org/freedesktop/dbus/asio/executor.h>

#include <set>
#include <string>
#include <thread>

namespace dbus = org::freedesktop::dbus;
namespace location = com::ubuntu::location;

namespace
{
enum class Command
{
    unknown,
    is_online,
    does_satellite_based_positioning,
    does_report_wifi_and_cell_ids,
    visible_space_vehicles
};

const std::map<std::string, Command> known_commands =
{
    {"is_online", Command::is_online},
    {"does_satellite_based_positioning", Command::does_satellite_based_positioning},
    {"does_report_wifi_and_cell_ids", Command::does_report_wifi_and_cell_ids},
    {"visible_space_vehicles", Command::visible_space_vehicles}
};
}
int main(int argc, char** argv)
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add("command",
                "Command to execute against a running service, known commands are:\n"
                "   is_online\n"
                "   does_satellite_based_positioning\n"
                "   does_report_wifi_and_cell_ids\n"
                "   visible_space_vehicles");

    if (!options.parse_from_command_line_args(argc, argv))
        return EXIT_FAILURE;

    if (options.value_count_for_key("help") > 0)
    {
        options.print_help(std::cout);
        return EXIT_SUCCESS;
    }

    if (options.value_count_for_key("command") == 0)
    {
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    auto command = Command::unknown;

    try
    {
        command = known_commands.at(options.value_for_key<std::string>("command"));
    } catch(const std::runtime_error& e)
    {
        std::cout << "Unknown command, aborting now." << std::endl;
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    dbus::Bus::Ptr bus
    {
        new dbus::Bus{options.bus()}
    };

    auto location_service =
            dbus::resolve_service_on_bus<location::service::Interface, location::service::Stub>(bus);

    switch(command)
    {
    case Command::is_online:
        std::cout << std::boolalpha << "Location service is "
                  << (location_service->is_online() ? "online" : "offline") << std::endl;
        break;
    case Command::does_report_wifi_and_cell_ids:
        std::cout << std::boolalpha << "Location service "
                  << (location_service->does_report_cell_and_wifi_ids() ? "does" : "does not")
                  << " report cell and wifi ids." << std::endl;
        break;
    case Command::does_satellite_based_positioning:
        std::cout << std::boolalpha << "Location service "
                  << (location_service->does_satellite_based_positioning() ? "does" : "does not")
                  << " satellite based positioning." << std::endl;
        break;
    case Command::visible_space_vehicles:
    {
        auto svs = location_service->visible_space_vehicles().get();
        std::cout << "Visible space vehicles:" << std::endl;
        for (const auto& sv : svs)
            std::cout << "\t" << sv << std::endl;
        break;
    }
    case Command::unknown:
        std::cout << "Unknown command, aborting now." << std::endl;
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

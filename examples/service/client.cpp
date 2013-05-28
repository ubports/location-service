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
#include "program_options.h"

#include "com/ubuntu/location/service/stub.h"

#include <org/freedesktop/dbus/resolver.h>
#include <org/freedesktop/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace dbus = org::freedesktop::dbus;

int main(int argc, char** argv)
{
    cul::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add(
        "bus", 
        "The well-known bus to connect to the service upon", 
        std::string{"session"});

    if (!options.parse_from_command_line_args(argc, argv))
        return EXIT_FAILURE;

    if (options.value_count_for_key("help") > 0)
    {
        options.print_help(std::cout);
        return EXIT_SUCCESS;
    }

    static const std::map<std::string, dbus::WellKnownBus> lut = 
    {
        {"session", dbus::WellKnownBus::session},
        {"system", dbus::WellKnownBus::system},
    };

    org::freedesktop::dbus::Bus::Ptr bus
    {
        new org::freedesktop::dbus::Bus{lut.at(options.value_for_key<std::string>("bus"))}
    };
    bus->install_executor(
        org::freedesktop::dbus::Executor::Ptr(
            new org::freedesktop::dbus::asio::Executor{bus}));
    std::thread t{[bus](){bus->run();}};
    
    auto location_service = 
            org::freedesktop::dbus::resolve_service_on_bus<culs::Interface, culs::Stub>(bus);
        
    auto s1 = location_service->create_session_for_criteria(com::ubuntu::location::Criteria{});
        
    s1->install_position_updates_handler(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Position>& new_position) {
            std::cout << "On position updated: " << new_position << std::endl;
        });
    s1->install_velocity_updates_handler(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Velocity>& new_velocity) {
            std::cout << "On velocity_changed " << new_velocity << std::endl;
        });
    s1->install_heading_updates_handler(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Heading>& new_heading) {
            std::cout << "On heading changed: " << new_heading << std::endl;
        });
        
    s1->start_position_updates();
    s1->start_velocity_updates();
    s1->start_heading_updates();
        
    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}

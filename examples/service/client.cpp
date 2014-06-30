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

#include <com/ubuntu/location/service/stub.h>

#include <core/dbus/resolver.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;
namespace dbus = core::dbus;

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

    dbus::Bus::Ptr bus
    {
        new dbus::Bus{lut.at(options.value_for_key<std::string>("bus"))}
    };
    bus->install_executor(dbus::asio::make_executor(bus));
    std::thread t{[bus](){bus->run();}};
    
    auto location_service = 
            dbus::resolve_service_on_bus<culs::Interface, culs::Stub>(bus);
        
    auto s1 = location_service->create_session_for_criteria(cul::Criteria{});
        
    s1->updates().position.changed().connect(
        [&](const cul::Update<cul::Position>& new_position) {
            std::cout << "On position updated: " << new_position << std::endl;
        });
    s1->updates().velocity.changed().connect(
        [&](const cul::Update<cul::Velocity>& new_velocity) {
            std::cout << "On velocity_changed " << new_velocity << std::endl;
        });
    s1->updates().heading.changed().connect(
        [&](const cul::Update<cul::Heading>& new_heading) {
            std::cout << "On heading changed: " << new_heading << std::endl;
        });
        
    s1->updates().position_status = culss::Interface::Updates::Status::enabled;
    s1->updates().heading_status = culss::Interface::Updates::Status::enabled;
    s1->updates().velocity_status = culss::Interface::Updates::Status::enabled;
        
    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}

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
#include <com/ubuntu/location/provider_factory.h>

#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/implementation.h>
#include <com/ubuntu/location/service/stub.h>

#include "program_options.h"
#include "daemon.h"

#include <org/freedesktop/dbus/announcer.h>
#include <org/freedesktop/dbus/resolver.h>
#include <org/freedesktop/dbus/asio/executor.h>

#include <thread>

namespace location = com::ubuntu::location;
namespace dbus = org::freedesktop::dbus;

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

int location::service::Daemon::main(int argc, char** argv)
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add_composed<std::vector<std::string>>(
        "provider",
        "The providers that should be added to the engine");

    if (!options.parse_from_command_line_args(argc, argv))
        return EXIT_FAILURE;

    if (options.value_count_for_key("help") > 0)
    {
        options.print_help(std::cout);
        return EXIT_SUCCESS;
    }

    if (options.value_count_for_key("provider") == 0)
    {
        std::cout << "A set of providers need to be specified. The following providers are known:" << std::endl;
        location::ProviderFactory::instance().enumerate(
            [](const std::string& name, const location::ProviderFactory::Factory&)
            {
                std::cout << "\t" << name << std::endl;
            });
        return EXIT_FAILURE;
    }

    auto selected_providers = options.value_for_key<std::vector<std::string>>("provider");

    std::map<std::string, location::ProviderFactory::Configuration> config_lut;
    std::set<location::Provider::Ptr> instantiated_providers;

    for (const std::string& provider : selected_providers)
    {
        std::cout << "Instantiating and configuring: " << provider << std::endl;
        options.enumerate_unrecognized_options(
            [&config_lut, provider](const std::string& s)
            {
                std::stringstream in(s);
                std::string key, value;

                std::getline(in, key, '=');
                std::getline(in, value, '=');

                std::size_t pos = key.find(provider);
                if (pos == std::string::npos)
                    return;
                static const std::string option_marker{"--"};
                static const std::string scope_separator{"::"};
                key = key.erase(key.find_first_of(option_marker), option_marker.size());
                key = key.erase(key.find_first_of(provider), provider.size());
                key = key.erase(key.find_first_of(scope_separator), scope_separator.size());

                std::cout << "\t" << key << " -> " << value << std::endl;

                config_lut[provider].put(key, value);
            });

        try
        {
            auto p = location::ProviderFactory::instance().create_provider_for_name_with_config(
                provider,
                config_lut[provider]);

            if (p)
                instantiated_providers.insert(p);
            else
                throw std::runtime_error("Problem instantiating provider");

        } catch(const std::runtime_error& e)
        {
            std::cerr << "Exception instantiating provider: " << e.what() << " ... Aborting now." << std::endl;
            return EXIT_FAILURE;
        }
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

    bus->install_executor(
        dbus::Executor::Ptr(
            new dbus::asio::Executor{bus}));

    location::service::DefaultConfiguration config;

    auto location_service =
            dbus::announce_service_on_bus<
                location::service::Interface,
                location::service::Implementation
            >(
                bus,
                config.the_engine(
                    instantiated_providers,
                    config.the_provider_selection_policy()),
                config.the_permission_manager());

    std::thread t{[bus](){bus->run();}};

    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}

int location::service::Daemon::Cli::main(int argc, char** argv)
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

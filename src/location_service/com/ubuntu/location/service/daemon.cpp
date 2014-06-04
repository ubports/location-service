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

#include <core/dbus/announcer.h>
#include <core/dbus/resolver.h>
#include <core/dbus/asio/executor.h>

#include <system_error>
#include <thread>

#include <signal.h>
#include <sys/signalfd.h>

namespace location = com::ubuntu::location;
namespace dbus = core::dbus;

namespace core
{
struct SigTermCatcher
{
    inline SigTermCatcher()
    {
        sigemptyset(&signal_mask);

        if (-1 == sigaddset(&signal_mask, SIGTERM))
            throw std::system_error(errno, std::system_category());

        if (-1 == sigprocmask(SIG_BLOCK, &signal_mask, NULL))
            throw std::system_error(errno, std::system_category());
    }

    inline void wait_for_signal()
    {
        int signal = -1;
        ::sigwait(&signal_mask, &signal);
    }

    sigset_t signal_mask;
};
}

namespace
{
enum class Command
{
    unknown,
    get,
    set
};

enum class Property
{
    unknown,
    is_online,
    does_satellite_based_positioning,
    does_report_wifi_and_cell_ids,
    visible_space_vehicles
};

const std::map<std::string, Property> known_properties =
{
    {"is_online", Property::is_online},
    {"does_satellite_based_positioning", Property::does_satellite_based_positioning},
    {"does_report_wifi_and_cell_ids", Property::does_report_wifi_and_cell_ids},
    {"visible_space_vehicles", Property::visible_space_vehicles}
};

struct NullReporter : public location::service::Harvester::Reporter
{
    NullReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    void start()
    {
    }

    /** @brief Tell the reporter to shut down its operation. */
    void stop()
    {
    }

    /**
     * @brief Triggers the reporter to send off the information.
     */
    void report(const location::Update<location::Position>&,
                const std::vector<location::connectivity::WirelessNetwork::Ptr>&,
                const std::vector<location::connectivity::RadioCell::Ptr>&)
    {
    }
};
}

int location::service::Daemon::main(int argc, const char** argv)
{
    core::SigTermCatcher sc;

    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add("testing", "Enables running the service without providers");
    options.add_composed<std::vector<std::string>>("provider",
                                                   "The providers that should be added to the engine");

    if (!options.parse_from_command_line_args(argc, argv))
        return EXIT_FAILURE;

    if (options.value_count_for_key("help") > 0)
    {
        options.print_help(std::cout);
        return EXIT_SUCCESS;
    }

    std::set<location::Provider::Ptr> instantiated_providers;

    if (options.value_count_for_key("testing") == 0 &&
        options.value_count_for_key("provider") == 0)
    {
        std::cout << "A set of providers need to be specified. The following providers are known:" << std::endl;
        location::ProviderFactory::instance().enumerate(
                    [](const std::string& name, const location::ProviderFactory::Factory&)
        {
            std::cout << "\t" << name << std::endl;
        });
        return EXIT_FAILURE;
    } else if(options.value_count_for_key("provider") > 0)
    {
        auto selected_providers = options.value_for_key<std::vector<std::string>>("provider");

        std::map<std::string, location::ProviderFactory::Configuration> config_lut;

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
    }

    dbus::Bus::Ptr bus
    {
        new dbus::Bus{options.bus()}
    };

    bus->install_executor(dbus::asio::make_executor(bus));

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
                config.the_permission_manager(),
                std::make_shared<NullReporter>());

    std::thread t{[bus](){bus->run();}};

    sc.wait_for_signal();

    bus->stop();

    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}

int location::service::Daemon::Cli::main(int argc, const char** argv)
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add("property",
                "Property to set/get from a running service, known properties are:\n"
                "   is_online [get/set]\n"
                "   does_satellite_based_positioning [get/set]\n"
                "   does_report_wifi_and_cell_ids [get/set]\n"
                "   visible_space_vehicles [get]",
                std::string("is_online"));
    options.add<std::string>("set", "Adjust the value of the property.");
    options.add("get", "Query the value of the property.");

    if (!options.parse_from_command_line_args(argc, argv))
    {
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    if (options.value_count_for_key("help") > 0)
    {
        options.print_help(std::cout);
        return EXIT_SUCCESS;
    }

    if (options.value_count_for_key("property") == 0)
    {
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    if (options.value_count_for_key("get") > 0 && options.value_count_for_key("set") > 0)
    {
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    auto property = Property::unknown;

    try
    {
        property = known_properties.at(options.value_for_key<std::string>("property"));
    } catch(const std::runtime_error& e)
    {
        std::cout << "Unknown property, aborting now." << std::endl;
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    auto command = Command::unknown;

    if (options.value_count_for_key("get") > 0)
        command = Command::get;
    else if (options.value_count_for_key("set") > 0)
        command = Command::set;

    dbus::Bus::Ptr bus
    {
        new dbus::Bus{options.bus()}
    };

    auto location_service =
            dbus::resolve_service_on_bus<location::service::Interface, location::service::Stub>(bus);

    switch (property)
    {
    case Property::is_online:
        switch (command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service is "
                      << (location_service->is_online() ? "online" : "offline") << std::endl;
            break;
        case Command::set:
        {
            auto set_value = options.value_for_key<std::string>("set");

            std::stringstream ss(set_value);
            bool flag = location_service->is_online();
            ss >> std::boolalpha >> flag;

            std::cout << "Adjusting is_online property to value: " << set_value << " -> ";

            location_service->is_online() = flag;

            if (location_service->is_online() != flag)
            {
                std::cout << "failed" << std::endl;
                return EXIT_FAILURE;
            }
            std::cout << "succeeded" << std::endl;
            break;
        }
        case Command::unknown: break;
        }
        break;
    case Property::does_report_wifi_and_cell_ids:
        switch (command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service "
                      << (location_service->does_report_cell_and_wifi_ids() ? "does" : "does not")
                      << " report cell and wifi ids." << std::endl;
            break;
        case Command::set:
        {
            auto set_value = options.value_for_key<std::string>("set");
            std::stringstream ss(set_value);
            bool flag = location_service->does_report_cell_and_wifi_ids();
            ss >> std::boolalpha >> flag;

            std::cout << "Adjusting does_report_cell_and_wifi_ids property to value: "
                      << std::boolalpha << flag << " -> ";
            location_service->does_report_cell_and_wifi_ids() = flag;
            if (location_service->does_report_cell_and_wifi_ids() != flag)
            {
                std::cout << "failed" << std::endl;
                return EXIT_FAILURE;
            }
            std::cout << "succeeded" << std::endl;
            break;
        }
        case Command::unknown: break;
        }
        break;
    case Property::does_satellite_based_positioning:
        switch (command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service "
                      << (location_service->does_satellite_based_positioning() ? "does" : "does not")
                      << " satellite based positioning." << std::endl;
            break;
        case Command::set:
        {
            auto set_value = options.value_for_key<std::string>("set");
            std::stringstream ss(set_value);
            bool flag = location_service->does_satellite_based_positioning();
            ss >> std::boolalpha >> flag;

            std::cout << "Adjusting does_satellite_based_positioning property to value: "
                      << std::boolalpha << flag << " -> ";
            location_service->does_satellite_based_positioning() = flag;
            if (location_service->does_satellite_based_positioning() != flag)
            {
                std::cout << "failed" << std::endl;
                return EXIT_FAILURE;
            }
            std::cout << "succeeded" << std::endl;
            break;
        }
        case Command::unknown: break;
        }
        break;
    case Property::visible_space_vehicles:
    {
        switch (command)
        {
        case Command::get:
        {
            auto svs = location_service->visible_space_vehicles().get();
            std::cout << "Visible space vehicles:" << std::endl;
            for (const auto& sv : svs)
                std::cout << "\t" << sv.second << std::endl;
            break;
        }
        case Command::set:
            std::cout << "Property visible_space_vehicles is not set-able, aborting now." << std::endl;
            options.print_help(std::cout);
            return EXIT_FAILURE;
        case Command::unknown: break;
        }
        break;
    }
    case Property::unknown:
        std::cout << "Unknown property, aborting now." << std::endl;
        options.print_help(std::cout);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

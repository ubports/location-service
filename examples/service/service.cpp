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

#include <com/ubuntu/location/provider_factory.h>

#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/implementation.h>

#include <core/dbus/announcer.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace dbus = core::dbus;

namespace
{
struct NullReporter : public culs::Harvester::Reporter
{
    NullReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    void start() override
    {
    }

    /** @brief Tell the reporter to shut down its operation. */
    void stop()
    {
    }

    /**
     * @brief Triggers the reporter to send off the information.
     */
    void report(const cul::Update<cul::Position>&,
                const std::vector<cul::connectivity::WirelessNetwork::Ptr>&,
                const std::vector<cul::connectivity::RadioCell::Ptr>&)
    {
    }
};
}

int main(int argc, char** argv)
{
    cul::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add(
        "bus", 
        "The well-known bus to announce the service upon", 
        std::string{"session"});
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
        cul::ProviderFactory::instance().enumerate(
            [](const std::string& name, const cul::ProviderFactory::Factory&)
            {
                std::cout << "\t" << name << std::endl;
            });
        return EXIT_FAILURE;
    }

    auto selected_providers = options.value_for_key<std::vector<std::string>>("provider");

    std::map<std::string, cul::ProviderFactory::Configuration> config_lut;
    std::set<cul::Provider::Ptr> instantiated_providers;

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
            auto p = cul::ProviderFactory::instance().create_provider_for_name_with_config(
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

    dbus::Bus::Ptr incoming
    {
        new dbus::Bus{lut.at(options.value_for_key<std::string>("bus"))}
    };
    incoming->install_executor(dbus::asio::make_executor(incoming));

    dbus::Bus::Ptr outgoing
    {
        new dbus::Bus{lut.at(options.value_for_key<std::string>("bus"))}
    };
    outgoing->install_executor(dbus::asio::make_executor(outgoing));

    culs::DefaultConfiguration config;

    culs::Implementation::Configuration configuration
    {
        incoming,
        outgoing,
        config.the_engine(instantiated_providers, config.the_provider_selection_policy()),
        config.the_permission_manager(incoming),
        culs::Harvester::Configuration
        {
            cul::connectivity::platform_default_manager(),
            std::make_shared<NullReporter>()
        }
    };

    auto location_service = std::make_shared<culs::Implementation>(configuration);
    
    std::thread t1{[incoming](){incoming->run();}};
    std::thread t2{[outgoing](){outgoing->run();}};
    
    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    return EXIT_SUCCESS;
}

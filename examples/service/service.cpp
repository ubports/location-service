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

#include <location/provider_factory.h>
#include <location/boost_ptree_settings.h>

#include <location/service/default_configuration.h>
#include <location/service/implementation.h>

#include <core/dbus/announcer.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace dbus = core::dbus;

namespace
{
struct NullReporter : public location::service::Harvester::Reporter
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
    void report(const location::Update<location::Position>&,
                const std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>&,
                const std::vector<com::ubuntu::location::connectivity::RadioCell::Ptr>&)
    {
    }
};
}

int main(int argc, char** argv)
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add(
        "bus", 
        "The well-known bus to announce the service upon", 
        std::string{"session"});
    options.add(
        "config-file",
        "The configuration we should read from/write to",
        std::string{"/var/run/ubuntu-location-service/config.ini"});
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

    location::service::DefaultConfiguration config;
    auto settings = std::make_shared<location::BoostPtreeSettings>(options.value_for_key<std::string>("config-file"));
    auto engine = config.the_engine(std::set<location::Provider::Ptr>{}, config.the_provider_selection_policy(), settings);

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
            auto result = std::async(std::launch::async, [provider, config_lut, engine] {
                return location::ProviderFactory::instance().create_provider_for_name_with_config(
                    provider,
                    config_lut.at(provider),
                    [engine](location::Provider::Ptr provider)
                    {
                        engine->add_provider(provider);
                    });
            });
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

    location::service::Implementation::Configuration configuration
    {
        incoming,
        outgoing,
        engine,
        config.the_permission_manager(incoming),
        location::service::Harvester::Configuration
        {
            com::ubuntu::location::connectivity::platform_default_manager(),
            std::make_shared<NullReporter>()
        }
    };

    auto location_service = std::make_shared<location::service::Implementation>(configuration);
    
    std::thread t1{[incoming](){incoming->run();}};
    std::thread t2{[outgoing](){outgoing->run();}};
    
    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    return EXIT_SUCCESS;
}

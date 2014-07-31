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

#include <core/posix/signal.h>

#include <system_error>
#include <thread>

#include <signal.h>
#include <sys/signalfd.h>

namespace location = com::ubuntu::location;
namespace dbus = core::dbus;

namespace
{
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

location::ProgramOptions init_daemon_options()
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add("testing", "Enables running the service without providers");
    options.add_composed<std::vector<std::string>>("provider",
                                                   "The providers that should be added to the engine");

    return options;
}

location::ProgramOptions& mutable_daemon_options()
{
    static location::ProgramOptions options = init_daemon_options();
    return options;
}
}

location::service::Daemon::Configuration location::service::Daemon::Configuration::from_command_line_args(
        int argc,
        const char** argv,
        location::service::Daemon::DBusConnectionFactory factory)
{
    location::service::Daemon::Configuration result;

    if (!mutable_daemon_options().parse_from_command_line_args(argc, (const char**)argv))
        throw std::runtime_error{"Could not parse command-line, aborting..."};

    result.incoming = factory(mutable_daemon_options().bus());
    result.outgoing= factory(mutable_daemon_options().bus());

    if (mutable_daemon_options().value_count_for_key("testing") == 0 && mutable_daemon_options().value_count_for_key("provider") == 0)
    {
        std::stringstream ss;
        ss << "A set of providers need to be specified. The following providers are known:" << std::endl;
        location::ProviderFactory::instance().enumerate(
                    [&ss](const std::string& name, const location::ProviderFactory::Factory&)
        {
            ss << "\t" << name << std::endl;
        });
        throw std::runtime_error{ss.str()};
    }

    if(mutable_daemon_options().value_count_for_key("provider") > 0)
    {
        result.providers = mutable_daemon_options().value_for_key<std::vector<std::string>>("provider");

        for (const std::string& provider : result.providers)
        {
            mutable_daemon_options().enumerate_unrecognized_options(
                        [&result, provider](const std::string& s)
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

                result.provider_options[provider].put(key, value);
            });
        }
    }

    return result;
}

void location::service::Daemon::print_help(std::ostream& out)
{
    mutable_daemon_options().print_help(out);
}

int location::service::Daemon::main(const location::service::Daemon::Configuration& config)
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_term,
        core::posix::Signal::sig_int
    });

    trap->signal_raised().connect([trap](const core::posix::Signal&)
    {
        trap->stop();
    });

    const location::Configuration empty_provider_configuration;

    std::set<location::Provider::Ptr> instantiated_providers;

    for (const std::string& provider : config.providers)
    {
        std::cout << "Instantiating and configuring: " << provider << std::endl;

        try
        {
            auto p = location::ProviderFactory::instance().create_provider_for_name_with_config(
                        provider,
                        config.provider_options.count(provider) > 0 ?
                            config.provider_options.at(provider) : empty_provider_configuration);

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

    config.incoming->install_executor(dbus::asio::make_executor(config.incoming));
    config.outgoing->install_executor(dbus::asio::make_executor(config.outgoing));

    location::service::DefaultConfiguration dc;

    location::service::Implementation::Configuration configuration
    {
        config.incoming,
        config.outgoing,
        dc.the_engine(instantiated_providers, dc.the_provider_selection_policy()),
        dc.the_permission_manager(config.incoming),
        location::service::Harvester::Configuration
        {
            location::connectivity::platform_default_manager(),
            std::make_shared<NullReporter>()
        }
    };

    location::service::Implementation location_service
    {
        configuration
    };

    std::thread t1{[&config](){config.incoming->run();}};
    std::thread t2{[&config](){config.incoming->run();}};
    std::thread t3{[&config](){config.incoming->run();}};
    std::thread t4{[&config](){config.outgoing->run();}};

    trap->run();

    config.incoming->stop();
    config.outgoing->stop();

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    if (t3.joinable())
        t3.join();

    if (t4.joinable())
        t4.join();

    return EXIT_SUCCESS;
}

namespace
{
location::ProgramOptions init_cli_options()
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add("property",
                "Property to set/get from a running service, known properties are:\n"
                "   is_online [get/set]\n"
                "   does_satellite_based_positioning [get/set]\n"
                "   does_report_wifi_and_cell_ids [get/set]\n"
                "   visible_space_vehicles [get]",
                location::service::Daemon::Cli::Property::unknown);
    options.add<std::string>("set", "Adjust the value of the property.");
    options.add("get", "Query the value of the property.");

    return options;
}

location::ProgramOptions& mutable_cli_options()
{
    static location::ProgramOptions options = init_cli_options();
    return options;
}
}

location::service::Daemon::Cli::Configuration location::service::Daemon::Cli::Configuration::from_command_line_args(
        int argc,
        const char** argv,
        location::service::Daemon::DBusConnectionFactory factory)
{
    location::service::Daemon::Cli::Configuration result;

    if (!mutable_cli_options().parse_from_command_line_args(argc, (const char**)argv))
    {
        throw std::runtime_error{"Error parsing command line"};
    }

    if (mutable_cli_options().value_count_for_key("help") > 0)
    {
        throw std::runtime_error{"Error parsing command line"};
    }

    if (mutable_cli_options().value_count_for_key("get") > 0 && mutable_cli_options().value_count_for_key("set") > 0)
    {
        throw std::logic_error
        {
            "Both set and get specified, aborting..."
        };
    }

    result.bus = factory(mutable_cli_options().bus());

    result.property = mutable_cli_options().value_for_key<location::service::Daemon::Cli::Property>("property");

    if (mutable_cli_options().value_count_for_key("get") > 0)
    {
        result.command = Command::get;
    }
    else if (mutable_cli_options().value_count_for_key("set") > 0)
    {
        result.command = Command::set;
        result.new_value = mutable_cli_options().value_for_key<std::string>("set");
    }

    return result;
}

/** @brief Pretty-prints the CLI's help text to the given output stream. */
void location::service::Daemon::Cli::print_help(std::ostream& out)
{
    mutable_cli_options().print_help(out);
}

int location::service::Daemon::Cli::main(const location::service::Daemon::Cli::Configuration& config)
{
    auto location_service =
            dbus::resolve_service_on_bus<location::service::Interface, location::service::Stub>(config.bus);

    switch (config.property)
    {
    case Property::is_online:
        switch (config.command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service is "
                      << (location_service->is_online() ? "online" : "offline") << std::endl;
            break;
        case Command::set:
        {
            std::stringstream ss(config.new_value);
            bool flag = location_service->is_online();
            ss >> std::boolalpha >> flag;

            std::cout << "Adjusting is_online property to value: " << config.new_value << " -> ";

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
        switch (config.command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service "
                      << (location_service->does_report_cell_and_wifi_ids() ? "does" : "does not")
                      << " report cell and wifi ids." << std::endl;
            break;
        case Command::set:
        {
            std::stringstream ss(config.new_value);
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
        switch (config.command)
        {
        case Command::get:
            std::cout << std::boolalpha << "Location service "
                      << (location_service->does_satellite_based_positioning() ? "does" : "does not")
                      << " satellite based positioning." << std::endl;
            break;
        case Command::set:
        {
            std::stringstream ss(config.new_value);
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
        switch (config.command)
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
            location::service::Daemon::Cli::print_help(std::cout);
            return EXIT_FAILURE;
        case Command::unknown: break;
        }
        break;
    }
    case Property::unknown:
        std::cout << "Unknown property, aborting now." << std::endl;
        location::service::Daemon::Cli::print_help(std::cout);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/** @brief Parses a Cli property from the given input stream, throws std::runtime_error. */
std::istream& location::service::operator>>(std::istream& in, location::service::Daemon::Cli::Property& property)
{
    static const std::map<std::string, location::service::Daemon::Cli::Property> lut =
    {
        {"is_online", location::service::Daemon::Cli::Property::is_online},
        {"does_satellite_based_positioning", location::service::Daemon::Cli::Property::does_satellite_based_positioning},
        {"does_report_wifi_and_cell_ids", location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids},
        {"visible_space_vehicles", location::service::Daemon::Cli::Property::visible_space_vehicles}
    };

    std::string value; in >> value;

    auto it = lut.find(value);

    if (it == lut.end()) throw std::runtime_error
    {
        "Unknown property specified: " + value
    };

    property = it->second;
    return in;
}

/** @brief Parses a Cli property from the given input stream, throws std::runtime_error. */
std::ostream& location::service::operator<<(std::ostream& out, location::service::Daemon::Cli::Property property)
{
    switch (property)
    {
    case location::service::Daemon::Cli::Property::is_online:
        out << "is_online"; break;
    case location::service::Daemon::Cli::Property::does_satellite_based_positioning:
        out << "does_satellite_based_positioning"; break;
    case location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids:
        out << "does_report_wifi_and_cell_ids"; break;
    case location::service::Daemon::Cli::Property::visible_space_vehicles:
        out << "visible_space_vehicles"; break;
    case location::service::Daemon::Cli::Property::unknown:
        out << "unknown"; break;
    }

    return out;
}

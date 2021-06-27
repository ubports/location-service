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

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/boost_ptree_settings.h>
#include <com/ubuntu/location/provider_factory.h>

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/connectivity/dummy_connectivity_manager.h>

#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/demultiplexing_reporter.h>
#include <com/ubuntu/location/service/ichnaea_reporter.h>
#include <com/ubuntu/location/service/implementation.h>
#include <com/ubuntu/location/service/stub.h>
#include <com/ubuntu/location/service/system_configuration.h>

#include <com/ubuntu/location/service/runtime_tests.h>

#include "program_options.h"
#include "daemon.h"
#include "runtime.h"

#include <core/dbus/announcer.h>
#include <core/dbus/resolver.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/signal.h>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

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
    
    std::string config_path = location::service::SystemConfiguration::instance().runtime_persistent_data_dir().string();
    options.add("config-file",
                "The configuration we should read from/write to",
                config_path.append("/config.ini"));
    options.add_composed<std::vector<std::string>>(
                "provider",
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
        location::service::DBusConnectionFactory factory)
{
    location::service::Daemon::Configuration result;

    // Make sure options are cleared between runs (needed for testing)
    mutable_daemon_options().clear();

    if (!mutable_daemon_options().parse_from_command_line_args(argc, (const char**)argv))
        throw std::runtime_error{"Could not parse command-line, aborting..."};

    result.incoming = factory(mutable_daemon_options().bus());
    result.outgoing = factory(mutable_daemon_options().bus());

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

    auto settings = std::make_shared<location::BoostPtreeSettings>(mutable_daemon_options().value_for_key<std::string>("config-file"));
    result.settings = std::make_shared<location::SyncingSettings>(settings);

    return result;
}

void location::service::Daemon::print_help(std::ostream& out)
{
    mutable_daemon_options().print_help(out);
}

void location::service::Daemon::load_providers(const Configuration& config, std::shared_ptr<Engine> engine)
{
    for (const std::string& provider : config.providers)
    {
        std::cout << "Instantiating and configuring: " << provider << std::endl;

        try
        {
            auto result = std::async(std::launch::async, [provider, config, engine] {
                return location::ProviderFactory::instance().create_provider_for_name_with_config(
                    provider,
                    config.provider_options.count(provider) > 0 ?
                        config.provider_options.at(provider) : location::Configuration {},
                    [engine](Provider::Ptr provider)
                    {
                        engine->add_provider(provider);
                    }
                );
            });
        } catch(const std::runtime_error& e)
        {
            std::cerr << "Issue instantiating provider: " << e.what() << std::endl;
        }
    }
}

int location::service::Daemon::main(const location::service::Daemon::Configuration& config)
{
    // Ensure that log files dating back to before the fix
    // for lp:1447110 are removed and do not waste space.
    {
        static const boost::filesystem::path old_log_dir{"/var/log/ubuntu-location-service"};
        boost::system::error_code ec;
        boost::filesystem::remove_all(old_log_dir, ec);
    }
    // Setup logging for the daemon.
    FLAGS_logtostderr = true;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_max_log_size = 5;

    google::InitGoogleLogging("com.ubuntu.location");

    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_term,
        core::posix::Signal::sig_int
    });

    trap->signal_raised().connect([trap](const core::posix::Signal&)
    {
        trap->stop();
    });

    auto runtime = location::service::Runtime::create(4);

    location::service::DefaultConfiguration dc;
    auto engine = dc.the_engine(std::set<location::Provider::Ptr>{}, dc.the_provider_selection_policy(), config.settings);
    load_providers(config, engine);

    config.incoming->install_executor(dbus::asio::make_executor(config.incoming, runtime->service()));
    config.outgoing->install_executor(dbus::asio::make_executor(config.outgoing, runtime->service()));

    runtime->start();

    location::service::Implementation::Configuration configuration
    {
        config.incoming,
        config.outgoing,
        engine,
        dc.the_permission_manager(config.outgoing),
        location::service::Harvester::Configuration
        {
            std::make_shared<dummy::ConnectivityManager>(),
            std::make_shared<NullReporter>()
        }
    };

    auto location_service = std::make_shared<location::service::Implementation>(configuration);

    trap->run();

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
                "   state [get]\n"
                "   is_online [get/set]\n"
                "   does_satellite_based_positioning [get/set]\n"
                "   does_report_wifi_and_cell_ids [get/set]\n"
                "   visible_space_vehicles [get]\n"
                "   client_applications [get]",
                location::service::Daemon::Cli::Property::unknown);
    options.add<std::string>("set", "Adjust the value of the property.");
    options.add("get", "Query the value of the property.");
    options.add("test", "Executes runtime tests.");

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
        location::service::DBusConnectionFactory factory)
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
    else if (mutable_cli_options().value_count_for_key("test") > 0)
    {
        result.command = Command::test;
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
    if (config.command == Command::test)
        return location::service::execute_runtime_tests();

    auto location_service =
            dbus::resolve_service_on_bus<location::service::Interface, location::service::Stub>(config.bus);

    switch (config.property)
    {
    case Property::state:
        switch (config.command)
        {
        case Command::get:
            std::cout << "Location service is " << location_service->state() << std::endl;
            break;
        default:
            break;
        }
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
        default:
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
        default:
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
        default:
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
        default:
        case Command::unknown: break;
        }
        break;
    }
    case Property::client_applications:
    {
        switch (config.command)
        {
        case Command::get:
        {
            auto svs = location_service->client_applications().get();
            std::cout << "Client applications:" << std::endl;
            for (const auto& sv : svs)
                std::cout << "\t" << sv << std::endl;
            break;
        }
        case Command::set:
            std::cout << "Property client_applications is not set-able, aborting now." << std::endl;
            location::service::Daemon::Cli::print_help(std::cout);
            return EXIT_FAILURE;
        default:
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
        {"state", location::service::Daemon::Cli::Property::state},
        {"is_online", location::service::Daemon::Cli::Property::is_online},
        {"does_satellite_based_positioning", location::service::Daemon::Cli::Property::does_satellite_based_positioning},
        {"does_report_wifi_and_cell_ids", location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids},
        {"visible_space_vehicles", location::service::Daemon::Cli::Property::visible_space_vehicles},
        {"client_applications", location::service::Daemon::Cli::Property::client_applications},
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
    case location::service::Daemon::Cli::Property::state:
        out << "state"; break;
    case location::service::Daemon::Cli::Property::is_online:
        out << "is_online"; break;
    case location::service::Daemon::Cli::Property::does_satellite_based_positioning:
        out << "does_satellite_based_positioning"; break;
    case location::service::Daemon::Cli::Property::does_report_wifi_and_cell_ids:
        out << "does_report_wifi_and_cell_ids"; break;
    case location::service::Daemon::Cli::Property::visible_space_vehicles:
        out << "visible_space_vehicles"; break;
    case location::service::Daemon::Cli::Property::client_applications:
        out << "client_applications"; break;
    case location::service::Daemon::Cli::Property::unknown:
        out << "unknown"; break;
    }

    return out;
}

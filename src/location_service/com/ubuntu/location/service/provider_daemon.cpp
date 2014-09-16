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

#include <com/ubuntu/location/service/provider_daemon.h>

#include <com/ubuntu/location/configuration.h>
#include <com/ubuntu/location/provider_factory.h>

#include <com/ubuntu/location/providers/remote/skeleton.h>

#include <com/ubuntu/location/service/configuration.h>
#include <com/ubuntu/location/service/program_options.h>

#include <core/posix/signal.h>

namespace location = com::ubuntu::location;

namespace
{
location::ProgramOptions init_daemon_options()
{
    location::ProgramOptions options;

    options.add("help", "Produces this help message");
    options.add<std::string>("service-name",
                             "The name of the service under which the provider should be exposed.");
    options.add<std::string>("service-path",
                             "The dbus object path under which the provider is known.");
    options.add<std::string>("provider",
                             "The provider that should be exposed to the bus");

    return options;
}

location::ProgramOptions& mutable_daemon_options()
{
    static location::ProgramOptions options = init_daemon_options();
    return options;
}
}

location::service::ProviderDaemon::Configuration location::service::ProviderDaemon::Configuration::from_command_line_args(
        int argc, const char** argv, location::service::DBusConnectionFactory factory)
{
    if (!mutable_daemon_options().parse_from_command_line_args(argc, argv))
        throw std::runtime_error{"Could not parse command-line, aborting..."};

    location::service::ProviderDaemon::Configuration result;

    result.connection = factory(mutable_daemon_options().bus());

    auto service = core::dbus::Service::add_service(
                result.connection,
                mutable_daemon_options().value_for_key<std::string>("service-name"));

    result.object = service->add_object_for_path(core::dbus::types::ObjectPath
    {
        mutable_daemon_options().value_for_key<std::string>("service-path")
    });

    auto provider_name = mutable_daemon_options().value_for_key<std::string>("provider");
    location::Configuration config;

    mutable_daemon_options().enumerate_unrecognized_options([&config, provider_name](const std::string& s)
    {
        std::stringstream in(s);
        std::string key, value;

        std::getline(in, key, '=');
        std::getline(in, value, '=');

        std::size_t pos = key.find(provider_name);
        if (pos == std::string::npos)
            return;

        static const std::string option_marker{"--"};
        static const std::string scope_separator{"::"};

        key = key.erase(key.find_first_of(option_marker), option_marker.size());
        key = key.erase(key.find_first_of(provider_name), provider_name.size());
        key = key.erase(key.find_first_of(scope_separator), scope_separator.size());

        config.put(key, value);
    });

    result.provider = location::ProviderFactory::instance().create_provider_for_name_with_config(
                provider_name,
                config);

    return result;
}

int location::service::ProviderDaemon::main(const location::service::ProviderDaemon::Configuration& configuration)
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

    std::thread worker
    {
        [configuration]()
        {
            configuration.connection->run();
        }
    };

    auto skeleton = location::providers::remote::skeleton::create_with_configuration(location::providers::remote::skeleton::Configuration
    {
        configuration.object,
        configuration.connection,
        configuration.provider
    });

    trap->run();

    configuration.connection->stop();

    if (worker.joinable())
        worker.join();

    return EXIT_SUCCESS;
}

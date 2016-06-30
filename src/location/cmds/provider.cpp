/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
 *
 */

#include <location/cmds/provider.h>

#include <location/provider_factory.h>
#include <location/runtime.h>

#include <location/providers/remote/skeleton.h>

#include <core/dbus/service.h>
#include <core/dbus/asio/executor.h>
#include <core/posix/signal.h>

namespace cli = location::util::cli;

namespace
{
void die_if(bool b, std::ostream& out, const std::string& message)
{
    if (b) { out << message << std::endl; std::exit(EXIT_FAILURE); }
}
}

location::cmds::Provider::Provider()
    : CommandWithFlagsAndAction{cli::Name{"provider"}, cli::Usage{"provider"}, cli::Description{"executes a built-in provider"}},
      bus{core::dbus::WellKnownBus::system}
{
    flag(cli::make_flag(cli::Name{"bus"}, cli::Description{"bus instance to connect to, defaults to system"}, bus));
    flag(cli::make_flag(cli::Name{"service"}, cli::Description{"name of the service hosting the provider."}, service));
    flag(cli::make_flag(cli::Name{"path"}, cli::Description{"dbus object path hosting the provider."}, path));
    flag(cli::make_flag(cli::Name{"id"}, cli::Description{"id of the actual provider implementation."}, id));

    action([this](const Context& ctxt)
    {
        die_if(not service, ctxt.cout, "service name is missing");
        die_if(not path, ctxt.cout, "object path is missing");
        die_if(not id, ctxt.cout, "name of actual provider implementation is missing");

        // We exit cleanly for SIGINT and SIGTERM.
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_int, core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto impl = location::ProviderFactory::instance().create_provider_for_name_with_config(*id, location::Configuration{});

        auto rt = location::Runtime::create();
        rt->start();

        auto incoming = std::make_shared<core::dbus::Bus>(bus);
        incoming->install_executor(core::dbus::asio::make_executor(incoming, rt->service()));
        auto object = core::dbus::Service::add_service(incoming, *service)
                        ->add_object_for_path(core::dbus::types::ObjectPath{*path});

        location::providers::remote::skeleton::Configuration config{object, incoming, impl};
        auto skeleton = location::providers::remote::skeleton::create_with_configuration(config);

        trap->run();

        incoming->stop(); rt->stop();

        return EXIT_SUCCESS;
    });
}


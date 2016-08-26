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

#include <location/cmds/run.h>

#include <location/boost_ptree_settings.h>
#include <location/fusion_provider_selection_policy.h>
#include <location/runtime.h>
#include <location/serializing_bus.h>
#include <location/service_with_engine.h>
#include <location/settings.h>
#include <location/system_configuration.h>
#include <location/trust_store_permission_manager.h>

#include <location/dbus/skeleton/service.h>
#include <location/providers/dummy/provider.h>
#include <location/util/well_known_bus.h>

#include <core/dbus/asio/executor.h>
#include <core/posix/signal.h>

namespace cli = location::util::cli;

location::cmds::Run::Run()
    : CommandWithFlagsAndAction{cli::Name{"run"}, cli::Usage{"run"}, cli::Description{"runs the daemon"}},
      testing{false},
      bus{core::dbus::WellKnownBus::system},
      settings{SystemConfiguration::instance().runtime_persistent_data_dir()}
{
    flag(cli::make_flag(cli::Name{"testing"}, cli::Description{"whether we are running under testing, defaults to false"}, testing));
    flag(cli::make_flag(cli::Name{"bus"}, cli::Description{"bus instance to connect to, defaults to system"}, bus));
    flag(cli::make_flag(cli::Name{"config"}, cli::Description{"daemon configuration"}, config));
    flag(cli::make_flag(cli::Name{"settings"}, cli::Description{"path to runtime persistent state data"}, settings));

    action([this](const Context& ctxt)
    {
        account_for_lp1447110();

        // We exit cleanly for SIGINT and SIGTERM.
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });        

        auto rt = location::Runtime::create();

        // The engine instance is the core piece of functionality.
        auto engine = std::make_shared<location::Engine>(
            // We default to a fusion provider selection policy, providing
            // fusioned and filtered updates to sessions.
            std::make_shared<location::FusionProviderSelectionPolicy>(),
            // We serialize all messages passed through our internal bus via a specific strand on the runtime.
            location::SerializingBus::create(rt),
            // We default to a location::Settings implementation that reads state from
            // an ini file, immediately syncing back changes to the underlying file whenever
            // parameters change.
            std::make_shared<location::SyncingSettings>(std::make_shared<location::BoostPtreeSettings>(settings.string())));

        // We make sure that at least one provider is available to clients.
        if (testing)
        {
            ctxt.cout << "Running under testing..." << std::endl;
            engine->add_provider(std::make_shared<location::providers::dummy::Provider>());
        }        

        auto incoming = std::make_shared<core::dbus::Bus>(bus);
        auto outgoing = std::make_shared<core::dbus::Bus>(bus);

        incoming->install_executor(core::dbus::asio::make_executor(incoming, rt->service()));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing, rt->service()));

        auto service = core::dbus::Service::add_service<location::dbus::Service>(incoming);

        location::dbus::skeleton::Service::Configuration config
        {
            std::make_shared<location::ServiceWithEngine>(engine),
            incoming,
            outgoing,
            service,
            // We resolve credentials of incoming calls by talking back to the dbus daemon and subsequently
            // querying apparmor for the confinement profile.
            std::make_shared<location::dbus::skeleton::Service::DBusDaemonCredentialsResolver>(outgoing),
            std::make_shared<location::dbus::skeleton::Service::ObjectPathGenerator>(),
            // Incoming requests are verified by talking to a trust-store instance.
            SystemConfiguration::instance().create_permission_manager(outgoing)
        };

        rt->start();

        auto skeleton = std::make_shared<location::dbus::skeleton::Service>(config);

        trap->run();

        incoming->stop(); outgoing->stop(); rt->stop();

        return EXIT_SUCCESS;
    });
}

void location::cmds::Run::account_for_lp1447110() const
{
    static const boost::filesystem::path old_log_dir{"/var/log/ubuntu-location-service"};
    boost::system::error_code ec;
    boost::filesystem::remove_all(old_log_dir, ec);
}

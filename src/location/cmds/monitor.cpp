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

#include <location/cmds/monitor.h>

#include <location/criteria.h>
#include <location/dbus/stub/service.h>
#include <location/runtime.h>
#include <location/util/well_known_bus.h>

#include <core/dbus/asio/executor.h>
#include <core/posix/signal.h>

#include <type_traits>

namespace cli = location::util::cli;

namespace
{
struct Retry
{
    template<unsigned int retry_count, typename Operation>
    static auto times(const Operation& op) -> typename std::result_of<Operation>::type
    {
        for (unsigned int count = 0; count < retry_count; count++)
            try { return op(); }
            catch(...) {}

        throw std::runtime_error{"Operation failed"};
    }
};
}

location::cmds::Monitor::PrintingDelegate::PrintingDelegate(std::ostream& out) : out{out}
{
}

void location::cmds::Monitor::PrintingDelegate::on_new_position(const Update<Position>& pos)
{
    out << pos << std::endl;
}

void location::cmds::Monitor::PrintingDelegate::on_new_heading(const Update<units::Degrees>& heading)
{
    out << heading << std::endl;
}

void location::cmds::Monitor::PrintingDelegate::on_new_velocity(const Update<units::MetersPerSecond>& velocity)
{
    out << velocity << std::endl;
}

location::cmds::Monitor::Monitor(const std::shared_ptr<Delegate>& delegate)
    : CommandWithFlagsAndAction{cli::Name{"monitor"}, cli::Usage{"monitor"}, cli::Description{"monitors the daemon"}},
      delegate{delegate},
      bus{core::dbus::WellKnownBus::system}
{
    flag(cli::make_flag(cli::Name{"bus"}, cli::Description{"bus instance to connect to, defaults to system"}, bus));
    action([this](const Context& ctxt)
    {
        // We exit cleanly for SIGINT and SIGTERM.
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_int, core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto rt = location::Runtime::create();

        auto conn = std::make_shared<core::dbus::Bus>(bus);
        conn->install_executor(core::dbus::asio::make_executor(conn, rt->service()));

        rt->start();

        core::dbus::Service::Ptr service = core::dbus::Service::use_service<location::dbus::Service>(conn);

        auto stub = std::make_shared<location::dbus::stub::Service>(conn, service, service->object_for_path(core::dbus::types::ObjectPath{location::dbus::Service::path()}));
        auto session = stub->create_session_for_criteria(location::Criteria{});

        session->updates().position.changed().connect([this](const location::Update<location::Position>& pos)
        {
            Monitor::delegate->on_new_position(pos);
        });

        session->updates().heading.changed().connect([this](const location::Update<location::units::Degrees>& heading)
        {
            Monitor::delegate->on_new_heading(heading);
        });

        session->updates().velocity.changed().connect([this](const location::Update<location::units::MetersPerSecond>& velocity)
        {
            Monitor::delegate->on_new_velocity(velocity);
        });

        session->updates().position_status = location::Service::Session::Updates::Status::enabled;
        session->updates().heading_status = location::Service::Session::Updates::Status::enabled;
        session->updates().velocity_status = location::Service::Session::Updates::Status::enabled;

        ctxt.cout << "Enabled position/heading/velocity updates..." << std::endl;

        trap->run();

        session->updates().position_status = location::Service::Session::Updates::Status::disabled;
        session->updates().heading_status = location::Service::Session::Updates::Status::disabled;
        session->updates().velocity_status = location::Service::Session::Updates::Status::disabled;

        conn->stop(); rt->stop();

        return EXIT_SUCCESS;
    });
}

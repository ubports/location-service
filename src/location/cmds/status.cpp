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

#include <location/cmds/status.h>
#include <location/dbus/stub/service.h>
#include <location/service/runtime.h>
#include <location/util/well_known_bus.h>

#include <core/dbus/asio/executor.h>

namespace cli = location::util::cli;

location::cmds::Status::Status()
    : CommandWithFlagsAndAction{cli::Name{"status"}, cli::Usage{"status"}, cli::Description{"Query status of the daemon"}},
      bus{core::dbus::WellKnownBus::system}
{
    flag(cli::make_flag(cli::Name{"bus"}, cli::Description{"bus instance to connect to, defaults to system"}, bus));
    action([this](const Context& ctxt)
    {
        auto rt = location::service::Runtime::create();

        auto conn = std::make_shared<core::dbus::Bus>(bus);

        conn->install_executor(core::dbus::asio::make_executor(conn, rt->service()));

        auto service = core::dbus::Service::use_service<location::dbus::Service>(conn);
        auto stub = std::make_shared<location::dbus::stub::Service>(conn, service, service->object_for_path(core::dbus::types::ObjectPath{location::dbus::Service::path()}));

        rt->start();

        ctxt.cout << "  is online:                   " << std::boolalpha << stub->is_online() << std::endl
                  << "  state:                       " << stub->state() << std::endl
                  << "  satellite based positioning: " << std::boolalpha << stub->does_satellite_based_positioning() << std::endl
                  << "  reports cell & wifi ids:     " << std::boolalpha << stub->does_report_cell_and_wifi_ids() << std::endl;
        auto svs = stub->visible_space_vehicles().get();
        if (svs.size() > 0)
        {
            ctxt.cout << "  svs:" << std::endl;
            for (const auto& pair : svs)
                ctxt.cout << "    " << pair.second << std::endl;
        }
        else
        {
            ctxt.cout << "  svs:                         " << "none";
        }

        ctxt.cout << std::endl;

        conn->stop(); rt->stop();

        return EXIT_SUCCESS;
    });
}

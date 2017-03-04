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
#include <location/glib/runtime.h>
#include <location/glib/runtime.h>
#include <location/util/well_known_bus.h>

#include <core/dbus/asio/executor.h>

namespace cli = location::util::cli;

location::cmds::Status::PrintingDelegate::PrintingDelegate(std::ostream& out) : out{out}
{
}

// From Delegate.
void location::cmds::Status::PrintingDelegate::on_summary(const Summary& summary)
{
    out << "  is online:                   " << std::boolalpha  << summary.is_online                        << std::endl
        << "  state:                       "                    << summary.state                            << std::endl
        << "  satellite based positioning: " << std::boolalpha  << summary.does_satellite_based_positioning << std::endl
        << "  reports cell & wifi ids:     " << std::boolalpha  << summary.does_report_cell_and_wifi_ids    << std::endl;

    if (summary.svs.size() > 0)
    {
        out << "  svs:" << std::endl;
        for (const auto& pair : summary.svs)
            out << "    " << pair.second << std::endl;
    }
    else
    {
        out << "  svs:                         " << "none";
    }

    out << std::endl;
}

location::cmds::Status::Status(const std::shared_ptr<Delegate>& delegate)
    : CommandWithFlagsAndAction{cli::Name{"status"}, cli::Usage{"status"}, cli::Description{"queries the status of the daemon"}},
      delegate{delegate},
      bus{core::dbus::WellKnownBus::system}
{
    flag(cli::make_flag(cli::Name{"bus"}, cli::Description{"bus instance to connect to, defaults to system"}, bus));
    action([this](const Context& ctxt)
    {
        location::glib::Runtime runtime;

        location::dbus::stub::Service::create([this, &ctxt](const location::Result<location::dbus::stub::Service::Ptr>& result)
        {
            if (result)
            {
                auto service = result.value();

                Status::delegate->on_summary(Summary
                {
                    service->is_online(),
                    service->state(),
                    service->does_satellite_based_positioning(),
                    service->does_report_cell_and_wifi_ids(),
                    service->visible_space_vehicles()
                });
            }
            else
            {
                try
                {
                    result.rethrow();
                }
                catch(const std::exception& e)
                {
                    ctxt.cout << "Error querying status: " << e.what() << std::endl;
                }
                catch(...)
                {
                    ctxt.cout << "Error querying status.";
                }
            }

            location::glib::Runtime::instance()->stop();
        });

        return runtime.run();
    });
}

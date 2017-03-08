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

#ifndef LOCATION_CMDS_STATUS_H_
#define LOCATION_CMDS_STATUS_H_

#include <location/optional.h>
#include <location/service.h>
#include <location/dbus/bus.h>
#include <location/util/cli.h>
#include <location/visibility.h>

#include <boost/filesystem.hpp>

#include <iosfwd>

namespace location
{
namespace cmds
{
// Status queries the status of the daemon
class LOCATION_DLL_PUBLIC Status : public util::cli::CommandWithFlagsAndAction
{
public:
    // Summary bundles the state summary of a location::Service instance.
    struct Summary
    {
        bool is_online;
        Service::State state;
        bool does_satellite_based_positioning;
        bool does_report_cell_and_wifi_ids;
        std::map<SpaceVehicle::Key, SpaceVehicle> svs;
    };

    // Delegate abstracts handling of an incoming state summary.
    class Delegate : public util::DoNotCopyOrMove
    {
    public:
        // on_summary is called whenever a state summary of a service instance has been assembled.
        virtual void on_summary(const Summary& summary) = 0;
    };

    // PrintingDelegate is a Delegate printing to a std::ostream.
    class PrintingDelegate : public Delegate
    {
    public:
        // PrintingDelegate initializes a new instance with out.
        PrintingDelegate(std::ostream& out = std::cout);

        // From Delegate.
        void on_summary(const Summary& summary) override;

    private:
        std::ostream& out; // The output stream we print to.
    };

    // Status initializes a new instance.
    Status(const std::shared_ptr<Delegate>& delegate = std::make_shared<PrintingDelegate>());

private:
    std::shared_ptr<Delegate> delegate; // We dispatch summary information to this delegate implementation.
    dbus::Bus bus;                      // The bus we should connect to.
};
}
}

#endif // LOCATION_CMDS_STATUS_H_

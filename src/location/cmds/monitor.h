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

#ifndef LOCATION_CMDS_MONITOR_H_
#define LOCATION_CMDS_MONITOR_H_

#include <location/optional.h>
#include <location/position.h>
#include <location/update.h>
#include <location/visibility.h>

#include <location/dbus/bus.h>
#include <location/util/cli.h>

#include <iosfwd>

namespace location
{
namespace cmds
{
// Monitor connects to a running locationd instance, monitoring its activity.
class LOCATION_DLL_PUBLIC Monitor : public util::cli::CommandWithFlagsAndAction
{
public:
    // Delegate abstracts handling of incoming updates.
    class Delegate : public util::DoNotCopyOrMove
    {
    public:
        // on_new_position is invoked for every incoming position update.
        virtual void on_new_position(const Update<Position>& pos) = 0;
        // on_new_heading is invoked for every incoming heading update.
        virtual void on_new_heading(const Update<units::Degrees>& heading) = 0;
        // on_new_velocity is invoked for every incoming velocity update.
        virtual void on_new_velocity(const Update<units::MetersPerSecond>& velocity) = 0;
    };

    // PrintingDelegate implements Delegate, printing updates to the given output stream.
    class PrintingDelegate : public Delegate
    {
    public:
        // PrintingDelegate initializes a new instance with out.
        PrintingDelegate(std::ostream& out = std::cout);

        // From Delegate
        void on_new_position(const Update<Position>& pos) override;
        void on_new_heading(const Update<units::Degrees>& heading) override;
        void on_new_velocity(const Update<units::MetersPerSecond>& velocity) override;
    private:
        std::ostream& out;
    };

    // Monitor initializes a new instance.
    Monitor(const std::shared_ptr<Delegate>& delegate = std::make_shared<PrintingDelegate>());

private:
    std::shared_ptr<Delegate> delegate; // All updates are forwarded to a delegate.
    dbus::Bus bus;                      // The bus we should connect to.
};
}
}

#endif // LOCATION_CMDS_MONITOR_H_

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

#ifndef LOCATION_CMDS_RUN_H_
#define LOCATION_CMDS_RUN_H_

#include <location/optional.h>
#include <location/util/cli.h>
#include <location/util/well_known_bus.h>
#include <location/visibility.h>

#include <boost/filesystem.hpp>

#include <iosfwd>

namespace location
{
namespace cmds
{
// Run executes locationd, exposing the service via DBus.
class LOCATION_DLL_PUBLIC Run : public util::cli::CommandWithFlagsAndAction
{
public:
    // Run initializes a new instance.
    Run();

    
private:
    // Ensure that log files dating back to before the fix
    // for lp:1447110 are removed and do not waste space.
    void account_for_lp1447110() const;

    bool testing;                               // Whether we are running in a testing environment.
    core::dbus::WellKnownBus bus;               // The bus we should connect to.
    Optional<boost::filesystem::path> config;   // Optionally load configuration from this config file.
    boost::filesystem::path settings;           // Runtime persistent state settings are loaded from this file.
};
}
}

#endif // LOCATION_CMDS_RUN_H_

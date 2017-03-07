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

#ifndef LOCATION_CMDS_PROVIDER_H_
#define LOCATION_CMDS_PROVIDER_H_

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
// Provider executes an in-tree provider out-of-process.
class LOCATION_DLL_PUBLIC Provider : public util::cli::CommandWithFlagsAndAction
{
public:
    // Run initializes a new instance.
    Provider();

private:
    dbus::Bus bus;              // The bus we should connect to.
    Optional<std::string> id;   // The id of the actual provider implementation.
    Service::Ptr service;       // The service instance.
};
}
}

#endif // LOCATION_CMDS_PROVIDER_H_

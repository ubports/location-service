/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef LOCATION_CMDS_SET_H_
#define LOCATION_CMDS_SET_H_

#include <location/util/cli.h>
#include <location/visibility.h>

#include <iosfwd>

namespace location
{
namespace cmds
{

// Set persists a given key/value pair.
class LOCATION_DLL_PUBLIC Set : public util::cli::CommandWithFlagsAndAction
{
public:
    // Set initializes a new instance.
    Set();

private:
    Optional<std::string> key;
    Optional<std::string> value;
};

}  // namespace cmds
}  // namespace location

#endif // LOCATION_CMDS_SET_H_

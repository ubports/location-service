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

#ifndef LOCATION_CMDS_LIST_H_
#define LOCATION_CMDS_LIST_H_

#include <location/util/cli.h>

#include <iosfwd>

namespace location
{
namespace cmds
{
// List lists all known provider implementations
class List : public util::cli::CommandWithFlagsAndAction
{
public:
    // List initializes a new instance.
    List();
};
}
}

#endif // LOCATION_CMDS_LIST_H_

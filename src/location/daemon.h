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

#ifndef LOCATION_DAEMON_H_
#define LOCATION_DAEMON_H_

#include <location/util/cli.h>
#include <location/util/do_not_copy_or_move.h>
#include <location/visibility.h>

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace location
{
/// @brief Daemon implements biometryd.
class LOCATION_DLL_PUBLIC Daemon : private util::DoNotCopyOrMove
{
public:
    /// @brief Daemon creates a new instance, populating the map of known commands.
    Daemon();

    /// @brief run executes the daemon.
    int run(const std::vector<std::string>& args);

private:
    util::cli::CommandWithSubcommands cmd;
};
}

#endif // LOCATION_DAEMON_H_

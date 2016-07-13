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

#include <location/daemon.h>

#include <location/cmds/list.h>
#include <location/cmds/monitor.h>
#include <location/cmds/provider.h>
#include <location/cmds/run.h>
#include <location/cmds/status.h>
#include <location/cmds/test.h>

#include <glog/logging.h>

#include <iostream>

namespace cli = location::util::cli;

location::Daemon::Daemon()
    : cmd{cli::Name{"locationd"}, cli::Usage{"locationd"}, cli::Description{"locationd"}}
{    
    cmd.command(std::make_shared<location::cmds::List>());
    cmd.command(std::make_shared<location::cmds::Monitor>());
    cmd.command(std::make_shared<location::cmds::Provider>());
    cmd.command(std::make_shared<location::cmds::Run>());
    cmd.command(std::make_shared<location::cmds::Status>());
    cmd.command(std::make_shared<location::cmds::Test>());
}

int location::Daemon::run(const std::vector<std::string> &args)
{
    // Setup logging for the daemon.
    FLAGS_logtostderr = true;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_max_log_size = 5;

    google::InitGoogleLogging("locationd");

    return cmd.run(cli::Command::Context{std::cin, std::cout, args});
}

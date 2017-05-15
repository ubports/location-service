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

#include <location/cmds/set.h>

#include <location/util/settings.h>

namespace cli = location::util::cli;

location::cmds::Set::Set()
    : CommandWithFlagsAndAction{cli::Name{"set"}, cli::Usage{"set"}, cli::Description{"persists a key-value pair"}}
{
    flag(cli::make_flag(cli::Name{"key"}, cli::Description{"name of the setting"}, key));
    flag(cli::make_flag(cli::Name{"value"}, cli::Description{"value of the setting"}, value));

    action([this](const Context& ctxt)
    {
        if (not key)
        {
            ctxt.cout << "Missing parameter 'key'" << std::endl;
            return EXIT_FAILURE;
        }

        if (not value)
        {
            ctxt.cout << "Missing parameter 'value'" << std::endl;
            return EXIT_FAILURE;
        }

        util::settings::Source settings;
        settings.set_value(key.get(), value.get());

        return EXIT_SUCCESS;
    });
}

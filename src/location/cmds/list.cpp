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

#include <location/cmds/list.h>
#include <location/provider_factory.h>

namespace cli = location::util::cli;

location::cmds::List::List()
    : CommandWithFlagsAndAction{cli::Name{"list"}, cli::Usage{"list"}, cli::Description{"lists known provider implementations"}}
{
    action([this](const Context& ctxt)
    {
        location::ProviderFactory::instance().enumerate([&ctxt](const std::string& name, const location::ProviderFactory::Factory&)
        {
            ctxt.cout << "  - " << name << std::endl;
        });
        return EXIT_SUCCESS;
    });
}

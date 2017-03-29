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

#include <location/cmds/test.h>
#include <location/runtime_tests.h>

namespace cli = location::util::cli;

location::cmds::Test::Test()
    : CommandWithFlagsAndAction{cli::Name{"test"}, cli::Usage{"test"}, cli::Description{"executes runtime tests against the gps provider."}}
{
    flag(cli::make_flag(cli::Name{"test-suite"}, cli::Description{"test-suite that should be executed"}, test_suite));

    action([this](const Context& ctxt)
    {
        if (not test_suite)
        {
            ctxt.cout << "Missing parameter test-suite." << std::endl;
            return EXIT_FAILURE;
        }

        return execute_runtime_tests(*test_suite, ctxt.cout, ctxt.cout);
    });
}

/*
 * Copyright © 2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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
 */

#include <location/glib/util.h>

#include <boost/format.hpp>

std::exception_ptr location::glib::wrap_error_as_exception(GError* error)
{
    boost::format f("%s[%d]: %s");
    auto result = std::make_exception_ptr(
                std::runtime_error(
                    (f % g_quark_to_string(error->domain) % error->code % error->message).str()));

    g_error_free(error);
    return result;
}

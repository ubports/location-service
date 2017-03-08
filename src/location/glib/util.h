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

#ifndef LOCATION_GLIB_ERROR_H_
#define LOCATION_GLIB_ERROR_H_

#include <glib.h>

#include <exception>

namespace location
{
namespace glib
{

std::exception_ptr wrap_error_as_exception(GError* error);

}  // namespace glib
}  // namespace location

#endif  // LOCATION_GLIB_ERROR_H_

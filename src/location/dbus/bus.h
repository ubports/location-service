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

#ifndef LOCATION_DBUS_BUS_H_
#define LOCATION_DBUS_BUS_H_

#include <gio/gio.h>

#include <iosfwd>

namespace location
{
namespace dbus
{

enum class Bus
{
    session = G_BUS_TYPE_SESSION,
    system = G_BUS_TYPE_SYSTEM
};

std::ostream& operator<<(std::ostream& out, Bus bus);
std::istream& operator>>(std::istream& in, Bus& bus);

}  // namespace dbus
}  // namespace location

#endif  // LOCATION_DBUS_BUS_H_

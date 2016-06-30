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
#ifndef LOCATION_UTIL_WELL_KNOWN_BUS_H_
#define LOCATION_UTIL_WELL_KNOWN_BUS_H_

#include <core/dbus/well_known_bus.h>

#include <iosfwd>

namespace core
{
namespace dbus
{
// operator<< inserts bus into out.
std::ostream& operator<<(std::ostream& out, WellKnownBus bus);
// operator>> extracts bus from in.
std::istream& operator>>(std::istream& in, WellKnownBus& bus);
}
}

#endif // LOCATION_UTIL_WELL_KNOWN_BUS_H_

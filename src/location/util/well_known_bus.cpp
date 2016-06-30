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

#include <location/util/well_known_bus.h>

#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

namespace
{
typedef boost::bimap<std::string, core::dbus::WellKnownBus> Lut;
const Lut& lut()
{
    static Lut instance = boost::assign::list_of<Lut::relation>
            ("session", core::dbus::WellKnownBus::session)
            ("system",  core::dbus::WellKnownBus::system)
            ("starter", core::dbus::WellKnownBus::starter);

    return instance;
}
}

std::ostream& core::dbus::operator<<(std::ostream& out, core::dbus::WellKnownBus bus)
{
    return out << lut().right.at(bus);
}

std::istream& core::dbus::operator>>(std::istream& in, core::dbus::WellKnownBus& bus)
{
    std::string s; in >> s; bus = lut().left.at(s);
    return in;
}

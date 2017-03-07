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

#include <location/dbus/bus.h>

#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

#include <iostream>

namespace
{
typedef boost::bimap<std::string, location::dbus::Bus> Lut;

const Lut& lut()
{
    static Lut instance = boost::assign::list_of<Lut::relation>
            ("session", location::dbus::Bus::session)
            ("system",  location::dbus::Bus::system);

    return instance;
}

}  // namespace

std::ostream& location::dbus::operator<<(std::ostream& out, Bus bus)
{
    return out << lut().right.at(bus);
}

std::istream& location::dbus::operator>>(std::istream& in, Bus& bus)
{
    std::string s; in >> s; bus = lut().left.at(s);
    return in;
}

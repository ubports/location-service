// Copyright (C) 2016 Canonical Ltd.
// 
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <ichnaea/geolocate/fallback.h>

#include <iostream>
#include <type_traits>

std::ostream& ichnaea::geolocate::operator<<(std::ostream& out, Fallback fallback)
{
    switch (fallback)
    {
    case Fallback::ip: return out << "ipf";
    case Fallback::lac: return out << "lacf";
    }

    return out;
}

ichnaea::geolocate::Fallback ichnaea::geolocate::operator|(ichnaea::geolocate::Fallback lhs, ichnaea::geolocate::Fallback rhs)
{
    typedef typename std::underlying_type<Fallback>::type NT;
    return static_cast<Fallback>(static_cast<NT>(lhs) | static_cast<NT>(rhs));
}

ichnaea::geolocate::Fallback ichnaea::geolocate::operator&(ichnaea::geolocate::Fallback lhs, ichnaea::geolocate::Fallback rhs)
{
    typedef typename std::underlying_type<Fallback>::type NT;
    return static_cast<Fallback>(static_cast<NT>(lhs) & static_cast<NT>(rhs));
}

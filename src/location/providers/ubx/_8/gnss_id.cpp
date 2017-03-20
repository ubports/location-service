// Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#include <location/providers/ubx/_8/gnss_id.h>

#include <iostream>

namespace _8 = location::providers::ubx::_8;

std::ostream& _8::operator<<(std::ostream& out, GnssId gnss_id)
{
    switch (gnss_id)
    {
    case GnssId::gps: out << "gps"; break;
    case GnssId::sbas: out << "sbas"; break;
    case GnssId::galileo: out << "galileo"; break;
    case GnssId::beidou: out << "beidou"; break;
    case GnssId::imes: out << "imes"; break;
    case GnssId::qzss: out << "qzss"; break;
    case GnssId::glonass: out << "glonass"; break;
    }

    return out;
}

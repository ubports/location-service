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

#include <location/providers/ubx/_8/nav/sat.h>

#include <location/providers/ubx/_8/reader.h>

#include <iostream>

namespace nav = location::providers::ubx::_8::nav;

void nav::Sat::read(Reader& reader)
{
    itow = reader.read_unsigned_long();
    version = reader.read_unsigned_char();
    info.resize(reader.read_unsigned_char());

    reader.read_unsigned_char();
    reader.read_unsigned_char();

    for (std::size_t i = 0; i < info.size(); i++)
    {
        info[i].gnss_id = static_cast<GnssId>(reader.read_unsigned_char());
        info[i].satellite_id = reader.read_unsigned_char();
        info[i].carrier_to_noise = reader.read_unsigned_char();
        info[i].elevation = reader.read_signed_char();
        info[i].azimuth = reader.read_signed_short();
        info[i].pseudo_range_residual = reader.read_signed_short() * 0.1;
        info[i].flags = reader.read_signed_long();
    }
}

std::ostream& nav::operator<<(std::ostream& out, const Sat& sat)
{
    out << "nav-sat:" << std::endl
        << "  itow: " << sat.itow << std::endl
        << "  version: " << sat.version << std::endl
        << "  satellites: " << std::endl;
    for (const auto& si : sat.info)
        out << "    gnss: " << si.gnss_id << std::endl
            << "    satellite: " << std::uint32_t(si.satellite_id) << std::endl
            << "    cno: " << std::uint32_t(si.carrier_to_noise) << std::endl
            << "    elevation: " << std::int32_t(si.elevation) << std::endl
            << "    azimuth: " << std::int32_t(si.azimuth) << std::endl
            << "    prr: " << si.pseudo_range_residual << std::endl
            << "    flags: " << si.flags << std::endl
            << std::endl;

    return out;
}

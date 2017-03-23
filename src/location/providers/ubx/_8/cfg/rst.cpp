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

#include <location/providers/ubx/_8/cfg/rst.h>

#include <location/providers/ubx/bits.h>
#include <location/providers/ubx/_8/writer.h>

#include <cstdint>
#include <iostream>

namespace cfg = location::providers::ubx::_8::cfg;

std::size_t cfg::Rst::size() const
{
    return 4;
}

void cfg::Rst::write(Writer& writer) const
{
    writer.write_unsigned_short(bits);
    writer.write_unsigned_char(mode);
    writer.write_unsigned_char(0);
}

std::ostream& cfg::operator<<(std::ostream& out, const cfg::Rst& rst)
{
    return out << "cfg-rst:" << std::endl
        << "  bits: " << rst.bits << std::endl
        << "  mode: " << rst.mode << std::endl;
}

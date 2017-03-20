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

#include <location/providers/ubx/_8/cfg/msg.h>

#include <location/providers/ubx/bits.h>
#include <location/providers/ubx/_8/writer.h>

#include <cstdint>
#include <iostream>

namespace cfg = location::providers::ubx::_8::cfg;

std::size_t cfg::Msg::size() const
{
    return 8;
}

void cfg::Msg::write(Writer& writer) const
{
    writer.write_unsigned_char(configured_class_id);
    writer.write_unsigned_char(configured_message_id);
    writer.write_unsigned_char(rate[0]);
    writer.write_unsigned_char(rate[1]);
    writer.write_unsigned_char(rate[2]);
    writer.write_unsigned_char(rate[3]);
    writer.write_unsigned_char(rate[4]);
    writer.write_unsigned_char(rate[5]);
}

std::ostream& cfg::operator<<(std::ostream& out, const cfg::Msg& msg)
{
    return out << "cfg-msg:" << std::endl
        << "  configured class id: " << msg.configured_class_id << std::endl
        << "  configured msg id: " << msg.configured_message_id << std::endl
        << "  rates: " << "[" << msg.rate[0] << "," << msg.rate[1] << "," << msg.rate[2] << "," << msg.rate[3] << "," << msg.rate[4] << "," << msg.rate[5] << "]";
}

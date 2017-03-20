// Copyright (C) 2017 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#include <location/providers/ubx/_8/ack/ack.h>

#include <location/providers/ubx/_8/reader.h>

#include <iostream>

namespace ack = location::providers::ubx::_8::ack;

void ack::Ack::read(Reader& reader)
{
    ackd_class_id = reader.read_unsigned_char();
    ackd_message_id = reader.read_unsigned_char();
}

std::ostream& ack::operator<<(std::ostream& out, const ack::Ack& a)
{
    return out << "ack-ack:" << std::endl
               << "  class_id: " << a.ackd_class_id << std::endl
               << "  message_id: " << a.ackd_message_id << std::endl;
}

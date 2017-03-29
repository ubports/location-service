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

#ifndef UBX_8_ACK_ACK_H_
#define UBX_8_ACK_ACK_H_

#include <cstdint>
#include <iosfwd>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class Reader;

namespace ack
{

struct Ack
{
    static constexpr std::uint8_t class_id{0x05};
    static constexpr std::uint8_t message_id{0x01};

    void read(Reader& reader);

    std::uint8_t ackd_class_id;
    std::uint8_t ackd_message_id;
};

std::ostream& operator<<(std::ostream& out, const Ack& ack);

}  // namespace ack
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_ACK_ACK_H_

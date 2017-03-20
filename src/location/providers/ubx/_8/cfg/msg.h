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

#ifndef UBX_8_CFG_MSG_H_
#define UBX_8_CFG_MSG_H_

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

class Writer;

namespace cfg
{

struct Msg
{
    static constexpr std::uint8_t class_id{0x06};
    static constexpr std::uint8_t message_id{0x01};

    enum Port
    {
        ddc = 0,
        uart1 = 1,
        usb = 3,
        spi = 4
    };

    std::size_t size() const;
    void write(Writer& writer) const;

    std::uint8_t configured_class_id;
    std::uint8_t configured_message_id;
    std::uint8_t rate[6];
};

std::ostream& operator<<(std::ostream& out, const Msg& msg);

}  // namespace cfg
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_CFG_MSG_H_

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
#ifndef UBX_8_SCANNER_H_
#define UBX_8_SCANNER_H_

#include <cstdint>

#include <vector>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{
static constexpr const std::uint8_t sync_char_1{0xb5};
static constexpr const std::uint8_t sync_char_2{62};

enum class Class
{
    cfg = 0x06
};

enum class Id
{
};

struct Message
{
    Class cls;
    Id id;
    std::uint16_t length;
    std::vector<std::uint8_t> payload;
    std::uint8_t ck_a;
    std::uint8_t ck_b;
};
}
}
}
}

#endif // UBX_8_SCANNER_H_

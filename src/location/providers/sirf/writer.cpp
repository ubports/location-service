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

#include <location/providers/sirf/writer.h>

#include <arpa/inet.h>

#include <cstring>
#include <stdexcept>

namespace sirf = location::providers::sirf;

sirf::Writer::Writer(std::vector<std::uint8_t>::iterator begin,
                        std::vector<std::uint8_t>::iterator end)
    : begin{begin}, current{begin}, end{end} {}

void sirf::Writer::write_unsigned_char(std::uint8_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::uint8_t*>(&(*current)) = value;
    current += sizeof(value);

}

void sirf::Writer::write_signed_char(std::int8_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::int8_t*>(&(*current)) = value;
    current += sizeof(value);

}

void sirf::Writer::write_unsigned_short(std::uint16_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::uint16_t*>(&(*current)) = ::ntohs(value);
    current += sizeof(value);
}

void sirf::Writer::write_signed_short(std::int16_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::int16_t*>(&(*current)) = ::ntohs(value);
    current += sizeof(value);
}

void sirf::Writer::write_unsigned_long(std::uint32_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::uint32_t*>(&(*current)) = ::ntohl(value);
    current += sizeof(value);
}

void sirf::Writer::write_signed_long(std::int32_t value)
{
    if (current + sizeof(value) > end)
        throw std::out_of_range{"Write buffer exhausted"};

    *reinterpret_cast<std::int32_t*>(&(*current)) = ::ntohl(value);
    current += sizeof(value);
}

sirf::Writer sirf::Writer::slice(std::size_t size)
{
    if (current + size > end)
        throw std::out_of_range{"Write buffer exhausted"};

    Writer result{current, current + size};
    current += size;

    return result;
}

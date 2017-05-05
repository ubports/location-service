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

#include <location/providers/sirf/reader.h>

#include <arpa/inet.h>

#include <cstring>
#include <stdexcept>

namespace sirf = location::providers::sirf;

sirf::Reader::Reader(std::vector<std::uint8_t>::const_iterator begin,
                        std::vector<std::uint8_t>::const_iterator end)
    : begin{begin}, current{begin}, end{end} {}

std::uint8_t sirf::Reader::read_unsigned_char()
{
    if (current + sizeof(std::uint8_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = *current;
    ++current;
    return result;
}

std::int8_t sirf::Reader::read_signed_char()
{
    if (current + sizeof(std::int8_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = reinterpret_cast<const std::int8_t*>(&(*current));
    current += sizeof(std::int8_t);
    return *result;
}

std::uint16_t sirf::Reader::read_unsigned_short()
{
    if (current + sizeof(std::uint16_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = reinterpret_cast<const std::uint16_t*>(&(*current));
    current += sizeof(std::uint16_t);
    return ::htons(*result);
}

std::int16_t sirf::Reader::read_signed_short()
{
    if (current + sizeof(std::int16_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = reinterpret_cast<const std::int16_t*>(&(*current));
    current += sizeof(std::int16_t);
    return ::htons(*result);
}

std::uint32_t sirf::Reader::read_unsigned_long()
{
    if (current + sizeof(std::uint32_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = reinterpret_cast<const std::uint32_t*>(&(*current));
    current += sizeof(std::uint32_t);
    return ::htonl(*result);
}

std::int32_t sirf::Reader::read_signed_long()
{
    if (current + sizeof(std::int32_t) > end)
        throw std::out_of_range{"Read buffer exhausted"};

    auto result = reinterpret_cast<const std::int32_t*>(&(*current));
    current += sizeof(std::int32_t);
    return ::htonl(*result);
}

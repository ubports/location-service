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

#ifndef UBX_8_CODEC_H_
#define UBX_8_CODEC_H_

#include <location/providers/ubx/_8/checksum.h>
#include <location/providers/ubx/_8/magic.h>
#include <location/providers/ubx/_8/reader.h>
#include <location/providers/ubx/_8/writer.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

template<typename T>
inline T decode_message(const std::vector<std::uint8_t>& payload)
{
    T result;
    Reader reader{payload.begin(), payload.end()};
    result.read(reader);

    return result;
}

template<typename T>
inline std::vector<std::uint8_t> encode_message(const T& message)
{
    auto msg_size = message.size();

    std::uint16_t size =
            sizeof(std::uint8_t) + sizeof(std::uint8_t) +
            sizeof(std::uint8_t) + sizeof(std::uint8_t) +
            sizeof(std::uint16_t) +
            msg_size +
            sizeof(std::uint8_t) + sizeof(std::uint8_t);

    std::vector<std::uint8_t> result(size, 0);

    Writer writer{result.begin(), result.end()};
    writer.write_unsigned_char(sync_char_1);
    writer.write_unsigned_char(sync_char_2);
    writer.write_unsigned_char(T::class_id);
    writer.write_unsigned_char(T::message_id);
    writer.write_unsigned_short(msg_size);

    Writer payload = writer.slice(msg_size);
    message.write(payload);

    auto checksum = std::for_each(result.begin() + 2, result.end() - 2, Checksum{});

    writer.write_unsigned_char(checksum.ck_a());
    writer.write_unsigned_char(checksum.ck_b());

    return result;
}

}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_CODEC_H_

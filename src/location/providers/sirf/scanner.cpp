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

#include <location/providers/sirf/scanner.h>

#include <location/providers/sirf/codec.h>
#include <location/providers/sirf/magic.h>
#include <location/providers/sirf/message.h>

#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace sirf = location::providers::sirf;

sirf::Scanner::Scanner()
    : next{Expect::sync_char_begin_1},
      expected_size{0},
      message_id{0},
      payload_iterator{payload.end()},
      checksum_1{0},
      checksum_2{0}
{
}

std::tuple<sirf::Scanner::Expect, bool> sirf::Scanner::update(std::uint8_t c)
{
    bool consumed = false;

    switch (next)
    {
    case Expect::sync_char_begin_1:
        if (c == begin::sync_char_1)
        {
            reset();
            next = Expect::sync_char_begin_2;
            consumed = true;
        }
        break;
    case Expect::sync_char_begin_2:
        if (c == begin::sync_char_2)
        {
            next = Expect::length_1;
            consumed = true;
        }
        break;
    case Expect::length_1:
        expected_size |= (c << 8);
        next = Expect::length_2;
        consumed = true;
        break;
    case Expect::length_2:
        expected_size |= c;
        if (expected_size > 0)
            expected_size -= 1;
        payload.resize(expected_size);
        payload_iterator = payload.begin();
        next = Expect::message_id;
        consumed = true;
        break;
    case Expect::message_id:
        checksum(c);
        message_id = c;
        next = expected_size > 0 ? Expect::payload : Expect::checksum_1;
        consumed = true;
        break;
    case Expect::payload:
        checksum(c);
        *payload_iterator = c;
        ++payload_iterator;
        next = payload_iterator == payload.end()
                ? Expect::checksum_1
                : next;
        consumed = true;
        break;
    case Expect::checksum_1:
        checksum_1 = c;
        next = Expect::checksum_2;
        consumed = true;
        break;
    case Expect::checksum_2:
        checksum_2 = c;
        next = Expect::sync_char_end_1;
        consumed = true;
        break;
    case Expect::sync_char_end_1:
        if (c == end::sync_char_1)
        {
            next = Expect::sync_char_end_2;
            consumed = true;
        }
        break;
    case Expect::sync_char_end_2:
        if (c == end::sync_char_2)
        {
            next = Expect::nothing_more;
            consumed = true;
        }
        break;
    default:
        consumed = false;
        break;
    }

    return std::make_tuple(next, consumed);
}

sirf::Message sirf::Scanner::finalize()
{
    if (next != Expect::nothing_more)
        throw std::logic_error{"Not ready for extraction."};

    struct Scope
    {
        Scope(Scanner& scanner) : scanner{scanner} {}
        ~Scope() { scanner.reset(); }

        Scanner& scanner;
    } scope{*this};

    if (checksum() != ((checksum_1 << 8) | checksum_2))
        throw std::runtime_error("Failed to verify sirf protocol message integrity.");

    switch (message_id)
    {
    case GeodeticNavigationData::id:
        return decode_message<GeodeticNavigationData>(payload);
    default:
        break;
    }

    return Message{Null{}};
}

void sirf::Scanner::reset()
{
    checksum = Checksum{};
    next = Expect::sync_char_begin_1;
    expected_size = 0;
    message_id = 0;
    payload.clear();
    payload_iterator = payload.end();
    checksum_1 = checksum_2 = 0;
}

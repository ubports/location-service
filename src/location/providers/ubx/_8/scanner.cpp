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

#include <location/providers/ubx/_8/scanner.h>

#include <location/providers/ubx/_8/codec.h>
#include <location/providers/ubx/_8/magic.h>
#include <location/providers/ubx/_8/message.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace _8 = location::providers::ubx::_8;

namespace
{

template<std::uint8_t class_id, std::uint8_t message_id>
std::tuple<std::uint8_t, std::uint8_t> make_tuple()
{
    return std::make_tuple(class_id, message_id);
}

}  // namespace

_8::Scanner::Scanner()
    : next{Expect::sync_char_1},
      class_id{0},
      message_id{0},
      expected_size{0},
      payload_iterator{payload.end()}
{
    factories[make_tuple<ack::Ack::class_id, ack::Ack::message_id>()] =
            [](const std::vector<std::uint8_t>& payload) { return decode_message<ack::Ack>(payload); };
    factories[make_tuple<ack::Nak::class_id, ack::Nak::message_id>()] =
            [](const std::vector<std::uint8_t>& payload) { return decode_message<ack::Nak>(payload); };
    factories[make_tuple<cfg::Gnss::class_id, cfg::Gnss::message_id>()] =
            [](const std::vector<std::uint8_t>&) { return cfg::Gnss{}; };
    factories[make_tuple<cfg::Msg::class_id, cfg::Msg::message_id>()] =
            [](const std::vector<std::uint8_t>&) { return cfg::Msg{}; };
    factories[make_tuple<nav::Pvt::class_id, nav::Pvt::message_id>()] =
            [](const std::vector<std::uint8_t>& payload) { return decode_message<nav::Pvt>(payload); };
    factories[make_tuple<nav::Sat::class_id, nav::Sat::message_id>()] =
            [](const std::vector<std::uint8_t>& payload) { return decode_message<nav::Sat>(payload); };
}

std::tuple<_8::Scanner::Expect, bool> _8::Scanner::update(std::uint8_t c)
{
    bool consumed = false;

    // TODO(tvoss): This lacks a lot of validiation and verification.
    // UBX allows us to partially parse while we scan and carry out online
    // checksum calculation. Ideally, we would have a common class State that
    // captures behavior and transition logic.
    switch (next)
    {
    case Expect::sync_char_1:
        if (c == sync_char_1)
        {
            next = Expect::sync_char_2;
            consumed = true;
        }
        break;
    case Expect::sync_char_2:
        if (c == sync_char_2)
        {
            next = Expect::class_;
            consumed = true;
        }
        break;
    case Expect::class_:
        checksum(c);
        class_id = c;
        next = Expect::id;
        consumed = true;
        break;
    case Expect::id:
        checksum(c);
        message_id = c;
        next = Expect::length_1;
        consumed = true;
        break;
    case Expect::length_1:
        checksum(c);
        expected_size = c;
        next = Expect::length_2;
        consumed = true;
        break;
    case Expect::length_2:
        checksum(c);
        expected_size |= (c << 8);
        payload.resize(expected_size);
        payload_iterator = payload.begin();
        next = Expect::payload;
        consumed = true;
        break;
    case Expect::payload:
        checksum(c);
        *payload_iterator = c;
        ++payload_iterator;
        next = payload_iterator == payload.end()
                ? Expect::ck_a
                : next;
        consumed = true;
        break;
    case Expect::ck_a:
        ck_a = c;
        next = Expect::ck_b;
        consumed = true;
        break;
    case Expect::ck_b:
        ck_b = c;
        next = Expect::nothing_more;
        consumed = true;
        break;
    default:
        consumed = false;
        break;
    }

    return std::make_tuple(next, consumed);
}

_8::Message _8::Scanner::finalize()
{
    if (next != Expect::nothing_more)
        throw std::logic_error{"Not ready for extraction."};

    if (ck_a != checksum.ck_a() || ck_b != checksum.ck_b())
        throw std::runtime_error{"Verification failed."};

    auto it = factories.find(std::make_tuple(class_id, message_id));

    if (it == factories.end())
        throw std::runtime_error{"Could not decode message."};

    auto result = it->second(payload);

    checksum = Checksum{};
    next = Expect::sync_char_1;
    class_id = 0;
    message_id = 0;
    expected_size = 0;
    payload.clear();
    payload_iterator = payload.end();
    ck_a = ck_b = 0;

    return result;
}

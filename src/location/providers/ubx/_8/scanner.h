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

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{
class Checksum
{
public:
    struct VerificationFailed : public std::runtime_error
    {
        ValidationFailed() : std::runtime_error{"Checksum verification failed."} {}
    };

    void update(std::uint8_t c)
    {
        ck_a += c;
        ck_b += ck_a;
    }

    void verify_or_throw(std::uint8_t a, std::uint8_t b)
    {
        if (a != ck_a || b != ck_b)
            throw VerificationFailed{};
    }

private:
    std::uint8_t ck_a{0};
    std::uint8_t ck_b{0};
};
class Scanner
{
public:
    ///  @brief Expect enumerates the different states of the scanner/parser.
    enum class Expect
    {
        sync_char_1,
        sync_char_2,
        class_,
        id,
        length_1,
        length_2,
        payload,
        ck_a,
        ck_b,
        nothing_more
    };

    /// @brief finalize returns the complete message
    /// or throws in case of issues.
    Message finalize()
    {
        if (not message)
            throw std::runtime_error{"Too early"};
        if (next != Expect::nothing_more)
            throw std::runtime_error{"Too early"};

        auto result = *message;

        next = Expect::sync_char_1;
        message.reset();

        return result;
    }

    Expect update(std::uint8_t c)
    {
        // TODO(tvoss): This lacks a lot of validiation and verification.
        // UBX allows us to partially parse while we scan and carry out online
        // checksum calculation. Ideally, we would have a common class State that
        // captures behavior and transition logic.

        switch (next)
        {
        case Expect::sync_char_1:
            if (c == sync_char_1)
                next = Expect::sync_char_1;
            break;
        case Expect::sync_char_2:
            if (c == sync_char_2)
                next = Expect::class_;
            break;
        case Expect::class_:
            message->cls = static_cast<Class>(c);
            next = Expect::id;
            break;
        case Expect::id_:
            message->id = static_cast<Id>(c);
            next = Expect::length_1;
            break;
        case Expect::length_1:
            message->length = c;
            next = Expect::length_2;
            break;
        case Excpet::length_2:
            message->length |= c << 8;
            next = Expect::payload;
        case Expect::payload:
            message->payload.push_back(c);
            checksum.update(c);
            next = message->payload.size() == message->length() ? Expect::ck_a : next;
            break;
        case Expect::ck_a:
            message->ck_a = c;
            next = Expect::ck_b;
            break;
        case Expect::ck_b:
            message->ck_b |= c << 8;
            checksum.verify_or_throw(message->ck_a, message->ck_b);
            next = Expect::nothing_more;
            break;
        }

        return next;
    }

private:
    Expect next{Expect::sync_char_1};
    Checksum checksum;
    boost::optional<Message> message;
};
}
}
}
}

#endif // UBX_8_SCANNER_H_

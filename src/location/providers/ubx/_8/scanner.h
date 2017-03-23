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

#include <location/providers/ubx/_8/checksum.h>
#include <location/providers/ubx/_8/message.h>

#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

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

    Scanner();

    std::tuple<Expect, bool> update(std::uint8_t c);

    Message finalize();

private:

    void reset();

    Expect next;
    Checksum checksum;
    std::uint8_t class_id;
    std::uint8_t message_id;
    std::uint16_t expected_size;
    std::vector<uint8_t> payload;
    std::vector<uint8_t>::iterator payload_iterator;
    std::uint8_t ck_a;
    std::uint8_t ck_b;
    std::map<
        std::tuple<std::uint8_t, std::uint8_t>,
        std::function<Message(const std::vector<std::uint8_t>&)>
    > factories;
};
}
}
}
}

#endif // UBX_8_SCANNER_H_

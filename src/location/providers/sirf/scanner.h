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

#ifndef LOCATION_PROVIDERS_SIRF_SCANNER_H_
#define LOCATION_PROVIDERS_SIRF_SCANNER_H_

#include <location/providers/sirf/checksum.h>
#include <location/providers/sirf/message.h>

#include <cstdint>
#include <tuple>
#include <vector>

namespace location
{
namespace providers
{
namespace sirf
{

class Scanner
{
public:    
    ///  @brief Expect enumerates the different states of the scanner/parser.
    enum class Expect
    {
        sync_char_begin_1,
        sync_char_begin_2,        
        length_1,
        length_2,
        message_id,
        payload,
        checksum_1,
        checksum_2,
        sync_char_end_1,
        sync_char_end_2,
        nothing_more
    };    

    Scanner();

    std::tuple<Expect, bool> update(std::uint8_t c);

    Message finalize();

private:

    void reset();

    Expect next;
    Checksum checksum;    
    std::uint16_t expected_size;
    std::uint8_t message_id;
    std::vector<uint8_t> payload;
    std::vector<uint8_t>::iterator payload_iterator;
    std::uint8_t checksum_1;
    std::uint8_t checksum_2;
};

}
}
}

#endif // LOCATION_PROVIDERS_SIRF_SCANNER_H_

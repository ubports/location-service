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
#ifndef UBX_8_NMEA_TXT_H_
#define UBX_8_NMEA_TXT_H_

#include <location/providers/ubx/_8/nmea/talker.h>

#include <boost/optional.hpp>

#include <cstdint>

#include <string>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{
namespace nmea
{
/// @brief Text Transmission.
///
/// This message outputs various information on the receiver, such as power-up
/// screen,
/// software version etc.
struct Txt
{
    Talker talker;
    boost::optional<std::uint8_t> total_number_of_sentences;
    boost::optional<std::uint8_t> sentence_number;
    boost::optional<std::uint8_t> identifier;
    boost::optional<std::string> message;
};
}
}
}
}
}

#endif // UBX_8_NMEA_TXT_H_

// Copyright (C) 2017 Thomas Voss <thomas.voss@canonical.com>
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

#ifndef LOCATION_PROVIDERS_SIRF_SET_PROTOCOL_H_
#define LOCATION_PROVIDERS_SIRF_SET_PROTOCOL_H_

#include <location/providers/sirf/writer.h>

#include <cstdint>
#include <iostream>

namespace location
{
namespace providers
{
namespace sirf
{

// Switches the protocol to another protocol. For most software,
// the default protocol is SiRF binary.
struct SetProtocol
{
    enum
    {
        null = 0,
        sirf_binary = 1,
        nmea = 2,
        ascii = 3,
        rtcm = 4,
        user1 = 5,
        sirf_loc = 6,
        statistic = 7
    };

    static constexpr std::uint8_t id{0x87};

    std::size_t size() const
    {
        return sizeof(std::uint8_t) + sizeof(std::uint8_t);
    }

    void write(Writer& writer) const
    {
        writer.write_unsigned_char(id);
        writer.write_unsigned_char(protocol);
    }

    std::uint8_t protocol;
};

inline std::ostream& operator<<(std::ostream& out, const SetProtocol& sp)
{
    return out << "SetProtocol:" << std::endl
               << "  protocol: " << sp.protocol;
}

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif  // LOCATION_PROVIDERS_SIRF_SET_PROTOCOL_H_

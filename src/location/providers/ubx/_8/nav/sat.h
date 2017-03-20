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

#ifndef UBX_8_NAV_SAT_H_
#define UBX_8_NAV_SAT_H_

#include <location/providers/ubx/_8/gnss_id.h>

#include <cstdint>

#include <iosfwd>
#include <vector>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class Reader;

namespace nav
{

struct Sat
{
    static constexpr std::uint8_t class_id{0x01};
    static constexpr std::uint8_t message_id{0x35};

    struct Info
    {
        GnssId gnss_id;
        std::uint8_t satellite_id;
        std::uint8_t carrier_to_noise;
        std::int8_t elevation;
        std::int16_t azimuth;
        float pseudo_range_residual;
        std::uint32_t flags;
    };

    void read(Reader& reader);

    std::uint32_t itow;
    std::uint8_t version;
    std::vector<Info> info;
};

std::ostream& operator<<(std::ostream& out, const Sat& sat);

}  // namespace nav
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_NAV_SAT_H_

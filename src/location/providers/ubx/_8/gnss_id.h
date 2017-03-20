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

#ifndef UBX_8_GNSS_ID_H_
#define UBX_8_GNSS_ID_H_

#include <cstdint>

#include <iosfwd>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

enum class GnssId : std::uint8_t
{
    gps = 0,
    sbas = 1,
    galileo = 2,
    beidou = 3,
    imes = 4,
    qzss = 5,
    glonass = 6

};

std::ostream& operator<<(std::ostream& out, GnssId gnss_id);

}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namepsace location

#endif  // UBX_8_GNSS_ID_H_

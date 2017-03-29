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

#ifndef UBX_8_NAV_PVT_H_
#define UBX_8_NAV_PVT_H_

#include <location/providers/ubx/bits.h>

#include <iosfwd>

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

struct Pvt
{
    static constexpr std::uint8_t class_id{0x01};
    static constexpr std::uint8_t message_id{0x07};

    enum DateTimeValidity
    {
        valid_date = 1 << 0,
        valid_time = 1 << 1,
        fully_resolved = 1 << 2
    };

    enum FixType
    {
        no_fix = 0,
        dead_reckoning = 1,
        fix_2d = 2,
        fix_3d = 3,
        gnss_and_dead_reckoning = 4,
        only_time_fix = 5
    };

    void read(Reader& reader);

    std::uint32_t itow;
    std::uint16_t year;
    std::uint8_t month;
    std::uint8_t day;
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;
    DateTimeValidity date_time_validity;
    std::uint32_t time_accuracy;
    std::int32_t nanoseconds;
    FixType fix_type;
    std::uint8_t fix_status_flags;
    std::uint8_t additional_flags;
    std::uint8_t satellite_count;
    double longitude;
    double latitude;
    struct
    {
        std::int32_t above_ellipsoid;
        std::int32_t above_msl;
    } height;
    struct
    {
        std::uint32_t horizontal;
        std::uint32_t vertical;
        std::uint32_t speed;
        double heading;
    } accuracy;
    struct
    {
        std::int32_t north;
        std::int32_t east;
        std::int32_t down;
    } velocity;
    std::int32_t speed_over_ground;
    struct
    {
        double motion;
        double vehicle;
    } heading;
    std::uint16_t pdop;
};

std::ostream& operator<<(std::ostream& out, Pvt::DateTimeValidity validity);
std::ostream& operator<<(std::ostream& out, Pvt::FixType fix_type);
std::ostream& operator<<(std::ostream& out, const Pvt& pvt);

}  // namespace nav
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_NAV_PVT_H_

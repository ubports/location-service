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
#ifndef UBX_8_NMEA_GPS_FIX_MODE_H_
#define UBX_8_NMEA_GPS_FIX_MODE_H_

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
namespace gps
{
/// @brief FixMode enumerates all known, gps-specific fix modes.
enum class FixMode
{
    invalid = 0,
    gps_sps = 1,
    differential_gps_sps = 2,
    gps_pps = 3,
    real_time_kinematic = 4,
    floating_point_real_time_kinematic = 5,
    estimated = 6,
    manual_input = 7,
    simulator = 8
};
}
}
}
}
}
}

#endif // UBX_8_NMEA_GPS_FIX_MODE_H_

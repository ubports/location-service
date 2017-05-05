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
#ifndef LOCATION_NMEA_GSA_H_
#define LOCATION_NMEA_GSA_H_

#include <location/providers/ubx/dop.h>

#include <location/nmea/latitude.h>
#include <location/nmea/longitude.h>
#include <location/nmea/talker.h>

#include <location/nmea/gps/fix_mode.h>

#include <boost/optional.hpp>

#include <cstdint>
#include <vector>

namespace location
{
namespace nmea
{
/// @brief GNSS DOP and Active Satellites.
///
/// The GNSS receiver operating mode, satellites used for navigation, and DOP
/// values.
///   - If less than 12 SVs are used for navigation, the remaining fields are
///   left empty. If more
///     than 12 SVs are used for navigation, only the IDs of the first 12 are
///     output.
///   - The SV numbers (fields 'sv') are in the range of 1 to 32 for GPS
///   satellites, and 33 to 64
///     for SBAS satellites (33 = SBAS PRN 120, 34 = SBAS PRN 121, and so on)
///
/// In a multi-GNSS system this message will be output multiple times, once for
/// each GNSS.
struct Gsa
{
    enum class OperationMode
    {
        manual = 'M',   ///< Manual, forced to operate in 2D or 3D mode.
        automatic = 'A' ///< Automatic, allowed to switch between 2D and 3D.
    };

    enum FixMode
    {
        fix_not_available = 1, ///< Fix not available
        fix_in_2d = 2,         ///< 2d fix
        fix_in_3d = 3,         ///< 3d fix
    };

    Talker talker;
    boost::optional<OperationMode> operation_mode;
    boost::optional<FixMode> fix_mode;
    std::vector<boost::optional<std::uint8_t>> satellite_ids;
    boost::optional<float> pdop;
    boost::optional<float> hdop;
    boost::optional<float> vdop;
};
}
}

#endif // LOCATION_NMEA_GSA_H_

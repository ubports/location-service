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
#ifndef LOCATION_NMEA_GGA_H_
#define LOCATION_NMEA_GGA_H_

#include <location/providers/ubx/dop.h>

#include <location/nmea/cardinal_direction.h>
#include <location/nmea/latitude.h>
#include <location/nmea/longitude.h>
#include <location/nmea/talker.h>
#include <location/nmea/utc.h>

#include <location/nmea/gps/fix_mode.h>

#include <boost/optional.hpp>

#include <cstdint>

namespace location
{
namespace nmea
{

/// @brief Global positioning system fix data.
///
/// Time and position, together with GPS fixing related data (number of
/// satellites in use, and
/// the resulting HDOP, age of differential data if in use, etc.).
struct Gga
{
    Talker talker;
    boost::optional<Utc> utc;
    boost::optional<Latitude> latitude;
    boost::optional<CardinalDirection> latitude_direction;
    boost::optional<Longitude> longitude;
    boost::optional<CardinalDirection> longitude_direction;
    boost::optional<gps::FixMode> fix_mode;
    boost::optional<std::uint8_t> satellites_in_use;
    boost::optional<float> hdop;
    boost::optional<float> altitude;
    boost::optional<float> geoidal_separation;
    boost::optional<float> age; // [s]
    boost::optional<std::uint16_t> differential_reference_station;
};

}
}

#endif // LOCATION_NMEA_GGA_H_

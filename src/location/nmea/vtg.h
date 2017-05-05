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
#ifndef LOCATION_NMEA_VTG_H_
#define LOCATION_NMEA_VTG_H_

#include <location/nmea/mode.h>
#include <location/nmea/talker.h>

#include <boost/optional.hpp>

#include <cstdint>

namespace location
{
namespace nmea
{

/// @brief Course over ground and Ground speed.
///
/// Velocity is given as Course over Ground (COG) and Speed over Ground (SOG).
struct Vtg
{
    Talker talker;
    boost::optional<float> cog_true;
    boost::optional<float> cog_magnetic;
    boost::optional<float> sog_knots;
    boost::optional<float> sog_kmh;
    boost::optional<Mode> mode;
};

}
}

#endif // LOCATION_NMEA_VTG_H_

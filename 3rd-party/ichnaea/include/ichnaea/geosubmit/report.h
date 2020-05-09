// Copyright (C) 2016 Canonical Ltd.
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
#ifndef ICHNAEA_GEOSUBMIT_REPORT_H_
#define ICHNAEA_GEOSUBMIT_REPORT_H_

#include <ichnaea/bluetooth_beacon.h>
#include <ichnaea/radio_cell.h>
#include <ichnaea/wifi_access_point.h>

#include <boost/optional.hpp>

#include <chrono>
#include <set>

namespace ichnaea
{
namespace geosubmit
{
/// @brief Report encapsulates a set of wifi, cell and bt beacon measurements.
struct Report
{
    struct Position
    {
        enum class Source
        {
            manual,
            fusion,
            gps
        };

        double latitude;                                 ///< The latitude of the observation (WGS 84).
        double longitude;                                ///< The longitude of the observation (WGS 84).
        boost::optional<double> accuracy;                ///< The accuracy of the observed position in meters.
        boost::optional<std::chrono::milliseconds> age;  ///< The age of the position data (in milliseconds).
        boost::optional<double> altitude;                ///< The altitude at which the data was observed in meters above sea-level.
        boost::optional<double> altitude_accuracy;       ///< The accuracy of the altitude estimate in meters.
        boost::optional<double> heading;                 ///< The heading field denotes the direction of travel of the device and is specified in degrees, where 0° ≤ heading < 360°, counting clockwise relative to the true north.
        boost::optional<double> pressure;                ///< The air pressure in hPa (millibar).
        boost::optional<double> speed;                   ///< The speed field denotes the magnitude of the horizontal component of the device’s current velocity and is specified in meters per second.
        boost::optional<Source> source;                  ///< The source of the position information.
    };

    std::chrono::system_clock::time_point timestamp; ///< The time of observation of the data, measured in milliseconds since the UNIX epoch.
    Position position;                               ///< The actual position measurement.
    std::set<BluetoothBeacon> bluetooth_beacons;     ///< Visible bluetooth beacons.
    std::set<WifiAccessPoint> wifi_access_points;    ///< Visible access points.
    std::set<RadioCell> radio_cells;                 ///< Visible radio cells.
};

/// @brief operator<< inserts source into out.
std::ostream& operator<<(std::ostream& out, Report::Position::Source source);
}
}

#endif // ICHNAEA_GEOSUBMIT_REPORT_H_

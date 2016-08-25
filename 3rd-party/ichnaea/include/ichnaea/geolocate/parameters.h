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
#ifndef ICHNAEA_GEOLOCATE_PARAMETERS_H_
#define ICHNAEA_GEOLOCATE_PARAMETERS_H_

#include <ichnaea/bluetooth_beacon.h>
#include <ichnaea/radio_cell.h>
#include <ichnaea/wifi_access_point.h>

#include <ichnaea/geolocate/fallback.h>

#include <boost/optional.hpp>

#include <set>

namespace ichnaea
{
namespace geolocate
{
/// @brief Parameters encapsulates all input parameters to a geolocate request.
struct Parameters
{
    boost::optional<std::string> carrier;               ///< The clear text name of the cell carrier / operator.
    boost::optional<bool> consider_ip;                  ///< Should the clients IP address be used to locate it, defaults to true.
    boost::optional<RadioCell::MCC> mcc;                ///< The mobile country code stored on the SIM card.
    boost::optional<RadioCell::MNC> mnc;                ///< The mobile network code stored on the SIM card.
    boost::optional<RadioCell::RadioType> radio_type;   ///< Same as the radioType entry in each cell record.
    std::set<BluetoothBeacon> bluetooth_beacons;        ///< Visible bluetooth beacons.
    std::set<WifiAccessPoint> wifi_access_points;       ///< Visible access points.
    std::set<RadioCell> radio_cells;                    ///< Visible radio cells.
    boost::optional<Fallback> fallback;                 ///< Fallback setup.
};
}
}

#endif // ICHNAEA_GEOLOCATE_PARAMETERS_H_

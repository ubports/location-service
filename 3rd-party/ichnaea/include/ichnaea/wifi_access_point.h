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
#ifndef ICHNAEA_WIFI_ACCESS_POINT_H_
#define ICHNAEA_WIFI_ACCESS_POINT_H_

#include <boost/optional.hpp>

#include <chrono>
#include <string>

namespace ichnaea
{
struct WifiAccessPoint
{
    enum class Type
    {
        _802_11_a,
        _802_11_b,
        _802_11_g,
        _802_11_n,
        _802_11_ac
    };

    std::string bssid;                              ///< The BSSID of the WiFi network.
    boost::optional<Type> type;                     ///< The Wifi radio type.
    boost::optional<std::chrono::milliseconds> age; ///< The number of milliseconds since this network was last detected.
    boost::optional<unsigned int> channel;          ///< The WiFi channel, often 1 - 13 for networks in the 2.4GHz range.
    boost::optional<double> frequency;              ///< The frequency in MHz of the channel over which the client is communicating with the access point.
    boost::optional<double> signal_strength;        ///< The received signal strength (RSSI) in dBm.
    boost::optional<double> signal_to_noise_ratio;  ///< The current signal to noise ratio measured in dB.
    boost::optional<std::string> ssid;              ///< The SSID of the Wifi network.
};

/// @brief operator< returns true if lhs is smaller than rhs.
bool operator<(const WifiAccessPoint& lhs, const WifiAccessPoint& rhs);

/// @brief operator<< inserts type into out.
std::ostream& operator<<(std::ostream& out, WifiAccessPoint::Type type);
}

#endif // ICHNAEA_WIFI_ACCESS_POINT_H_

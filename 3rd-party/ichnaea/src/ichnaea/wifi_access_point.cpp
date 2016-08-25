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
#include <ichnaea/wifi_access_point.h>

#include <iostream>
#include <tuple>

bool ichnaea::operator<(const WifiAccessPoint& lhs, const WifiAccessPoint& rhs)
{
    return std::tie(lhs.age, lhs.bssid, lhs.channel, lhs.frequency, lhs.signal_strength, lhs.signal_to_noise_ratio, lhs.ssid) <
           std::tie(rhs.age, rhs.bssid, rhs.channel, rhs.frequency, rhs.signal_strength, rhs.signal_to_noise_ratio, rhs.ssid);
}

std::ostream& ichnaea::operator<<(std::ostream& out, ichnaea::WifiAccessPoint::Type type)
{
    switch (type)
    {
    case ichnaea::WifiAccessPoint::Type::_802_11_a: return out << "802.11a";
    case ichnaea::WifiAccessPoint::Type::_802_11_b: return out << "802.11b";
    case ichnaea::WifiAccessPoint::Type::_802_11_g: return out << "802.11g";
    case ichnaea::WifiAccessPoint::Type::_802_11_n: return out << "802.11n";
    case ichnaea::WifiAccessPoint::Type::_802_11_ac: return out << "802.11ac";
    }
    return out;
}

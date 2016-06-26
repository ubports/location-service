/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <location/connectivity/wireless_network.h>

#include <iostream>

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, com::ubuntu::location::connectivity::WirelessNetwork::Mode mode)
{
    switch (mode)
    {
    case com::ubuntu::location::connectivity::WirelessNetwork::Mode::unknown: out << "Mode::unknown"; break;
    case com::ubuntu::location::connectivity::WirelessNetwork::Mode::adhoc: out << "Mode::adhoc"; break;
    case com::ubuntu::location::connectivity::WirelessNetwork::Mode::infrastructure: out << "Mode::infrastructure"; break;
    }

    return out;
}

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, const com::ubuntu::location::connectivity::WirelessNetwork& wifi)
{
    return out << "("
               << "bssid: " << wifi.bssid().get() << ", "
               << "ssid: " << wifi.ssid().get() << ", "
               << "last seen: " << wifi.last_seen().get().time_since_epoch().count() << ", "
               << "mode: " << wifi.mode().get() << ", "
               << "frequency: " << wifi.frequency().get() << ", "
               << "strength: " << wifi.signal_strength().get()
               << ")";
}

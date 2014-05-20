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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_WIRELESS_NETWORK_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_WIRELESS_NETWORK_H_

#include <com/ubuntu/location/connectivity/bounded_integer.h>

#include <string>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace connectivity
{
struct WirelessNetwork
{
    enum class Mode
    {
        /** Mode is unknown. */
        unknown = 0,
        /** Indicates the object is part of an Ad-Hoc 802.11 network without a central coordinating access point. */
        adhoc = 1,
        /** The wireless device or access point is in infrastructure mode. */
        infrastructure = 2
    };

    inline friend std::ostream& operator<<(std::ostream& out, Mode mode)
    {
        switch (mode)
        {
        case Mode::unknown: out << "Mode::unknown"; break;
        case Mode::adhoc: out << "Mode::adhoc"; break;
        case Mode::infrastructure: out << "Mode::infrastructure"; break;
        }

        return out;
    }

    struct Tag
    {
        /** @brief Tags a frequency measurement for a wireless network. */
        struct Frequency {};
        /** @brief Tags the signal strength of a wireless network. */
        struct SignalStrength {};
    };

    /** Frequency that an individual AP operates on. */
    typedef BoundedInteger
    <
        Tag::Frequency,
        2412,
        5825
    > Frequency;

    /** Strength of signal for an individual AP. */
    typedef BoundedInteger
    <
        Tag::SignalStrength,
        0,
        100
    > SignalStrength;

    bool operator==(const WirelessNetwork& rhs) const
    {
        return bssid == rhs.bssid &&
               ssid == rhs.ssid &&
               mode == rhs.mode &&
               frequency == rhs.frequency &&
               signal_strength == rhs.signal_strength;
    }

    friend std::ostream& operator<<(std::ostream& out, const WirelessNetwork& wifi)
    {
        return out << "("
                   << "bssid: " << wifi.bssid << ", "
                   << "ssid: " << wifi.ssid << ", "
                   << "mode: " << wifi.mode << ", "
                   << "frequency: " << wifi.frequency << ", "
                   << "strength: " << wifi.signal_strength
                   << ")";
    }

    std::string bssid; ///< The BSSID of the network.
    std::string ssid; ///< The SSID of the network.
    Mode mode; ///< The mode of the network.
    Frequency frequency; ///< Frequency of the network/AP.
    SignalStrength signal_strength; ///< Signal quality of the network/AP in percent.
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_WIRELESS_NETWORK_H_

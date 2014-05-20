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

#include <iosfwd>
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
    /** @brief Enumerates all known operational modes of networks/aps. */
    enum class Mode
    {
        /** Mode is unknown. */
        unknown = 0,
        /** Indicates the object is part of an Ad-Hoc 802.11 network without a central coordinating access point. */
        adhoc = 1,
        /** The wireless device or access point is in infrastructure mode. */
        infrastructure = 2
    };

    /** @cond */
    struct Tag
    {
        /** @brief Tags a frequency measurement for a wireless network. */
        struct Frequency {};
        /** @brief Tags the signal strength of a wireless network. */
        struct SignalStrength {};
    };
    /** @endcond */

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

    std::string bssid; ///< The BSSID of the network.
    std::string ssid; ///< The SSID of the network.
    Mode mode; ///< The mode of the network.
    Frequency frequency; ///< Frequency of the network/AP.
    SignalStrength signal_strength; ///< Signal quality of the network/AP in percent.
};

/** @brief Returns true iff lhs equals rhs. */
bool operator==(const WirelessNetwork& lhs, const WirelessNetwork& rhs);

/** @brief Pretty-prints the given mode to the given output stream. */
std::ostream& operator<<(std::ostream& out, WirelessNetwork::Mode mode);

/** @brief Pretty-prints the given wireless network to the given output stream. */
std::ostream& operator<<(std::ostream& out, const WirelessNetwork& wifi);
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_WIRELESS_NETWORK_H_

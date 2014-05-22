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

#include <core/property.h>

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
    typedef std::shared_ptr<WirelessNetwork> Ptr;

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

    /** @cond */
    WirelessNetwork() = default;
    WirelessNetwork(const WirelessNetwork&) = delete;
    WirelessNetwork(WirelessNetwork&&) = delete;

    virtual ~WirelessNetwork() = default;

    WirelessNetwork& operator=(const WirelessNetwork&) = delete;
    WirelessNetwork& operator=(WirelessNetwork&&) = delete;
    /** @endcond */

    /** @brief Timestamp when the network became visible. */
    virtual const core::Property<std::chrono::system_clock::time_point>& timestamp() const = 0;

    /** @brief Returns the BSSID of the network */
    virtual const core::Property<std::string>& bssid() const = 0;

    /** @brief Returns the SSID of the network. */
    virtual const core::Property<std::string>& ssid() const = 0;

    /** @brief Returns the mode of the network. */
    virtual const core::Property<Mode>& mode() const = 0;

    /** @brief Returns the frequency that the network/AP operates upon. */
    virtual const core::Property<Frequency>& frequency() const = 0;

    /** @brief Returns the signal quality of the network/AP in percent. */
    virtual const core::Property<SignalStrength>& signal_strength() const = 0;
};

/** @brief Pretty-prints the given mode to the given output stream. */
std::ostream& operator<<(std::ostream& out, WirelessNetwork::Mode mode);

/** @brief Pretty-prints the given wireless network to the given output stream. */
std::ostream& operator<<(std::ostream& out, const WirelessNetwork& wifi);
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_WIRELESS_NETWORK_H_

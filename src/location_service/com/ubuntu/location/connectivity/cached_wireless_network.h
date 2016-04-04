/*
 * Copyright © 2012-2014 Canonical Ltd.
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
#ifndef CACHED_WIRELESS_NETWORK_H_
#define CACHED_WIRELESS_NETWORK_H_

#include <com/ubuntu/location/connectivity/wireless_network.h>

#include <com/ubuntu/location/logging.h>

#include "nm.h"

namespace detail
{

// Implements the WirelessNetwork interface relying on a remote NetworkManager instance,
// caching all of the interesting properties.
struct CachedWirelessNetwork : public com::ubuntu::location::connectivity::WirelessNetwork
{
    // Just to save some typing.
    typedef std::shared_ptr<CachedWirelessNetwork> Ptr;

    // Constructs a new instance associated with the ap and the (remote) device
    // it belongs to. Please note that the caching nature of the class ensures that
    // ap and device stubs are kept alive.
    CachedWirelessNetwork(
            const org::freedesktop::NetworkManager::Device& device,
            const org::freedesktop::NetworkManager::AccessPoint& ap);

    ~CachedWirelessNetwork();

    // Timestamp when the network became visible.
    const core::Property<std::chrono::system_clock::time_point>& last_seen() const override;

    // Returns the BSSID of the network
    const core::Property<std::string>& bssid() const override;

    // Returns the SSID of the network.
    const core::Property<std::string>& ssid() const override;

    // Returns the mode of the network.
    const core::Property<Mode>& mode() const override;

    // Returns the frequency that the network/AP operates upon.
    const core::Property<Frequency>& frequency() const override;

    // Returns the signal quality of the network/AP in percent.
    const core::Property<SignalStrength>& signal_strength() const override;

    // Called whenever a property of an access point changes.
    void on_access_point_properties_changed(const std::map<std::string, core::dbus::types::Variant>& dict);

    // The cached network manager device associated to the access point.
    org::freedesktop::NetworkManager::Device device_;
    // The actual access point stub.
    org::freedesktop::NetworkManager::AccessPoint access_point_;

    // Encapsulates all event connections that have to be cut on destruction.
    struct
    {
        core::dbus::Signal
        <
            org::freedesktop::NetworkManager::AccessPoint::PropertiesChanged,
            org::freedesktop::NetworkManager::AccessPoint::PropertiesChanged::ArgumentType
        >::SubscriptionToken ap_properties_changed;
    } connections;

    core::Property<std::chrono::system_clock::time_point> last_seen_;
    core::Property<std::string> bssid_;
    core::Property<std::string> ssid_;
    core::Property<WirelessNetwork::Mode> mode_;
    core::Property<WirelessNetwork::Frequency> frequency_;
    core::Property<WirelessNetwork::SignalStrength> signal_strength_;
};
}

#endif // CACHED_WIRELESS_NETWORK_H_

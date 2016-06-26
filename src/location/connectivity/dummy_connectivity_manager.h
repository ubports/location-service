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

#ifndef DUMMY_CONNECTIVITY_MANAGER_H_
#define DUMMY_CONNECTIVITY_MANAGER_H_

#include <location/connectivity/manager.h>

namespace dummy
{
struct ConnectivityManager : public com::ubuntu::location::connectivity::Manager
{
    const core::Property<com::ubuntu::location::connectivity::State>& state() const
    {
        return properties.state;
    }

    const core::Property<bool>& is_wifi_enabled() const
    {
        return properties.is_wifi_enabled;
    }

    const core::Property<bool>& is_wwan_enabled() const
    {
        return properties.is_wwan_enabled;
    }

    const core::Property<bool>& is_wifi_hardware_enabled() const
    {
        return properties.is_wifi_hardware_enabled;
    }

    const core::Property<bool>& is_wwan_hardware_enabled() const
    {
        return properties.is_wwan_hardware_enabled;
    }

    const core::Property<com::ubuntu::location::connectivity::Characteristics>& active_connection_characteristics() const
    {
        return properties.active_connection_characteristics;
    }

    void request_scan_for_wireless_networks()
    {
    }

    const core::Signal<>& wireless_network_scan_finished() const
    {
        return sigs.wireless_network_scan_finished;
    }

    const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wireless_network_added() const
    {
        return sigs.wireless_network_added;
    }

    const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wireless_network_removed() const
    {
        return sigs.wireless_network_removed;
    }

    void enumerate_visible_wireless_networks(const std::function<void(const com::ubuntu::location::connectivity::WirelessNetwork::Ptr&)>&) const
    {
    }

    const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& connected_cell_added() const
    {
        return sigs.connected_cell_added;
    }

    const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& connected_cell_removed() const
    {
        return sigs.connected_cell_removed;
    }

    void enumerate_connected_radio_cells(const std::function<void(const com::ubuntu::location::connectivity::RadioCell::Ptr&)>&) const
    {
    }

    struct
    {
        core::Property<bool> is_wifi_enabled{false};
        core::Property<bool> is_wwan_enabled{false};
        core::Property<bool> is_wifi_hardware_enabled{false};
        core::Property<bool> is_wwan_hardware_enabled{false};
        core::Property<com::ubuntu::location::connectivity::State> state{com::ubuntu::location::connectivity::State::unknown};
        core::Property<com::ubuntu::location::connectivity::Characteristics> active_connection_characteristics{};
    } properties;

    struct
    {
        core::Signal<> wireless_network_scan_finished;
        core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_added;
        core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_removed;
        core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_added;
        core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_removed;
    } sigs;
};
}

#endif // DUMMY_CONNECTIVITY_MANAGER_H_

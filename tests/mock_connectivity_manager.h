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

#ifndef MOCK_CONNECTIVITY_MANAGER_H_
#define MOCK_CONNECTIVITY_MANAGER_H_

#include <com/ubuntu/location/connectivity/manager.h>

#include <core/property.h>

#include <gmock/gmock.h>

struct MockConnectivityManager : public com::ubuntu::location::connectivity::Manager
{
    /**
     * @brief Returns the getable/observable connectivity state of the system.
     */
    MOCK_CONST_METHOD0(state, const core::Property<com::ubuntu::location::connectivity::State>&());

    /**
     * @brief request_scan_for_wireless_networks schedules a scan for visible wireless networks.
     */
    MOCK_METHOD0(request_scan_for_wireless_networks, void());

    /**
     * @brief wireless_network_added is emitted whenever a new wifi becomes visible.
     */
    MOCK_CONST_METHOD0(wireless_network_added, const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& ());

    /**
     * @brief wireless_network_removed is emitted whenever a wifi disappears.
     */
    MOCK_CONST_METHOD0(wireless_network_removed, const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& ());

    /**
     * @brief Enumerate all wireless networks visible to the device.
     */
    MOCK_CONST_METHOD1(
            enumerate_visible_wireless_networks,
            void(const std::function<void(const com::ubuntu::location::connectivity::WirelessNetwork::Ptr&)>&));

    /**
     * @brief Enumerate all radio cells that the device is connected to.
     */
    MOCK_CONST_METHOD1(
            enumerate_connected_radio_cells,
            void(const std::function<void(const com::ubuntu::location::connectivity::RadioCell::Ptr&)>&));

};

#endif // MOCK_CONNECTIVITY_MANAGER_H_

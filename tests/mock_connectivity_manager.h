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
     * @brief All wireless networks visible to the device.
     */
    MOCK_METHOD0(visible_wireless_networks,
                 const core::Property<
                    std::vector<
                        com::ubuntu::location::connectivity::WirelessNetwork::Ptr
                    >
                 >&());

    /**
     * @brief All radio cells that the device is connected to.
     */
    MOCK_METHOD0(connected_radio_cells,
                 const core::Property<
                    std::vector<
                        com::ubuntu::location::connectivity::RadioCell
                    >
                 >&());
};

#endif // MOCK_CONNECTIVITY_MANAGER_H_

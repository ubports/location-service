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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_

#include <com/ubuntu/location/connectivity/radio_cell.h>
#include <com/ubuntu/location/connectivity/wireless_network.h>

#include <core/property.h>

#include <string>
#include <vector>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace connectivity
{
/**
 * @brief The Manager class encapsulates access to network/radio information
 */
class Manager
{
public:
    struct Errors
    {
        Errors() = delete;
        /**
         * @brief The ConnectivityManagementNotSupported struct is thrown if underlying
         * platform does not provide support for connectivity mgmt.
         */
        struct ConnectivityManagementNotSupported : public std::runtime_error
        {
            ConnectivityManagementNotSupported()
                : std::runtime_error(
                      "Underlying platform does not provide support for connectivity mgmt.")
            {
            }
        };
    };

    Manager(const Manager& rhs) = delete;
    virtual ~Manager() = default;

    Manager& operator=(const Manager& rhs) = delete;
    bool operator==(const Manager& rhs) const = delete;

    /**
     * @brief request_scan_for_wireless_networks schedules a scan for visible wireless networks.
     *
     * Please note that the implementation is required to operate asynchronously. Results of the scan
     * are reported via emitting the changed() signal on the visible_wireless_networks() property.
     *
     */
    virtual void request_scan_for_wireless_networks() = 0;

    /**
     * @brief All wireless networks visible to the device.
     *
     * If users would like to receive updates about visible wireless networks, they
     * should connect to the Property's changed signal as in (all error handling omitted):
     *
     * auto connectivity_manager = com::ubuntu::location::connectivity::platform_default_manager();
     *
     * // Subscribe to visible wireless network updates.
     * connectivity_manager->visible_wireless_networks().changed().connect([](const std::vector<WirelessNetwork>& networks)
     * {
     *     for (const auto& network : networks)
     *         std::cout << network.ssid << std::endl;
     * });
     *
     * @return A getable/observable property carrying the visible wireless networks.
     */
    virtual const core::Property<std::vector<WirelessNetwork>>& visible_wireless_networks() = 0;

    /**
     * @brief All radio cells that the device is connected to.
     *
     * In case of multiple sims, the size of connected radio cells is larger than 1.
     * In case of no radio connection, the size of connected radio cells is 0.
     *
     * @return A getable/observable property carrying the visible radio cells.
     */
    virtual const core::Property<std::vector<RadioCell>>& connected_radio_cells() = 0;

protected:
    Manager() = default;
};

/**
 * @brief Provides access to a platform-specific implementation/instance of a connectivity manager.
 * @throw Manager::Errors::ConnectivityManagementNotSupported.
 * @return An instance of a connectivity manager.
 */
const std::shared_ptr<Manager>& platform_default_manager();
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_

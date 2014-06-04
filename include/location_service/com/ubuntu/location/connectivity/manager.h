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

#include <memory>
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
/** @brief Enumerates all known system connectivity states. */
enum class State
{
    /** The state is unknown. */
    unknown,
    /** The system is not connected to any network. */
    none,
    /** The system is behind a captive portal and cannot reach the full internet. */
    portal,
    /**
     * The system is connected to a network, but does not appear to be able to reach
     * the full internet.
     */
    limited,
    /** The system is connected to a network, and appears to be able to reach the full internet. */
    full
};

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
         * @brief The ConnectivityManagementNotSupported struct is thrown if the underlying
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

    /** @cond */
    Manager(const Manager& rhs) = delete;
    virtual ~Manager() = default;

    Manager& operator=(const Manager& rhs) = delete;
    bool operator==(const Manager& rhs) const = delete;
    /** @endcond */

    /**
     * @brief Returns the getable/observable connectivity state of the system.
     *
     * Please note that this requires the underlying networking state to
     * support connectivity state tracking. Right now, e.g. NetworkManager needs
     * custom entries in /etc/NetworkManager/NetworkManager.conf to enable this
     * functionality.
     *
     */
    virtual const core::Property<State>& state() const = 0;

    /**
     * @brief request_scan_for_wireless_networks schedules a scan for visible wireless networks.
     * @throws std::runtime_error to indicate issues arising from the underlying networking stack.
     *
     * Please note that the implementation is required to operate asynchronously. Results of the scan
     * are reported via emitting the changed() signal on the visible_wireless_networks() property.
     *
     * Please also note that calling this function is usually not required. The underlying
     * networking stack is updating the list of available wireless networks very frequently.
     * In addition, the results from scans requested by other system components are reported
     * to consumers of this API, too.
     *
     */
    virtual void request_scan_for_wireless_networks() = 0;

    /**
     * @brief wireless_network_added is emitted whenever a new wifi becomes visible.
     */
    virtual const core::Signal<WirelessNetwork::Ptr>& wireless_network_added() const = 0;

    /**
     * @brief wireless_network_removed is emitted whenever a wifi disappears.
     */
    virtual const core::Signal<WirelessNetwork::Ptr>& wireless_network_removed() const = 0;

    /**
     * @brief Enumerates all wireless networks visible to the device.
     */
    virtual void enumerate_visible_wireless_networks(const std::function<void(const WirelessNetwork::Ptr&)>&) const = 0;

    /**
     * @brief Enumerates all radio cells that the device is connected to.
     */
    virtual void enumerate_connected_radio_cells(const std::function<void(const RadioCell::Ptr&)>&) const = 0;

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

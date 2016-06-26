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
#ifndef LOCATION_CONNECTIVITY_MANAGER_H_
#define LOCATION_CONNECTIVITY_MANAGER_H_

#include <location/connectivity/radio_cell.h>
#include <location/connectivity/wireless_network.h>

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
    unknown = 0,
    /** @brief Networking is inactive and all devices are disabled. */
    asleep = 10,
    /** @brief There is no active network connection. */
    disconnected = 20,
    /** @brief Network connections are being cleaned up. */
    disconnecting = 30,
    /**
     * @brief A network device is connecting to a network and there is no other
     * available network connection.
     */
    connecting = 40,
    /** @brief A network device is connected, but there is only link-local connectivity. */
    connected_local = 50,
    /** @brief A network device is connected, but there is only site-local connectivity. */
    connected_site = 60,
    /** @brief A network device is connected, with global network connectivity. */
    connected_global = 70
};

/** @brief Pretty prints the given state to the given output stream */
std::ostream& operator<<(std::ostream& out, State state);

/** @brief Summarizes characteristics of network connections. */
enum class Characteristics : std::uint32_t
{
    /** @brief Nothing special about the characteristics. */
    none = 0,
    /** @brief The connection goes via wifi. */
    connection_goes_via_wifi = 1 << 0,
    /** @brief The connection goes via a mobile-broadband connection. */
    connection_goes_via_wwan = 1 << 1,
    /** @brief The connection goes via a roaming mobile-broadband connection. */
    connection_is_roaming = 1 << 2,
    /** @brief The connection has monetary costs. No data should be transfered. */
    connection_has_monetary_costs = 1 << 3,
    /** @brief The connection is volume limited. No large files should be transfered. */
    connection_is_volume_limited = 1 << 4,
    /** @brief the connection is bandwidth limited. Large transfer should be postponed. */
    connection_is_bandwith_limited = 1 << 5
};

/** @brief Bitwise or operator for Characteristics flags. */
Characteristics operator|(Characteristics l, Characteristics r);

/** @brief Bitwise and operator for Characteristics flags. */
Characteristics operator&(Characteristics l, Characteristics r);

/** @brief Pretty prints the given charateristics to the given output stream */
std::ostream& operator<<(std::ostream& out, Characteristics characteristics);

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
     */
    virtual const core::Property<State>& state() const = 0;

    /**
      * @brief Returns a getable/observable boolean property that indicates the state of the wifi subsystem.
      *
      * If the property's value is false, the Wifi subsystem is turned off (e.g., in flight mode).
      */
    virtual const core::Property<bool>& is_wifi_enabled() const = 0;

    /**
      * @brief Returns a getable/observable boolean property that indicates the state of the wwan subsystem.
      *
      * If the property's value is false, the WWan subsystem is turned off (e.g., in flight mode).
      */
    virtual const core::Property<bool>& is_wwan_enabled() const = 0;

    /**
      * @brief Returns a getable/observable boolean property that indicates the state of the wifi hardware.
      *
      * If the property's value is false, the Wifi HW is turned off.
      */
    virtual const core::Property<bool>& is_wifi_hardware_enabled() const = 0;

    /**
      * @brief Returns a getable/observable boolean property that indicates the state of the wwan hardware.
      *
      * If the property's value is false, the WWan HW is turned off.
      */
    virtual const core::Property<bool>& is_wwan_hardware_enabled() const = 0;


    /**
     * @brief Returns a getable/observable property that describes the characteristics
     * of the active network connection.
     */
    virtual const core::Property<Characteristics>& active_connection_characteristics() const = 0;

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
     * @brief wireless_network_scan_finished is emitted when a scan for wireless networks ends.
     *
     * Please note that the signal may also be raised for scans that have been
     * initiated by other system components.
     *
     */
    virtual const core::Signal<>& wireless_network_scan_finished() const = 0;

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
     * @brief connected_cell_added is emitted whenever the underlying modem connects to a new cell.
     */
    virtual const core::Signal<RadioCell::Ptr>& connected_cell_added() const = 0;

    /**
     * @brief connected_cell_removed is emitted whenever the underlying modem disconnects from a cell.
     */
    virtual const core::Signal<RadioCell::Ptr>& connected_cell_removed() const = 0;

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

#endif // LOCATION_CONNECTIVITY_MANAGER_H_

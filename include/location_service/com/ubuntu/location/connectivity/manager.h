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
     * @brief All wireless networks visible to the device.
     * @return A getable/observable property carrying the visible wireless networks.
     */
    virtual const core::Property<std::vector<WirelessNetwork>>& visible_wireless_networks() = 0;

    /**
     * @brief All radio cells visible to the device.
     * @return A getable/observable property carrying the visible radio cells.
     */
    virtual const core::Property<std::vector<RadioCell>>& visible_radio_cells() = 0;

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

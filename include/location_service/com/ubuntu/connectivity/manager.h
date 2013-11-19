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
#ifndef LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_MANAGER_H_

#include <future>
#include <memory>

namespace com
{
namespace ubuntu
{
template<typename T> class Property;
namespace connectivity
{
class AuthenticationProvider;
class Connection;
class Link;
class Network;

/**
 * @brief Provides access to high-level connectivity aspects of the system.
 *
 * Please note that functionality defined in the connectivity namespace
 * is subject to security mediation and any function can throw a SecurityException
 * at any time.
 */
class Manager
{
public:
    /**
     * @brief Enumerator for the different airplane modes.
     */
    enum class AirplaneMode
    {
        off, ///< Airplane mode is off.
        on ///< Airplane mode is on.
    };

    /**
     * @brief The RequestConnectionResult enumerates the different results of a connection attempt.
     */
    enum class RequestConnectionResult
    {
        success, ///< Connection has been successfully established.
        failure ///< There was an error establishing the connection.
    };

    Manager(const Manager&) = delete;
    virtual ~Manager() = default;

    Manager& operator=(const Manager&) = delete;
    bool operator==(const Manager&) const = delete;

    /**
     * @brief Provides mutable access to the airplane mode of the system.
     */
    virtual Property<AirplaneMode>& airplane_mode() = 0;

    /**
     * @brief Provides non-mutable access to the current data connection.
     */
    virtual const Property<std::shared_ptr<Connection>>& current_data_connection() const = 0;

    /**
     * @brief Requests a connection to a network.
     * @param network The network to connect to.
     * @param auth_provider Implementation of AuthProvider to handle authentication requests.
     * @return A waitable future encapsulating the result of the async connection attempt.
     */
    virtual std::future<RequestConnectionResult> request_connection_to(
        const std::shared_ptr<Network>& network,
        const std::shared_ptr<AuthenticationProvider>& auth_provider) = 0;

    /**
     * @brief Provides non-mutable access to all links known to the system.
     */
    virtual const Property<std::set<std::shared_ptr<Link>>>& available_links() = 0;

protected:
    Manager() = default;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_MANAGER_H_

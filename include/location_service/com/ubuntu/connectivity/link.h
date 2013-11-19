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
#ifndef LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_LINK_H_
#define LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_LINK_H_

#include <memory>
#include <set>

namespace com
{
namespace ubuntu
{
template<typename T> class Property;
namespace connectivity
{
class Network;
/**
 * @brief The Link class represents a piece of HW that enables a system to
 * establish connections to the outside world.
 */
class Link
{
public:
    /**
     * @brief The Medium enum describes whether a link is wired or wireless.
     */
    enum class Medium
    {
        wireless,
        wired
    };

    /**
     * @brief Mode of the link, either enabled or disabled.
     */
    enum class Mode
    {
        disabled, ///< The link is disabled and any connection attempt will fail.
        enabled ///< The link is enabled and connection are theoretically possible.
    };

    /**
     * @brief Enumeration of types that are known to the connectivity system.
     */
    enum class Type
    {
        ethernet, ///< Wired ethernet links.
        wifi, ///< Wireless LAN links.
        radio, ///< Radio links aka modems.
        bluetooth, ///< Bluetooth links.
        nfc ///< NFC-based links.
    };

    /**
     * @brief Lightweight type to ease identifying links and storing instances in associative containers.
     */
    typedef unsigned int Id;

    Link(const Link&) = delete;
    virtual ~Link() = default;

    Link& operator=(const Link&) = delete;
    bool operator==(const Link&) const = delete;

    /**
     * @brief Queries the type of the link.
     */
    virtual Type type() const = 0;

    /**
     * @brief Queries the medium the link runs over.
     */
    virtual Medium medium() const = 0;

    /**
     * @brief Queries the human readable name of the link.
     */
    virtual std::string name() const = 0;

    /**
     * @brief Queries the unique id of the link.
     */
    Id id() const;

    /**
      * @brief Queries the available networks for this link.
      */
    virtual const Property<std::set<std::shared_ptr<Network>>>& available_networks() = 0;

    /**
     * @brief Readable/observable property for the link mode.
     */
    virtual const Property<Mode>& mode() = 0;

protected:
    Link() = default;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_LINK_H_

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
#ifndef LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_LINK_H_
#define LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_LINK_H_

#include <com/ubuntu/connectivity/radio/cell.h>

namespace com
{
namespace ubuntu
{
namespace connectivity
{
namespace radio
{
/**
 * @brief The Link class represents a radio link.
 */
class Link : public com::ubuntu::connectivity::Link
{
public:
    Link();

    /**
     * @brief Provides immutable access to the visible radio cells.
     */
    const Property<Cell>& visible_cells() const;

    // From com::ubuntu::connectivity::Link
    Type type() const;

    Medium medium() const = 0;

    std::string name() const = 0;

    const Property<std::set<std::shared_ptr<Network>>& available_networks();

    const Property<Mode>& mode();
};
}
}
}
}

#define // LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_LINK_H_

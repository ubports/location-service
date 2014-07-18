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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SATELLITE_BASED_POSITIONING_STATE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SATELLITE_BASED_POSITIONING_STATE_H_

namespace com
{
namespace ubuntu
{
namespace location
{
/**
 * @brief The SatelliteBasedPositioningState enum describes whether satellite
 * assisted positioning is enabled or disabled.
 */
enum class SatelliteBasedPositioningState
{
    on, ///< Satellite assisted positioning is on.
    off ///< Satellite assisted positioning is off.
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SATELLITE_BASED_POSITIONING_STATE_H_

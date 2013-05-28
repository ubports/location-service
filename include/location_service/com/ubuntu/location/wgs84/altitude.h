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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_ALTITUDE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_ALTITUDE_H_

#include "com/ubuntu/location/units/units.h"
#include "com/ubuntu/location/wgs84/coordinate.h"

namespace com
{
namespace ubuntu
{
namespace location
{
namespace wgs84
{
namespace tag
{
struct Altitude;
}
typedef Coordinate<tag::Altitude, units::Length> Altitude;

template<>
struct CoordinateTraits<Altitude>
{
    static void check_and_throw_if_invalid(const typename Altitude::Quantity&) {}
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_ALTITUDE_H_

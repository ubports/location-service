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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_POSITION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_POSITION_H_

#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

#include <bitset>
#include <ostream>

namespace com
{
namespace ubuntu
{
namespace location
{
class Position
{
  public:
    enum Flag
    {
        latitude_flag = 0,
        longitude_flag = 1,
        altitude_flag = 2
    };

    typedef std::bitset<3> Flags;

    Position();    
    Position(
        const wgs84::Latitude& latitude,
        const wgs84::Longitude& longitude);
    Position(
        const wgs84::Latitude& latitude,
        const wgs84::Longitude& longitude,
        const wgs84::Altitude& altitude);

    bool operator==(const Position& rhs) const;
    bool operator!=(const Position& rhs) const;

    const Flags& flags() const;

    bool has_latitude() const;
    Position& latitude(const wgs84::Latitude& lat);
    const wgs84::Latitude& latitude() const;

    bool has_longitude() const;
    Position& longitude(const wgs84::Longitude& lon);
    const wgs84::Longitude& longitude() const;

    bool has_altitude() const;
    Position& altitude(const wgs84::Altitude& alt);
    const wgs84::Altitude& altitude() const;

  private:
    template<typename> friend struct Codec;
    
    struct
    {
        Flags flags;
        wgs84::Latitude latitude;
        wgs84::Longitude longitude;
        wgs84::Altitude altitude;
    } fields;
};

std::ostream& operator<<(std::ostream& out, const Position& position);

units::Quantity<units::Length> haversine_distance(const Position& p1, const Position& p2);
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_POSITION_H_

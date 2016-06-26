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
#include <location/position.h>

#include <bitset>
#include <ostream>

location::Position::Position(
        const location::wgs84::Latitude& latitude,
        const location::wgs84::Longitude& longitude)
    : latitude(latitude),
      longitude(longitude)
{
}

location::Position::Position(
        const location::wgs84::Latitude& latitude,
        const location::wgs84::Longitude& longitude,
        const location::wgs84::Altitude& altitude)
    : latitude(latitude),
      longitude(longitude),
      altitude(altitude)
{
}

location::Position::Position(const location::wgs84::Latitude& lat,
                        const location::wgs84::Longitude& lon,
                        const location::wgs84::Altitude& alt,
                        const location::units::Quantity<location::units::Length>& hor_acc)
    : location::Position::Position(lat, lon, alt)
{
    accuracy.horizontal = hor_acc;
}

location::Position::Position(const location::wgs84::Latitude& lat,
                        const location::wgs84::Longitude& lon,
                        const location::wgs84::Altitude& alt,
                        const location::units::Quantity<location::units::Length>& hor_acc,
                        const location::units::Quantity<location::units::Length>& ver_acc)
    : location::Position::Position(lat, lon, alt, hor_acc)
{
    accuracy.vertical = ver_acc;
}

bool location::Position::operator==(const location::Position& rhs) const
{
    return latitude == rhs.latitude &&
            longitude == rhs.longitude &&
            altitude == rhs.altitude &&
            accuracy.horizontal == rhs.accuracy.horizontal &&
            accuracy.vertical == rhs.accuracy.vertical;
}

bool location::Position::operator!=(const location::Position& rhs) const
{
    return !(*this == rhs);
}

std::ostream& location::operator<<(std::ostream& out, const location::Position& position)
{
    out << "Position("
        << "lat: " << position.latitude << ", "
        << "lon: " << position.longitude << ", ";
    out << "alt: ";
    if (position.altitude)
        out << *position.altitude;
    else
        out << "n/a";
    out << ", ";
    out << "hor.acc.: ";
    if (position.accuracy.horizontal)
        out << *position.accuracy.horizontal;
    else
        out << "n/a";
    out << ", ";
    out << "ver.acc.: ";
    if (position.accuracy.vertical)
        out << *position.accuracy.vertical;
    else
        out << "n/a";
    out << ")";
    return out;
}

location::units::Quantity<location::units::Length> location::haversine_distance(const location::Position& p1, const location::Position& p2)
{
    static const units::Quantity<units::Length> radius_of_earth {6371 * units::kilo * units::Meters};
    auto dLat = p2.latitude - p1.latitude;
    auto dLon = p2.longitude - p1.longitude;
    auto a =
        std::pow(units::sin(dLat.value/2.), 2) +
        std::pow(units::sin(dLon.value/2.), 2) * units::cos(p1.latitude.value) * units::cos(p2.latitude.value);

    auto c = 2. * std::atan2(std::sqrt(a), std::sqrt(1.-a));
    return radius_of_earth * c;
}

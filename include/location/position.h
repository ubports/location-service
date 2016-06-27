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
#ifndef LOCATION_POSITION_H_
#define LOCATION_POSITION_H_

#include <location/wgs84/altitude.h>
#include <location/wgs84/latitude.h>
#include <location/wgs84/longitude.h>

#include <location/optional.h>

#include <bitset>
#include <ostream>

namespace location
{
/**
 * @brief The Position struct models a position in the wgs84 coordinate system.
 */
struct Position
{
    struct Accuracy
    {
        typedef units::Quantity<units::Length> Horizontal;
        typedef units::Quantity<units::Length> Vertical;

        Optional<Horizontal> horizontal{};
        Optional<Vertical> vertical{};
    };

    Position() = default;
    Position(const wgs84::Latitude&, const wgs84::Longitude&);
    Position(const wgs84::Latitude&, const wgs84::Longitude&, const wgs84::Altitude&);
    Position(const wgs84::Latitude&, const wgs84::Longitude&, const wgs84::Altitude&, const units::Quantity<units::Length>& hor_acc);
    Position(const wgs84::Latitude&, const wgs84::Longitude&, const wgs84::Altitude&, const units::Quantity<units::Length>& hor_acc, const units::Quantity<units::Length>& ver_acc);

    bool operator==(const Position& rhs) const;
    bool operator!=(const Position& rhs) const;

    wgs84::Latitude latitude = wgs84::Latitude{};
    wgs84::Longitude longitude = wgs84::Longitude{};
    Optional<wgs84::Altitude> altitude = Optional<wgs84::Altitude>{};
    Accuracy accuracy{};
};

std::ostream& operator<<(std::ostream& out, const Position& position);

units::Quantity<units::Length> haversine_distance(const Position& p1, const Position& p2);
}

#endif // LOCATION_POSITION_H_

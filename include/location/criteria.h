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
#ifndef LOCATION_CRITERIA_H_
#define LOCATION_CRITERIA_H_

#include <location/features.h>
#include <location/optional.h>
#include <location/units/units.h>

namespace location
{
/**
 * @brief Summarizes criteria of a client session with respect to functionality
 * and accuracy for position, velocity and heading measurements.
 */
struct Criteria
{
    Features requirements; ///< These features are required.

    struct Accuracy
    {
        Optional<units::Meters> horizontal;         ///< The client requires measurements of at least this horizontal accuracy.
        Optional<units::Meters> vertical;           ///< The client requires measurements of at least this vertical accuracy.
        Optional<units::MetersPerSecond> velocity;  ///< The client requires measurements of at least this velocity accuracy.
        Optional<units::Degrees> heading;           ///< The client requires measurements of at least this heading accuracy.
    } accuracy = Accuracy{};
};
}

#endif // LOCATION_CRITERIA_H_

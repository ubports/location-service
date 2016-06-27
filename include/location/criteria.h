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
    /**
     * @brief satisfies checks whether this instance also satisfies another criteria instance.
     * @param rhs The other criteria instance
     * @return true iff this instance also satisfies the other instance, else false.
     */
    bool satisfies(const Criteria& rhs) const;

    struct Requires
    {
        bool position = true; ///< The client needs position measurements.
        bool altitude = false; ///< The client needs altitude measurements.
        bool velocity = false; ///< The client needs velocity measurments.
        bool heading = false; ///< The client needs heading measurements.
    } requires = Requires{};

    struct Accuracy
    {
        units::Quantity<units::Length> horizontal = 3000 * units::Meters; ///< The client requires measurements of at least this horizontal accuracy.
        Optional<units::Quantity<units::Length>> vertical; ///< The client requires measurements of at least this vertical accuracy.
        Optional<units::Quantity<units::Velocity>> velocity; ///< The client requires measurements of at least this velocity accuracy.
        Optional<units::Quantity<units::PlaneAngle>> heading; ///< The client requires measurements of at least this heading accuracy.
    } accuracy = Accuracy{};
};

/**
 * @brief operator + merges lhs and rhs such that satisfying the new criteria satisfies lhs and rhs.
 */
Criteria operator+(const Criteria& lhs, const Criteria& rhs);
}

#endif // LOCATION_CRITERIA_H_

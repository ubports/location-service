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

#include <location/units/units.h>

#include <location/optional.h>

#include <iosfwd>
#include <memory>

namespace location
{
/// @brief The Position struct models a position in the wgs84 coordinate system.
///
/// Position features a named parameter interface that allows to gradually refine
/// a Position instance as in:
///   auto pos = Position{51. * units::degrees, 7. * units::degrees}.altitude{5. * units::meters};
class Position
{
public:
    /// @brief Accuracy bundles up optional horizontal and vertical
    /// error estimates.
    ///
    /// Accuracy features a named parameter interface, such that calling
    /// code can gradually assemble an Accuracy as in:
    ///   auto acc = location::Accuracy{}.vertical(3.* location::units::meters);
    class Accuracy
    {
    public:
        /// @cond
        Accuracy();
        ~Accuracy();
        Accuracy(const Accuracy&);
        Accuracy(Accuracy&&);
        Accuracy& operator=(const Accuracy&);
        Accuracy& operator=(Accuracy&&);
        bool operator==(const Accuracy& rhs) const;
        bool operator!=(const Accuracy& rhs) const;
        /// @endcond

        /// @brief horizontal returns an immutable reference to the optional horizontal error estimate.
        const Optional<units::Meters>& horizontal() const;
        /// @brief horizontal returns a mutable reference to the optional horizontal error estimate.
        Optional<units::Meters>& horizontal();
        /// @brief horizontal adjusts the horizontal error estimate to value.
        Accuracy& horizontal(const units::Meters& value);
        /// @brief horizontal adjusts the horizontal error estimate to value.
        Accuracy horizontal(const units::Meters& value) const;

        /// @brief vertical returns an immutable reference to the optional vertical error estimate.
        const Optional<units::Meters>& vertical() const;
        /// @brief vertical returns an immutable reference to the optional vertical error estimate.
        Optional<units::Meters>& vertical();
        /// @brief vertical adjusts the vertical error estimate to value.
        Accuracy& vertical(const units::Meters& value);
        /// @brief vertical adjusts the vertical error estimate to value.
        Accuracy vertical(const units::Meters& value) const;

    private:
        /// @cond
        struct Private;
        std::unique_ptr<Private> d;
        /// @endcond
    };

    /// Position initializes a new instance with latitude and longitude.
    explicit Position(const units::Degrees& latitude = units::Degrees{0}, const units::Degrees& longitude = units::Degrees{0});

    /// @cond
    ~Position();
    Position(const Position&);
    Position(Position&&);
    Position& operator=(const Position&);
    Position& operator=(Position&&);
    bool operator==(const Position& rhs) const;
    bool operator!=(const Position& rhs) const;
    /// @endcond

    /// @brief latitude returns an immutable reference to the latitude component of the position.
    const units::Degrees& latitude() const;
    /// @brief latitude returns a mutable reference to the latitude component of the position.
    units::Degrees& latitude();
    /// @brief latitute adjusts the latitude component to value.
    Position& latitude(const units::Degrees& value);
    /// @brief latitute adjusts the latitude component to value.
    Position latitude(const units::Degrees& value) const;

    /// @brief longitude returns an immutable reference to the longitude component of the position.
    const units::Degrees& longitude() const;
    /// @brief longitude returns a mutable reference to the longitude component of the position.
    units::Degrees& longitude();
    /// @brief longitude adjusts the longitude component to value.
    Position& longitude(const units::Degrees& value);
    /// @brief longitude adjusts the longitude component to value.
    Position longitude(const units::Degrees& value) const;

    /// @brief altitude returns an immutable reference to the optional altitude component of the position.
    const Optional<units::Meters>& altitude() const;
    /// @brief altitude returns a mutable reference to the optional altitude component of the position.
    Optional<units::Meters>& altitude();
    /// @brief altitude adjusts the altitude component to value.
    Position& altitude(const units::Meters& value);
    /// @brief altitude adjusts the altitude component to value.
    Position altitude(const units::Meters& value) const;

    /// @brief accuracy returns an immutable reference to the accuracy of the position estimate.
    const Accuracy& accuracy() const;
    /// @brief accuracy returns a mutable reference to the accuracy of the position estimate.
    Accuracy& accuracy();

private:
    struct Private;
    std::unique_ptr<Private> d;
};

/// @brief operator<< inserts position into out.
std::ostream& operator<<(std::ostream& out, const Position& position);
/// @brief operator>> extracts position from in.
std::istream& operator>>(std::istream& in, Position& position);

/// @brief haversine_distance returns the distance between p1 and p2 according
/// to the haversine distance.
///
/// See https://en.wikipedia.org/wiki/Haversine_formula for background.
units::Meters haversine_distance(const Position& p1, const Position& p2);
}

#endif // LOCATION_POSITION_H_

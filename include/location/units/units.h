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
#ifndef LOCATION_UNITS_UNITS_H_
#define LOCATION_UNITS_UNITS_H_

#include <location/visibility.h>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/angle/gradians.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/prefixes.hpp>

namespace location
{
/// @brief units bundles up all physical units of measurement
/// used in locationd.
namespace units
{
/// @brief Quantity models an actual measurement with unit Unit.
///
/// By default, we configure floating point accuracy.
template<typename Unit>
using Quantity = boost::units::quantity<Unit, double>;

/// @brief Degrees is a measurement of a plane angle.
using Degrees = Quantity<boost::units::degree::plane_angle>;

/// @brief degrees is a static instance of Degrees.
///
/// Enables safe construction of Degrees instances as in:
///   auto measurement = 1.5 * degrees;
static typename Degrees::unit_type degrees;

/// @brief Meters is a measurement of a length.
using Meters = Quantity<boost::units::si::length>;

/// @brief meters is a static instance of Meters.
///
/// Enables safe construction of Meters instances as in:
///   auto measurement = 1.5 * meters;
static typename Meters::unit_type meters;

/// @brief MetersPerSecond is a measurement of a velocity
using MetersPerSecond = Quantity<boost::units::si::velocity>;

/// @brief meters_per_second is a static instance of Meters.
///
/// Enables safe construction of Meters instances as in:
///   auto measurement = 1.5 * meters;
static typename MetersPerSecond::unit_type meters_per_second;

/// @cond
///
/// Pull in some common helpers to ease interaction
/// with Quantity instances. In particular, automagic
/// handling of trigonometric functions no matter if radians or
/// degrees are passed in.
using boost::units::si::kilo;
using boost::units::sin;
using boost::units::cos;
using boost::units::atan2;
/// @endcond

/// @brief roughly_equals determines whether two Quantity<> are mostly equal
/// except for floating point noise.
template<typename Unit>
LOCATION_DLL_PUBLIC inline bool roughly_equals(const Quantity<Unit>& lhs, const Quantity<Unit>& rhs)
{
    return std::fabs(lhs.value()-rhs.value()) <= std::numeric_limits<double>::epsilon();
}

/// @brief raw returns the untyped numeric value contained in a Quantity<T> instance.
///
/// Please use this with caution and only to cross api boundaries.
template<typename T>
LOCATION_DLL_PUBLIC inline typename Quantity<T>::value_type raw(const Quantity<T>& quantity)
{
    return quantity.value();
}
}
}

#endif // LOCATION_UNITS_UNITS_H_


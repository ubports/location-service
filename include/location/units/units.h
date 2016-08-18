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

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/angle/gradians.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/prefixes.hpp>

namespace location
{
namespace units
{
template<typename Unit>
using Quantity = boost::units::quantity<Unit, double>;

typedef Quantity<boost::units::degree::plane_angle> Degrees;
static boost::units::degree::plane_angle degrees;

typedef Quantity<boost::units::si::length> Meters;
static boost::units::si::length meters;

typedef Quantity<boost::units::si::velocity> MetersPerSecond;
static boost::units::si::velocity meters_per_second;

using boost::units::si::kilo;

using boost::units::sin;
using boost::units::cos;
using boost::units::atan2;

template<typename Unit>
inline bool roughly_equals(const Quantity<Unit>& lhs, const Quantity<Unit>& rhs)
{
    return std::fabs(lhs.value()-rhs.value()) <= std::numeric_limits<double>::epsilon();
}

template<typename T>
inline typename Quantity<T>::value_type raw(const Quantity<T>& quantity)
{
    return quantity.value();
}
}
}

#endif // LOCATION_UNITS_UNITS_H_


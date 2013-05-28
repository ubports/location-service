#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_UNITS_UNITS_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_UNITS_UNITS_H_

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/angle/gradians.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/prefixes.hpp>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace units
{
typedef boost::units::degree::plane_angle PlaneAngle;
static const PlaneAngle Degree;
static const PlaneAngle Degrees;
typedef boost::units::gradian::plane_angle Gradians;

typedef boost::units::si::length Length;
static const Length Meter;
static const Length Meters;

using boost::units::si::kilo;

typedef boost::units::si::velocity Velocity;
static const Velocity MeterPerSecond;
static const Velocity MetersPerSecond;

template<typename Unit>
using Quantity = boost::units::quantity<Unit, double>;

typedef boost::units::si::dimensionless Dimensionless;

using boost::units::sin;
using boost::units::cos;
using boost::units::atan2;
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_UNITS_UNITS_H_


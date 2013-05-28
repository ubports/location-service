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

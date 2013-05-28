#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LATITUDE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LATITUDE_H_

#include "com/ubuntu/location/units/units.h"
#include "com/ubuntu/location/wgs84/coordinate.h"

#include <stdexcept>

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
struct Latitude;
}
typedef Coordinate<tag::Latitude, units::PlaneAngle> Latitude;

template<>
struct CoordinateTraits<Latitude>
{
    static double min()
    {
        return -90;
    };
    static double max()
    {
        return 90;
    };

    static void check_and_throw_if_invalid(const typename Latitude::Quantity& coordinate)
    {
        if (coordinate.value() < min())
            throw std::out_of_range(u8"Latitude(coordinate.value() < min())");
        if (coordinate.value() > max())
            throw std::out_of_range(u8"Latitude(coordinate.value() > max())");
    }
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LATITUDE_H_

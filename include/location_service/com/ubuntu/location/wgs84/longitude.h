#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LONGITUDE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LONGITUDE_H_

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
struct Longitude;
}
typedef Coordinate<tag::Longitude, units::PlaneAngle> Longitude;

template<>
struct CoordinateTraits<Longitude>
{
    static double min()
    {
        return -180;
    };
    static double max()
    {
        return 180;
    };

    static void check_and_throw_if_invalid(const typename Longitude::Quantity& coordinate)
    {
        if (coordinate.value() < min())
            throw std::out_of_range(u8"Longitude(coordinate.value() < min())");
        if (coordinate.value() > max())
            throw std::out_of_range(u8"Longitude(coordinate.value() > max())");
    }
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_LATITUDE_H_

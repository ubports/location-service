#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_COORDINATE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_COORDINATE_H_

#include "com/ubuntu/location/units/units.h"

#include <ostream>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace wgs84
{
template<typename Coordinate>
struct CoordinateTraits
{
    static void check_and_throw_if_invalid(const typename Coordinate::Quantity& coordinate)
    {
    }
};

template<typename Tag, typename UnitType>
struct Coordinate
{
    typedef UnitType Unit;
    typedef units::Quantity<Unit> Quantity;

    explicit Coordinate(const Quantity& value = Quantity()) : value(value)
    {
        CoordinateTraits<Coordinate<Tag,UnitType>>::check_and_throw_if_invalid(value);
    }

    bool operator==(const Coordinate<Tag,UnitType>& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const Coordinate<Tag,UnitType>& rhs) const
    {
        return value != rhs.value;
    }

    Quantity value;
};

template<typename T, typename U>
Coordinate<T,U> operator-(const Coordinate<T,U>& lhs, const Coordinate<T,U>& rhs)
{
    return Coordinate<T,U> {rhs.value - lhs.value};
}

template<typename T, typename U>
std::ostream& operator<<(std::ostream& out, const Coordinate<T,U>& coord)
{
    out << "Coordinate(" << coord.value << ")";
    return out;
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_WGS84_COORDINATE_H_

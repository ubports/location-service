#include "com/ubuntu/location/position.h"

#include <bitset>
#include <ostream>

namespace cul = com::ubuntu::location;

cul::Position::Position() : fields{Flags("000"), wgs84::Latitude{}, wgs84::Longitude{}, wgs84::Altitude{}}
{
}

cul::Position::Position(
    const cul::wgs84::Latitude& latitude,
    const cul::wgs84::Longitude& longitude)
        : fields{Flags("011"), latitude, longitude, wgs84::Altitude{}}
{
}          

cul::Position::Position(
    const cul::wgs84::Latitude& latitude,
    const cul::wgs84::Longitude& longitude,
    const cul::wgs84::Altitude& altitude)
        : fields{Flags{"111"}, latitude, longitude, altitude}
{
}

bool cul::Position::operator==(const cul::Position& rhs) const
{
    return fields.latitude == rhs.fields.latitude && fields.longitude == rhs.fields.longitude && fields.altitude == rhs.fields.altitude;
}

bool cul::Position::operator!=(const cul::Position& rhs) const
{
    return !(fields.latitude == rhs.fields.latitude && fields.longitude == rhs.fields.longitude && fields.altitude == rhs.fields.altitude);
}

const cul::Position::Flags& cul::Position::flags() const
{
    return fields.flags;
}

bool cul::Position::has_latitude() const
{
    return fields.flags.test(latitude_flag);
}

cul::Position& cul::Position::latitude(const cul::wgs84::Latitude& lat)
{
    fields.flags.set(latitude_flag);
    fields.latitude = lat;
    return *this;
}

const cul::wgs84::Latitude& cul::Position::latitude() const
{
    return fields.latitude;
}

bool cul::Position::has_longitude() const
{
    return fields.flags.test(longitude_flag);
}

cul::Position& cul::Position::longitude(const cul::wgs84::Longitude& lon)
{
    fields.flags.set(longitude_flag);
    fields.longitude = lon;
    return *this;
}

const cul::wgs84::Longitude& cul::Position::longitude() const
{
    return fields.longitude;
}

bool cul::Position::has_altitude() const
{
    return fields.flags.test(altitude_flag);
}

cul::Position& cul::Position::altitude(const cul::wgs84::Altitude& alt)
{
    fields.flags.set(altitude_flag);
    fields.altitude = alt;
    return *this;
}

const cul::wgs84::Altitude& cul::Position::altitude() const
{
    return fields.altitude;
}

std::ostream& cul::operator<<(std::ostream& out, const cul::Position& position)
{
    out << "Position(" << position.latitude() << ", " << position.longitude() << ", " << position.altitude() << ")";
    return out;
}

cul::units::Quantity<cul::units::Length> cul::haversine_distance(const cul::Position& p1, const cul::Position& p2)
{
    static const units::Quantity<units::Length> radius_of_earth {6371 * units::kilo* units::Meters};
    auto dLat = p2.latitude() - p1.latitude();
    auto dLon = p2.longitude() - p1.longitude();
    auto a =
        std::pow(units::sin(dLat.value/2.), 2) +
        std::pow(units::sin(dLon.value/2.), 2) * units::cos(p1.latitude().value) * units::cos(p2.latitude().value);

    auto c = 2. * std::atan2(std::sqrt(a), std::sqrt(1.-a));
    return radius_of_earth * c;
}
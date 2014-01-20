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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SPACE_VEHICLE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SPACE_VEHICLE_H_

#include <com/ubuntu/location/units/units.h>

#include <iostream>
#include <map>

namespace com
{
namespace ubuntu
{
namespace location
{
/** @brief A space-vehicle as visible to providers. */
struct SpaceVehicle
{
    /** @brief Numeric Id of an individual SpaceVehicle. */
    typedef std::uint32_t Id;

    enum class Type
    {
        unknown, ///< Unknown (and thus invalid) type.
        beidou, ///< People's Republic of China's regional system, currently limited to Asia and the West Pacific
        galileo, ///< A global system being developed by the European Union and other partner countries, planned to be operational by 2014 (and fully deployed by 2019).
        glonass, ///< Russia's global navigation system. Fully operational worldwide.
        gps, ///< Fully operational worldwide.
        compass, ///< People's Republic of China's global system, planned to be operational by 2020.
        irnss, ///< India's regional navigation system, planned to be operational by 2014, covering India and Northern Indian Ocean.
        qzss ///< Japanese regional system covering Asia and Oceania.
    };    

    struct Key
    {
        Type type = Type::unknown; ///< The positioning system this vehicle belongs to.
        Id id = 0; ///< Unique id of the space vehicle.

        inline bool operator==(const SpaceVehicle::Key& rhs) const
        {
            return type == rhs.type && id == rhs.id;
        }

        inline bool operator<(const SpaceVehicle::Key& rhs) const
        {
            if (type != rhs.type)
                return type < rhs.type;

            return id < rhs.id;
        }
    };

    inline bool operator==(const SpaceVehicle& rhs) const
    {
        return key == rhs.key &&
                has_almanac_data == rhs.has_almanac_data &&
                has_ephimeris_data == rhs.has_ephimeris_data &&
                used_in_fix == rhs.used_in_fix &&
                units::roughly_equals(azimuth, rhs.azimuth) &&
                units::roughly_equals(elevation, rhs.elevation);
    }

    inline bool operator<(const SpaceVehicle& rhs) const
    {
        return key < rhs.key;
    }

    Key key; ///< Unique key identifying an instance.
    float snr = -std::numeric_limits<float>::max(); ///< Signal to noise ratio;
    bool has_almanac_data = false; ///< Almanac data available for this vehicle.
    bool has_ephimeris_data = false; ///< Ephimeris data is available for this vehicle.
    bool used_in_fix = false;  ///< This vehicle has been used to obtain a fix.
    units::Quantity<units::PlaneAngle> azimuth; ///< Azimuth of SV.
    units::Quantity<units::PlaneAngle> elevation; ///< Elevation of SV.
};

inline std::ostream& operator<<(std::ostream& out, const SpaceVehicle& sv)
{
    static const std::map<SpaceVehicle::Type, std::string> lut =
    {
        {SpaceVehicle::Type::unknown, "unknown"},
        {SpaceVehicle::Type::beidou, "beidou"},
        {SpaceVehicle::Type::galileo, "galileo"},
        {SpaceVehicle::Type::glonass, "glonass"},
        {SpaceVehicle::Type::gps, "gps"},
        {SpaceVehicle::Type::compass, "compass"},
        {SpaceVehicle::Type::irnss, "irnss"},
        {SpaceVehicle::Type::qzss, "qzss" }
    };
    return out << "("
               << "type: " << lut.at(sv.key.type) << ", "
               << "prn: " << sv.key.id << ", "
               << "snr: " << sv.snr << ", "
               << "has_almanac_data: " << sv.has_almanac_data << ", "
               << "has_ephimeris_data: " << sv.has_ephimeris_data << ", "
               << "used_in_fix: " << sv.used_in_fix << ", "
               << "azimuth: " << sv.azimuth << ", "
               << "elevation: " << sv.elevation
               << ")";
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SPACE_VEHICLE_H_

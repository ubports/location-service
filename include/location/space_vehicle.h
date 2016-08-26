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
#ifndef LOCATION_SPACE_VEHICLE_H_
#define LOCATION_SPACE_VEHICLE_H_

#include <location/optional.h>
#include <location/visibility.h>
#include <location/units/units.h>

#include <iostream>
#include <map>

namespace location
{
/// @brief A space-vehicle as visible to providers relying on satellite-assisted positioning.
///
/// Features a named-parameter interface such that instances can be assembled in place as in:
///   SpaceVehicle({SpaceVehicle::Type::gps, 42}).snr(-23).azimuth(34. * units::degrees).elevation(12 * units::degrees);
class LOCATION_DLL_PUBLIC SpaceVehicle
{
public:
    /// @brief Numeric Id of an individual SpaceVehicle.
    typedef std::uint32_t Id;

    /// @brief Enumerates all known space-vehicle types.
    enum class Type
    {
        unknown,    ///< Unknown (and thus invalid) type.
        beidou,     ///< People's Republic of China's regional system, currently limited to Asia and the West Pacific
        galileo,    ///< A global system being developed by the European Union and other partner countries, planned to be operational by 2014 (and fully deployed by 2019).
        glonass,    ///< Russia's global navigation system. Fully operational worldwide.
        gps,        ///< Fully operational worldwide.
        compass,    ///< People's Republic of China's global system, planned to be operational by 2020.
        irnss,      ///< India's regional navigation system, planned to be operational by 2014, covering India and Northern Indian Ocean.
        qzss        ///< Japanese regional system covering Asia and Oceania.
    };

    /// @brief Uniquely identifies a space vehicle, given its type and its id.
    class Key
    {
    public:
        /// @brief Key initializes a new instance with type and id.
        explicit Key(Type type = Type::unknown, Id id = Id{0});
        /// @cond
        Key(const Key& other);
        Key(Key&& other);
        ~Key();
        Key& operator=(const Key& other);
        Key& operator=(Key&& other);
        /// @endcond

        /// @brief type returns the Type of a SpaceVehicle.
        const Type& type() const;
        /// @brief id returns the numeric id of a SpaceVehicle.
        const Id& id() const;

    private:
        /// @cond
        struct Private;
        std::unique_ptr<Private> d;
        /// @endcond
    };

    /// @brief SpaceVehicle initializes a new instance with key.
    explicit SpaceVehicle(const Key& key = Key{});
    /// @cond
    SpaceVehicle(const SpaceVehicle& other);
    SpaceVehicle(SpaceVehicle&& other);
    ~SpaceVehicle();
    SpaceVehicle& operator=(const SpaceVehicle& other);
    SpaceVehicle& operator=(SpaceVehicle&& other);
    /// @endcond

    /// @brief Unique key identifying an instance.
    const Key& key() const;
    Key& key();
    SpaceVehicle& key(const Key& value);
    SpaceVehicle key(const Key& value) const;
    /// @brief Signal to noise ratio.
    const Optional<float>& snr() const;
    Optional<float>& snr();
    SpaceVehicle& snr(float value);
    SpaceVehicle snr(float value) const;
    /// @brief Returns true if almanac data is available for this vehicle.
    const Optional<bool>& has_almanac_data() const;
    Optional<bool>& has_almanac_data();
    SpaceVehicle& has_almanac_data(bool value);
    SpaceVehicle has_almanac_data(bool value) const;
    /// @brief Returns true if ephemeris data is available for this vehicle.
    const Optional<bool>& has_ephimeris_data() const;
    Optional<bool>& has_ephimeris_data();
    SpaceVehicle& has_ephimeris_data(bool value);
    SpaceVehicle has_ephimeris_data(bool value) const;
    /// @brief Returns true if this vehicle has been used to obtain a fix.
    const Optional<bool>& used_in_fix() const;
    Optional<bool>& used_in_fix();
    SpaceVehicle& used_in_fix(bool value);
    SpaceVehicle used_in_fix(bool value) const;
    /// @brief Returns the azimuth of the vehicle.
    const Optional<units::Degrees>& azimuth() const;
    Optional<units::Degrees>& azimuth();
    SpaceVehicle& azimuth(const units::Degrees& value);
    SpaceVehicle azimuth(const units::Degrees& value) const;
    /// @brief Returns the elevation of the vehicle.
    const Optional<units::Degrees>& elevation() const;
    Optional<units::Degrees>& elevation();
    SpaceVehicle& elevation(const units::Degrees& value);
    SpaceVehicle elevation(const units::Degrees& value) const;

private:
    /// @cond
    struct Private;
    std::unique_ptr<Private> d;
    /// @endcond
};

/// @brief operator== returns true iff lhs and rhs are equal.
LOCATION_DLL_PUBLIC bool operator==(const SpaceVehicle& lhs, const SpaceVehicle& rhs);
/// @brief operator< returns true iff lhs and rhs are equal.
LOCATION_DLL_PUBLIC bool operator<(const SpaceVehicle& lhs, const SpaceVehicle& rhs);
/// @brief operator== returns true iff lhs and rhs are equal.
LOCATION_DLL_PUBLIC bool operator==(const SpaceVehicle::Key& lhs, const SpaceVehicle::Key& rhs);
/// @brief operator< returns true if lhs compares less than rhs.
LOCATION_DLL_PUBLIC bool operator<(const SpaceVehicle::Key& lhs, const SpaceVehicle::Key& rhs);

/// @brief operator<< inserts type into out.
LOCATION_DLL_PUBLIC std::ostream& operator<<(std::ostream& out, const SpaceVehicle::Type& type);
/// @brief operator<< inserts key into out.
LOCATION_DLL_PUBLIC std::ostream& operator<<(std::ostream& out, const SpaceVehicle::Key& key);
/// @brief operator<< inserts sv into out.
LOCATION_DLL_PUBLIC std::ostream& operator<<(std::ostream& out, const SpaceVehicle& sv);
}

#endif // LOCATION_SPACE_VEHICLE_H_

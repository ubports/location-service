// Copyright (C) 2017 Thomas Voss <thomas.voss@canonical.com>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LOCATION_PROVIDERS_SIRF_GEODETIC_NAVIGATION_DATA_H_
#define LOCATION_PROVIDERS_SIRF_GEODETIC_NAVIGATION_DATA_H_

#include <location/providers/sirf/reader.h>

#include <cstdint>
#include <iostream>

namespace location
{
namespace providers
{
namespace sirf
{

struct GeodeticNavigationData
{
    enum NavValid
    {
        valid_navigation = 0,
        solution_not_overdetermined = 1 << 0,
        invalid_dr_sensor_data = 1 << 3,
        invalid_dr_calibration = 1 << 4,
        unavailable_dr_gps_calibration = 1 << 5,
        invalid_dr_position_fix = 1 << 6,
        invalid_heading = 1 << 7,
        no_tracker_data_available = 1 << 15
    };

    enum NavType
    {
        no_navigation_fix = 0,
        one_sv_kf_solution = 1,
        two_sv_kf_solution = 2,
        three_sv_kf_solution = 3,
        four_sv_kf_solution = 4,
        two_dim_least_squares_solution = 5,
        three_dim_least_squares_solution = 6,
        dr_solution = 7,
        trickle_power_in_use = 1 << 3,
        altitude_hold_off = 0,
        altitude_hold_kf = 1,
        altitude_hold_user_input = 2,
        altitude_hold_user_input_always = 3,
        dop_limits_exceeded = 1 << 6,
        dgps_corrections_applied = 1 << 7,
        dr_solution_type_sensor = 1,
        dr_solution_type_velocity = 0,
        navigation_solution_overdetermined = 1 << 9,
        velocity_dr_timeout_exceeded = 1 << 10,
        fix_edited_by_mi_functions = 1 << 11,
        invalid_velocity = 1 << 12,
        altitude_hold_disabled = 1 << 13,

    };

    static constexpr std::uint8_t id{0x29};

    std::size_t size() const
    {
        return 91;
    }

    void read(Reader& reader)
    {
        nav_valid = reader.read_unsigned_short();
        nav_type = reader.read_unsigned_short();
        extended_week_number = reader.read_unsigned_short();
        time_of_week = reader.read_unsigned_long();
        utc.year = reader.read_unsigned_short();
        utc.month = reader.read_unsigned_char();
        utc.day = reader.read_unsigned_char();
        utc.hour = reader.read_unsigned_char();
        utc.minute = reader.read_unsigned_char();
        utc.seconds = reader.read_unsigned_short();
        satellite_id_list = reader.read_unsigned_long();
        latitude = reader.read_signed_long() * 1e-7;
        longitude = reader.read_signed_long() * 1e-7;
        altitude.above_ellipsoid = reader.read_signed_long() * 1e-2;
        altitude.above_mean_sea_level = reader.read_signed_long() * 1e-2;
        map_datum = reader.read_signed_char();
        over_ground.speed = reader.read_unsigned_short() * 1e-2;
        over_ground.course = reader.read_unsigned_short() * 1e-2;
        magnetic_variation = reader.read_signed_short();
        climb_rate = reader.read_signed_short() * 1e-2;
        heading_rate = reader.read_signed_short() * 1e-2;
        error.horizontal_position = reader.read_unsigned_long() * 1e-2;
        error.vertical_position = reader.read_unsigned_long() * 1e-2;
        error.time = reader.read_unsigned_long() * 1e-2;
        error.horizontal_velocity = reader.read_unsigned_short() * 1e-2;
        clock.bias = reader.read_signed_long() * 1e-2;
        clock.bias_error = reader.read_unsigned_long() * 1e-2;
        clock.drift = reader.read_signed_long() * 1e-2;
        clock.drift_error = reader.read_unsigned_long() * 1e-2;
        distance_traveled = reader.read_unsigned_long();
        distance_traveled_error = reader.read_unsigned_short();
        error.heading = reader.read_unsigned_short() * 1e-2;
        svs_in_fix = reader.read_unsigned_char();
        hdop = reader.read_unsigned_char() / 5.f;
        mode_info = reader.read_unsigned_char();
    }

    std::uint16_t nav_valid;
    std::uint16_t nav_type;
    std::uint16_t extended_week_number;
    std::uint32_t time_of_week;
    struct
    {
        std::uint16_t year;
        std::uint8_t month;
        std::uint8_t day;
        std::uint8_t hour;
        std::uint8_t minute;
        std::uint16_t seconds;
    } utc;
    std::uint32_t satellite_id_list;
    float latitude;
    float longitude;
    struct
    {
        float above_ellipsoid;
        float above_mean_sea_level;
    } altitude;
    std::int8_t map_datum;
    struct
    {
        std::uint16_t speed;
        std::uint16_t course;
    } over_ground;
    std::int16_t magnetic_variation;
    float climb_rate;
    float heading_rate;
    struct
    {
        float horizontal_position;
        float vertical_position;
        float time;
        float horizontal_velocity;
        float heading;
    } error;
    struct
    {
        float bias;
        float bias_error;
        float drift;
        float drift_error;
    } clock;
    std::uint32_t distance_traveled;
    std::uint32_t distance_traveled_error;
    std::uint8_t svs_in_fix;
    std::uint8_t hdop;
    std::uint8_t mode_info;
};

inline std::ostream& operator<<(std::ostream& out, const GeodeticNavigationData& gnd)
{
    return out << "GeodeticNavigationData:" << std::endl
               << "  nav-valid: " << gnd.nav_valid << std::endl
               << "  nav-type: " << gnd.nav_type << std::endl
               << "  extended-week-number: " << gnd.extended_week_number << std::endl
               << "  time-of-week: " << gnd.time_of_week << std::endl
               << "  utc.year: " << gnd.utc.year << std::endl
               << "  utc.month: " << std::uint32_t(gnd.utc.month) << std::endl
               << "  utc.day: " << std::uint32_t(gnd.utc.day) << std::endl
               << "  utc.hour: " << std::uint32_t(gnd.utc.hour) << std::endl
               << "  utc.minute: " << std::uint32_t(gnd.utc.minute) << std::endl
               << "  utc.seconds: " << gnd.utc.seconds << std::endl
               << "  satellite-id-list: " << gnd.satellite_id_list << std::endl
               << "  latitude: " << gnd.latitude << std::endl
               << "  longitude: " << gnd.longitude << std::endl
               << "  altitude.above-ellipsoid: " << gnd.altitude.above_ellipsoid << std::endl
               << "  altitude.above-msl: " << gnd.altitude.above_mean_sea_level << std::endl
               << "  map-datum: " << std::int32_t(gnd.map_datum) << std::endl
               << "  over-ground.speed: " << gnd.over_ground.speed << std::endl
               << "  over-ground.course: " << gnd.over_ground.course << std::endl
               << "  climb-rate: " << gnd.climb_rate << std::endl
               << "  heading-rate: " << gnd.heading_rate << std::endl
               << "  error.horizontal-position: " << gnd.error.horizontal_position << std::endl
               << "  error.vertical-position: " << gnd.error.vertical_position << std::endl
               << "  error.time: " << gnd.error.time << std::endl
               << "  error.horizontal-velocity: " << gnd.error.horizontal_velocity << std::endl
               << "  error.heading: " << gnd.error.heading << std::endl
               << "  clock.bias: " << gnd.clock.bias << std::endl
               << "  clock.bias-error: " << gnd.clock.bias_error << std::endl
               << "  clock.drift: " << gnd.clock.drift << std::endl
               << "  clock.drift-error: " << gnd.clock.drift_error << std::endl
               << "  distance-traveled: " << gnd.distance_traveled << std::endl
               << "  distance-traveled-error: " << gnd.distance_traveled_error << std::endl
               << "  svs-in-fix: " << std::uint32_t(gnd.svs_in_fix) << std::endl
               << "  hdop: " << std::uint32_t(gnd.hdop) << std::endl
               << "  mode-info: " << std::uint32_t(gnd.mode_info);
}

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif  // LOCATION_PROVIDERS_SIRF_GEODETIC_NAVIGATION_DATA_H_

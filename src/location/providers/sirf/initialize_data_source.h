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

#ifndef LOCATION_PROVIDERS_SIRF_INITIALIZE_DATA_SOURCE_H_
#define LOCATION_PROVIDERS_SIRF_INITIALIZE_DATA_SOURCE_H_

#include <location/providers/sirf/writer.h>

#include <cstdint>
#include <iostream>

namespace location
{
namespace providers
{
namespace sirf
{

struct InitializeDataSource
{
    enum ResetConfiguration
    {
        data_valid = 1 << 0,
        clear_ephemeris_data = 1 << 1,
        clear_all_history = 1 << 2,
        factory_reset = 1 << 3,
        enable_nav_lib_data = 1 << 4,
        enable_debug_data = 1 << 5,
        rtc_is_not_precise = 1 << 6,
        reset = 1 << 7
    };

    static constexpr std::uint8_t id{128};
    static constexpr std::uint8_t max_number_channels{12};

    std::size_t size() const
    {
        return 25;
    }

    void write(Writer& writer) const
    {
        writer.write_unsigned_char(id);
        writer.write_signed_long(ecef_x);
        writer.write_signed_long(ecef_y);
        writer.write_signed_long(ecef_z);
        writer.write_signed_long(clock_drift);
        writer.write_unsigned_long(ecef_x);
        writer.write_unsigned_short(week_number);
        writer.write_unsigned_char(channels);
        writer.write_unsigned_char(reset_configuration);
    }

    std::int32_t ecef_x;
    std::int32_t ecef_y;
    std::int32_t ecef_z;
    std::int32_t clock_drift;
    std::uint32_t time_of_week;
    std::uint16_t week_number;
    std::uint8_t channels;
    std::uint8_t reset_configuration;
};

inline std::ostream& operator<<(std::ostream& out, const InitializeDataSource& ids)
{
    return out << "InitializeDataSource:" << std::endl
               << "  ecef-x: " << ids.ecef_x << std::endl
               << "  ecef-y: " << ids.ecef_y << std::endl
               << "  ecef-z: " << ids.ecef_z << std::endl
               << "  clock-drift: " << ids.clock_drift << std::endl
               << "  time-of-week: " << ids.time_of_week << std::endl
               << "  week_number: " << ids.week_number << std::endl
               << "  channels: " << std::uint32_t(ids.channels) << std::endl
               << "  reset-config: " << std::uint32_t(ids.reset_configuration) << std::endl;
}

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif  // LOCATION_PROVIDERS_SIRF_INITIALIZE_DATA_SOURCE_H_

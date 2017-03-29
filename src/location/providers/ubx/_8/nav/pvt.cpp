// Copyright (C) 2017 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#include <location/providers/ubx/_8/nav/pvt.h>

#include <location/providers/ubx/_8/reader.h>

#include <iostream>

namespace nav = location::providers::ubx::_8::nav;

void nav::Pvt::read(Reader& reader)
{
    itow = reader.read_unsigned_long();
    year = reader.read_unsigned_short();
    month = reader.read_unsigned_char();
    day = reader.read_unsigned_char();
    hour = reader.read_unsigned_char();
    minute = reader.read_unsigned_char();
    second = reader.read_unsigned_char();
    date_time_validity = static_cast<DateTimeValidity>(reader.read_unsigned_char());
    time_accuracy = reader.read_unsigned_long();
    nanoseconds = reader.read_signed_long();
    fix_type = static_cast<FixType>(reader.read_unsigned_char());
    fix_status_flags = reader.read_unsigned_char();
    additional_flags = reader.read_unsigned_char();
    satellite_count = reader.read_unsigned_char();
    longitude = reader.read_signed_long() * 1e-7;
    latitude = reader.read_signed_long() * 1e-7;
    height.above_ellipsoid = reader.read_signed_long();
    height.above_msl = reader.read_signed_long();
    accuracy.horizontal = reader.read_unsigned_long();
    accuracy.vertical = reader.read_unsigned_long();
    velocity.north = reader.read_signed_long();
    velocity.east = reader.read_signed_long();
    velocity.down = reader.read_signed_long();
    speed_over_ground = reader.read_unsigned_long();
    heading.motion = reader.read_signed_long() * 1e-5;
    accuracy.speed = reader.read_unsigned_long();
    accuracy.heading = reader.read_unsigned_long() * 1e-5;
    pdop = reader.read_unsigned_short() * 0.01;

    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();

    heading.vehicle = reader.read_signed_long() * 1e-5;

    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();
    reader.read_unsigned_char();
}

std::ostream& nav::operator<<(std::ostream& out, nav::Pvt::DateTimeValidity validity)
{
    return out
            << "date valid="        << std::boolalpha << (0 != (validity & Pvt::DateTimeValidity::valid_date)) << " "
            << "time valid="        << std::boolalpha << (0 != (validity & Pvt::DateTimeValidity::valid_time)) << " "
            << "fully resolved="    << std::boolalpha << (0 != (validity & Pvt::DateTimeValidity::fully_resolved));
}

std::ostream& nav::operator<<(std::ostream& out, nav::Pvt::FixType fix_type)
{
    switch (fix_type)
    {
    case Pvt::FixType::no_fix:
        out << "no fix";
        break;
    case Pvt::FixType::dead_reckoning:
        out << "dead reckoning";
        break;
    case Pvt::FixType::fix_2d:
        out << "2D fix";
        break;
    case Pvt::FixType::fix_3d:
        out << "3D fix";
        break;
    case Pvt::FixType::gnss_and_dead_reckoning:
        out << "gnss & dr";
        break;
    case Pvt::FixType::only_time_fix:
        out << "only time";
        break;
    }

    return out;
}

std::ostream& nav::operator<<(std::ostream& out, const nav::Pvt& pvt)
{
    out << "nav-pvt:" << std::endl
        <<   " itow: " << std::uint32_t(pvt.itow) << std::endl
        <<   " year: " << std::uint32_t(pvt.year) << std::endl
        <<   " month: " << std::uint32_t(pvt.month) << std::endl
        <<   " day: " << std::uint32_t(pvt.day) << std::endl
        <<   " hour: " << std::uint32_t(pvt.hour) << std::endl
        <<   " minute: " << std::uint32_t(pvt.minute) << std::endl
        <<   " seconds: " << std::uint32_t(pvt.second) << std::endl
        <<   " dtv: " << pvt.date_time_validity << std::endl
        <<   " time_accuracy: " << pvt.time_accuracy << std::endl
        <<   " nanoseconds: " << pvt.nanoseconds << std::endl
        <<   " fix_type: " << pvt.fix_type << std::endl
        <<   " fix_status_flags: " << std::uint32_t(pvt.fix_status_flags) << std::endl
        <<   " additional_flags: " << std::uint32_t(pvt.additional_flags) << std::endl
        <<   " satellite_count: " << std::uint32_t(pvt.satellite_count) << std::endl
        <<   " longitude: " << pvt.longitude << std::endl
        <<   " latitude: " << pvt.latitude << std::endl
        <<   " height.Ellipsoid: " << pvt.height.above_ellipsoid << std::endl
        <<   " height.MSL: " << pvt.height.above_msl << std::endl
        <<   " accuracy.horizontal: " << pvt.accuracy.horizontal << std::endl
        <<   " accuracy.vertical: " << pvt.accuracy.vertical << std::endl
        <<   " accuracy.speed: " << pvt.accuracy.speed << std::endl
        <<   " accuracy.heading: " << pvt.accuracy.heading << std::endl;

    return out;
}

// Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
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
#ifndef UBX_8_NMEA_FUSION_ADAPT_H_
#define UBX_8_NMEA_FUSION_ADAPT_H_

#define BOOST_SPIRIT_DEBUG
#define FUSION_MAX_VECTOR_SIZE 15

#include <location/providers/ubx/_8/nmea/sentence.h>

#include <boost/fusion/adapted/struct.hpp>

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Latitude,
    (uint32_t, degrees)
    (double, minutes))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Longitude,
    (uint32_t, degrees)
    (double, minutes))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Date,
    (std::uint8_t, day)
    (std::uint8_t, month)
    (std::uint8_t, year))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Utc,
    (std::uint8_t, hours)
    (std::uint8_t, minutes)
    (double, seconds))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Gsa,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<location::providers::ubx::_8::nmea::Gsa::OperationMode>, operation_mode)
    (boost::optional<location::providers::ubx::_8::nmea::Gsa::FixMode>, fix_mode)
    (std::vector<boost::optional<std::uint8_t>>, satellite_ids)
    (boost::optional<location::providers::ubx::Dop<location::providers::ubx::tag::Positional>>, pdop)
    (boost::optional<location::providers::ubx::Dop<location::providers::ubx::tag::Horizontal>>, hdop)
    (boost::optional<location::providers::ubx::Dop<location::providers::ubx::tag::Vertical>>, vdop))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Gga,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<location::providers::ubx::_8::nmea::Utc>, utc)
    (boost::optional<location::providers::ubx::_8::nmea::Latitude>, latitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::providers::ubx::_8::nmea::Longitude>, longitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<location::providers::ubx::_8::nmea::gps::FixMode>, fix_mode)
    (boost::optional<std::uint8_t>, satellites_in_use)
    (boost::optional<location::providers::ubx::Dop<location::providers::ubx::tag::Horizontal>>, hdop)
    (boost::optional<float>, altitude)
    (boost::optional<float>, geoidal_separation)
    (boost::optional<float>, age)
    (boost::optional<std::uint16_t>, differential_reference_station))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Gll,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<location::providers::ubx::_8::nmea::Latitude>, latitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::providers::ubx::_8::nmea::Longitude>, longitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<location::providers::ubx::_8::nmea::Utc>, utc)
    (boost::optional<location::providers::ubx::_8::nmea::Status>, status)
    (boost::optional<location::providers::ubx::_8::nmea::Mode>, mode))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Gsv::Info,
    (boost::optional<std::uint8_t>, satellite_id)
    (boost::optional<std::uint8_t>, elevation)
    (boost::optional<std::uint16_t>, azimuth)
    (boost::optional<std::uint8_t>, snr))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Gsv,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<std::uint8_t>, sentence_count)
    (boost::optional<std::uint8_t>, sentence_number)
    (boost::optional<std::uint8_t>, satellites_count)
    (std::vector<location::providers::ubx::_8::nmea::Gsv::Info>, satellites_info))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Rmc,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<location::providers::ubx::_8::nmea::Utc>, utc)
    (boost::optional<location::providers::ubx::_8::nmea::Status>, status)
    (boost::optional<location::providers::ubx::_8::nmea::Latitude>, latitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::providers::ubx::_8::nmea::Longitude>, longitude)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<float>, speed_over_ground)(boost::optional<float>, course_over_ground)
    (boost::optional<location::providers::ubx::_8::nmea::Date>, date)
    (boost::optional<float>, magnetic_variation)
    (boost::optional<location::providers::ubx::_8::nmea::CardinalDirection>, cardinal_direction)
    (boost::optional<location::providers::ubx::_8::nmea::Mode>, mode))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Txt,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<std::uint8_t>, total_number_of_sentences)
    (boost::optional<std::uint8_t>, sentence_number)
    (boost::optional<std::uint8_t>, identifier)
    (boost::optional<std::string>, message))

BOOST_FUSION_ADAPT_STRUCT(
    location::providers::ubx::_8::nmea::Vtg,
    (location::providers::ubx::_8::nmea::Talker, talker)
    (boost::optional<float>, cog_true)
    (boost::optional<float>, cog_magnetic)
    (boost::optional<float>, sog_knots)
    (boost::optional<float>, sog_kmh)
    (boost::optional<location::providers::ubx::_8::nmea::Mode>, mode))

// clang-format on

#endif // UBX_8_NMEA_FUSION_ADAPT_H_

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
#ifndef LOCATION_NMEA_FUSION_ADAPT_H_
#define LOCATION_NMEA_FUSION_ADAPT_H_

#define BOOST_SPIRIT_DEBUG
#define FUSION_MAX_VECTOR_SIZE 15

#include <location/nmea/sentence.h>

#include <boost/fusion/adapted/struct.hpp>

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Latitude,
    (uint32_t, degrees)
    (double, minutes))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Longitude,
    (uint32_t, degrees)
    (double, minutes))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Date,
    (std::uint8_t, day)
    (std::uint8_t, month)
    (std::uint8_t, year))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Utc,
    (std::uint8_t, hours)
    (std::uint8_t, minutes)
    (double, seconds))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Gsa,
    (location::nmea::Talker, talker)
    (boost::optional<location::nmea::Gsa::OperationMode>, operation_mode)
    (boost::optional<location::nmea::Gsa::FixMode>, fix_mode)
    (std::vector<boost::optional<std::uint8_t>>, satellite_ids)
    (boost::optional<float>, pdop)
    (boost::optional<float>, hdop)
    (boost::optional<float>, vdop))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Gga,
    (location::nmea::Talker, talker)
    (boost::optional<location::nmea::Utc>, utc)
    (boost::optional<location::nmea::Latitude>, latitude)
    (boost::optional<location::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::nmea::Longitude>, longitude)
    (boost::optional<location::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<location::nmea::gps::FixMode>, fix_mode)
    (boost::optional<std::uint8_t>, satellites_in_use)
    (boost::optional<float>, hdop)
    (boost::optional<float>, altitude)
    (boost::optional<float>, geoidal_separation)
    (boost::optional<float>, age)
    (boost::optional<std::uint16_t>, differential_reference_station))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Gll,
    (location::nmea::Talker, talker)
    (boost::optional<location::nmea::Latitude>, latitude)
    (boost::optional<location::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::nmea::Longitude>, longitude)
    (boost::optional<location::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<location::nmea::Utc>, utc)
    (boost::optional<location::nmea::Status>, status)
    (boost::optional<location::nmea::Mode>, mode))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Gsv::Info,
    (boost::optional<std::uint8_t>, satellite_id)
    (boost::optional<std::uint8_t>, elevation)
    (boost::optional<std::uint16_t>, azimuth)
    (boost::optional<std::uint8_t>, snr))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Gsv,
    (location::nmea::Talker, talker)
    (boost::optional<std::uint8_t>, sentence_count)
    (boost::optional<std::uint8_t>, sentence_number)
    (boost::optional<std::uint8_t>, satellites_count)
    (std::vector<location::nmea::Gsv::Info>, satellites_info))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Rmc,
    (location::nmea::Talker, talker)
    (boost::optional<location::nmea::Utc>, utc)
    (boost::optional<location::nmea::Status>, status)
    (boost::optional<location::nmea::Latitude>, latitude)
    (boost::optional<location::nmea::CardinalDirection>, latitude_direction)
    (boost::optional<location::nmea::Longitude>, longitude)
    (boost::optional<location::nmea::CardinalDirection>, longitude_direction)
    (boost::optional<float>, speed_over_ground)(boost::optional<float>, course_over_ground)
    (boost::optional<location::nmea::Date>, date)
    (boost::optional<float>, magnetic_variation)
    (boost::optional<location::nmea::CardinalDirection>, cardinal_direction)
    (boost::optional<location::nmea::Mode>, mode))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Txt,
    (location::nmea::Talker, talker)
    (boost::optional<std::uint8_t>, total_number_of_sentences)
    (boost::optional<std::uint8_t>, sentence_number)
    (boost::optional<std::uint8_t>, identifier)
    (boost::optional<std::string>, message))

BOOST_FUSION_ADAPT_STRUCT(
    location::nmea::Vtg,
    (location::nmea::Talker, talker)
    (boost::optional<float>, cog_true)
    (boost::optional<float>, cog_magnetic)
    (boost::optional<float>, sog_knots)
    (boost::optional<float>, sog_kmh)
    (boost::optional<location::nmea::Mode>, mode))

// clang-format on

#endif // LOCATION_NMEA_FUSION_ADAPT_H_

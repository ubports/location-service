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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_CLIENT_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_CLIENT_H_

#include <com/ubuntu/location/providers/gps/android_hardware_abstraction_layer.h>

#include <boost/asio.hpp>
#include <boost/endian/buffers.hpp>

#include <chrono>
#include <string>

namespace com { namespace ubuntu { namespace location { namespace providers { namespace gps { namespace sntp {
// Please see https://tools.ietf.org/html/rfc4330 for a discussion of sntp
// and for a detailed explanation of the different types and algorithms.

namespace detail
{
// enum_constant returns a compile-time constant assembled from 4 or at least
// 3 individual characters.
template<std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d = 0>
constexpr uint32_t enum_constant()
{
    return (a << 24) | (b << 16) | (c << 8) | d;
}
}

// offset_1900_to_1970 returns the number of seconds that elapsed between 1900 and 1970.
const std::chrono::seconds& offset_1900_to_1970();

// LeapIndicator enumerates warnings of an impending
// leap second to be inserted/deleted in the last minute of the current
// day.
enum class LeapIndicator : std::uint8_t
{
    no_warning = 0,
    last_minute_has_61_seconds = 1,
    last_minute_has_59_seconds = 2,
    alarm = 3
};

// Mode enumerates the different protocol modes.
enum class Mode : std::uint8_t
{
    reserved = 0,
    symmetric_active = 1,
    symmetric_passive = 2,
    client = 3,
    server = 4,
    broadcast = 5,
    reserved_for_control = 6,
    reserved_for_private_use = 7
};

// ReferenceIdentifier enumerates known references for NTP-servers
// on stratum 0 or 1.
enum class ReferenceIdentifier : std::uint32_t
{
    locl = detail::enum_constant<'L','O','C','L'>(),    // uncalibrated local clock
    cesm = detail::enum_constant<'C','E','S','M'>(),    // calibrated Cesium clock
    rbdm = detail::enum_constant<'R','B','D','M'>(),    // calibrated Rubidium clock
    pps  = detail::enum_constant<'P','P','S'>(),        // calibrated quartz clock or other pulse-per-second source
    irig = detail::enum_constant<'I','R','I','G'>(),    // Inter-Range Instrumentation Group
    acts = detail::enum_constant<'A','C','T','S'>(),    // NIST telephone modem service
    usno = detail::enum_constant<'U','S','N','O'>(),    // USNO telephone modem service
    ptb  = detail::enum_constant<'P','T','B'>(),        // PTB (Germany) telephone modem service
    tdf  = detail::enum_constant<'T','D','F'>(),        // Allouis (France) Radio 164 kHz
    dcf  = detail::enum_constant<'D','C','F'>(),        // Mainflingen (Germany) Radio 77.5 kHz
    msf  = detail::enum_constant<'M','S', 'F'>(),       // Rugby (UK) Radio 60 kHz
    wwv  = detail::enum_constant<'W','W','V'>(),        // Ft. Collins (US) Radio 2.5, 5, 10, 15, 20 MHz
    wwvb = detail::enum_constant<'W','W','V','B'>(),    // Boulder (US) Radio 60 kHz
    wwvh = detail::enum_constant<'W','W','V','H'>(),    // Kauai Hawaii (US) Radio 2.5, 5, 10, 15 MHz
    chu  = detail::enum_constant<'C','H','U'>(),        // Ottawa (Canada) Radio 3330, 7335, 14670 kHz
    lorc = detail::enum_constant<'L','O','R','C'>(),    // LORAN-C radionavigation system
    omeg = detail::enum_constant<'O','M','E','G'>(),    // OMEGA radionavigation system
    gps  = detail::enum_constant<'G','P','S'>()         // Global Positioning Service
};

// Timestamp models a fixed-point NTP timestamp relative to 01.01.1900
template<typename Seconds, typename FractionalSeconds = Seconds>
struct Timestamp
{
    // To stay on the safe side, we only allow for the types specified below.
    // With that, we make sure that we only ever use endian-save types.
    static_assert(std::is_same<Seconds, boost::endian::big_int16_buf_t>::value ||
                  std::is_same<Seconds, boost::endian::big_uint16_buf_t>::value ||
                  std::is_same<Seconds, boost::endian::big_int32_buf_t>::value ||
                  std::is_same<Seconds, boost::endian::big_uint32_buf_t>::value,
                  "Timestamp<> only supports boost::endian::big_{{u}int{16,32}}_buf_t");

    // from_milliseconds_since_epoch fills in seconds/fractional_seconds from
    // ms, converting to NTP's reference time.
    void from_milliseconds_since_epoch(const std::chrono::milliseconds& ts)
    {
        using s = std::chrono::seconds;
        using ms = std::chrono::milliseconds;

        auto secs = std::chrono::duration_cast<s>(ts);
        auto msecs = std::chrono::duration_cast<ms>(ts - secs);

        secs += offset_1900_to_1970();

        seconds = secs.count();
        fractional_seconds = msecs.count() * std::numeric_limits<typename FractionalSeconds::value_type>::max() + 1 / 1000;
    }

    // to_milliseconds_since_epoch returns a unix timestamp calculated from
    // an NTP-timestamp.
    std::chrono::milliseconds to_milliseconds_since_epoch() const
    {
        return std::chrono::seconds{seconds.value()} - offset_1900_to_1970() +
               std::chrono::milliseconds{(fractional_seconds.value() * 1000) / std::numeric_limits<typename FractionalSeconds::value_type>::max()};
    }

    Seconds seconds;
    FractionalSeconds fractional_seconds;
};

// Duration is just an alias to an sntp::Timestamp.
template<typename T, typename U>
using Duration = Timestamp<T, U>;

// Packet models the SNTP wire format.
struct Packet
{
    // LeapIndicatorVersionMode helps in handling the bitfield setup
    // for the first byte of an sntp::Packet in a portable and endianess-safe way.
    struct LeapIndicatorVersionMode
    {
        // leap_indicator extracts the LeapIndicator from the underlying data.
        LeapIndicator leap_indicator() const;
        // leap_indicator sets the value of the leap_indicatorin in the underyling
        // data to li.
        LeapIndicatorVersionMode& leap_indicator(LeapIndicator li);

        // version extracts the version from the underlying data.
        std::uint8_t version() const;
        // version adjusts the version to version in the underlying data.
        LeapIndicatorVersionMode& version(std::uint8_t version);

        // mode extracts the mode from the underlying data.
        Mode mode() const;
        // mode adjusts the mode to m in the underlying data.
        LeapIndicatorVersionMode& mode(Mode m);

        boost::endian::big_uint8_buf_t livm;
    };

    LeapIndicatorVersionMode livm;
    boost::endian::big_uint8_buf_t stratum;
    boost::endian::big_uint8_buf_t poll_interval;
    boost::endian::big_int8_buf_t precision;
    Duration<boost::endian::big_int16_buf_t, boost::endian::big_uint16_buf_t> root_delay;
    Duration<boost::endian::big_uint16_buf_t, boost::endian::big_uint16_buf_t> root_dispersion;
    boost::endian::big_uint32_buf_t reference_identifier;
    Timestamp<boost::endian::big_uint32_buf_t> reference;
    Timestamp<boost::endian::big_uint32_buf_t> originate;
    Timestamp<boost::endian::big_uint32_buf_t> receive;
    Timestamp<boost::endian::big_uint32_buf_t> transmit;
};
// The SNTP version we are working with.
static constexpr const std::uint8_t version = 3;

// SntpClient provides access to querying time from an NTP server in
// a unicast scenario.
class Client
{
public:
    // Response is returned in a request for time information.
    struct Response
    {
        // The raw packet that was returned from the request.
        sntp::Packet packet;
        // Time computed from the NTP transaction.
        std::chrono::milliseconds ntp_time;
        // Reference clock value corresponding to the NTP time.
        std::chrono::milliseconds ntp_time_reference;
        // Round-trip time of the last NTP transaction.
        std::chrono::milliseconds round_trip_time;
    };

    // request_time_with_timeout reaches out to the SNTP server known under host,
    // and tries to determine the current UTC reference time. The operation aborts after
    // timeout milliseconds.
    //
    // std::runtime_error is thrown in case of issues.
    Response request_time(const std::string& host, const std::chrono::milliseconds& timeout, boost::asio::io_service& ios);
    sntp::Packet request(boost::asio::ip::udp::socket& socket);
};
}
}

}}}}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_CLIENT_H_

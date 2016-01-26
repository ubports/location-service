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

#include <com/ubuntu/location/providers/gps/sntp_client.h>

#include <boost/asio/ip/udp.hpp>

#include <fstream>

namespace gps = com::ubuntu::location::providers::gps;

namespace ip = boost::asio::ip;

namespace
{
// boot_time returns the time since the epoch when the system booted.
std::chrono::system_clock::time_point boot_time()
{
    std::ifstream in{"/proc/stat"};
    std::string line;

    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss{line};
        std::string name; ss >> name;

        if (name != "btime")
            continue;

        std::uint64_t ts; ss >> ts;
        return std::chrono::system_clock::time_point{std::chrono::seconds{ts}};
    }

    // Fallback to the best estimate we can give.
    return std::chrono::system_clock::now();
}

std::chrono::milliseconds elapsed_realtime_since_boot()
{
    static const auto btime = boot_time();
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - btime);
}

namespace sntp
{
enum class LeapIndicator : std::uint8_t
{
    no_warning = 0,
    last_minute_has_61_seconds = 1,
    last_minute_has_59_seconds = 2,
    alarm = 3
};

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

enum class ReferenceIdentifier : std::uint32_t
{
    locl = 'LOCL',
    cesm = 'CESM',
    rbdm = 'RBDM',
    pps = 'PPS',
    irig = 'IRIG',
    acts = 'ACTS',
    usno = 'USNO',
    ptb = 'PTB',
    tdf = 'TDF',
    dcf = 'DCF',
    msf = 'MSF',
    wwv = 'WWV',
    wwvb = 'WWVB',
    wwvh = 'WWVH',
    chu = 'CHU',
    lorc = 'LORC',
    omeg = 'OMEG',
    gps = 'GPS'
};

static const std::chrono::seconds& offset_1900_to_1970()
{
    static const auto secs = ((365 * 70) + 17) * 24 * 60 * 60;
    static const std::chrono::seconds seconds{secs};
    return seconds;
}

template<typename Seconds, typename FractionalSeconds = Seconds>
struct Timestamp
{
    void from_milliseconds_since_epoch(const std::chrono::milliseconds& ms)
    {
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
        auto msecs = ms - secs;

        secs += offset_1900_to_1970();

        seconds = secs.count();
        fractional_seconds = msecs.count() * std::numeric_limits<FractionalSeconds>::max() / 1000;
    }

    std::chrono::milliseconds to_milliseconds_since_epoch() const
    {
        std::uint32_t n_seconds{seconds};
        std::uint32_t n_fractional_seconds{fractional_seconds};

        return std::chrono::seconds{ntohl(n_seconds)} - offset_1900_to_1970() +
               std::chrono::milliseconds{(ntohl(n_fractional_seconds) * 1000) / std::numeric_limits<FractionalSeconds>::max()};
    }

    Seconds seconds;
    FractionalSeconds fractional_seconds;
};

template<typename T, typename U>
using Duration = Timestamp<T, U>;

struct Packet
{
    /*   2 */ LeapIndicator leap_indicator : 2;
    /*   5 */ std::uint8_t version : 3;
    /*   8 */ Mode mode : 3;
    /*  16 */ std::uint8_t stratum;
    /*  24 */ std::uint8_t poll_interval;
    /*  32 */ std::int8_t precision;
    /*  64 */ Duration<std::int16_t, std::uint16_t> root_delay;
    /*  96 */ Duration<std::uint16_t, std::uint16_t> root_dispersion;
    /* 128 */ ReferenceIdentifier reference_identifier;
    /* 192 */ Timestamp<std::uint32_t> reference;
    /* 256 */ Timestamp<std::uint32_t> originate;
    /* 320 */ Timestamp<std::uint32_t> receive;
    /* 384 */ Timestamp<std::uint32_t> transmit;
};

static constexpr const std::uint8_t version = 3;
}
}

gps::SntpClient::SntpClient(boost::asio::io_service& service) : service{service}
{
}

gps::SntpClient::Response gps::SntpClient::request_time_with_timeout(const std::string& host, const boost::asio::yield_context& ctxt)
{
    ip::udp::resolver resolver(service);
    ip::udp::resolver::query query{ip::udp::v4(), host, "sntp"};

    boost::system::error_code ec;
    auto iter = resolver.async_resolve(query, ctxt[ec]);

    if (ec)
        throw std::runtime_error{"Failed to resolve host name: " + ec.message()};

    auto socket = ip::udp::socket{service};
    socket.async_connect(*iter, ctxt[ec]);

    if (ec)
        throw std::runtime_error{"Failed to connect to endpoint: " + ec.message()};

    sntp::Packet packet; ::memset(&packet, 0, sizeof(packet));
    packet.mode = sntp::Mode::client;
    packet.version = sntp::version;
    packet.transmit.from_milliseconds_since_epoch(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                      std::chrono::system_clock::now().time_since_epoch()));
    auto request = elapsed_realtime_since_boot();
    {
        auto transferred = socket.async_send(boost::asio::buffer(&packet, sizeof(packet)), ctxt[ec]);

        if (transferred < sizeof(packet) || ec)
            throw std::runtime_error{"Failed to write sntp::Packet from socket: " + ec.message()};

        transferred = socket.async_receive(boost::asio::buffer(&packet, sizeof(packet)), ctxt[ec]);

        if (transferred < sizeof(packet) || ec)
            throw std::runtime_error{"Failed to read sntp::Packet from socket: " + ec.message()};
    }
    auto response = elapsed_realtime_since_boot();

    auto originate = packet.originate.to_milliseconds_since_epoch();
    auto receive = packet.receive.to_milliseconds_since_epoch();
    auto transmit = packet.transmit.to_milliseconds_since_epoch();

    auto rtt = response - request - (transmit - receive);
    auto offset = ((receive - originate) + (transmit - response))/2;

    return {response + offset, response};
}

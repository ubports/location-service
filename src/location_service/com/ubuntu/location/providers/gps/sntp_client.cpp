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

#include <com/ubuntu/location/time_since_boot.h>

#include <boost/asio/ip/udp.hpp>
#include <boost/endian/buffers.hpp>

#include <bitset>
#include <fstream>
#include <iostream>

namespace location = com::ubuntu::location;
namespace gps = location::providers::gps;
namespace ip = boost::asio::ip;

namespace
{
struct Now
{
    std::chrono::nanoseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
    std::chrono::nanoseconds ticks = location::time_since_boot();
};
}

const std::chrono::seconds& gps::sntp::offset_1900_to_1970()
{
    static const std::uint64_t secs = ((365ull * 70ull) + 17ull) * 24ull * 60ull * 60ull;
    static const std::chrono::seconds seconds{secs};
    return seconds;
}

gps::sntp::LeapIndicator gps::sntp::Packet::LeapIndicatorVersionMode::leap_indicator() const
{
    return static_cast<LeapIndicator>((livm.value() >> 6) & 0x3);
}

gps::sntp::Packet::LeapIndicatorVersionMode& gps::sntp::Packet::LeapIndicatorVersionMode::leap_indicator(LeapIndicator li)
{
    livm = livm.value() | (static_cast<uint8_t>(li) << 6);
    return *this;
}

std::uint8_t gps::sntp::Packet::LeapIndicatorVersionMode::version() const
{
    return (livm.value() >> 3) & 0x7;
}

gps::sntp::Packet::LeapIndicatorVersionMode& gps::sntp::Packet::LeapIndicatorVersionMode::version(std::uint8_t version)
{
    livm = livm.value() | (version << 3);
    return *this;
}

gps::sntp::Mode gps::sntp::Packet::LeapIndicatorVersionMode::mode() const
{
    return static_cast<Mode>(livm.value() & 0x7);
}

gps::sntp::Packet::LeapIndicatorVersionMode& gps::sntp::Packet::LeapIndicatorVersionMode::mode(Mode m)
{
    livm = static_cast<uint8_t>(m) | livm.value();
    return *this;
}

gps::sntp::Client::Response gps::sntp::Client::request_time(const std::string& host, const std::chrono::milliseconds& timeout, boost::asio::io_service& ios)
{
    ip::udp::resolver resolver(ios);
    ip::udp::resolver::query query{ip::udp::v4(), host, "ntp"};

    auto socket = ip::udp::socket{ios};
    socket.connect(*resolver.resolve(query));

    sntp::Packet packet;

    Now before;
    {
        bool timed_out{false};

        boost::asio::deadline_timer timer{ios};
        timer.expires_from_now(boost::posix_time::milliseconds{timeout.count()});
        timer.async_wait([&timed_out, &socket](const boost::system::error_code& ec)
        {
            if (ec) return;

            timed_out = true;

            socket.shutdown(ip::udp::socket::shutdown_both);
            socket.close();
        });

        packet = request(socket);
        timer.cancel();

        if (timed_out)
            throw std::runtime_error("Operation timed out.");
    }
    Now after;

    auto originate = packet.originate.to_milliseconds_since_epoch();
    auto receive = packet.receive.to_milliseconds_since_epoch();
    auto transmit = packet.transmit.to_milliseconds_since_epoch();

    auto rtt = after.ticks - before.ticks - (transmit - receive);
    auto offset = ((receive - originate) + (transmit - after.time))/2;

    return
    {
        packet,
        std::chrono::duration_cast<std::chrono::milliseconds>(after.time + offset),
        std::chrono::duration_cast<std::chrono::milliseconds>(after.ticks),
        std::chrono::duration_cast<std::chrono::milliseconds>(rtt)
    };
}

gps::sntp::Packet gps::sntp::Client::request(boost::asio::ip::udp::socket& socket)
{
    sntp::Packet packet;
    packet.transmit.from_milliseconds_since_epoch(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()));
    packet.livm.mode(sntp::Mode::client).version(sntp::version);

    socket.send(boost::asio::buffer(&packet, sizeof(packet)));
    socket.receive(boost::asio::buffer(&packet, sizeof(packet)));

    return packet;
}

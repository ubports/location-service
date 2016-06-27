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

#include <location/providers/gps/sntp_client.h>

#include <location/time_since_boot.h>

#include <boost/asio/ip/udp.hpp>
#include <boost/endian/buffers.hpp>

#include <bitset>
#include <fstream>
#include <future>
#include <iostream>
#include <thread>

namespace gps = location::providers::gps;
namespace ip = boost::asio::ip;

namespace
{

template<typename T>
T sync_or_throw(std::future<T>& f)
{
    return f.get();
}

template<>
void sync_or_throw<>(std::future<void>& f)
{
    f.get();
}

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
    ip::udp::resolver resolver{ios};
    ip::udp::socket socket{ios};
    bool timed_out{false};

    std::promise<ip::udp::resolver::iterator> promise_resolve;
    auto future_resolve= promise_resolve.get_future();

    boost::asio::deadline_timer timer{ios};
    timer.expires_from_now(boost::posix_time::milliseconds{timeout.count()});
    timer.async_wait([&timed_out, &resolver, &socket](const boost::system::error_code& ec)
    {
        if (ec)
            return;

        timed_out = true;

        resolver.cancel();
        socket.shutdown(ip::udp::socket::shutdown_both);
        socket.close();
    });

    ip::udp::resolver::query query{ip::udp::v4(), host, "ntp"};

    resolver.async_resolve(query, [&promise_resolve](const boost::system::error_code& ec, ip::udp::resolver::iterator it)
    {
        if (ec)
            promise_resolve.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        else
            promise_resolve.set_value(it);
    });

    auto it = sync_or_throw(future_resolve);

    std::promise<void> promise_connect;
    auto future_connect = promise_connect.get_future();

    socket.async_connect(*it, [&promise_connect](const boost::system::error_code& ec)
    {
        if (ec)
            promise_connect.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        else
            promise_connect.set_value();
    });

    sync_or_throw(future_connect);

    sntp::Packet packet;

    Now before;
    {        
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

    std::promise<std::size_t> promise_send;
    auto future_send = promise_send.get_future();

    socket.async_send(boost::asio::buffer(&packet, sizeof(packet)), [&promise_send](const boost::system::error_code& ec, std::size_t transferred)
    {
        if (ec)
            promise_send.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        else
            promise_send.set_value(transferred);
    });

    sync_or_throw(future_send);

    std::promise<std::size_t> promise_receive;
    auto future_receive = promise_receive.get_future();

    socket.async_receive(boost::asio::buffer(&packet, sizeof(packet)), [&promise_receive](const boost::system::error_code& ec, std::size_t transferred)
    {
        if (ec)
            promise_receive.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        else
            promise_receive.set_value(transferred);
    });

    sync_or_throw(future_receive);

    return packet;
}

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

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <chrono>
#include <string>

namespace com { namespace ubuntu { namespace location { namespace providers { namespace gps {
class SntpClient
{
public:
    struct Response
    {
        std::chrono::milliseconds ntp_time;
        std::chrono::milliseconds ntp_time_reference;
    };

    // Creates a new instance multiplexing operations onto service.
    SntpClient(boost::asio::io_service& service);
    // request_time_with_timeout reaches out to the SNTP server known under host,
    // and tries to determine the current UTC reference time. The operation aborts after
    // timeout milliseconds.
    Response request_time_with_timeout(const std::string& host, const boost::asio::yield_context& ctxt);

protected:
    boost::asio::io_service& service;
};
}}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_CLIENT_H_

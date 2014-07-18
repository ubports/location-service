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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NET_CPP_GPS_XTRA_DOWNLOADER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NET_CPP_GPS_XTRA_DOWNLOADER_H_

#include "android_hardware_abstraction_layer.h"

#include <core/net/http/client.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>

namespace com { namespace ubuntu { namespace location { namespace providers { namespace gps { namespace android {
struct NetCppGpsXtraDownloader : public GpsXtraDownloader
{
    NetCppGpsXtraDownloader() : http_client{core::net::http::make_client()}
    {
    }

    /** @brief Executes the actual download, throws if no xtra servers are given in the config. */
    virtual std::vector<char> download_xtra_data(const Configuration& config) override
    {
        if (config.xtra_hosts.empty()) throw std::runtime_error
        {
            "Missing xtra hosts."
        };

        std::uniform_int_distribution<std::size_t> dist{0, config.xtra_hosts.size() - 1};

        auto host = config.xtra_hosts.at(dist(dre));

        auto rc = core::net::http::Request::Configuration::from_uri_as_string(host);

        rc.header.add("Accept", "*/*");
        rc.header.add("Accept", "application/vnd.wap.mms-message");
        rc.header.add("Accept", "application/vnd.wap.sic");
        rc.header.add(x_wap_profile_key, x_wap_profile_value);

        auto request = http_client->get(rc);
        request->set_timeout(config.timeout);

        auto response = request->execute([](const core::net::http::Request::Progress&)
        {
            return core::net::http::Request::Progress::Next::continue_operation;
        });

        if (response.status != core::net::http::Status::ok)
        {
            std::stringstream ss{"Request for xtra data on "};
            ss << host << " did not succeed: " << response.status;
            throw std::runtime_error{ss.str()};
        }

        return std::vector<char>(response.body.begin(), response.body.end());
    }

    // Client instance to talk to xtra servers.
    std::shared_ptr<core::net::http::Client> http_client;
    // Random number generator for load balancing purposes.
    std::default_random_engine dre;
};
}}}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NET_CPP_GPS_XTRA_DOWNLOADER_H_

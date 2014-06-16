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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_ICHNAEA_REPORTER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_ICHNAEA_REPORTER_H_

#include <com/ubuntu/location/service/harvester.h>

#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>
#include <core/net/http/status.h>

#include <json/json.h>

#include <thread>

namespace json = Json;

namespace com{namespace ubuntu{namespace location{namespace service
{
namespace ichnaea
{
namespace submit
{
constexpr const char* resource
{
    "/v1/submit?key="
};

const core::net::http::Status error
{
    core::net::http::Status::bad_request
};

const core::net::http::Status success
{
    core::net::http::Status::no_content
};
}

struct Reporter : public Harvester::Reporter
{
    static constexpr const char* nick_name_header{"X-Nickname"};

    struct Json
    {
        static constexpr const char* radio{"radio"};
        static constexpr const char* lat{"lat"};
        static constexpr const char* lon{"lon"};
        static constexpr const char* accuracy{"accuracy"};
        static constexpr const char* altitude{"altitude"};
        static constexpr const char* altitude_accuracy{"altitude_accuracy"};

        static constexpr const char* items{"items"};

        static constexpr const char* cell{"cell"};
        static constexpr const char* wifi{"wifi"};

        struct Cell
        {
            static constexpr const char* radio{"radio"};
            static constexpr const char* mcc{"mcc"};
            static constexpr const char* mnc{"mnc"};
            static constexpr const char* lac{"lac"};
            static constexpr const char* cid{"cid"};
            static constexpr const char* psc{"psc"};
            static constexpr const char* asu{"asu"};
        };

        struct Wifi
        {
            static constexpr const char* channel{"channel"};
            static constexpr const char* frequency{"frequency"};
            static constexpr const char* key{"key"};
            static constexpr const char* signal{"signal"};
        };
    };

    // Creation-time options for the ICHNAEA reporter
    struct Configuration
    {
        // Uri of the ICHNAEA instance we want to submit to.
        std::string uri;
        // API key for the submission
        std::string key;
        // Nickname for the submission
        std::string nick_name;
    };

    Reporter(const Configuration& configuration);
    ~Reporter();

    void start() override;
    void stop() override;
    void report(
            const Update<Position>& update,
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            const std::vector<connectivity::RadioCell::Ptr>& cells) override;

    static void convert_wifis_to_json(
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            json::Value& destination);

    static void convert_cells_to_json(
            const std::vector<connectivity::RadioCell::Ptr>& cells,
            json::Value& destination);

    core::net::http::Request::Configuration submit_request_config;
    std::shared_ptr<core::net::http::Client> http_client;
    std::thread http_client_worker;
};
}
}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_ICHNAEA_REPORTER_H_

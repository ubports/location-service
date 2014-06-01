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

    struct Configuration
    {
        // Uri of the ICHNAEA instance we want to submit to.
        std::string uri;
        // API key for the submission
        std::string key;
    };

    Reporter(const Configuration& configuration)
        : http_client(core::net::http::make_client())
    {
        auto uri = configuration.uri +
                   ichnaea::submit::resource +
                   configuration.key;

        submit_request_config = core::net::http::Request::Configuration::from_uri_as_string(uri);
        submit_request_config.ssl.verify_host = false;
        submit_request_config.ssl.verify_peer = false;
    }

    ~Reporter()
    {
        stop();
    }

    void start() override
    {
        http_client_worker = std::move(std::thread
        {
           [this]()
           {
               http_client->run();
           }
        });
    }

    void stop() override
    {
        http_client->stop();

        if (http_client_worker.joinable())
            http_client_worker.join();
    }

    void report(
            const Update<Position>& update,
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            const std::vector<connectivity::RadioCell>& cells) override
    {
        json::Value submit;
        json::Value item;

        item[Json::radio] = "gsm"; // We currently only support gsm radio types.
        item[Json::lat] = update.value.latitude.value.value();
        item[Json::lon] = update.value.longitude.value.value();

        if (update.value.accuracy.horizontal)
            item[Json::accuracy] = (*update.value.accuracy.horizontal).value();
        if (update.value.altitude)
            item[Json::altitude] = (*update.value.altitude).value.value();
        if (update.value.accuracy.vertical)
            item[Json::altitude_accuracy] = (*update.value.accuracy.vertical).value();

        if (!wifis.empty())
            ichnaea::Reporter::convert_wifis_to_json(wifis, item[Json::wifi]);

        if (!cells.empty())
            ichnaea::Reporter::convert_cells_to_json(cells, item[Json::cell]);

        submit[Json::items].append(item);

        json::FastWriter writer;

        VLOG(10) << "Submitting: " << writer.write(submit);

        auto request = http_client->post(
                    submit_request_config,
                    writer.write(submit),
                    core::net::http::ContentType::json);

        request->async_execute(
                    core::net::http::Request::Handler()
                        .on_response([](const core::net::http::Response& response)
                        {
                            if (response.status != ichnaea::submit::success)
                                LOG(ERROR) << "Error submitting to ichnaea: " << response.body;
                            else
                                LOG(INFO) << "Succesfully submitted to ichnaea.";
                        })
                        .on_error([](const core::net::Error& e)
                        {
                            LOG(ERROR) << "Networking error while submitting to ichnaea: " << e.what();
                        }));
    }

    static void convert_wifis_to_json(
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            json::Value& destination)
    {
        for (const auto& wifi : wifis)
        {
            // We do not harvest any Wifi marked with '_nomap'.
            if (wifi->ssid().get().find("_nomap") != std::string::npos)
                continue;

            json::Value w;
            w[Json::Wifi::key] = wifi->bssid().get();

            if (wifi->frequency().get().is_valid())
                w[Json::Wifi::frequency] = static_cast<int>(wifi->frequency().get());

            // We have a relative signal strength percentage in the wifi record.
            // TODO(tvoss): Check how that could be translated to RSSI.
            //wifi[Json::Wifi::signal] = -50;

            destination.append(w);
        }
    }

    static void convert_cells_to_json(
            const std::vector<connectivity::RadioCell>& cells,
            json::Value& destination)
    {
        for (const auto& cell : cells)
        {
            json::Value c;

            switch (cell.type())
            {
            case connectivity::RadioCell::Type::gsm:
            {
                c[Json::Cell::radio] = "gsm";

                const auto& details = cell.gsm();

                if (details.mobile_country_code.is_valid())
                    c[Json::Cell::mcc] = details.mobile_country_code.get();
                if (details.mobile_network_code.is_valid())
                    c[Json::Cell::mnc] = details.mobile_network_code.get();
                if (details.location_area_code.is_valid())
                    c[Json::Cell::lac] = details.location_area_code.get();
                if (details.id.is_valid())
                    c[Json::Cell::cid] = details.id.get();
                if  (details.strength.is_valid())
                    c[Json::Cell::asu] = details.strength.get();

                break;
            }
            case connectivity::RadioCell::Type::umts:
            {
                c[Json::Cell::radio] = "umts";

                const auto& details = cell.umts();

                if (details.mobile_country_code.is_valid())
                    c[Json::Cell::mcc] = details.mobile_country_code.get();
                if (details.mobile_network_code.is_valid())
                    c[Json::Cell::mnc] = details.mobile_network_code.get();
                if (details.location_area_code.is_valid())
                    c[Json::Cell::lac] = details.location_area_code.get();
                if (details.id.is_valid())
                    c[Json::Cell::cid] = details.id.get();
                if  (details.strength.is_valid())
                    c[Json::Cell::asu] = details.strength.get();

                break;
            }
            case connectivity::RadioCell::Type::lte:
            {
                c[Json::Cell::radio] = "lte";

                const auto& details = cell.lte();

                if (details.mobile_country_code.is_valid())
                    c[Json::Cell::mcc] = details.mobile_country_code.get();
                if (details.mobile_network_code.is_valid())
                    c[Json::Cell::mnc] = details.mobile_network_code.get();
                if (details.tracking_area_code.is_valid())
                    c[Json::Cell::lac] = details.tracking_area_code.get();
                if (details.id.is_valid())
                    c[Json::Cell::cid] = details.id.get();
                if (details.physical_id.is_valid())
                    c[Json::Cell::psc] = details.physical_id.get();
                if  (details.strength.is_valid())
                    c[Json::Cell::asu] = details.strength.get();
                break;
            }
            default:
                break;
            }

            destination.append(c);
        }
    }

    core::net::http::Request::Configuration submit_request_config;
    std::shared_ptr<core::net::http::Client> http_client;
    std::thread http_client_worker;
};
}
}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_ICHNAEA_REPORTER_H_

// Copyright (C) 2016 Canonical Ltd.
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
#include <ichnaea/client.h>
#include <ichnaea/codec.h>

#include <ichnaea/util/json.hpp>

#include <core/net/uri.h>
#include <core/net/http/client.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>
#include <core/net/http/status.h>

#include <boost/lexical_cast.hpp>

namespace http = core::net::http;
namespace json = nlohmann;

ichnaea::Client::Client(const std::string& host, const std::string& api_key, const std::shared_ptr<http::Client>& http_client)
    : host{host}, api_key{api_key}, http_client{http_client}
{
}

ichnaea::Client::Client(const std::string& api_key, const std::shared_ptr<http::Client>& http_client)
    : Client{Client::default_host, api_key, http_client}
{
}

void ichnaea::Client::geolocate(const geolocate::Parameters& parameters, const std::function<void(const Response<geolocate::Result>&)>& cb)
{
    json::json root; root & parameters;

    http::Request::Configuration config;
    config.uri = http_client->uri_to_string(core::net::make_uri(host, {"v1", "geolocate"}, {{"key", api_key}}));
    http::Request::Handler handler;
    handler.on_response([cb](const http::Response& response)
    {
        auto root = json::json::parse(response.body);

        if (response.status != http::Status::ok)
        {
            Error e{http::Status::bad_request, ""}; root & std::ref(e);
            cb(Response<geolocate::Result>{e});
        }
        else
        {
            geolocate::Result r; root & std::ref(r);
            cb(Response<geolocate::Result>{r});
        }
    });

    http_client->post(config, root.dump(), "application/json")->async_execute(handler);
}

void ichnaea::Client::geosubmit(const geosubmit::Parameters& parameters, const std::function<void(const Response<geosubmit::Result>&)>& cb)
{
    if (parameters.reports.empty())
        return;

    json::json root; root & parameters;

    http::Request::Configuration config;
    config.uri = http_client->uri_to_string(core::net::make_uri(host, {"v2", "geosubmit"}, {}));
    http::Request::Handler handler;
    handler.on_response([cb](const http::Response& response)
    {
        auto root = json::json::parse(response.body);

        if (response.status != http::Status::ok)
        {
            Error e{http::Status::bad_request, ""}; root & std::ref(e);
            cb(Response<geosubmit::Result>{e});
        }
        else
        {
            geosubmit::Result r; root & std::ref(r);
            cb(Response<geosubmit::Result>{r});
        }
    });

    http_client->post(config, root.dump(), "application/json")->async_execute(handler);
}

void ichnaea::Client::region(const region::Parameters& parameters, const std::function<void(const Response<region::Result>&)>& cb)
{
    json::json root; root & parameters;

    http::Request::Configuration config;
    config.uri = http_client->uri_to_string(core::net::make_uri(host, {"v1", "country"}, {{"key", api_key}}));
    http::Request::Handler handler;
    handler.on_response([cb](const http::Response& response)
    {
        auto root = json::json::parse(response.body);

        if (response.status != http::Status::ok)
        {
            Error e{http::Status::bad_request, ""}; root & std::ref(e);
            cb(Response<region::Result>{e});
        }
        else
        {
            region::Result r; root & std::ref(r);
            cb(Response<region::Result>{r});
        }
    });

    http_client->post(config, root.dump(), "application/json")->async_execute(handler);
}

// Copyright (C) 2017 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#include <location/providers/ubx/_8/assist_now_online_client.h>

#include <core/net/uri.h>
#include <core/net/http/response.h>

#include <iostream>

namespace _8 = location::providers::ubx::_8;
namespace http = core::net::http;

namespace
{

template<typename T>
std::string join(const std::set<T>& set)
{
    std::stringstream ss; bool first = true;

    for (auto element : set)
    {
        if (!first)
            ss << ",";

        ss << element;
        first = false;
    }

    return ss.str();
}

template<typename T, typename U>
std::string join(const std::set<std::pair<T, U>>& set)
{
    std::stringstream ss; bool first = true;

    for (auto element : set)
    {
        if (!first)
            ss << ",";

        ss << element.first << ":" << element.second;
        first = false;
    }

    return ss.str();
}

}  // namespace

_8::AssistNowOnlineClient::AssistNowOnlineClient(const std::shared_ptr<http::Client>& http_client)
    : http_client{http_client}, worker{[this]() { _8::AssistNowOnlineClient::http_client->run(); }}
{
}

_8::AssistNowOnlineClient::~AssistNowOnlineClient()
{
    http_client->stop();
    if (worker.joinable())
        worker.join();
}

void _8::AssistNowOnlineClient::request_assistance_data(const Parameters& parameters, const std::function<void(const Result<std::string>&)>& cb)
{
    core::net::Uri::QueryParameters query
    {
        {"gnss", join(parameters.gnss)},
        {"datatype", join(parameters.data_types)}
    };

    if (parameters.filter_on_pos)
        query.push_back({"filteronpos", "notstrictlyneeded"});

    if (!parameters.filter_on_svs.empty())
        query.push_back({"filteronsvs", join(parameters.filter_on_svs)});

    if (parameters.position)
    {
        const auto& position = parameters.position.get();
        query.push_back({"lat", std::to_string(position.latitude().value())});
        query.push_back({"lon", std::to_string(position.longitude().value())});

        if (position.altitude())
        {
            const auto& altitude = position.altitude().get();
            query.push_back({"alt", std::to_string(altitude.value())});
        }

        if (position.accuracy().horizontal())
        {
            const auto& accuracy = position.accuracy().horizontal().get();
            query.push_back({"pacc", std::to_string(accuracy.value())});
        }
    }

    auto uri = service_url + parameters.token;
    for (const auto& param : query)
        uri += (boost::format("&%1%=%2%") % param.first % param.second).str();

    auto request = http_client->get(http::Request::Configuration::from_uri_as_string(uri));

    request->async_execute(http::Request::Handler().on_response([cb](const http::Response& response)
    {
        if (response.status == http::Status::ok)
            cb(make_result(response.body));
        else
            cb(make_error_result<std::string>(
                   std::make_exception_ptr(
                       std::runtime_error{response.body})));
    }));
}

bool _8::operator<(AssistNowOnlineClient::DataType lhs, AssistNowOnlineClient::DataType rhs)
{
    using UT = typename std::underlying_type<AssistNowOnlineClient::DataType>::type;
    return static_cast<UT>(lhs) < static_cast<UT>(rhs);
}

std::ostream& _8::operator<<(std::ostream& out, AssistNowOnlineClient::DataType rhs)
{
    switch (rhs)
    {
    case AssistNowOnlineClient::DataType::almanac:
        out << "alm";
        break;
    case AssistNowOnlineClient::DataType::ephemeris:
        out << "eph";
        break;
    case AssistNowOnlineClient::DataType::position:
        out << "pos";
        break;
    case AssistNowOnlineClient::DataType::aux:
        out << "aux";
        break;
    }

    return out;
}

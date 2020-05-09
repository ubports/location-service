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
#ifndef ICHNAEA_CLIENT_H_
#define ICHNAEA_CLIENT_H_

#include <ichnaea/response.h>

#include <ichnaea/geolocate/parameters.h>
#include <ichnaea/geolocate/result.h>

#include <ichnaea/geosubmit/parameters.h>
#include <ichnaea/geosubmit/result.h>

#include <ichnaea/region/parameters.h>
#include <ichnaea/region/result.h>

#include <functional>
#include <memory>
#include <string>

namespace core { namespace net { namespace http { class Client; }}}

namespace ichnaea
{
/// @brief Client provides access to the ichnaea service offered by Mozilla.
class Client
{
public:
    /// @brief The default host we talk to.
    static constexpr const char* default_host{"https://location.services.mozilla.com"};

    /// @brief Client initializes a new instance with host, api_key and http_client.
    Client(const std::string& host, const std::string& api_key, const std::shared_ptr<core::net::http::Client>& http_client);

    /// @brief Client initializes a new instance with api_key and http_client.
    Client(const std::string& api_key, const std::shared_ptr<core::net::http::Client>& http_client);

    /// @brief geolocate queries the service instance for a position estimate for parameters,
    /// reporting the response to cb.
    void geolocate(const geolocate::Parameters& parameters, const std::function<void(const Response<geolocate::Result>&)>& cb);
    
    /// @brief geosubmit feeds new location data to the service, reporting the status of the operation to cb.
    void geosubmit(const geosubmit::Parameters& parameters, const std::function<void(const Response<geosubmit::Result>&)>& cb);

    /// @brief region resolves the country for a given location, reporting the status of the operation to cb.
    void region(const region::Parameters& parameters, const std::function<void(const Response<region::Result>&)>& cb);
private:
    std::string host;
    std::string api_key;
    std::shared_ptr<core::net::http::Client> http_client;
};
}

#endif // ICHNAEA_CLIENT_H_

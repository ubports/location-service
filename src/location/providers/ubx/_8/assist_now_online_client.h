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

#ifndef UBX_8_ASSIST_NOW_ONLINE_CLIENT_H_
#define UBX_8_ASSIST_NOW_ONLINE_CLIENT_H_

#include <location/providers/ubx/_8/gnss_id.h>

#include <location/optional.h>
#include <location/position.h>
#include <location/result.h>

#include <core/net/http/client.h>

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <set>
#include <string>
#include <thread>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class AssistNowOnlineClient
{
public:
    static constexpr const char* service_url{"http://online-live1.services.u-blox.com/GetOnlineData.ashx?token="};

    enum class DataType
    {
        ephemeris = 0,
        almanac = 1,
        aux = 2,
        position = 3
    };

    struct Parameters
    {
        // The authorization token supplied by u-blox when a client registers to
        // use the service.
        std::string token;
        // List of the GNSS for which data should be
        // returned. Valid GNSS are: GnssId::gps, GnssId::qzss and GnssId::glonass.
        std::set<GnssId> gnss;
        // List of data types required by the client.
        std::set<DataType> data_types;
        // Approximate user latitude, longitude and altitude in WGS 84 expressed in degrees and
        // fractional degrees.
        Optional<Position> position;
        // The timing accuracy (see time parameters note below).
        Optional<std::chrono::seconds> time_accuracy;
        // Typical latency between the time the server receives the request, and
        // the time when the assistance data arrives at the u-blox receiver. The
        // server can use this value to correct the time being transmitted to the
        // client.
        Optional<std::chrono::seconds> latency;
        // If set to true, the ephemeris data returned to the client will only contain
        // data for the satellites which are likely to be visible from the
        // approximate position provided by the lat, lon, alt and pacc parameters.
        bool filter_on_pos = false;
        // List of u-blox gnssId:svId pairs. The ephemeris data
        // returned to the client will only contain data for the listed satellites.
        std::set<std::pair<GnssId, std::uint32_t>> filter_on_svs;
    };

    explicit AssistNowOnlineClient(const std::shared_ptr<core::net::http::Client>& http_client);
    ~AssistNowOnlineClient();

    // request_asisstance_data reaches out to the AssistNow service via the configured http client
    // with parameters, reporting the assistance data blob to cb (on success).
    void request_assistance_data(const Parameters& parameters, const std::function<void(const Result<std::string>&)>& cb);

private:
    std::shared_ptr<core::net::http::Client> http_client;
    std::thread worker;
};

bool operator<(AssistNowOnlineClient::DataType lhs, AssistNowOnlineClient::DataType rhs);
std::ostream& operator<<(std::ostream& out, AssistNowOnlineClient::DataType rhs);

}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namepsace location

#endif  // UBX_8_ASSIST_NOW_ONLINE_CLIENT_H_

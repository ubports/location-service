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

#include <json/json.h>

#include <com/ubuntu/location/service/ichnaea_reporter.h>

#include <com/ubuntu/location/logging.h>

#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>
#include <core/net/http/status.h>

#include <thread>

namespace json
{
Object Object::parse_from_string(const std::string& s)
{
    return Object{json_tokener_parse(s.c_str())};
}

Object Object::create_array()
{
    return Object{json_object_new_array()};
}

Object Object::create_object()
{
    return Object{json_object_new_object()};
}

Object::Object(json_object* object) : object(object)
{
}

Object::Object(const Object& rhs) : object(json_object_get(rhs.object))
{
}

Object::~Object()
{
    json_object_put(object);
}

Object& Object::operator=(const Object& rhs)
{
    json_object_put(object);
    object = json_object_get(rhs.object);

    return *this;
}

std::string Object::to_plain_string()
{
    return std::string
    {
        json_object_to_json_string_ext(object, JSON_C_TO_STRING_PLAIN)
    };
}

Object Object::get(const std::string& name) const
{
    json_object* result{nullptr};

    json_object_object_get_ex(object, name.c_str(), &result);

    if (not result) throw std::out_of_range
    {
        name.c_str()
    };

    return Object{result};
}

namespace
{
template<json_type type> void throw_if_type_mismatch(json_object* object)
{
    if (not json_object_is_type(object, type)) throw std::logic_error
    {
        "Type mismatch."
    };
}
}
// Attempts to resolve the object to a boolean value.
// Throws std::logic_error in case of type mismatches.
bool Object::to_bool() const
{
    throw_if_type_mismatch<json_type_boolean>(object);
    return json_object_get_boolean(object);
}

std::int32_t Object::to_int32() const
{
    throw_if_type_mismatch<json_type_int>(object);
    return json_object_get_int(object);
}

std::int64_t Object::to_int64() const
{
    throw_if_type_mismatch<json_type_int>(object);
    return json_object_get_int64(object);
}

double Object::to_double() const
{
    throw_if_type_mismatch<json_type_double>(object);
    return json_object_get_double(object);
}

std::string Object::to_string() const
{
    throw_if_type_mismatch<json_type_string>(object);
    return std::string{json_object_get_string(object)};
}

void Object::put_array(const std::string& name, Object array)
{
    json_object_object_add(object, name.c_str(), json_object_get(array.object));
}

void Object::put_object(const std::string& name, Object other)
{
    json_object_object_add(object, name.c_str(), json_object_get(other.object));
}

void Object::put_boolean(const std::string& name, bool value)
{
    json_object_object_add(object, name.c_str(), json_object_new_boolean(value));
}

void Object::put_int32(const std::string& name, std::int32_t value)
{
    json_object_object_add(object, name.c_str(), json_object_new_int(value));
}

void Object::put_int64(const std::string& name, std::int64_t value)
{
    json_object_object_add(object, name.c_str(), json_object_new_int64(value));
}

void Object::put_double(const std::string& name, double value)
{
    json_object_object_add(object, name.c_str(), json_object_new_double(value));
}

void Object::put_string(const std::string& name, const std::string& value)
{
    json_object_object_add(object, name.c_str(), json_object_new_string_len(value.c_str(), value.size()));
}

std::size_t Object::array_size() const
{
    throw_if_type_mismatch<json_type_array>(object);
    return json_object_array_length(object);
}

void Object::append(Object other)
{
    throw_if_type_mismatch<json_type_array>(object);
    json_object_array_add(object, json_object_get(other.object));
}

void Object::put_object_for_index(std::size_t index, Object other)
{
    throw_if_type_mismatch<json_type_array>(object);
    json_object_array_put_idx(object, index, json_object_get(other.object));
}

// Queries the object at index 'index'.
// Throws std::logic_error if the object does not represent an array.
// Throws std::out_of_range if the index exceeds the bounds of the array.
Object Object::get_object_for_index(std::size_t index)
{
    throw_if_type_mismatch<json_type_array>(object);
    return Object
    {
        json_object_get(json_object_array_get_idx(object, index))
    };
}
}

namespace location = com::ubuntu::location;

location::service::ichnaea::Reporter::Reporter(
        const location::service::ichnaea::Reporter::Configuration& configuration)
    : http_client(core::net::http::make_client())
{
    auto uri = configuration.uri +
            ichnaea::submit::resource +
            configuration.key;

    submit_request_config = core::net::http::Request::Configuration::from_uri_as_string(uri);
    submit_request_config.ssl.verify_host = false;
    submit_request_config.ssl.verify_peer = false;

    if (not configuration.nick_name.empty())
        submit_request_config.header.add(Reporter::nick_name_header, configuration.nick_name);
}

location::service::ichnaea::Reporter::~Reporter()
{
    stop();
}

void location::service::ichnaea::Reporter::start()
{
    http_client_worker = std::move(
                std::thread
                {
                    [this]()
                    {
                        http_client->run();
                    }
                });
}

void location::service::ichnaea::Reporter::stop()
{
    http_client->stop();

    if (http_client_worker.joinable())
        http_client_worker.join();
}

void location::service::ichnaea::Reporter::report(
        const location::Update<location::Position>& update,
        const std::vector<location::connectivity::WirelessNetwork::Ptr>& wifis,
        const std::vector<location::connectivity::RadioCell::Ptr>& cells)
{
    json::Object submit = json::Object::create_object();
    json::Object items = json::Object::create_array();
    json::Object item = json::Object::create_object();

    item.put_string(Json::radio, "gsm"); // We currently only support gsm radio types.
    item.put_double(Json::lat, update.value.latitude.value.value());
    item.put_double(Json::lon, update.value.longitude.value.value());

    if (update.value.accuracy.horizontal)
        item.put_double(Json::accuracy, (*update.value.accuracy.horizontal).value());
    if (update.value.altitude)
        item.put_double(Json::altitude, (*update.value.altitude).value.value());
    if (update.value.accuracy.vertical)
        item.put_double(Json::altitude_accuracy, (*update.value.accuracy.vertical).value());

    if (!wifis.empty())
    {
        json::Object w = json::Object::create_array();
        ichnaea::Reporter::convert_wifis_to_json(wifis, w);
        item.put_array(Json::wifi, w);
    }

    if (!cells.empty())
    {
        json::Object c = json::Object::create_array();
        ichnaea::Reporter::convert_cells_to_json(cells, c);
        item.put_array(Json::cell, c);
    }

    items.append(item);
    submit.put_array(Json::items, items);

    auto string_representation = submit.to_plain_string();

    VLOG(10) << "Submitting: " << string_representation;

    auto request = http_client->post(
                submit_request_config,
                string_representation,
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

void location::service::ichnaea::Reporter::convert_wifis_to_json(
        const std::vector<location::connectivity::WirelessNetwork::Ptr>& wifis,
        json::Object& destination)
{
    for (const auto& wifi : wifis)
    {
        // We do not harvest any Wifi marked with '_nomap'.
        if (wifi->ssid().get().find("_nomap") != std::string::npos)
            continue;

        json::Object w = json::Object::create_object();
        w.put_string(Json::Wifi::key, wifi->bssid().get());

        if (wifi->frequency().get().is_valid())
            w.put_int32(Json::Wifi::frequency, static_cast<int>(wifi->frequency().get()));

        // We have a relative signal strength percentage in the wifi record.
        // TODO(tvoss): Check how that could be translated to RSSI.
        //wifi[Json::Wifi::signal] = -50;

        destination.append(w);
    }
}

void location::service::ichnaea::Reporter::convert_cells_to_json(
        const std::vector<location::connectivity::RadioCell::Ptr>& cells,
        json::Object& destination)
{
    for (const auto& cell : cells)
    {
        json::Object c = json::Object::create_object();

        switch (cell->type())
        {
        case connectivity::RadioCell::Type::gsm:
        {
            c.put_string(Json::Cell::radio, "gsm");

            const auto& details = cell->gsm();

            if (details.mobile_country_code.is_valid())
                c.put_int32(Json::Cell::mcc, details.mobile_country_code.get());
            if (details.mobile_network_code.is_valid())
                c.put_int32(Json::Cell::mnc, details.mobile_network_code.get());
            if (details.location_area_code.is_valid())
                c.put_int32(Json::Cell::lac, details.location_area_code.get());
            if (details.id.is_valid())
                c.put_int32(Json::Cell::cid, details.id.get());
            if  (details.strength.is_valid())
                c.put_int32(Json::Cell::asu, details.strength.get());

            break;
        }
        case connectivity::RadioCell::Type::umts:
        {
            c.put_string(Json::Cell::radio, "umts");

            const auto& details = cell->umts();

            if (details.mobile_country_code.is_valid())
                c.put_int32(Json::Cell::mcc, details.mobile_country_code.get());
            if (details.mobile_network_code.is_valid())
                c.put_int32(Json::Cell::mnc, details.mobile_network_code.get());
            if (details.location_area_code.is_valid())
                c.put_int32(Json::Cell::lac, details.location_area_code.get());
            if (details.id.is_valid())
                c.put_int32(Json::Cell::cid, details.id.get());
            if  (details.strength.is_valid())
                c.put_int32(Json::Cell::asu, details.strength.get());

            break;
        }
        case connectivity::RadioCell::Type::lte:
        {
            c.put_string(Json::Cell::radio, "lte");

            const auto& details = cell->lte();

            if (details.mobile_country_code.is_valid())
                c.put_int32(Json::Cell::mcc, details.mobile_country_code.get());
            if (details.mobile_network_code.is_valid())
                c.put_int32(Json::Cell::mnc, details.mobile_network_code.get());
            if (details.tracking_area_code.is_valid())
                c.put_int32(Json::Cell::lac, details.tracking_area_code.get());
            if (details.id.is_valid())
                c.put_int32(Json::Cell::cid, details.id.get());
            if (details.physical_id.is_valid())
                c.put_int32(Json::Cell::psc, details.physical_id.get());
            if  (details.strength.is_valid())
                c.put_int32(Json::Cell::asu, details.strength.get());
            break;
        }
        default:
            break;
        }

        destination.append(c);
    }
}

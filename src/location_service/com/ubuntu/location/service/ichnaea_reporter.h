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

#include <thread>

// Forward declare the opaque json_object handle from json-c here.
struct json_object;

// We are wrapping libjson-c and introduce a generic Object type for that purpose.
namespace json
{
// For internal purposes only
class Object
{
public:
    // Parses a new object from the given string.
    // Throws std::runtime_error in case of parsing errors.
    static Object parse_from_string(const std::string& s);
    // Creates a new object of type array.
    static Object create_array();
    // Creates a new object of type object.
    static Object create_object();

    // Shallow copy, only increments reference count.
    Object(const Object& rhs);
    // Decrements the reference count of the object.
    ~Object();

    // Shallow copy, decrements reference count of this object,
    // increments reference count of object contained in rhs.
    Object& operator=(const Object& rhs);

    // Encodes this object instance as a valid JSON string without any
    // unneccessary whitespace
    std::string to_plain_string();

    // Resolves the object with the given name.
    // Throws std::out_of_range if no object with the given name is known.
    Object get(const std::string& name) const;

    // Attempts to resolve the object to a boolean value.
    // Throws std::logic_error in case of type mismatches.
    bool to_bool() const;
    // Attempts to resolve the object to an integer value of 32bit width.
    // Throws std::logic_error in case of type mismatches.
    std::int32_t to_int32() const;
    // Attempts to resolve the object to an integer value of 64bit width.
    // Throws std::logic_error in case of type mismatches.
    std::int64_t to_int64() const;
    // Attempts to resolve the object to a floating point value.
    // Throws std::logic_error in case of type mismatches.
    double to_double() const;
    // Attempts to resolve the object to a string value.
    // Throws std::logic_error in case of type mismatches.
    std::string to_string() const;

    // Adds the given array under the given name to this object instance.
    void put_array(const std::string& name, Object array);
    // Adds the given object under the given name to this object instance.
    void put_object(const std::string& name, Object other);
    // Adds the given boolean under the given name to this object instance.
    void put_boolean(const std::string& name, bool value);
    // Adds the given integer under the given name to this object instance.
    void put_int32(const std::string& name, std::int32_t value);
    // Adds the given integer under the given name to this object instance.
    void put_int64(const std::string& name, std::int64_t value);
    // Adds the given floating point value under the given name to this object instance.
    void put_double(const std::string& name, double value);
    // Adds the given string value under the given name to this object instance.
    void put_string(const std::string& name, const std::string& value);

    // Only valid for array objects
    // Returns the size of the array, throws std::logic_error if the object
    // does not represent an array.
    std::size_t array_size() const;
    // Appends an object to the end of the array, throws std::logic_error if
    // the object does not represent an array.
    void append(Object other);
    // Replaces the object at index 'index' with the given instance.
    // Throws std::logic_error if the object does not represent an array.
    // Throws std::out_of_range if the index exceeds the bounds of the array.
    void put_object_for_index(std::size_t index, Object other);
    // Queries the object at index 'index'.
    // Throws std::logic_error if the object does not represent an array.
    // Throws std::out_of_range if the index exceeds the bounds of the array.
    Object get_object_for_index(std::size_t index);

private:
    Object(json_object* object);
    json_object* object;
};
}

namespace com{namespace ubuntu{namespace location{namespace service
{
/**
 * @brief All types and functions that are used to communicate with instances
 * of the Mozilla location service go here. Please see for further details:
 *
 *   - https://mozilla-ichnaea.readthedocs.org/en/latest/api/index.html#submit
 *   - https://github.com/mozilla/MozStumbler
 */
namespace ichnaea
{
namespace submit
{
/** @brief Resource path for wifi- and cell-id submissions.  */
constexpr const char* resource
{
    "/v1/submit?key="
};
/** @brief Http code marking errors for Mozilla location service instances. */
const core::net::http::Status error
{
    core::net::http::Status::bad_request
};
/** @brief Http code marking a successful submission request to Mozilla location service instances. */
const core::net::http::Status success
{
    core::net::http::Status::no_content
};
}

struct Reporter : public Harvester::Reporter
{
    /** @brief Submissions can be tagged with a nick-name for tracking on leaderboards. */
    static constexpr const char* nick_name_header{"X-Nickname"};

    /** @brief The JSON-dialect of the Mozilla location service is described here. */
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

    /** Creation-time options for the ICHNAEA reporter */
    struct Configuration
    {
        /** Uri of the ICHNAEA instance we want to submit to. */
        std::string uri;
        /** API key for the submission */
        std::string key;
        /** Nickname for the submission */
        std::string nick_name;
    };

    /** @brief Constructs a new instance with the given parameters. */
    Reporter(const Configuration& configuration);
    /** @brief Stops the reporter instance and frees all related resources. */
    ~Reporter();

    /** @brief Starts the reporter and prepares for submission. */
    void start() override;    
    /** @brief Stops the reporter. */
    void stop() override;

    /**
     * @brief Announced a position update, together with visible wifis and cells to the reporter.
     * @throws std::runtime_error if wifis and cells are empty.
     */
    void report(
            const Update<Position>& update,
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            const std::vector<connectivity::RadioCell::Ptr>& cells) override;



    /** @brief Encodes a collection of wifis into the Mozilla loation service JSON dialect. */
    static void convert_wifis_to_json(
            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
            json::Object& destination);

    /** @brief Encodes a collection of radio cells into the Mozilla loation service JSON dialect. */
    static void convert_cells_to_json(
            const std::vector<connectivity::RadioCell::Ptr>& cells,
            json::Object& destination);

    /** @brief The http request configuration for submissions to the mozilla location service. */
    core::net::http::Request::Configuration submit_request_config;
    /** @brief The http client instance used to talk to Mozilla location service instances. */
    std::shared_ptr<core::net::http::Client> http_client;
    /** @brief Worker thread for dispatching the http client instance. */
    std::thread http_client_worker;
};
}
}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_ICHNAEA_REPORTER_H_

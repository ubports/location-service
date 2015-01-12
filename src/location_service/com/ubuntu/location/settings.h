/*
 * Copyright © 2014 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SETTINGS_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SETTINGS_H_

#include <cstdint>

#include <memory>
#include <sstream>
#include <string>

namespace com
{
namespace ubuntu
{
namespace location
{
// A simple interface for reading/writing values given a string key.
struct Settings
{
    // To safe us some typing
    typedef std::shared_ptr<Settings> Ptr;

    // Collects possible exceptions reported from getters.
    struct Error
    {
        Error() = delete;

        // Thrown if no value of the specified type is available for
        // the given key.
        struct NoValueForKey : public std::runtime_error
        {
            NoValueForKey(const std::string& key);
        };
    };

    // The usual boilerplate
    Settings() = default;
    Settings(const Settings&) = delete;
    virtual ~Settings() = default;
    Settings& operator=(const Settings&) = delete;

    // Syncs the current settings to implementation-specific backends.
    virtual void sync() = 0;

    // Returns true iff a value is known for the given key.
    virtual bool has_value_for_key(const std::string& key) const = 0;

    // Convenience wrapper.
    template<typename T>
    inline T get_enum_for_key(const std::string& key, T default_value);

    template<typename T>
    inline T get_enum_for_key_or_throw(const std::string& key);

    // Tries to read the value for the given key and returns the default_value if
    // no value is known.
    std::string get_string_for_key(const std::string& key, const std::string& default_value);

    // Gets an integer value known for the given key, or throws Error::NoValueForKey.
    virtual std::string get_string_for_key_or_throw(const std::string& key) = 0;

    // Convenience wrapper.
    template<typename T>
    inline bool set_enum_for_key(const std::string& key, T value);

    // Sets values known for the given key.
    virtual bool set_string_for_key(const std::string& key, const std::string& value) = 0;
};

template<typename T>
inline T Settings::get_enum_for_key(const std::string& key, T default_value)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    std::stringstream dss; dss << default_value;
    std::stringstream ss{get_string_for_key(key, dss.str())};
    T result; ss >> result;
    return result;
}

template<typename T>
inline T Settings::get_enum_for_key_or_throw(const std::string& key)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    std::stringstream ss{get_string_for_key_or_throw(key)};
    T result; ss >> result;
    return result;
}

template<typename T>
inline bool Settings::set_enum_for_key(const std::string& key, T value)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    std::stringstream ss; ss << value;
    return set_string_for_key(key, ss.str());
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SETTINGS_H_

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
    inline T get_enum_for_key_or_throw(const std::string& key, T default_value);

    template<typename T>
    inline T get_enum_for_key_or_throw(const std::string& key);

    // Tries to read the value for the given key and returns the default_value if
    // no value is known.
    std::int32_t get_int_for_key(const std::string& key, std::uint32_t default_value);

    // Gets an integer value known for the given key, or throws Error::NoValueForKey.
    virtual std::int32_t get_int_for_key_or_throw(const std::string& key) = 0;

    // Convenience wrapper.
    template<typename T>
    inline bool set_enum_for_key(const std::string& key, T value);

    // Sets values known for the given key.
    virtual bool set_int_for_key(const std::string& key, std::int32_t value) = 0;
};

template<typename T>
inline T Settings::get_enum_for_key_or_throw(const std::string& key, T default_value)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    return static_cast<T>(get_int_for_key(key, static_cast<std::int32_t>(default_value)));
}

template<typename T>
inline T Settings::get_enum_for_key_or_throw(const std::string& key)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    return static_cast<T>(get_int_for_key_or_throw(key));
}

template<typename T>
inline bool Settings::set_enum_for_key(const std::string& key, T value)
{
    static_assert(std::is_enum<T>::value, "Only enum types are supported.");
    return set_int_for_key(key, static_cast<std::int32_t>(value));
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SETTINGS_H_

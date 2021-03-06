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

#include <com/ubuntu/location/settings.h>

namespace location = com::ubuntu::location;

location::Settings::Error::NoValueForKey::NoValueForKey(const std::string& key)
    : std::runtime_error{"location::Settings: Could not find value for key: " + key}
{
}

std::string location::Settings::get_string_for_key(const std::string& key, const std::string& default_value)
{
    if (not has_value_for_key(key))
        return default_value;

    return get_string_for_key_or_throw(key);
}

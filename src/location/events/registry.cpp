/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/events/registry.h>

namespace
{
location::Event::Type increment(location::Event::Type type, std::size_t value)
{
    return static_cast<location::Event::Type>(static_cast<std::size_t>(type) + value);
}
}

location::events::Registry& location::events::Registry::instance()
{
    static Registry registry;
    return registry;
}

std::string location::events::Registry::find(location::Event::Type type) const
{
    std::lock_guard<std::mutex> lg{guard};
    return type_name_lut.at(type);
}

location::Event::Type location::events::Registry::find(const std::string& name) const
{
    std::lock_guard<std::mutex> lg{guard};
    return name_type_lut.at(name);
}

location::Event::Type location::events::Registry::insert(const std::string& name)
{
    std::lock_guard<std::mutex> lg{guard};

    auto it = name_type_lut.find(name);
    if (it == name_type_lut.end())
    {
        std::tie(it, std::ignore) = name_type_lut.insert(
                    std::make_pair(
                        name,
                        increment(location::Event::Type::first_user_defined_type, counter++)));

        type_name_lut[it->second] = name;

        return it->second;
    }

    throw std::runtime_error{"Event " + name + " is already known to the registry."};
}

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
#ifndef LOCATION_EVENTS_REGISTRY_H_
#define LOCATION_EVENTS_REGISTRY_H_

#include <location/event.h>

#include <map>
#include <mutex>
#include <string>

namespace location
{
namespace events
{
class Registry
{
public:
    // instance returns the singleton instance of Registry.
    static Registry& instance();

    // find returns the name assigned to type or throws std::out_of_range.
    std::string find(location::Event::Type type) const;

    // find returns the type assigned to name or throws std::out_of_range.
    location::Event::Type find(const std::string& name) const;

    // insert makes name known and assigns it a unique type in the scope of
    // the process.
    location::Event::Type insert(const std::string& name);

private:
    // Boilerplate to prevent the outside from creating/deleting/copying/moving
    // Registry instances.
    Registry() = default;
    Registry(const Registry&) = delete;
    Registry(Registry&&) = delete;
    ~Registry() = default;
    Registry& operator=(const Registry&) = delete;
    Registry& operator=(Registry&&) = delete;

    mutable std::mutex guard;
    std::size_t counter{0};
    std::map<location::Event::Type, std::string> type_name_lut;
    std::map<std::string, location::Event::Type> name_type_lut;
};
}
}

#endif // LOCATION_EVENTS_REGISTRY_H_

/*
 * Copyright © 2017 Canonical Ltd.
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

#ifndef LOCATION_UTIL_SETTINGS_H_
#define LOCATION_UTIL_SETTINGS_H_

#include <location/optional.h>
#include <location/visibility.h>

#include <boost/property_tree/ptree.hpp>

#include <sstream>
#include <string>

namespace location
{
namespace util
{
namespace settings
{

class LOCATION_DLL_PUBLIC Source
{
public:
    explicit Source(const boost::property_tree::ptree& ptree = boost::property_tree::ptree{});

    template<typename T>
    T get_value(const std::string& key, T value) const
    {
        if (auto v = get(key))
        {
            std::istringstream iss{*v};
            iss >> value;
        }

        return value;
    }

    template<typename T>
    void set_value(const std::string& key, const T& value)
    {
        std::ostringstream oss;
        oss << value;

        set(key, oss.str());
    }

private:
    Optional<std::string> get(const std::string& key) const;
    void set(const std::string& key, const std::string& value);

    boost::property_tree::ptree ptree;
};

}  // namespace settings
}  // namespace util
}  // namespace location

#endif // LOCATION_UTIL_SETTINGS_H_

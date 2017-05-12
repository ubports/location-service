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

#include <location/util/settings.h>

#include <core/posix/this_process.h>

#include <boost/filesystem.hpp>

namespace settings = location::util::settings;

settings::Source::Source(const boost::property_tree::ptree& ptree)
    : ptree{ptree}
{
}

location::Optional<std::string> settings::Source::get(const std::string& key) const
{
    namespace env = core::posix::this_process::env;
    namespace fs = boost::filesystem;

    static const std::string snap_path = env::get("SNAP_DATA");

    fs::path path{snap_path};
    auto path_from_key = key;
    std::replace(path_from_key.begin(), path_from_key.end(), '.', '/');
    path /= path_from_key;

    if (ptree.count(key) > 0)
        return ptree.get_value(key);

    if (fs::exists(path))
    {
        std::ifstream in{path.string()};
        std::string content;

        std::copy(std::istream_iterator<char>{in}, std::istream_iterator<char>{}, std::back_inserter(content));
        return content;
    }

    return Optional<std::string>{};
}

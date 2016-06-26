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

#include <location/service/state.h>

#include <map>
#include <type_traits>

namespace
{
typedef typename std::underlying_type<location::service::State>::type UT;

typedef std::pair<std::map<location::service::State, std::string>, std::map<std::string, location::service::State>> Lut;

const Lut& lut()
{
    static const Lut instance
    {
        {
            {location::service::State::disabled, "disabled"},
            {location::service::State::enabled, "enabled"},
            {location::service::State::active, "active"}
        },
        {
            {"disabled", location::service::State::disabled},
            {"enabled", location::service::State::enabled},
            {"active", location::service::State::active}
        }
    };

    return instance;
}
}

bool location::service::operator==(location::service::State lhs, location::service::State rhs)
{
    return static_cast<UT>(lhs) == static_cast<UT>(rhs);
}

bool location::service::operator!=(location::service::State lhs, location::service::State rhs)
{
    return static_cast<UT>(lhs) != static_cast<UT>(rhs);
}

std::ostream& location::service::operator<<(std::ostream& out, location::service::State state)
{
    return out << lut().first.at(state);
}

std::istream& location::service::operator>>(std::istream& in, location::service::State& state)
{
    std::string s; in >> s; state = lut().second.at(s);
    return in;
}

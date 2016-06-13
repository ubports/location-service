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

#include <com/ubuntu/location/service/state.h>

#include <map>
#include <type_traits>

namespace culs = com::ubuntu::location::service;

namespace
{
typedef typename std::underlying_type<culs::State>::type UT;

typedef std::pair<std::map<culs::State, std::string>, std::map<std::string, culs::State>> Lut;

const Lut& lut()
{
    static const Lut instance
    {
        {
            {culs::State::disabled, "disabled"},
            {culs::State::enabled, "enabled"},
            {culs::State::active, "active"}
        },
        {
            {"disabled", culs::State::disabled},
            {"enabled", culs::State::enabled},
            {"active", culs::State::active}
        }
    };

    return instance;
}
}

bool culs::operator==(culs::State lhs, culs::State rhs)
{
    return static_cast<UT>(lhs) == static_cast<UT>(rhs);
}

bool culs::operator!=(culs::State lhs, culs::State rhs)
{
    return static_cast<UT>(lhs) != static_cast<UT>(rhs);
}

std::ostream& culs::operator<<(std::ostream& out, culs::State state)
{
    return out << lut().first.at(state);
}

std::istream& culs::operator>>(std::istream& in, culs::State& state)
{
    std::string s; in >> s; state = lut().second.at(s);
    return in;
}

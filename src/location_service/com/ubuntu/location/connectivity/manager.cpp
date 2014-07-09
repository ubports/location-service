/*
 * Copyright © 2012-2014 Canonical Ltd.
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

#include <com/ubuntu/location/connectivity/manager.h>

namespace connectivity = com::ubuntu::location::connectivity;

std::ostream& connectivity::operator<<(std::ostream& out, connectivity::State state)
{
    switch (state)
    {
    case State::unknown: out << "State::unknown"; break;
    case State::asleep: out << "State::asleep"; break;
    case State::disconnected: out << "State::disconnected"; break;
    case State::disconnecting: out << "State::disconnecting"; break;
    case State::connecting: out << "State::connecting"; break;
    case State::connected_local: out << "State::connected_local"; break;
    case State::connected_site: out << "State::connected_site"; break;
    case State::connected_global: out << "State::connected_global"; break;
    }

    return out;
}

connectivity::Characteristics connectivity::operator|(connectivity::Characteristics l, connectivity::Characteristics r)
{
    return static_cast<connectivity::Characteristics>(static_cast<std::uint32_t>(l) | static_cast<std::uint32_t>(r));
}

connectivity::Characteristics connectivity::operator&(connectivity::Characteristics l, connectivity::Characteristics r)
{
    return static_cast<connectivity::Characteristics>(static_cast<std::uint32_t>(l) & static_cast<std::uint32_t>(r));
}

std::ostream& connectivity::operator<<(std::ostream& out, connectivity::Characteristics characteristics)
{
    bool first{true};

    out << "[";

    if ((characteristics & connectivity::Characteristics::connection_has_monetary_costs) != connectivity::Characteristics::none)
        out << "connection_has_monetary_costs"; first = false;
    if ((characteristics & connectivity::Characteristics::connection_is_bandwith_limited) != connectivity::Characteristics::none)
        out << (first ? "" : ", ") << "connection_is_bandwidth_limited"; first = false;
    if ((characteristics & connectivity::Characteristics::connection_is_volume_limited) != connectivity::Characteristics::none)
        out << (first ? "" : ", ") << "connection_is_volume_limited"; first = false;

    out << "]";
    return out;
}

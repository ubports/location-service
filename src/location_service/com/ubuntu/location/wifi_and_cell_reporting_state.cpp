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

#include <com/ubuntu/location/wifi_and_cell_reporting_state.h>

#include <iostream>
#include <unordered_map>

namespace location = com::ubuntu::location;

namespace std
{
template<>
struct hash<location::WifiAndCellIdReportingState>
{
    std::size_t operator()(const location::WifiAndCellIdReportingState& s) const
    {
        static const std::hash<std::uint32_t> hash;
        return hash(static_cast<std::uint32_t>(s));
    }
};
}

std::ostream& location::operator<<(std::ostream& out, location::WifiAndCellIdReportingState state)
{
    static constexpr const char* the_unknown_state
    {
        "WifiAndCellIdReportingState::unknown"
    };

    static const std::unordered_map<location::WifiAndCellIdReportingState, std::string> lut
    {
        {location::WifiAndCellIdReportingState::off, "WifiAndCellIdReportingState::off"},
        {location::WifiAndCellIdReportingState::on, "WifiAndCellIdReportingState::on"}
    };

    auto it = lut.find(state);
    if (it != lut.end())
        out << it->second;
    else
        out << the_unknown_state;

    return out;
}

std::istream& location::operator>>(std::istream& in, location::WifiAndCellIdReportingState& state)
{
    static const std::unordered_map<std::string, location::WifiAndCellIdReportingState> lut
    {
        {"WifiAndCellIdReportingState::off", location::WifiAndCellIdReportingState::off},
        {"WifiAndCellIdReportingState::on", location::WifiAndCellIdReportingState::on}
    };

    std::string s; in >> s;
    auto it = lut.find(s);
    if (it != lut.end())
        state = it->second;
    else throw std::runtime_error
    {
        "location::operator>>(std::istream&, WifiAndCellIdReportingState&): "
        "Could not resolve state " + s
    };

    return in;
}

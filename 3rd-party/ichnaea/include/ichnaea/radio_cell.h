// Copyright (C) 2016 Canonical Ltd.
// 
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ICHNAEA_RADIO_CELL_H_
#define ICHNAEA_RADIO_CELL_H_

#include <boost/optional.hpp>

#include <chrono>
#include <iosfwd>

namespace ichnaea
{
/// @brief CellTower models an individual radio cell tower.
struct RadioCell
{
    enum class RadioType
    {
        gsm,
        wcdma,
        lte
    };

    /// @cond
    typedef unsigned int MCC;
    typedef unsigned int MNC;
    typedef unsigned int LAC;
    typedef unsigned int Id;
    typedef unsigned int PSC;
    /// @endcond

    RadioType radio_type;                    ///< The type of radio network.
    bool serving;                            ///< Whether this is the serving or a neighboring cell.
    MCC mcc;                                 ///< The mobile country code.
    MNC mnc;                                 ///< The mobile network code.
    LAC lac;                                 ///< The location area code for GSM and WCDMA networks. The tracking area code for LTE networks.
    Id id;                                   ///< The cell id or cell identity.
    std::chrono::milliseconds age;           ///< The number of milliseconds since this networks was last detected.
    PSC psc;                                 ///< The primary scrambling code for WCDMA and physical cell id for LTE.
    boost::optional<double> asu;             ///< The arbitrary strength unit indicating the signal strength if a direct signal strength reading is not available.
    boost::optional<double> signal_strength; ///< The signal strength for this cell network, either the RSSI or RSCP.
    double timing_advance;                   ///< The timing advance value for this cell network.
};

/// @brief operator<< inserts radio_type into out.
std::ostream& operator<<(std::ostream& out, RadioCell::RadioType radio_type);
}

#endif // ICHNAEA_RADIO_CELL_H_

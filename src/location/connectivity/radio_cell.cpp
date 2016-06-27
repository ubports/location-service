/*
 * Copyright © 2012-2013 Canonical Ltd.
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

#include <location/connectivity/radio_cell.h>

bool com::ubuntu::location::connectivity::operator==(const com::ubuntu::location::connectivity::RadioCell::Gsm& lhs,
                                        const com::ubuntu::location::connectivity::RadioCell::Gsm& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
           lhs.mobile_network_code == rhs.mobile_network_code &&
           lhs.location_area_code == rhs.location_area_code &&
           lhs.id == rhs.id &&
           lhs.strength == rhs.strength;
}

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, const com::ubuntu::location::connectivity::RadioCell::Gsm& gsm)
{
    out << "("
        << "mcc: " << gsm.mobile_country_code << ", "
        << "mnc: " << gsm.mobile_network_code << ", "
        << "lac: " << gsm.location_area_code << ", "
        << "id: " << gsm.id << ", "
        << "asu: " << gsm.strength << ")";

    return out;
}

bool com::ubuntu::location::connectivity::operator==(const com::ubuntu::location::connectivity::RadioCell::Umts& lhs, const com::ubuntu::location::connectivity::RadioCell::Umts& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
            lhs.mobile_network_code == rhs.mobile_network_code &&
            lhs.location_area_code == rhs.location_area_code &&
            lhs.id == rhs.id &&
            lhs.strength == rhs.strength;
}

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, const com::ubuntu::location::connectivity::RadioCell::Umts& umts)
{
    out << "("
        << "mcc: " << umts.mobile_country_code << ", "
        << "mnc: " << umts.mobile_network_code << ", "
        << "lac: " << umts.location_area_code << ", "
        << "id: " << umts.id << ", "
        << "asu: " << umts.strength << ")";

    return out;
}

bool com::ubuntu::location::connectivity::operator==(const com::ubuntu::location::connectivity::RadioCell::Lte& lhs,
                                        const com::ubuntu::location::connectivity::RadioCell::Lte& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
            lhs.mobile_network_code == rhs.mobile_network_code &&
            lhs.tracking_area_code == rhs.tracking_area_code &&
            lhs.id == rhs.id &&
            lhs.physical_id == rhs.physical_id &&
            lhs.strength == rhs.strength;
}

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, const com::ubuntu::location::connectivity::RadioCell::Lte& lte)
{
    out << "("
        << "mcc: " << lte.mobile_country_code << ", "
        << "mnc: " << lte.mobile_network_code << ", "
        << "lac: " << lte.tracking_area_code << ", "
        << "id: " << lte.id << ", "
        << "id: " << lte.physical_id << ", "
        << "asu: " << lte.strength << ")";

    return out;
}

bool com::ubuntu::location::connectivity::operator==(const com::ubuntu::location::connectivity::RadioCell& lhs,
                                        const com::ubuntu::location::connectivity::RadioCell& rhs)
{
    if (lhs.type() != rhs.type())
        return false;

    switch(lhs.type())
    {
    case com::ubuntu::location::connectivity::RadioCell::Type::gsm: return lhs.gsm() == rhs.gsm();
    case com::ubuntu::location::connectivity::RadioCell::Type::umts: return lhs.umts() == rhs.umts();
    case com::ubuntu::location::connectivity::RadioCell::Type::lte: return lhs.lte() == rhs.lte();
    default: return true;
    }

    return false;
}

std::ostream& com::ubuntu::location::connectivity::operator<<(std::ostream& out, const com::ubuntu::location::connectivity::RadioCell& cell)
{
    switch (cell.type())
    {
    case com::ubuntu::location::connectivity::RadioCell::Type::gsm: out << "gsm" << cell.gsm(); break;
    case com::ubuntu::location::connectivity::RadioCell::Type::umts: out << "umts" << cell.umts(); break;
    case com::ubuntu::location::connectivity::RadioCell::Type::lte: out << "lte" << cell.lte(); break;
    case com::ubuntu::location::connectivity::RadioCell::Type::unknown: break;
    }

    return out;
}

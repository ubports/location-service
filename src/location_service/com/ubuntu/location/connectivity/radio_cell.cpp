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

#include <com/ubuntu/location/connectivity/radio_cell.h>

namespace location = com::ubuntu::location;

location::connectivity::RadioCell::RadioCell() : radio_type(Type::gsm), detail{Gsm()}
{
}

location::connectivity::RadioCell::RadioCell(const location::connectivity::RadioCell::Gsm& gsm)
    : radio_type(Type::gsm), detail{gsm}
{
}

location::connectivity::RadioCell::RadioCell(const location::connectivity::RadioCell::Umts& umts)
    : radio_type(Type::umts), detail{umts}
{
}

location::connectivity::RadioCell::RadioCell(const location::connectivity::RadioCell::Lte& lte)
    : radio_type(Type::lte), detail{lte}
{
}

location::connectivity::RadioCell::RadioCell(const location::connectivity::RadioCell& rhs) : radio_type(rhs.radio_type)
{
    switch(radio_type)
    {
    case location::connectivity::RadioCell::Type::gsm: detail.gsm = rhs.detail.gsm; break;
    case location::connectivity::RadioCell::Type::umts: detail.umts = rhs.detail.umts; break;
    case location::connectivity::RadioCell::Type::lte: detail.lte = rhs.detail.lte; break;
    case location::connectivity::RadioCell::Type::unknown: break;
    }
}

location::connectivity::RadioCell& location::connectivity::RadioCell::operator=(const location::connectivity::RadioCell& rhs)
{
    radio_type = rhs.radio_type;
    switch(radio_type)
    {
    case location::connectivity::RadioCell::Type::gsm: detail.gsm = rhs.detail.gsm; break;
    case location::connectivity::RadioCell::Type::umts: detail.umts = rhs.detail.umts; break;
    case location::connectivity::RadioCell::Type::lte: detail.lte = rhs.detail.lte; break;
    case location::connectivity::RadioCell::Type::unknown: break;
    }

    return *this;
}

location::connectivity::RadioCell::Type location::connectivity::RadioCell::type() const
{
    return radio_type;
}

const location::connectivity::RadioCell::Gsm& location::connectivity::RadioCell::gsm() const
{
    if (radio_type != location::connectivity::RadioCell::Type::gsm)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.gsm;
}

const location::connectivity::RadioCell::Umts& location::connectivity::RadioCell::umts() const
{
    if (radio_type != location::connectivity::RadioCell::Type::umts)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.umts;
}

const location::connectivity::RadioCell::Lte& location::connectivity::RadioCell::lte() const
{
    if (radio_type != Type::lte)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.lte;
}

location::connectivity::RadioCell::Detail::Detail() : none(location::connectivity::RadioCell::None{})
{
}

location::connectivity::RadioCell::Detail::Detail(const location::connectivity::RadioCell::Gsm& gsm) : gsm(gsm)
{
}

location::connectivity::RadioCell::Detail::Detail(const location::connectivity::RadioCell::Umts& umts) : umts(umts)
{
}

location::connectivity::RadioCell::Detail::Detail(const location::connectivity::RadioCell::Lte& lte) : lte(lte)
{
}

bool location::connectivity::operator==(const location::connectivity::RadioCell::Gsm& lhs,
                                        const location::connectivity::RadioCell::Gsm& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
           lhs.mobile_network_code == rhs.mobile_network_code &&
           lhs.location_area_code == rhs.location_area_code &&
           lhs.id == rhs.id &&
           lhs.strength == rhs.strength;
}

std::ostream& location::connectivity::operator<<(std::ostream& out, const location::connectivity::RadioCell::Gsm& gsm)
{
    out << "("
        << "mcc: " << gsm.mobile_country_code << ", "
        << "mnc: " << gsm.mobile_network_code << ", "
        << "lac: " << gsm.location_area_code << ", "
        << "id: " << gsm.id << ", "
        << "asu: " << gsm.strength << ")";

    return out;
}

bool location::connectivity::operator==(const location::connectivity::RadioCell::Umts& lhs, const location::connectivity::RadioCell::Umts& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
            lhs.mobile_network_code == rhs.mobile_network_code &&
            lhs.location_area_code == rhs.location_area_code &&
            lhs.id == rhs.id &&
            lhs.strength == rhs.strength;
}

std::ostream& location::connectivity::operator<<(std::ostream& out, const location::connectivity::RadioCell::Umts& umts)
{
    out << "("
        << "mcc: " << umts.mobile_country_code << ", "
        << "mnc: " << umts.mobile_network_code << ", "
        << "lac: " << umts.location_area_code << ", "
        << "id: " << umts.id << ", "
        << "asu: " << umts.strength << ")";

    return out;
}

bool location::connectivity::operator==(const location::connectivity::RadioCell::Lte& lhs,
                                        const location::connectivity::RadioCell::Lte& rhs)
{
    return lhs.mobile_country_code == rhs.mobile_country_code &&
            lhs.mobile_network_code == rhs.mobile_network_code &&
            lhs.tracking_area_code == rhs.tracking_area_code &&
            lhs.id == rhs.id &&
            lhs.physical_id == rhs.physical_id &&
            lhs.strength == rhs.strength;
}

std::ostream& location::connectivity::operator<<(std::ostream& out, const location::connectivity::RadioCell::Lte& lte)
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

bool location::connectivity::operator==(const location::connectivity::RadioCell& lhs,
                                        const location::connectivity::RadioCell& rhs)
{
    if (lhs.type() != rhs.type())
        return false;

    switch(lhs.type())
    {
    case location::connectivity::RadioCell::Type::gsm: return lhs.gsm() == rhs.gsm();
    case location::connectivity::RadioCell::Type::umts: return lhs.umts() == rhs.umts();
    case location::connectivity::RadioCell::Type::lte: return lhs.lte() == rhs.lte();
    default: return true;
    }

    return false;
}

std::ostream& location::connectivity::operator<<(std::ostream& out, const location::connectivity::RadioCell& cell)
{
    switch (cell.type())
    {
    case location::connectivity::RadioCell::Type::gsm: out << cell.gsm(); break;
    case location::connectivity::RadioCell::Type::umts: out << cell.umts(); break;
    case location::connectivity::RadioCell::Type::lte: out << cell.lte(); break;
    case location::connectivity::RadioCell::Type::unknown: break;
    }

    return out;
}

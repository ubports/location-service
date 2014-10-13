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

#include <com/ubuntu/location/connectivity/cached_radio_cell.h>

#include <core/posix/this_process.h>

namespace
{
// We use this for debugging purposes.
const bool also_apply_cell_change_heuristics_to_gsm_cells =
    core::posix::this_process::env::get(
            "COM_UBUNTU_LOCATION_CONNECTIVITY_DATA_CELL_FOR_GSM_TOO",
            "false") == "true";

std::int64_t timeout_in_seconds()
{
    auto value = core::posix::this_process::env::get("COM_UBUNTU_LOCATION_CONNECTIVITY_DATA_CELL_TIMEOUT", "60");
    std::stringstream ss(value);
    std::uint64_t result; ss >> result;

    return result;
}

// We only activate our heuristics if we are running on problematic hardware
bool is_running_on_problematic_modem_firmware(const org::Ofono::Manager::Modem& modem)
{
    // This is somewhat ambigious and we certainly want to
    // maintain a database of problematic modem firmware in the
    // future.
    auto path = modem.object->path().as_string();
    return path.find("ril") != std::string::npos;
}
}

const std::map<std::string, com::ubuntu::location::connectivity::RadioCell::Type>& detail::CachedRadioCell::type_lut()
{
    static const std::map<std::string, com::ubuntu::location::connectivity::RadioCell::Type> lut
    {
        {
            org::Ofono::Manager::Modem::NetworkRegistration::Technology::gsm(),
            com::ubuntu::location::connectivity::RadioCell::Type::gsm
        },
        {
            org::Ofono::Manager::Modem::NetworkRegistration::Technology::lte(),
            com::ubuntu::location::connectivity::RadioCell::Type::lte
        },
        {
            org::Ofono::Manager::Modem::NetworkRegistration::Technology::umts(),
            com::ubuntu::location::connectivity::RadioCell::Type::umts
        },
        {
            org::Ofono::Manager::Modem::NetworkRegistration::Technology::edge(),
            com::ubuntu::location::connectivity::RadioCell::Type::gsm
        },
        {
            org::Ofono::Manager::Modem::NetworkRegistration::Technology::hspa(),
            com::ubuntu::location::connectivity::RadioCell::Type::umts
        },
        {std::string(), com::ubuntu::location::connectivity::RadioCell::Type::unknown}
    };

    return lut;
}

detail::CachedRadioCell::CellChangeHeuristics::CellChangeHeuristics(
        boost::asio::io_service& io_service,
        bool needed)
    : needed(needed),
      io_service(io_service),
      invalidation_timer(io_service),
      valid(true)
{
}

detail::CachedRadioCell::CachedRadioCell(const org::Ofono::Manager::Modem& modem, boost::asio::io_service& io_service)
    : RadioCell(),      
      cell_change_heuristics(io_service, is_running_on_problematic_modem_firmware(modem)),
      roaming(false),
      radio_type(Type::gsm),
      modem(modem),
      detail{}
{
    auto status = query_status();

    if (not is_registered_or_roaming(status))
        return;

    radio_type = query_technology();

    if (radio_type == com::ubuntu::location::connectivity::RadioCell::Type::unknown)
        return;

    auto lac = query_lac();
    auto mcc = query_mnc();
    auto mnc = query_mnc();
    auto cell_id = query_cid();
    auto strength = query_strength();

    switch(radio_type)
    {
    case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
    {
        com::ubuntu::location::connectivity::RadioCell::Gsm gsm
        {
            com::ubuntu::location::connectivity::RadioCell::Gsm::MCC{mcc},
            com::ubuntu::location::connectivity::RadioCell::Gsm::MNC{mnc},
            com::ubuntu::location::connectivity::RadioCell::Gsm::LAC{lac},
            com::ubuntu::location::connectivity::RadioCell::Gsm::ID{cell_id},
            com::ubuntu::location::connectivity::RadioCell::Gsm::SignalStrength::from_percent(strength/100.f)
        };
        VLOG(1) << gsm;
        detail.gsm = gsm;
        break;
    }
    case com::ubuntu::location::connectivity::RadioCell::Type::lte:
    {
        com::ubuntu::location::connectivity::RadioCell::Lte lte
        {
            com::ubuntu::location::connectivity::RadioCell::Lte::MCC{mcc},
            com::ubuntu::location::connectivity::RadioCell::Lte::MNC{mnc},
            com::ubuntu::location::connectivity::RadioCell::Lte::TAC{lac},
            com::ubuntu::location::connectivity::RadioCell::Lte::ID{cell_id},
            com::ubuntu::location::connectivity::RadioCell::Lte::PID{},
            com::ubuntu::location::connectivity::RadioCell::Lte::SignalStrength::from_percent(strength/100.f)
        };
        VLOG(1) << lte;
        detail.lte = lte;
        break;
    }
    case com::ubuntu::location::connectivity::RadioCell::Type::umts:
    {
        com::ubuntu::location::connectivity::RadioCell::Umts umts
        {
            com::ubuntu::location::connectivity::RadioCell::Umts::MCC{mcc},
            com::ubuntu::location::connectivity::RadioCell::Umts::MNC{mnc},
            com::ubuntu::location::connectivity::RadioCell::Umts::LAC{lac},
            com::ubuntu::location::connectivity::RadioCell::Umts::ID{cell_id},
            com::ubuntu::location::connectivity::RadioCell::Umts::SignalStrength::from_percent(strength/100.f)
        };
        VLOG(1) << umts;
        detail.umts = umts;
        break;
    }
    default:
        break;
    }

    roaming = status == org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::roaming;

    execute_cell_change_heuristics_if_appropriate();

    // And we finally subscribe to property changes.
    connections.network_registration_properties_changed = modem.network_registration.signals.property_changed->connect([this](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
    {
        on_network_registration_property_changed(tuple);
    });
}

detail::CachedRadioCell::~CachedRadioCell()
{
    // TODO(tvoss): Reenable this once we know why the modem cache gets flushed.
    // modem.network_registration.signals.property_changed->disconnect(connections.network_registration_properties_changed);
}

const core::Property<bool>& detail::CachedRadioCell::is_roaming() const
{
    return roaming;
}

const core::Property<bool>& detail::CachedRadioCell::is_valid() const
{
    return cell_change_heuristics.valid;
}

const core::Signal<>& detail::CachedRadioCell::changed() const
{
    return on_changed;
}

com::ubuntu::location::connectivity::RadioCell::Type detail::CachedRadioCell::type() const
{
    return radio_type;
}

const com::ubuntu::location::connectivity::RadioCell::Gsm& detail::CachedRadioCell::gsm() const
{
    if (radio_type != com::ubuntu::location::connectivity::RadioCell::Type::gsm)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.gsm;
}

const com::ubuntu::location::connectivity::RadioCell::Umts& detail::CachedRadioCell::umts() const
{
    if (radio_type != RadioCell::Type::umts)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.umts;
}

const com::ubuntu::location::connectivity::RadioCell::Lte& detail::CachedRadioCell::lte() const
{
    if (radio_type != RadioCell::Type::lte)
        throw std::runtime_error("Bad access to unset network type.");

    return detail.lte;
}

void detail::CachedRadioCell::on_network_registration_property_changed(const std::tuple<std::string, core::dbus::types::Variant>& tuple)
{
    VLOG(10) << "Property changed on modem " << modem.object->path() << " for network registration: " << std::get<0>(tuple);

    const auto& key = std::get<0>(tuple);
    const auto& variant = std::get<1>(tuple);

    bool did_cell_identity_change = false;

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::Technology::name())
    {
        did_cell_identity_change = true;

        auto value = variant.as<
                    org::Ofono::Manager::Modem::NetworkRegistration::Technology::ValueType
                >();

        auto it = type_lut().find(value);

        if (it == type_lut().end())
        {
            VLOG(1) << "Unknown technology for connected cell: " << value;
            return;
        }

        if (it->second == com::ubuntu::location::connectivity::RadioCell::Type::unknown)
        {
            VLOG(1) << "Unknown technology for connected cell: " + value;
            return;
        }

        if (radio_type == it->second)
            return;

        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            switch(it->second)
            {
            case com::ubuntu::location::connectivity::RadioCell::Type::umts:
                detail.umts.location_area_code = detail.gsm.location_area_code;
                detail.umts.mobile_network_code = detail.gsm.mobile_network_code;
                detail.umts.mobile_country_code = detail.gsm.mobile_country_code;
                detail.umts.strength.reset();
                detail.umts.id.reset();
                break;
            case com::ubuntu::location::connectivity::RadioCell::Type::lte:
                detail.lte.tracking_area_code = detail.gsm.location_area_code;
                detail.lte.mobile_network_code = detail.gsm.mobile_network_code;
                detail.lte.mobile_country_code = detail.gsm.mobile_country_code;
                detail.lte.strength.reset();
                detail.lte.id.reset();
                break;
            default:
                break;
            }
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            switch(it->second)
            {
            case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
                detail.gsm.location_area_code = detail.umts.location_area_code;
                detail.gsm.mobile_network_code = detail.umts.mobile_network_code;
                detail.gsm.mobile_country_code = detail.umts.mobile_country_code;
                detail.gsm.strength.reset();
                detail.gsm.id.reset();
                break;
            case com::ubuntu::location::connectivity::RadioCell::Type::lte:
                detail.lte.tracking_area_code = detail.umts.location_area_code;
                detail.lte.mobile_network_code = detail.umts.mobile_network_code;
                detail.lte.mobile_country_code = detail.umts.mobile_country_code;
                detail.lte.strength.reset();
                detail.lte.id.reset();
                break;
            default:
                break;
            }
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            switch(it->second)
            {
            case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
                detail.gsm.location_area_code = detail.lte.tracking_area_code;
                detail.gsm.mobile_network_code = detail.lte.mobile_network_code;
                detail.gsm.mobile_country_code = detail.lte.mobile_country_code;
                detail.gsm.strength.reset();
                detail.gsm.id.reset();
                break;
            case com::ubuntu::location::connectivity::RadioCell::Type::umts:
                detail.umts.location_area_code = detail.lte.tracking_area_code;
                detail.umts.mobile_network_code = detail.lte.mobile_network_code;
                detail.umts.mobile_country_code = detail.lte.mobile_country_code;
                detail.umts.strength.reset();
                detail.umts.id.reset();
                break;
            default:
                break;
            }
        default:
            // We take the default path here, specifically for cases where
            // we started out with Technology::unknown.
            break;
        };

        radio_type = it->second;

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::CellId::name())
    {
        did_cell_identity_change = true;

        auto value = variant.as<
                    org::Ofono::Manager::Modem::NetworkRegistration::CellId::ValueType
                >();

        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            detail.gsm.id.set(value);
            VLOG(1) << detail.gsm;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            detail.umts.id.set(value);
            VLOG(1) << detail.umts;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            detail.lte.id.set(value);
            VLOG(1) << detail.lte;
            break;
        default:
            break;
        };

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode::name())
    {
        did_cell_identity_change = true;

        auto value = variant.as<
                    org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode::ValueType
                >();
        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            detail.gsm.location_area_code.set(value);
            VLOG(1) << detail.gsm;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            detail.umts.location_area_code.set(value);
            VLOG(1) << detail.umts;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            detail.lte.tracking_area_code.set(value);
            VLOG(1) << detail.lte;
            break;
        default:
            break;
        };

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode::name())
    {
        did_cell_identity_change = true;

        std::stringstream ss
        {
            variant.as<
                org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode::ValueType
            >()
        };
        int value{-1}; ss >> value;

        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            detail.gsm.mobile_country_code.set(value);
            VLOG(1) << detail.gsm;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            detail.umts.mobile_country_code.set(value);
            VLOG(1) << detail.umts;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            detail.lte.mobile_country_code.set(value);
            VLOG(1) << detail.lte;
            break;
        default:
            break;
        };

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode::name())
    {
        did_cell_identity_change = true;

        std::stringstream ss
        {
            variant.as<
                org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode::ValueType
            >()
        };
        int value{-1}; ss >> value;

        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            detail.gsm.mobile_network_code.set(value);
            VLOG(1) << detail.gsm;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            detail.umts.mobile_network_code.set(value);
            VLOG(1) << detail.umts;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            detail.lte.mobile_network_code.set(value);
            VLOG(1) << detail.lte;
            break;
        default:
            break;
        };

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::Strength::name())
    {
        auto value = variant.as<
                    org::Ofono::Manager::Modem::NetworkRegistration::Strength::ValueType
                >();

        switch(radio_type)
        {
        case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
            detail.gsm.strength
                    = com::ubuntu::location::connectivity::RadioCell::Gsm::SignalStrength::from_percent(value/100.f);
            VLOG(1) << detail.gsm;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::umts:
            detail.umts.strength
                    = com::ubuntu::location::connectivity::RadioCell::Umts::SignalStrength::from_percent(value/100.f);
            VLOG(1) << detail.umts;
            break;
        case com::ubuntu::location::connectivity::RadioCell::Type::lte:
            detail.lte.strength
                    = com::ubuntu::location::connectivity::RadioCell::Lte::SignalStrength::from_percent(value/100.f);
            VLOG(1) << detail.lte;
            break;
        default:
            break;
        };

        if (cell_change_heuristics.valid.get())
            on_changed();
    }

    if (key == org::Ofono::Manager::Modem::NetworkRegistration::Status::name())
    {
        roaming =
                variant.as<org::Ofono::Manager::Modem::NetworkRegistration::Status::ValueType>() ==
                org::Ofono::Manager::Modem::NetworkRegistration::Status::roaming;
    }

    if (did_cell_identity_change)
        execute_cell_change_heuristics_if_appropriate();
}

void detail::CachedRadioCell::execute_cell_change_heuristics_if_appropriate()
{
    // Heuristics to ensure that
    // whenever the cell identity changes, we start a timer and invalidate the
    // cell to account for situations where the underlying modem firmware does not
    // report cell changes when on a 3G data connection.
    if (cell_change_heuristics.needed && // Only carry out this step if it is actually required
        (radio_type == com::ubuntu::location::connectivity::RadioCell::Type::umts ||
         also_apply_cell_change_heuristics_to_gsm_cells)) // and if it's a 3G cell.
    {
        static const boost::posix_time::seconds timeout{timeout_in_seconds()};

        std::lock_guard<std::mutex> lg(cell_change_heuristics.guard);

        cell_change_heuristics.invalidation_timer.expires_from_now(timeout);
        cell_change_heuristics.invalidation_timer.async_wait([this](boost::system::error_code ec)
        {
            if (not ec)
                cell_change_heuristics.valid = false;
        });

        cell_change_heuristics.valid = is_cell_details_valid();
    }
}

// Queries the status from the Ofono NetworkRegistration.
org::Ofono::Manager::Modem::NetworkRegistration::Status::Value detail::CachedRadioCell::query_status()
{
    auto status =
            modem.network_registration.get
            <
                org::Ofono::Manager::Modem::NetworkRegistration::Status
            >();

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::unregistered)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::unregistered;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::registered)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::registered;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::searching)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::searching;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::denied)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::denied;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::unknown)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::unknown;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::unregistered)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::unregistered;

    if (status == org::Ofono::Manager::Modem::NetworkRegistration::Status::roaming)
        return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::roaming;

    return org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::unknown;
}

// Queries the technology from the Ofono NetworkRegistration.
com::ubuntu::location::connectivity::RadioCell::Type detail::CachedRadioCell::query_technology()
{
    auto technology =
            modem.network_registration.get<
                org::Ofono::Manager::Modem::NetworkRegistration::Technology
            >();

    auto it = type_lut().find(technology);

    if (it == type_lut().end())
        return com::ubuntu::location::connectivity::RadioCell::Type::unknown;

    return it->second;
}

// Queries the cell id from the Ofono NetworkRegistration.
int detail::CachedRadioCell::query_cid()
{
    return modem.network_registration.get
    <
        org::Ofono::Manager::Modem::NetworkRegistration::CellId
    >();
}

// Queries the location area code from the Ofono NetworkRegistration.
std::uint16_t detail::CachedRadioCell::query_lac()
{
    return modem.network_registration.get
    <
        org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode
    >();
}

// Queries the mobile network code from the Ofono NetworkRegistration.
int detail::CachedRadioCell::query_mnc()
{
    std::stringstream ssmnc
    {
        modem.network_registration.get
        <
            org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode
        >()
    };
    int mnc{0}; ssmnc >> mnc;
    return mnc;
}

// Queries the mobile country code from the Ofono NetworkRegistration.
int detail::CachedRadioCell::query_mcc()
{
    std::stringstream ssmcc
    {
        modem.network_registration.get
        <
            org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode
        >()
    };
    int mcc{0}; ssmcc >> mcc;
    return mcc;
}

// Queries the signal strength from the Ofono NetworkRegistration.
std::int8_t detail::CachedRadioCell::query_strength()
{
    return modem.network_registration.get
    <
        org::Ofono::Manager::Modem::NetworkRegistration::Strength
    >();
}

// Returns true iff status is either roaming or registered.
bool detail::CachedRadioCell::is_registered_or_roaming(org::Ofono::Manager::Modem::NetworkRegistration::Status::Value status)
{
    return status == org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::registered ||
           status == org::Ofono::Manager::Modem::NetworkRegistration::Status::Value::roaming;
}

// Returns true iff all the parts of the cell are populated with valid values.
bool detail::CachedRadioCell::is_cell_details_valid()
{
    switch(radio_type)
    {
    case com::ubuntu::location::connectivity::RadioCell::Type::unknown:
        return false;
    case com::ubuntu::location::connectivity::RadioCell::Type::gsm:
        return is_gsm_valid();
    case com::ubuntu::location::connectivity::RadioCell::Type::umts:
        return is_umts_valid();
    case com::ubuntu::location::connectivity::RadioCell::Type::lte:
        return is_lte_valid();
    }

    return false;
}

// Retuns true iff the GSM cell details are valid.
bool detail::CachedRadioCell::is_gsm_valid()
{
   return gsm().mobile_country_code.is_valid() &&
          gsm().mobile_network_code.is_valid() &&
          gsm().location_area_code.is_valid() &&
          gsm().id.is_valid() &&
          gsm().strength.is_valid();
}

// Returns true iff the Umts cell details are valid.
bool detail::CachedRadioCell::is_umts_valid()
{
    return umts().mobile_country_code.is_valid() &&
           umts().mobile_network_code.is_valid() &&
           umts().location_area_code.is_valid() &&
           umts().id.is_valid() &&
           umts().strength.is_valid();
}

// Returns true iff the Lte cell details are valid.
bool detail::CachedRadioCell::is_lte_valid()
{
    return lte().mobile_country_code.is_valid() &&
           lte().mobile_network_code.is_valid() &&
           lte().tracking_area_code.is_valid() &&
           lte().id.is_valid() &&
           lte().physical_id.is_valid() &&
           lte().strength.is_valid();
}

detail::CachedRadioCell::Detail::Detail() : none(detail::CachedRadioCell::None{})
{
}

detail::CachedRadioCell::Detail::Detail(const com::ubuntu::location::connectivity::RadioCell::Gsm& gsm) : gsm(gsm)
{
}

detail::CachedRadioCell::Detail::Detail(const com::ubuntu::location::connectivity::RadioCell::Umts& umts) : umts(umts)
{
}

detail::CachedRadioCell::Detail::Detail(const com::ubuntu::location::connectivity::RadioCell::Lte& lte) : lte(lte)
{
}

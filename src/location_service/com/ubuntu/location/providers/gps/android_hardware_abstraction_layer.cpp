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

#include "android_hardware_abstraction_layer.h"

#if defined(COM_UBUNTU_LOCATION_SERVICE_HAVE_NET_CPP)
#include "net_cpp_gps_xtra_downloader.h"
#else
#include "null_gps_xtra_downloader.h"
#endif

#include <com/ubuntu/location/clock.h>
#include <com/ubuntu/location/configuration.h>
#include <com/ubuntu/location/logging.h>

#include <com/ubuntu/location/connectivity/manager.h>

#include <boost/property_tree/ini_parser.hpp>

#include <random>

namespace gps = com::ubuntu::location::providers::gps;
namespace android = com::ubuntu::location::providers::gps::android;

namespace location = com::ubuntu::location;

namespace
{
std::shared_ptr<android::GpsXtraDownloader> create_xtra_downloader()
{
#if defined(COM_UBUNTU_LOCATION_SERVICE_HAVE_NET_CPP)
    return std::make_shared<android::NetCppGpsXtraDownloader>();
#else
    return std::make_shared<android::NullGpsXtraDownloader>();
#endif // COM_UBUNTU_LOCATION_SERVICE_HAVE_NET_CPP
}
}

/** @brief Reads a configuration from a gps.conf file in INI format. */
android::GpsXtraDownloader::Configuration android::GpsXtraDownloader::Configuration::from_gps_conf_ini_file(std::istream& in)
{
    android::GpsXtraDownloader::Configuration result;

    location::Configuration config;
    boost::property_tree::read_ini(in, config);

    if (config.count("XTRA_SERVER_1") > 0)
        result.xtra_hosts.push_back(config.get<std::string>("XTRA_SERVER_1"));
    if (config.count("XTRA_SERVER_2") > 0)
        result.xtra_hosts.push_back(config.get<std::string>("XTRA_SERVER_2"));
    if (config.count("XTRA_SERVER_3") > 0)
        result.xtra_hosts.push_back(config.get<std::string>("XTRA_SERVER_3"));

    result.timeout = std::chrono::milliseconds{1500};

    return result;
}

android::HardwareAbstractionLayer::SuplAssistant::SuplAssistant(android::HardwareAbstractionLayer& hal) : impl(hal)
{
}

const core::Property<gps::HardwareAbstractionLayer::SuplAssistant::Status>& android::HardwareAbstractionLayer::SuplAssistant::status() const
{
    return impl.status;
}

core::Property<gps::HardwareAbstractionLayer::SuplAssistant::Status>& android::HardwareAbstractionLayer::SuplAssistant::status()
{
    return impl.status;
}

const core::Property<gps::HardwareAbstractionLayer::SuplAssistant::IpV4Address>& android::HardwareAbstractionLayer::SuplAssistant::server_ip() const
{
    return impl.server_ip;
}

core::Property<gps::HardwareAbstractionLayer::SuplAssistant::IpV4Address>& android::HardwareAbstractionLayer::SuplAssistant::server_ip()
{
    return impl.server_ip;
}

void android::HardwareAbstractionLayer::SuplAssistant::set_server(const std::string& host_name, std::uint16_t port)
{
    // TODO(tvoss): Reenable this once the platform api HAL changes land.
    u_hardware_gps_agps_set_server_for_type(
                impl.hal.impl.gps_handle,
                U_HARDWARE_GPS_AGPS_TYPE_SUPL,
                host_name.c_str(),
                port);
}

void android::HardwareAbstractionLayer::SuplAssistant::notify_data_connection_open_via_apn(const std::string& name)
{
    // TODO(tvoss): Reenable this once the platform api HAL changes land.
    u_hardware_gps_agps_notify_connection_is_open(
                impl.hal.impl.gps_handle,
                name.c_str());
}

void android::HardwareAbstractionLayer::SuplAssistant::notify_data_connection_closed()
{
    // TODO(tvoss): Reenable this once the platform api HAL changes land.
    u_hardware_gps_agps_notify_connection_is_closed(
                impl.hal.impl.gps_handle);
}

void android::HardwareAbstractionLayer::SuplAssistant::notify_data_connection_not_available()
{
    // TODO(tvoss): Reenable this once the platform api HAL changes land.
    u_hardware_gps_agps_notify_connection_not_available(
                impl.hal.impl.gps_handle);
}

android::HardwareAbstractionLayer::SuplAssistant::Impl::Impl(android::HardwareAbstractionLayer& hal) : hal(hal)
{
}

void android::HardwareAbstractionLayer::on_nmea_update(int64_t timestamp, const char *nmea, int length, void *context)
{
    VLOG(200) << __PRETTY_FUNCTION__ << ": "
             << "timestamp=" << timestamp << " "
             << "nmea=" << nmea << " "
             << "length=" << length << " "
             << "context=" << context;
}

void android::HardwareAbstractionLayer::on_xtra_download_request(void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": "
            << "context=" << context;

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);

    try
    {
        auto xtra_gps_data = thiz->impl.gps_xtra_downloader->download_xtra_data(thiz->impl.gps_xtra_configuration);
        if (not xtra_gps_data.empty())
            u_hardware_gps_inject_xtra_data(thiz->impl.gps_handle, &xtra_gps_data.front(), xtra_gps_data.size());
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << "Error downloading GPS Xtra data: " << e.what();
    } catch(...)
    {
        SYSLOG(ERROR) << "Error downloading GPS Xtra data.";
    }

}

void android::HardwareAbstractionLayer::on_gps_ni_notify(UHardwareGpsNiNotification* notification, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": "
            << "notification=" << notification << " "
            << "context=" << context;
}

void android::HardwareAbstractionLayer::on_ril_request_set_id(uint32_t flags, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": "
            << "flags=" << flags << " "
            << "context=" << context;
}

void android::HardwareAbstractionLayer::on_ril_request_reference_location(uint32_t flags, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": "
            << "flags=" << flags << " "
            << "context=" << context;
}

void android::HardwareAbstractionLayer::on_location_update(UHardwareGpsLocation* location, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);

    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_LAT_LONG)
    {
        location::Position pos
        {
            location::wgs84::Latitude{location->latitude * location::units::Degrees},
            location::wgs84::Longitude{location->longitude * location::units::Degrees}
        };

        if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_ACCURACY)
            pos.accuracy.horizontal = location->accuracy * location::units::Meters;

        if(location->flags & U_HARDWARE_GPS_LOCATION_HAS_ALTITUDE)
            pos.altitude = location::wgs84::Altitude{location->altitude * location::units::Meters};

        // The Android HAL does not provide us with accuracy information for
        // altitude measurements. We just leave out that field.

        thiz->position_updates()(pos);

        VLOG(1) << pos;
    }

    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_SPEED)
    {
        location::Velocity v{location->speed * location::units::MetersPerSecond};
        thiz->velocity_updates()(v);
        VLOG(1) << v;
    }

    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_BEARING)
    {
        location::Heading h{location->bearing * location::units::Degrees};
        thiz->heading_updates()(h);
        VLOG(1) << h;
    }
}

void android::HardwareAbstractionLayer::on_status_update(uint16_t status, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": status=" << status << ", context=" << context;
    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);
    thiz->chipset_status() = static_cast<gps::ChipsetStatus>(status);
}

void android::HardwareAbstractionLayer::on_sv_status_update(UHardwareGpsSvStatus* sv_info, void* context)
{
    VLOG(20) << __PRETTY_FUNCTION__ << ": "
             << "count: " << sv_info->num_svs << " "
             << "a: " << sv_info->almanac_mask << " "
             << "e: " << sv_info->ephemeris_mask << " "
             << "u: " << sv_info->used_in_fix_mask << " ";

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);

    std::set<location::SpaceVehicle> svs;

    for (int i = 0; i < sv_info->num_svs; i++)
    {
        location::SpaceVehicle sv;

        // PRN is in the range of [1, 32], adjusting it to make sure we
        // can use it for bitfield flag testing operations.
        int prn = sv_info->sv_list[i].prn - 1;
        if (prn < 0)
            continue;

        auto shift = (1 << prn);

        sv.key.type = location::SpaceVehicle::Type::gps;
        sv.key.id = sv_info->sv_list[i].prn;
        sv.snr = sv_info->sv_list[i].snr;
        sv.has_almanac_data = sv_info->almanac_mask & shift;
        sv.has_ephimeris_data = sv_info->ephemeris_mask & shift;
        sv.used_in_fix = sv_info->used_in_fix_mask & shift;
        sv.azimuth = sv_info->sv_list[i].elevation * location::units::Degrees;
        sv.elevation = sv_info->sv_list[i].azimuth * location::units::Degrees;

        svs.insert(sv);
    }

    thiz->space_vehicle_updates()(svs);
}

void android::HardwareAbstractionLayer::on_set_capabilities(uint32_t capabilities, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);
    thiz->capabilities() = capabilities;
}

void android::HardwareAbstractionLayer::on_agps_status_update(UHardwareGpsAGpsStatus* status, void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    if (status->type == U_HARDWARE_GPS_AGPS_TYPE_C2K)
    {
        VLOG(1) << "\t We only support SUPL at this point in time.";
        return;
    }

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);
    thiz->impl.supl_assistant.status() = static_cast<gps::HardwareAbstractionLayer::SuplAssistant::Status>(status->status);
    thiz->impl.supl_assistant.server_ip() = gps::HardwareAbstractionLayer::SuplAssistant::IpV4Address{status->ipaddr};

    VLOG(1) << int(thiz->impl.supl_assistant.status().get()) << ", "
            << status->ipaddr << ", "
            << (int)thiz->impl.supl_assistant.server_ip().get().triplets[0] << "."
            << (int)thiz->impl.supl_assistant.server_ip().get().triplets[1] << "."
            << (int)thiz->impl.supl_assistant.server_ip().get().triplets[2] << "."
            << (int)thiz->impl.supl_assistant.server_ip().get().triplets[3];

    switch (status->status)
    {
    case U_HARDWARE_GPS_REQUEST_AGPS_DATA_CONN:
        VLOG(1) << "U_HARDWARE_GPS_REQUEST_AGPS_DATA_CONN";
        break;
    case U_HARDWARE_GPS_RELEASE_AGPS_DATA_CONN:
        VLOG(1) << "U_HARDWARE_GPS_RELEASE_AGPS_DATA_CONN";
        break;
    case U_HARDWARE_GPS_AGPS_DATA_CONNECTED:
        VLOG(1) << "U_HARDWARE_GPS_AGPS_DATA_CONNECTED";
        break;
    case U_HARDWARE_GPS_AGPS_DATA_CONN_DONE:
        VLOG(1) << "U_HARDWARE_GPS_AGPS_DATA_CONN_DONE";
        break;
    case U_HARDWARE_GPS_AGPS_DATA_CONN_FAILED:
        VLOG(1) << "U_HARDWARE_GPS_AGPS_DATA_CONN_FAILED";
        break;
    }
}

void android::HardwareAbstractionLayer::on_request_utc_time(void* context)
{
    VLOG(1) << __PRETTY_FUNCTION__;

    auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);

    if (thiz->impl.utc_time_request_handler)
    {
        thiz->impl.utc_time_request_handler();
    } else
    {
        auto now = location::Clock::now().time_since_epoch();
        auto thiz = static_cast<android::HardwareAbstractionLayer*>(context);

        static const int zero_uncertainty = 0;

        u_hardware_gps_inject_time(
                    thiz->impl.gps_handle,
                    now.count(),
                    now.count(),
                    zero_uncertainty);
    }
}

android::HardwareAbstractionLayer::HardwareAbstractionLayer(const android::HardwareAbstractionLayer::Configuration& config)
    : impl(this, config)
{
}

std::uint32_t android::HardwareAbstractionLayer::capabilities() const
{
    return impl.capabilities;
}

std::uint32_t& android::HardwareAbstractionLayer::capabilities()
{
    return impl.capabilities;
}

// From gps::HardwareAbstractionLayer
gps::HardwareAbstractionLayer::SuplAssistant& android::HardwareAbstractionLayer::supl_assistant()
{
    return impl.supl_assistant;
}

const core::Signal<location::Position>& android::HardwareAbstractionLayer::position_updates() const
{
    return impl.position_updates;
}

core::Signal<location::Position>& android::HardwareAbstractionLayer::position_updates()
{
    return impl.position_updates;
}

const core::Signal<location::Heading>& android::HardwareAbstractionLayer::heading_updates() const
{
    return impl.heading_updates;
}

core::Signal<location::Heading>& android::HardwareAbstractionLayer::heading_updates()
{
    return impl.heading_updates;
}

const core::Signal<location::Velocity>& android::HardwareAbstractionLayer::velocity_updates() const
{
    return impl.velocity_updates;
}

core::Signal<location::Velocity>& android::HardwareAbstractionLayer::velocity_updates()
{
    return impl.velocity_updates;
}

const core::Signal<std::set<location::SpaceVehicle> >& android::HardwareAbstractionLayer::space_vehicle_updates() const
{
    return impl.space_vehicle_updates;
}

core::Signal<std::set<location::SpaceVehicle> >& android::HardwareAbstractionLayer::space_vehicle_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    return impl.space_vehicle_updates;
}

void android::HardwareAbstractionLayer::delete_all_aiding_data()
{
    // TODO(tvoss): We should expose this flag in gps.h.
    static const std::uint16_t delete_all = 0xFFFF;
    u_hardware_gps_delete_aiding_data(impl.gps_handle, delete_all);
}

const core::Property<gps::ChipsetStatus>& android::HardwareAbstractionLayer::chipset_status() const
{
    return impl.chipset_status;
}

core::Property<gps::ChipsetStatus>& android::HardwareAbstractionLayer::chipset_status()
{
    return impl.chipset_status;
}

bool android::HardwareAbstractionLayer::is_capable_of(gps::AssistanceMode mode) const
{
    bool result = false;

    switch(mode)
    {
    case gps::AssistanceMode::standalone:
        result = true;
        break;
    case gps::AssistanceMode::mobile_station_assisted:
        result = capabilities() & U_HARDWARE_GPS_CAPABILITY_MSA;
        break;
    case gps::AssistanceMode::mobile_station_based:
        result = capabilities() & U_HARDWARE_GPS_CAPABILITY_MSB;
        break;
    }

    return result;
}

bool android::HardwareAbstractionLayer::is_capable_of(gps::PositionMode mode) const
{
    bool result = false;
    switch(mode)
    {
    case gps::PositionMode::single_shot:
        result = capabilities() & U_HARDWARE_GPS_CAPABILITY_SINGLE_SHOT;
        break;
    case gps::PositionMode::periodic:
        result = capabilities() & U_HARDWARE_GPS_CAPABILITY_SCHEDULING;
        break;
    }

    return result;
}

bool android::HardwareAbstractionLayer::is_capable_of(gps::Capability capability) const
{
    bool result = false;

    switch(capability)
    {
    case gps::Capability::on_demand_time_injection:
        result = capabilities() & U_HARDWARE_GPS_CAPABILITY_ON_DEMAND_TIME;
        break;
    default:
        break;
    }

    return result;
}

bool android::HardwareAbstractionLayer::start_positioning()
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": " << this << ", " << impl.gps_handle;
    return u_hardware_gps_start(impl.gps_handle);
}

bool android::HardwareAbstractionLayer::stop_positioning()
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": " << this << ", " << impl.gps_handle;
    return u_hardware_gps_stop(impl.gps_handle);
}

bool android::HardwareAbstractionLayer::set_assistance_mode(gps::AssistanceMode mode)
{
    if (!is_capable_of(mode))
        return false;

    impl.assistance_mode = mode;
    return impl.dispatch_updated_modes_to_driver();
}

bool android::HardwareAbstractionLayer::set_position_mode(gps::PositionMode mode)
{
    switch(mode)
    {
    case gps::PositionMode::single_shot:
        if (!(capabilities() & U_HARDWARE_GPS_CAPABILITY_SINGLE_SHOT))
            return false;
        break;
    case gps::PositionMode::periodic:
        if (!(capabilities() & U_HARDWARE_GPS_CAPABILITY_SCHEDULING))
            return false;
        break;
    }

    impl.position_mode = mode;
    return impl.dispatch_updated_modes_to_driver();
}

bool android::HardwareAbstractionLayer::inject_reference_position(const location::Position& position)
{
    // TODO(tvoss): We should expose the int return type of the underyling
    //  Android HAL to capture errors here.
    UHardwareGpsLocation loc;
    loc.size = sizeof(loc);
    loc.flags = U_HARDWARE_GPS_LOCATION_HAS_LAT_LONG;
    loc.latitude = position.latitude.value.value();
    loc.longitude = position.longitude.value.value();

    if (position.accuracy.horizontal)
    {
        loc.flags |= U_HARDWARE_GPS_LOCATION_HAS_ACCURACY;
        loc.accuracy = (*position.accuracy.horizontal).value();
    }

    u_hardware_gps_inject_location(impl.gps_handle, loc);

    return true;
}

bool android::HardwareAbstractionLayer::inject_reference_time(
        const std::chrono::microseconds& reference_time,
        const std::chrono::microseconds& sample_time)
{
    if (!is_capable_of(gps::Capability::on_demand_time_injection))
        return false;

    // TODO(tvoss): We should expose the int return type of the underyling
    //  Android HAL to capture errors here.
    u_hardware_gps_inject_time(impl.gps_handle,
                               reference_time.count(),
                               sample_time.count(),
                               10);
    return true;
}

android::HardwareAbstractionLayer::Impl::Impl(
        android::HardwareAbstractionLayer* parent,
        const android::HardwareAbstractionLayer::Configuration& configuration)
    : capabilities(0),
      assistance_mode(gps::AssistanceMode::mobile_station_based),
      position_mode(gps::PositionMode::periodic),
      supl_assistant(*parent),
      gps_xtra_configuration(configuration.gps_xtra.configuration),
      gps_xtra_downloader(configuration.gps_xtra.downloader)
{
    ::memset(&gps_params, 0, sizeof(gps_params));

    gps_params.location_cb = HardwareAbstractionLayer::on_location_update;
    gps_params.status_cb = HardwareAbstractionLayer::on_status_update;
    gps_params.sv_status_cb = HardwareAbstractionLayer::on_sv_status_update;
    gps_params.set_capabilities_cb = HardwareAbstractionLayer::on_set_capabilities;
    gps_params.request_utc_time_cb = HardwareAbstractionLayer::on_request_utc_time;
    gps_params.agps_status_cb = HardwareAbstractionLayer::on_agps_status_update;

    gps_params.nmea_cb = HardwareAbstractionLayer::on_nmea_update;
    gps_params.xtra_download_request_cb = HardwareAbstractionLayer::on_xtra_download_request;
    gps_params.gps_ni_notify_cb = HardwareAbstractionLayer::on_gps_ni_notify;
    gps_params.request_setid_cb = HardwareAbstractionLayer::on_ril_request_set_id;
    gps_params.request_refloc_cb = HardwareAbstractionLayer::on_ril_request_reference_location;

    gps_params.context = parent;

    gps_handle = u_hardware_gps_new(std::addressof(gps_params));

    dispatch_updated_modes_to_driver();
}

bool android::HardwareAbstractionLayer::Impl::dispatch_updated_modes_to_driver()
{
    static const std::map<gps::AssistanceMode, std::uint32_t> assistance_mode_lut =
    {
        {gps::AssistanceMode::mobile_station_assisted, U_HARDWARE_GPS_POSITION_MODE_MS_ASSISTED},
        {gps::AssistanceMode::mobile_station_based, U_HARDWARE_GPS_POSITION_MODE_MS_BASED},
        {gps::AssistanceMode::standalone, U_HARDWARE_GPS_POSITION_MODE_STANDALONE}
    };

    static const std::map<gps::PositionMode, std::uint32_t> position_mode_lut =
    {
        {gps::PositionMode::periodic, U_HARDWARE_GPS_POSITION_RECURRENCE_PERIODIC},
        {gps::PositionMode::single_shot, U_HARDWARE_GPS_POSITION_RECURRENCE_SINGLE}
    };

    auto am = assistance_mode_lut.at(assistance_mode);
    auto pm = position_mode_lut.at(position_mode);

    static const uint32_t preferred_accuracy_in_meters = 0;
    static const uint32_t preferred_ttff_in_ms = 0;
    static const std::chrono::milliseconds min_interval{500};

    return u_hardware_gps_set_position_mode(
                gps_handle,
                am,
                pm,
                min_interval.count(),
                preferred_accuracy_in_meters,
                preferred_ttff_in_ms);
}

namespace
{
android::GpsXtraDownloader::Configuration gps_xtra_downloader_configuration(std::istream& in)
{
    android::GpsXtraDownloader::Configuration config;

    try
    {
        config = android::GpsXtraDownloader::Configuration::from_gps_conf_ini_file(in);
        return config;
    } catch(const std::exception& e)
    {
        // TODO(tvoss): We should log the error here
        // For now, we silently fallback to a set of default options.
    } catch(...)
    {
        // For now, we silently fallback to a set of default options.
    }

    config.xtra_hosts =
    {
        "http://xtra1.gpsonextra.net/xtra2.bin",
        "http://xtra2.gpsonextra.net/xtra2.bin",
        "http://xtra3.gpsonextra.net/xtra2.bin"
    };

    return config;
}
}

std::shared_ptr<gps::HardwareAbstractionLayer> gps::HardwareAbstractionLayer::create_default_instance()
{
    // We prefer /system/etc/gps.conf as it contains device-specific
    // configuration options. However, if that file is not present, we
    // fallback to /etc/gps.conf which contains a vanilla, non-harmful
    // configuration.
    std::ifstream in_system_gps_conf
    {
        "/system/etc/gps.conf"
    };

    std::ifstream in_gps_conf
    {
        "/etc/gps.conf"
    };

    static android::HardwareAbstractionLayer::Configuration config
    {
        {
            create_xtra_downloader(),
            gps_xtra_downloader_configuration((in_system_gps_conf ? in_system_gps_conf : in_gps_conf))
        }
    };

    static std::shared_ptr<gps::HardwareAbstractionLayer> instance
    {
        new android::HardwareAbstractionLayer
        {
            config
        }
    };

    return instance;
}

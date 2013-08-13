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
#include "com/ubuntu/location/providers/gps/provider.h"

#include "com/ubuntu/location/logging.h"

#include <ubuntu/hardware/gps.h>

namespace cul = com::ubuntu::location;
namespace culg = com::ubuntu::location::providers::gps;

namespace
{
static const std::map<uint16_t, std::string> status_lut = 
{
    {U_HARDWARE_GPS_STATUS_NONE, "U_HARDWARE_GPS_STATUS_NONE"},
    {U_HARDWARE_GPS_STATUS_SESSION_BEGIN, "U_HARDWARE_GPS_STATUS_SESSION_BEGIN"},
    {U_HARDWARE_GPS_STATUS_SESSION_END, "U_HARDWARE_GPS_STATUS_SESSION_END"},
    {U_HARDWARE_GPS_STATUS_ENGINE_ON, "U_HARDWARE_GPS_STATUS_ENGINE_ON"},
    {U_HARDWARE_GPS_STATUS_ENGINE_OFF, "U_HARDWARE_GPS_STATUS_ENGINE_OFF"}
};
}

struct culg::Provider::Private
{

static void on_location_update(UHardwareGpsLocation* location, void* context)
{
    auto thiz = static_cast<culg::Provider*>(context);
        
    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_LAT_LONG)
    {
        VLOG(1) << "location->flags & U_HARDWARE_GPS_LOCATION_HAS_LAT_LONG";

        cul::Position pos;
        pos.latitude(cul::wgs84::Latitude{location->latitude * cul::units::Degrees});
        pos.longitude(cul::wgs84::Longitude{location->longitude * cul::units::Degrees});
        if(location->flags & U_HARDWARE_GPS_LOCATION_HAS_ALTITUDE)
            pos.altitude(cul::wgs84::Altitude{location->altitude * cul::units::Meters});
        
        thiz->deliver_position_updates(cul::Update<cul::Position>{pos, cul::Clock::now()});
    }
    
    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_SPEED)
    {
        VLOG(1) << "location->flags & U_HARDWARE_GPS_LOCATION_HAS_SPEED";
        
        cul::Velocity v{location->speed * cul::units::MetersPerSecond};
        thiz->deliver_velocity_updates(cul::Update<cul::Velocity>{v, cul::Clock::now()});
    }

    if (location->flags & U_HARDWARE_GPS_LOCATION_HAS_BEARING)
    {
        VLOG(1) << "location->flags & U_HARDWARE_GPS_LOCATION_HAS_BEARING";
        
        cul::Heading h{location->bearing * cul::units::Degrees};
        thiz->deliver_heading_updates(cul::Update<cul::Heading>{h, cul::Clock::now()});
    }
}

static void on_status_update(uint16_t status, void* /*context*/)
{
    VLOG(1) << "Status = " << status_lut.at(status);
}

static void on_sv_status_update(UHardwareGpsSvStatus* sv_info, void* /*context*/)
{
    VLOG(1) << "SV status update: [#svs: " << sv_info->num_svs << "]";
}
    
static void on_nmea_update(int64_t /*timestamp*/, const char* /*nmea*/, int /*length*/, void* /*context*/)
{
}

static void on_set_capabilities(uint32_t capabilities, void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": " << capabilities;
}

static void on_request_utc_time(void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__;
}

static void on_agps_status_update(UHardwareGpsAGpsStatus* /*status*/, void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__;
}

static void on_gps_ni_notification(UHardwareGpsNiNotification* /*notification*/, void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__;
}
    
static void on_agps_ril_request_set_id(uint32_t /*flags*/, void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__;
}

static void on_agps_ril_request_ref_lock(uint32_t /*flags*/, void* /*context*/)
{
    VLOG(1) << __PRETTY_FUNCTION__;
}

    static void on_gps_xtra_download_request(void*)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
    }

    void start() 
    { 
        u_hardware_gps_start(gps_handle);
    }
    
    void stop() 
    { 
        u_hardware_gps_stop(gps_handle);
    }

    UHardwareGpsParams gps_params;
    UHardwareGps gps_handle;
};



std::string culg::Provider::class_name()
{
    return "gps::Provider";
}

cul::Provider::Ptr culg::Provider::create_instance(const cul::ProviderFactory::Configuration&)
{
    return cul::Provider::Ptr{new culg::Provider{}};
}

const cul::Provider::FeatureFlags& culg::Provider::default_feature_flags()
{
    static const cul::Provider::FeatureFlags flags{"001"};
    return flags;
}

const cul::Provider::RequirementFlags& culg::Provider::default_requirement_flags()
{
    static const cul::Provider::RequirementFlags flags{"1010"};
    return flags;
}

culg::Provider::Provider()
        : cul::Provider(
              culg::Provider::default_feature_flags(), 
              culg::Provider::default_requirement_flags()),
          d(new Private())
{
    d->gps_params.location_cb = culg::Provider::Private::on_location_update;
    d->gps_params.status_cb = culg::Provider::Private::on_status_update;
    d->gps_params.sv_status_cb = culg::Provider::Private::on_sv_status_update;
    d->gps_params.nmea_cb = culg::Provider::Private::on_nmea_update;
    d->gps_params.set_capabilities_cb = culg::Provider::Private::on_set_capabilities;
    d->gps_params.request_utc_time_cb = culg::Provider::Private::on_request_utc_time;
    d->gps_params.xtra_download_request_cb = culg::Provider::Private::on_gps_xtra_download_request;
    
    d->gps_params.agps_status_cb = culg::Provider::Private::on_agps_status_update;
    
    d->gps_params.gps_ni_notify_cb = culg::Provider::Private::on_gps_ni_notification;
    
    d->gps_params.request_setid_cb = culg::Provider::Private::on_agps_ril_request_set_id;
    d->gps_params.request_refloc_cb = culg::Provider::Private::on_agps_ril_request_ref_lock;
    d->gps_params.context = this;
    
    d->gps_handle = u_hardware_gps_new(std::addressof(d->gps_params));
}

culg::Provider::~Provider() noexcept
{
    d->stop();
    u_hardware_gps_delete(d->gps_handle);
}

bool culg::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culg::Provider::start_position_updates()
{
    d->start();
}

void culg::Provider::stop_position_updates()
{
    d->stop();
}

void culg::Provider::start_velocity_updates()
{
    d->start();
}

void culg::Provider::stop_velocity_updates()
{
    d->stop();
}    

void culg::Provider::start_heading_updates()
{
    d->start();
}

void culg::Provider::stop_heading_updates()
{
    d->stop();
}

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
#ifndef LOCATION_PROVIDERS_GPS_ANDROID_HARDWARE_ABSTRACTION_LAYER_H_
#define LOCATION_PROVIDERS_GPS_ANDROID_HARDWARE_ABSTRACTION_LAYER_H_

#include <location/providers/gps/hardware_abstraction_layer.h>
#include <location/providers/gps/sntp_client.h>

#include <ubuntu/hardware/gps.h>

namespace location { namespace providers { namespace gps
{
namespace android
{

/** @brief Models a downloader for GPS xtra data from HTTP sources. */
struct GpsXtraDownloader
{
    static constexpr const char* x_wap_profile_key
    {
        "x-wap-profile"
    };

    static constexpr const char* x_wap_profile_value
    {
        "http://www.openmobilealliance.org/tech/profiles/UAPROF/ccppschema-20021212#"
    };

    /** @brief Configuration options specific to a GpsXtraDownloader implementation. */
    struct Configuration
    {
        /** @brief Reads a configuration from a gps.conf file in INI format. */
        static Configuration from_gps_conf_ini_file(std::istream& in);

        /** @brief Timeout on gps xtra download operations. */
        std::chrono::milliseconds timeout
        {
            30000
        };

        /** Set of hosts serving GPS xtra data. */
        std::vector<std::string> xtra_hosts
        {
        };
    };

    GpsXtraDownloader() = default;
    virtual ~GpsXtraDownloader() = default;

    /** brief Downloads a GPS xtra data package from one of the servers given in config. */
    virtual std::vector<char> download_xtra_data(const Configuration& config) = 0;
};

struct HardwareAbstractionLayer : public gps::HardwareAbstractionLayer
{    
    /** @brief A ReferenceTimeSource provides samples of an arbitrary reference time source. */
    class ReferenceTimeSource
    {
    public:
        /** @cond */
        typedef std::shared_ptr<ReferenceTimeSource> Ptr;

        virtual ~ReferenceTimeSource() = default;
        /** @endcond */

        /** @brief sample returns a sample of the current ReferenceTimeSource. */
        virtual ReferenceTimeSample sample() = 0;

    protected:
        ReferenceTimeSource() = default;
    };

    /** @brief Implements gps::HardwareAbstractionLayer::SuplAssistant interface for the android gps HAL. */
    struct SuplAssistant : public gps::HardwareAbstractionLayer::SuplAssistant
    {
        /** @brief Bootstraps access to the Android GPS HAL assistance features. */
        SuplAssistant(android::HardwareAbstractionLayer& hal);
        /** @brief Getable/observable access to the status of the assistant instance. */
        const core::Property<Status>& status() const override;
        /** @brief Getable/setable/observable access to the status of the assistant instance. */
        core::Property<Status>& status();
        /** @brief Getable/observable access to the ip the assistance instance uses. */
        const core::Property<IpV4Address>& server_ip() const override;
        /** @brief Getable/setable/observable access to the ip the assistance instance uses. */
        core::Property<IpV4Address>& server_ip();
        /** @brief Informs the assistant instance about the SUPL server it should talk to. */
        void set_server(const std::string& host_name, std::uint16_t port) override;
        /** @brief Notifies the assistant instance that a data connection is open now. */
        void notify_data_connection_open_via_apn(const std::string& name) override;
        /** @brief Notifies the assistant instance the the data connection is closed now. */
        void notify_data_connection_closed() override;
        /** @brief Notifies the assistant instance that a data connection is not available right now. */
        void notify_data_connection_not_available() override;

        struct Impl
        {
            Impl(android::HardwareAbstractionLayer& hal);

            // The parent HardwareAbstractionLayer instance.
            android::HardwareAbstractionLayer& hal;
            // The status of the assisted GPS HAL.
            core::Property<gps::HardwareAbstractionLayer::SuplAssistant::Status> status;
            // The SUPL server address as reported by the Android GPS HAL.
            core::Property<gps::HardwareAbstractionLayer::SuplAssistant::IpV4Address> server_ip;
        } impl;
    };

    /**
     * @brief Called whenever new NMEA data is available from the chipset.
     * @param [in] timestamp Indicates the time when the NMEA sentence was generated in the chipset.
     * @param [in] nmea NMEA sentence.
     * @param [in] length Length of the NMEA sentence.
     * @param [in] context The app-specific callback context.
     */
    static void on_nmea_update(int64_t timestamp, const char *nmea, int length, void *context);

    /**
     * @brief Called whenever the chipset requests GPS xtra data.
     * @param context The app-specific callback context.
     */
    static void on_xtra_download_request(void* context);

    /**
     * @brief Called whenever a new network-initiated message is available.
     * @param [in] notification The notification object.
     * @param [in] context The app-specific callback context.
     */
    static void on_gps_ni_notify(UHardwareGpsNiNotification* notification, void* context);

    /**
     * @brief Called by the chipset for RIL-specific ID generation.
     * @param [in] flags The flags for the request
     * @param [in] context The app-specific callback context.
     */
    static void on_ril_request_set_id(uint32_t flags, void* context);

    /**
     * @brief Called whenever the GPS chipsets requires cell information from RIL.
     * @param [in] flags The flags for the request.
     * @param [in] context The app-specific callback context.
     */
    static void on_ril_request_reference_location(uint32_t flags, void* context);

    /**
     * @brief Called whenever the GPS chipsets has got a location update available.
     * @param [in] location The new location.
     * @param [in] context The app-specific callback context.
     */
    static void on_location_update(UHardwareGpsLocation* location, void* context);

    /**
     * @brief Called whenever the GPS chipset status changes.
     * @param [in] status The new status of the GPS chipset.
     * @param [in] context The app-specific callback context.
     */
    static void on_status_update(uint16_t status, void* context);

    /**
     * @brief Called whenever satellite visibility changes.
     * @param [in] sv_info The new satellite visibility info.
     * @param [in] context The app-specific callback context.
     */
    static void on_sv_status_update(UHardwareGpsSvStatus* sv_info, void* context);

    /**
     * @brief Called to report the GPS chipset's capabilities
     * @param [in] capabilities Bitfield of capability flags.
     * @param [in] context The app-specific callback context.
     */
    static void on_set_capabilities(uint32_t capabilities, void* context);

    /**
     * @brief Called to report assisted gps status updates.
     * @param [in] status The new assisted gps status.
     * @param [in] context The app-specific callback context.
     */
    static void on_agps_status_update(UHardwareGpsAGpsStatus* status, void* context);

    /**
     * @brief Called whenever the GPS chipsets requires a reference time.
     * @param [in] context The app-specific callback context.
     */
    static void on_request_utc_time(void* context);

    struct Configuration
    {
        struct
        {
            std::shared_ptr<GpsXtraDownloader> downloader;
            GpsXtraDownloader::Configuration configuration;
        } gps_xtra;

        ReferenceTimeSource::Ptr reference_time_source;
    };

    HardwareAbstractionLayer(const Configuration& configuration);

    /** @brief Reports the raw android capabilities. */
    std::uint32_t capabilities() const;

    /** @brief Reports the raw android capabilities. */
    std::uint32_t& capabilities();

    // From gps::HardwareAbstractionLayer
    gps::HardwareAbstractionLayer::SuplAssistant& supl_assistant() override;

    const core::Signal<location::Position>& position_updates() const override;
    core::Signal<location::Position>& position_updates();

    const core::Signal<location::units::Degrees>& heading_updates() const override;
    core::Signal<location::units::Degrees>& heading_updates();

    const core::Signal<location::units::MetersPerSecond>& velocity_updates() const override;
    core::Signal<location::units::MetersPerSecond>& velocity_updates();

    const core::Signal<std::set<location::SpaceVehicle> >& space_vehicle_updates() const override;
    core::Signal<std::set<location::SpaceVehicle> >& space_vehicle_updates();

    void delete_all_aiding_data() override;

    const core::Property<gps::ChipsetStatus>& chipset_status() const override;
    core::Property<gps::ChipsetStatus>& chipset_status();

    bool is_capable_of(gps::AssistanceMode mode) const override;
    bool is_capable_of(gps::PositionMode mode) const override;
    bool is_capable_of(gps::Capability capability) const override;

    bool start_positioning() override;
    bool stop_positioning() override;

    bool set_assistance_mode(gps::AssistanceMode mode) override;
    bool set_position_mode(gps::PositionMode mode) override;

    bool inject_reference_position(const location::Position& position) override;
    bool inject_reference_time(const ReferenceTimeSample& sample) override;

    struct Impl
    {
        // Bootstraps access to the GPS chipset, wiring up all callbacks.
        Impl(android::HardwareAbstractionLayer* parent, const android::HardwareAbstractionLayer::Configuration& configuration);

        // Adjusts the assistance and positioning mode in one go, returning false in case of issues.
        bool dispatch_updated_modes_to_driver();

        // The parameter bundle for the hardware GPS instance.
        UHardwareGpsParams gps_params;
        // The actual handle to the hardware GPS instance.
        UHardwareGps gps_handle;

        // Capabilities bitfield for the hardware GPS instance.
        std::uint32_t capabilities;
        // The current assistance mode.
        gps::AssistanceMode assistance_mode;
        // The current position mode.
        gps::PositionMode position_mode;

        // An implementation of the gps::HardwareAbstractionLayer::SuplAssistant interface.
        SuplAssistant supl_assistant;

        // An implementation of ReferenceTimeSource.
        ReferenceTimeSource::Ptr reference_time_source;

        // Emitted whenever the set of visible space vehicles changes.
        core::Signal<std::set<location::SpaceVehicle>> space_vehicle_updates;
        // Emitted whenever the position as reported by the GPS chipset changes.
        core::Signal<location::Position> position_updates;
        // Emitted whenever the heading as reported by the GPS chipset changes.
        core::Signal<location::units::Degrees> heading_updates;
        // Emitted whenever the velocity as reported by the GPS chipset changes.
        core::Signal<location::units::MetersPerSecond> velocity_updates;
        // Emitted whenever the chipset status changes.
        core::Property<gps::ChipsetStatus> chipset_status;

        // GPS Xtra configuration.
        GpsXtraDownloader::Configuration gps_xtra_configuration;
        // GPS xtra downloader implementation.
        std::shared_ptr<GpsXtraDownloader> gps_xtra_downloader;
    } impl;
};
}
}}}

#endif // LOCATION_PROVIDERS_GPS_ANDROID_HARDWARE_ABSTRACTION_LAYER_H_

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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_HARDWARE_ABSTRACTION_LAYER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_HARDWARE_ABSTRACTION_LAYER_H_

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/velocity.h>

#include <core/property.h>

#include <chrono>
#include <memory>
#include <cstdint>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace gps
{
/**
 * @brief The ChipsetStatus enum summarizes the gps chipset status.
 */
enum class ChipsetStatus
{
    /** Status is unknown. */
    unknown = 0,
    /** Chipset has begun navigating. */
    session_begin = 1,
    /** Chipset has stopped navigating. */
    session_end = 2,
    /** Chipset is powered on. */
    engine_on = 3,
    /** Chipset is powered off. */
    engine_off = 4
};

/**
 * @brief The Capabilities enum summarizes chipset capabilities.
 */
enum class Capability
{
    /** The GPS chipset supports on-demand time injection. */
    on_demand_time_injection
};

/**
 * @brief The AssistanceMode enum summarizes the different assisted gps modes that
 * the provider supports.
 */
enum class AssistanceMode
{
    /** Provider runs on its own without any external assistance. */
    standalone,

    /** In MSB mode A-GPS operation, the A-GPS device receives ephemeris,
     * reference location, reference time and other optional assistance data from the A-GPS server.
     * With the help of the above data, the A-GPS device receives signals from
     * the visible satellites and calculates the position.
     */
    mobile_station_based,

    /** In MSA mode A-GPS operation, the A-GPS capable device receives acquisition assistance,
     * reference time and other optional assistance data from a mobile service provider.
     * The mobile service provider continuously logs GPS information (mainly the almanac) from
     * the GPS satellites using a A-GPS server in its system. With the help of the
     * above data (the data received from the mobile device and the data already present in A-GPS server)
     * the A-GPS server calculates the position and sends it back to the A-GPS device.
     */
    mobile_station_assisted
};

enum class PositionMode
{
    /** Continually provide position updates. */
    periodic,

    /** Obtain a single position estimate. */
    single_shot
};

class HardwareAbstractionLayer
{
public:

    class SuplAssistant
    {
    public:
        /**
         * @brief Simple union for reinterpreting an IP address given as a 32bit unsigned integer into the 4 triplets.
         */
        union IpV4Address
        {
            IpV4Address(std::uint32_t ip = 0) : ip(ip)
            {
            }

            inline bool operator==(const IpV4Address& rhs) const
            {
                return ip == rhs.ip;
            }

            inline bool operator!=(const IpV4Address& rhs) const
            {
                return ip != rhs.ip;
            }

            std::uint32_t ip;
            std::uint8_t triplets[4];
        };

        /**
         * @brief Current status of a SuplAssistant instance.
         */
        enum class Status
        {
            /** The assistant requires a data connection. */
            request_data_connection = 1,
            /** The assistant no longer requires a data connection. */
            release_data_connection = 2,
            /** The data connection has been initiated. */
            data_connection_initiated = 3,
            /** The data transfer has been successfully completed. */
            data_connection_completed = 4,
            /** The data transfer failed. */
            data_connection_failed = 5
        };

        virtual ~SuplAssistant() = default;
        SuplAssistant(const SuplAssistant&) = delete;

        SuplAssistant& operator=(const SuplAssistant&) = delete;
        bool operator==(const SuplAssistant&) = delete;

        /**
         * @brief Getable/observable access to the status of the assistant instance.
         */
        virtual const core::Property<Status>& status() const = 0;

        /**
         * @brief Getable/observable access to the ip the assistance instance uses.
         */
        virtual const core::Property<IpV4Address>& server_ip() const = 0;

        /**
         * @brief set_server informs the assistant instance about the SUPL server it should talk to.
         * @param host_name The host name of the SUPL server.
         * @param port The port of the SUPL server.
         */
        virtual void set_server(const std::string& host_name, std::uint16_t port) = 0;

        /**
          * @brief Notifies the assistant instance that a data connection is open now.
          */
        virtual void notify_data_connection_open_via_apn(const std::string& name) = 0;

        /**
          * @brief Notifies the assistant instance the the data connection is closed now.
          */
        virtual void notify_data_connection_closed() = 0;

        /**
          * @brief Notifies the assistant instance that a data connection is not available right now.
          */
        virtual void notify_data_connection_not_available() = 0;

    protected:
        SuplAssistant() = default;
    };

    /**
     * @brief UtcTimeRequestHandler handles chipset requests for time synchronization
     *
     * An implementation is expected to provide the new reference time in
     * milliseconds since 01.01.1970 to the chipset via a call to inject reference time.
     */
    typedef std::function<void()> UtcTimeRequestHandler;

    /**
     * @brief Returns the default instance for accessing the actual HW.
     */
    static std::shared_ptr<HardwareAbstractionLayer> create_default_instance();

    virtual ~HardwareAbstractionLayer() = default;

    HardwareAbstractionLayer(const HardwareAbstractionLayer&) = delete;
    HardwareAbstractionLayer& operator=(const HardwareAbstractionLayer&) = delete;
    bool operator==(const HardwareAbstractionLayer&) const = delete;

    /**
     * @brief supl_assistant provides access to the SuplAssistant instance that the HAL is using.
     * @return A valid instance if the HAL uses assisted GPS or a nullptr if not.
     */
    virtual SuplAssistant& supl_assistant() = 0;

    /**
     * @brief Signal for delivery of position updates.
     */
    virtual const core::Signal<Position>& position_updates() const = 0;

    /**
     * @brief Signal for delivery of heading updates.
     */
    virtual const core::Signal<Heading>& heading_updates() const = 0;

    /**
     * @brief Signal for delivery of velocity updates.
     */
    virtual const core::Signal<Velocity>& velocity_updates() const = 0;

    /**
     * @brief Signal for delivery of satellite visibility updates.
     */
    virtual const core::Signal<std::set<SpaceVehicle>>& space_vehicle_updates() const = 0;

    /**
      * @brief Requests the chipset to drop all aiding data, including almanac, ephimeris and ionospheric data.
      *
      * Calling this function results in a cold start the next time the positioning is initiated.
      *
      */
    virtual void delete_all_aiding_data() = 0;

    /**
     * @brief Observable/getable property modelling the status of the chipset.
     * @return A non-mutable reference to the property.
     */
    virtual const core::Property<ChipsetStatus>& chipset_status() const = 0;

    /**
     * @brief Checks whether the chipset and driver support the specified assistance mode.
     * @param mode The assistance mode to check for.
     * @return true iff chipset and driver support the specified mode.
     */
    virtual bool is_capable_of(AssistanceMode mode) const = 0;

    /**
     * @brief Checks whether the chipset and driver support the specified position mode.
     * @param mode The position mode to check for.
     * @return true iff chipset and driver support the specified mode.
     */
    virtual bool is_capable_of(PositionMode mode) const = 0;

    /**
     * @brief Checks whether the chipset and driver support the specified capability.
     * @param capability The capability to check for.
     * @return true iff chipset and driver support the specified capability.
     */
    virtual bool is_capable_of(Capability capability) const = 0;

    /**
     * @brief Starts the actual positioning process of the underlying HW.
     * @return true iff starting the positioning engine was successful.
     */
    virtual bool start_positioning() = 0;

    /**
     * @brief Stops the actual positioning process of the underlying HW.
     * @return true iff stopping the positioning engine was successful.
     */
    virtual bool stop_positioning() = 0;

    /**
     * @brief Dispatches the requested mode change to the driver/hw.
     * @param mode The new assistance mode.
     * @return true iff the mode change was carried out successfully.
     */
    virtual bool set_assistance_mode(AssistanceMode mode) = 0;

    /**
     * @brief Dispatches the requested mode change to the driver/hw.
     * @param mode The new position mode.
     * @return true iff the mode change was carried out successfully.
     */
    virtual bool set_position_mode(PositionMode mode) = 0;

    /**
     * @brief Injects a reference position to the underlying gps driver/chipset.
     * @return true iff the injection was successful, false otherwise.
     */
    virtual bool inject_reference_position(const Position& position) = 0;

    /**
     * @brief Injects a new reference time to the underlying gps driver/chipset.
     * @param reference_time The new reference time.
     * @param sample_time When the reference time was sampled.
     * @return true iff the injection was successful, false otherwise.
     */
    virtual bool inject_reference_time(const std::chrono::microseconds& reference_time,
                                       const std::chrono::microseconds& sample_time) = 0;

protected:
    HardwareAbstractionLayer() = default;
};
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_HARDWARE_ABSTRACTION_LAYER_H_

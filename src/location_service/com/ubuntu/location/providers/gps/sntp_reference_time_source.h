/*
 * Copyright © 2016 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_REFERENCE_TIME_SOURCE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_REFERENCE_TIME_SOURCE_H_

#include <com/ubuntu/location/providers/gps/android_hardware_abstraction_layer.h>

#include <com/ubuntu/location/providers/gps/sntp_client.h>

namespace com { namespace ubuntu { namespace location { namespace providers { namespace gps {

class SntpReferenceTimeSource : public android::HardwareAbstractionLayer::ReferenceTimeSource
{
public:

    /** @brief Configuration summarizes default parameter available to implementations. */
    struct Configuration
    {
        /** @brief Reads a configuration from a gps.conf file in INI format. */
        static Configuration from_gps_conf_ini_file(std::istream& in);

        std::string host{"pool.ntp.org"}; /**< NTP host. */
        std::chrono::milliseconds timeout{5000}; /**< Timeout when querying the NTP server. */
    };

    SntpReferenceTimeSource(const Configuration& configuration);

    gps::HardwareAbstractionLayer::ReferenceTimeSample sample() override;

private:
    const Configuration config;
};

}}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_SNTP_REFERENCE_TIME_SOURCE_H_

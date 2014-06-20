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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NULL_GPS_XTRA_DOWNLOADER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NULL_GPS_XTRA_DOWNLOADER_H_

#include "android_hardware_abstraction_layer.h"

#include <com/ubuntu/location/logging.h>

namespace com { namespace ubuntu { namespace location { namespace providers { namespace gps { namespace android {
/** @brief Implements a GPS xtra downloader that just logs download attempts. */
struct NullGpsXtraDownloader : public android::GpsXtraDownloader
{
    std::vector<char> download_xtra_data(const Configuration& config) override
    {
        VLOG(10) << __PRETTY_FUNCTION__ << "\n"
                 << "  Nr. xtra hosts: " << config.xtra_hosts.size();

        return std::vector<char>{};
    }
};
}}}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_NULL_GPS_XTRA_DOWNLOADER_H_

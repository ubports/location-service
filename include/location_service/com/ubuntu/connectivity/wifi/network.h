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
#ifndef LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_WIFI_NETWORK_H_
#define LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_WIFI_NETWORK_H_

#include <com/ubuntu/connectivity/network.h>

namespace com
{
namespace ubuntu
{
namespace connectivity
{
namespace wifi
{
class Network : public com::ubuntu::connectivity::Network
{
public:
    struct Information
    {
        std::string mac;
        std::string bssid;
        int frequency; // in [Mhz]
        int channel;
        int signal_strength; // in [Db]
    };

    Network(const Information& information);

    const Information& information() const;

    // From com::ubuntu::connectivity::Network
    const Property<bool>& requires_authentication() const;

    virtual std::string name() const;

    const Property<double>& signal_to_noise_ratio() const;
    const std::shared_ptr<Link>& link() const;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_WIFI_NETWORK_H_


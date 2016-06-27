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

#include <location/providers/gps/sntp_reference_time_source.h>

#include <location/configuration.h>
#include <location/service/runtime.h>

#include <boost/property_tree/ini_parser.hpp>

#include <future>

namespace gps = location::providers::gps;

gps::SntpReferenceTimeSource::Configuration gps::SntpReferenceTimeSource::Configuration::from_gps_conf_ini_file(std::istream& in)
{
    gps::SntpReferenceTimeSource::Configuration result;

    location::Configuration config;
    boost::property_tree::read_ini(in, config);

    if (config.count("NTP_SERVER") > 0)
        result.host = config.get<std::string>("NTP_SERVER");

    return result;
}
gps::SntpReferenceTimeSource::SntpReferenceTimeSource(const Configuration& config)
    : config(config)
{
}

gps::HardwareAbstractionLayer::ReferenceTimeSample gps::SntpReferenceTimeSource::sample()
{
    auto rt = location::service::Runtime::create();
    rt->start();

    location::providers::gps::sntp::Client sntp_client;
    auto result = sntp_client.request_time(config.host, config.timeout, rt->service());

    return {result.ntp_time, result.ntp_time_reference, result.round_trip_time};
}

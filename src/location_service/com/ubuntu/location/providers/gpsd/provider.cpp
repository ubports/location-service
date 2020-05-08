/*
 * Copyright © 2020 UBports foundation
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
 * Authored by: Marius Gripsgard <marius@ubports.com>
 */

#include "provider.h"

#include <memory>

namespace cul = com::ubuntu::location;
namespace culpg = com::ubuntu::location::providers::gpsd;

void culpg::Provider::loop()
{
    m_gpsd->stream(WATCH_ENABLE | WATCH_JSON);
    std::cout << "GPSD waiting for data" << std::endl;

    while(m_run){
        // block for up to .5 seconds
        if (m_gpsd->waiting(500)){
            auto gpsdata = m_gpsd->read();

            if(gpsdata){
                if (gpsdata->status == STATUS_FIX) {
                    if(gpsdata->fix.longitude!=gpsdata->fix.longitude || gpsdata->fix.altitude!=gpsdata->fix.altitude){
                        std::cout << "GPSD could not get a GPS fix." << std::endl;
                    } else {
                        on_data(gpsdata);
                    }
                }
            } else {
                std::cout << "GPSD Error" << std::endl;
            }
        } else {
            // gps_stream disables itself after a few seconds.. in this case, gps_waiting returns false.
            // we want to re-register for updates and keep looping!
            std::cout << "GPSD enabeling stream again!" << std::endl;
            m_gpsd->stream(WATCH_ENABLE | WATCH_JSON);
        }

        //just a sleep for good measure.
        sleep(1);
    }


    // Shutdown stream
    std::cout << "GPSD stream stopped" << std::endl;
    m_gpsd->stream(WATCH_DISABLE);
}

void culpg::Provider::start()
{
    if (m_run)
        return;

    m_run = true;
    if (!m_worker.joinable())
        m_worker = std::move(std::thread{&culpg::Provider::loop, this});
}

void culpg::Provider::stop()
{
    if (!m_run)
        return;

    m_run = false;
    if (m_worker.joinable())
        m_worker.join();
}

void culpg::Provider::on_data(struct gps_data_t *gpsdata)
{
    auto data = gpsdata->fix;

    if (data.latitude && data.longitude) {
        cul::Position pos
        {
            cul::wgs84::Latitude{data.latitude * cul::units::Degrees},
            cul::wgs84::Longitude{data.longitude * cul::units::Degrees}
        };

        if (data.mode == MODE_3D && data.altitude)
            pos.altitude = cul::wgs84::Altitude{data.altitude * cul::units::Meters};

        if (data.epy)
            pos.accuracy.horizontal = data.epy * cul::units::Meters;
        else // If we have no latitude position uncertainty set to 10m
            pos.accuracy.horizontal = 10 * cul::units::Meters;

        cul::Update<cul::Position> update(pos);
        mutable_updates().position(update);
    }

    if (data.speed)
    {
        cul::Update<cul::Velocity> update
        {
            data.speed * cul::units::MetersPerSecond,
            cul::Clock::now()
        };
        mutable_updates().velocity(update);
    }

    if (data.track)
    {
        cul::Update<cul::Heading> update
        {
            data.track * cul::units::Degrees,
            cul::Clock::now()
        };

        mutable_updates().heading(update);
    }
}

cul::Provider::Ptr culpg::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culpg::Provider::Configuration pConfig;
    pConfig.host = config.count(Configuration::key_host()) > 0 ?
                   config.get<std::string>(Configuration::key_host()) : "localhost";
    pConfig.port = config.count(Configuration::key_port()) > 0 ?
                   config.get<std::string>(Configuration::key_port()) : DEFAULT_GPSD_PORT;
    return cul::Provider::Ptr{new culpg::Provider{pConfig}};
}

culpg::Provider::Provider(const culpg::Provider::Configuration& config) 
        : com::ubuntu::location::Provider(config.features, config.requirements)
 {
    std::cout << "GPSD DEBUG config: host " << config.host << " port: " << config.port << std::endl;
    m_gpsd = std::make_shared<gpsmm>(config.host.c_str(), config.port.c_str());
}

culpg::Provider::~Provider() noexcept
{
    stop();
}

bool culpg::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culpg::Provider::start_position_updates()
{
    start();
}

void culpg::Provider::stop_position_updates()
{
    stop();
}

void culpg::Provider::start_velocity_updates()
{
    start();
}

void culpg::Provider::stop_velocity_updates()
{
    stop();
}    

void culpg::Provider::start_heading_updates()
{
    start();
}

void culpg::Provider::stop_heading_updates()
{
    stop();
}    

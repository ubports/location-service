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

#include <core/net/http/client.h>
#include <com/ubuntu/location/logging.h>

#include <memory>

namespace cul = com::ubuntu::location;
namespace culpg = com::ubuntu::location::providers::mls;
namespace connectivity = com::ubuntu::location::connectivity;

void culpg::Provider::start()
{
    if (m_run)
        return;

    m_run = true;
    on_timeout(boost::system::error_code{});

    http_worker = std::move(std::thread([this]()
    {
        http_client->run();
    }));
}

void culpg::Provider::stop()
{
    if (!m_run)
        return;

    m_run = false;
    boost::system::error_code ec;
    timer.cancel(ec);

    http_client->stop();
    if (http_worker.joinable())
        http_worker.join();

}

cul::Provider::Ptr culpg::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culpg::Provider::Configuration pConfig;
    pConfig.host = config.count(Configuration::key_host()) > 0 ?
                   config.get<std::string>(Configuration::key_host()) : ichnaea::Client::default_host;
    pConfig.api_key = config.count(Configuration::key_api()) > 0 ?
                   config.get<std::string>(Configuration::key_api()) : "test";
    return cul::Provider::Ptr{new culpg::Provider{pConfig}};
}

culpg::Provider::Provider(const culpg::Provider::Configuration& config) 
        : com::ubuntu::location::Provider(config.features, config.requirements),
          http_client{core::net::http::make_client()},
          ichnaea_client{std::make_shared<ichnaea::Client>(config.host, config.api_key, http_client)},
          stop_requested{false},
          rt{cul::service::Runtime::create(1)},
          timer{rt->service()}
{
    connectivity_manager = connectivity::platform_default_manager();

    connectivity_manager->wireless_network_scan_finished().connect([this]()
    {
        VLOG(1) << "Wireless network scan finished.";
        std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wifis;

        connectivity_manager->enumerate_visible_wireless_networks([&wifis](const com::ubuntu::location::connectivity::WirelessNetwork::Ptr& wifi)
        {
            wifis.push_back(wifi);
        });

        std::vector<com::ubuntu::location::connectivity::RadioCell::Ptr> cells;

        connectivity_manager->enumerate_connected_radio_cells([&cells](const com::ubuntu::location::connectivity::RadioCell::Ptr& cell)
        {
            cells.push_back(cell);
        });


        ichnaea::geolocate::Parameters params;
        params.consider_ip = true;

        for (auto wifi : wifis)
        {
            ichnaea::WifiAccessPoint ap;
            ap.bssid = wifi->bssid().get();
            ap.ssid = wifi->ssid().get();
            ap.frequency = wifi->frequency().get();
            ap.signal_strength = wifi->signal_strength().get();

            params.wifi_access_points.insert(ap);
        }

        for (auto cell : cells)
        {
            ichnaea::RadioCell rc;

            rc.radio_type = ichnaea::RadioCell::RadioType::lte;
            rc.serving = true;
            rc.mcc = 0;
            rc.mnc = 0;
            rc.lac = 0;
            rc.id = 0;
            rc.age = std::chrono::duration_values<std::chrono::milliseconds>::zero();
            rc.psc = 0;
            rc.timing_advance = 0;

            switch (cell->type()) {
                case connectivity::RadioCell::Type::gsm: {
                    auto info = cell->gsm();
                    rc.radio_type = ichnaea::RadioCell::RadioType::gsm;
                    rc.mcc = info.mobile_country_code;
                    rc.mnc = info.mobile_network_code;
                    rc.lac = info.location_area_code;
                    rc.id = info.id;
                    rc.signal_strength = info.strength;

                    break;
                }

                case connectivity::RadioCell::Type::umts: {
                    auto info = cell->umts();
                    rc.radio_type = ichnaea::RadioCell::RadioType::wcdma;
                    rc.mcc = info.mobile_country_code;
                    rc.mnc = info.mobile_network_code;
                    rc.lac = info.location_area_code;
                    rc.id = info.id;
                    rc.signal_strength = info.strength;

                    break;
                }
                case connectivity::RadioCell::Type::lte: {
                    auto info = cell->lte();
                    rc.radio_type = ichnaea::RadioCell::RadioType::lte;
                    rc.mcc = info.mobile_country_code;
                    rc.mnc = info.mobile_network_code;
                    rc.lac = info.tracking_area_code;
                    rc.psc = info.physical_id;
                    rc.id = info.id;
                    rc.signal_strength = info.strength;

                    break;
                }
                default:
                    continue;
            }

            params.radio_cells.insert(rc);
        }

        try
        {
            ichnaea_client->geolocate(params, [this](const ichnaea::Response<ichnaea::geolocate::Result>& response)
            {
                if (not response.is_error())
                {
                    cul::Position pos
                    {
                        cul::wgs84::Latitude{response.result().location.lat * cul::units::Degrees},
                        cul::wgs84::Longitude{response.result().location.lon * cul::units::Degrees}
                    };
                    pos.accuracy.horizontal = response.result().accuracy * cul::units::Meters;

                    cul::Update<cul::Position> update(pos);
                    mutable_updates().position(update);
                }
                else
                {
                    VLOG(1) << "Service returned error for location query: " << response.error();
                }
            });
        }
        catch (const std::runtime_error& e)
        {
            VLOG(1) << "Error querying location for new wifi readings: " << e.what();
        }
    });

    rt->start();
}

void culpg::Provider::on_timeout(const boost::system::error_code& ec)
{
    if (ec) return;

    auto thiz = shared_from_this();
    std::weak_ptr<culpg::Provider> wp{thiz};

    if (connectivity_manager)
        connectivity_manager->request_scan_for_wireless_networks();

    timer.expires_from_now(boost::posix_time::seconds{10});
    timer.async_wait([wp](const boost::system::error_code& ec)
    {
        if (auto sp = wp.lock()) sp->on_timeout(ec);
    });
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

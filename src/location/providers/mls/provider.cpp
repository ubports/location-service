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

#include "provider.h"

#include <location/glib/runtime.h>
#include <location/logging.h>

#include <core/net/http/client.h>
#include <core/posix/this_process.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>
#include <thread>

namespace connectivity = com::ubuntu::location::connectivity;
namespace env = core::posix::this_process::env;
namespace fs = boost::filesystem;
namespace mls = location::providers::mls;

namespace
{

struct SettingsHelper
{
    template<typename T>
    static T get_value(std::string key, T&& default_value)
    {
        static const std::string snap_path = env::get("SNAP_DATA");

        boost::filesystem::path path{snap_path};
        std::string key_path{key};
        std::replace_copy(key.begin(), key.end(), key_path.begin(), '.', '/');
        path /= key_path;

        LOG(INFO) << "Reading setting from " << path.string();

        T value{default_value};

        if (fs::exists(path))
        {
            std::ifstream in{path.string().c_str()};
            in >> value;
        }

        return value;
    }
};

namespace options
{

constexpr const char* host{"mls.provider.host"};
constexpr const char* api_key{"mls.provider.api_key"};

}  // namespace options
}  // namespace

void mls::Provider::add_to_registry()
{
    ProviderRegistry::instance().add_provider_for_name("mls::Provider", [](const ProviderRegistry::Configuration& configuration)
    {
        return mls::Provider::create_instance(configuration);
    },
    {
        {options::host, "connect to this ichnaea host"},
        {options::api_key, "use this key to authenticate requests to the ichnaea host"}
    });
}

location::Provider::Ptr mls::Provider::create_instance(const location::ProviderRegistry::Configuration& config)
{
    mls::Configuration configuration;

    configuration.host = config.get(options::host, SettingsHelper::get_value<std::string>(options::host, ichnaea::Client::default_host));
    configuration.api_key = config.get(options::api_key, SettingsHelper::get_value<std::string>(options::api_key, "test"));

    return location::Provider::Ptr{new mls::Provider{configuration}};
}

mls::Provider::Provider(const mls::Configuration& config)
    : http_client{core::net::http::make_client()},
      ichnaea_client{std::make_shared<ichnaea::Client>(config.host, config.api_key, http_client)},
      stop_requested{false},
      rt{location::Runtime::create(1)},
      timer{rt->service()},
      http_worker{[this]() { http_client->run(); }}
{
    connectivity::platform_default_manager([this](const std::shared_ptr<connectivity::Manager>& manager)
    {
        connectivity_manager = manager;

        connectivity_manager->wireless_network_scan_finished().connect([this]()
        {
            VLOG(1) << "Wireless network scan finished.";
            std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wifis;

            connectivity_manager->enumerate_visible_wireless_networks([&wifis](const com::ubuntu::location::connectivity::WirelessNetwork::Ptr& wifi)
            {
                wifis.push_back(wifi);
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

            try
            {
                ichnaea_client->geolocate(params, [this](const ichnaea::Response<ichnaea::geolocate::Result>& response)
                {
                    if (not response.is_error())
                    {
                        Update<Position> update
                        {
                            Position{response.result().location.lat * units::degrees, response.result().location.lon * units::degrees},
                            Clock::now()
                        };
                        update.value.accuracy().horizontal(response.result().accuracy * units::meters);
                        updates.position(update);
                    }
                    else
                    {
                        LOG(WARNING) << "Service returned error for location query: " << response.error();
                    }
                });
            }
            catch (const std::runtime_error& e)
            {
                LOG(WARNING) << "Error querying location for new wifi readings: " << e.what();
            }
        });
    });

    rt->start();
}

mls::Provider::~Provider() noexcept
{
    deactivate();
    rt->stop();
}

void mls::Provider::on_new_event(const Event&)
{
}

location::Provider::Requirements mls::Provider::requirements() const
{
    return Requirements::none;
}

bool mls::Provider::satisfies(const location::Criteria&)
{
    return true;
}

void mls::Provider::enable()
{
}

void mls::Provider::disable()
{
}

void mls::Provider::activate()
{
    on_timeout(boost::system::error_code{});
}

void mls::Provider::deactivate()
{
    boost::system::error_code ec; timer.cancel(ec);
}

const core::Signal<location::Update<location::Position>>& mls::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& mls::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& mls::Provider::velocity_updates() const
{
    return updates.velocity;
}

void mls::Provider::on_timeout(const boost::system::error_code& ec)
{
    if (ec) return;

    auto thiz = shared_from_this();
    std::weak_ptr<Provider> wp{thiz};


    glib::Runtime::instance()->dispatch([this, wp]()
    {
        if (auto sp = wp.lock())
        {
            if (connectivity_manager)
                connectivity_manager->request_scan_for_wireless_networks();
        }
    });

    timer.expires_from_now(boost::posix_time::seconds{10});
    timer.async_wait([wp](const boost::system::error_code& ec)
    {
        if (auto sp = wp.lock()) sp->on_timeout(ec);
    });
}

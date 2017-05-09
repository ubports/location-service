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
#ifndef LOCATION_PROVIDERS_MLS_PROVIDER_H_
#define LOCATION_PROVIDERS_MLS_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_registry.h>
#include <location/runtime.h>

#include <location/connectivity/manager.h>

#include <ichnaea/client.h>

#include <boost/asio/deadline_timer.hpp>

#include <thread>

namespace location
{
namespace providers
{
namespace mls
{
// Summarizes the configuration options known to the mls provider.
struct Configuration
{
    // Host implementing the ichnaea REST API
    std::string host{ichnaea::Client::default_host};
    // API key that should be used to talk to the host.
    std::string api_key{"test"};
};

class Provider : public std::enable_shared_from_this<Provider>, public location::Provider
{
  public:
    // For integration with the Provider factory.
    static void add_to_registry();
    // Instantiates a new provider instance, populating the configuration object
    // from the provided property bundle. Please see dummy::Configuration::Keys
    // for the list of known options.
    static Provider::Ptr create_instance(const ProviderRegistry::Configuration&);

    // Creates a new provider instance from the given configuration.
    Provider(const Configuration& config = Configuration{});
    // Cleans up all resources and stops the updates.
    ~Provider() noexcept;

    // From location::Provider.
    void on_new_event(const Event& event) override;

    void enable() override;
    void disable() override;
    void activate() override;
    void deactivate() override;

    Requirements requirements() const override;
    bool satisfies(const Criteria& criteria) override;
    const core::Signal<Update<Position>>& position_updates() const override;
    const core::Signal<Update<units::Degrees>>& heading_updates() const override;
    const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

  private:
    // on_timeout is called whenever an async wait on a timer finishes.
    // ec indicates whether the operation triggering the invocation was successful or not.
    void on_timeout(const boost::system::error_code& ec);

    std::shared_ptr<com::ubuntu::location::connectivity::Manager> connectivity_manager;
    std::shared_ptr<core::net::http::Client> http_client;
    std::shared_ptr<ichnaea::Client> ichnaea_client;
    std::atomic<bool> stop_requested{false};
    std::shared_ptr<location::Runtime> rt;
    boost::asio::deadline_timer timer;
    std::thread http_worker{};

    struct
    {
        core::Signal<Update<Position>> position;
        core::Signal<Update<units::Degrees>> heading;
        core::Signal<Update<units::MetersPerSecond>> velocity;
    } updates;
};
}
}
}

#endif // LOCATION_PROVIDERS_DUMMY_PROVIDER_H_

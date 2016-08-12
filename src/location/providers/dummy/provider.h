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
#ifndef LOCATION_PROVIDERS_DUMMY_PROVIDER_H_
#define LOCATION_PROVIDERS_DUMMY_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_factory.h>

#include <thread>

namespace location
{
namespace providers
{
namespace dummy
{
// Summarizes the configuration options known to the dummy provider.
struct Configuration
{
    // All configuration keys known to the dummy provider.
    struct Keys
    {
        static constexpr const char* update_period
        {
            "UpdatePeriodInMs"
        };
        static constexpr const char* reference_position_lat
        {
            "ReferenceLocationLat"
        };
        static constexpr const char* reference_position_lon
        {
            "ReferenceLocationLon"
        };
        static constexpr const char* reference_position_alt
        {
            "ReferenceLocationAlt"
        };
        static constexpr const char* reference_horizontal_accuracy
        {
            "ReferenceHorizontalAccuracy"
        };
        static constexpr const char* reference_vertical_accuracy
        {
            "ReferenceVerticalAccuracy"
        };
        static constexpr const char* reference_velocity
        {
            "ReferenceVelocity"
        };
        static constexpr const char* reference_heading
        {
            "ReferenceHeading"
        };
    };

    // Updates are delivered every update_period milliseconds.
    std::chrono::milliseconds update_period{10};

    // The reference position that is delivered in every upate cycle.
    Position reference_position
    {
        wgs84::Latitude
        {
            9. * units::Degrees
        },
        wgs84::Longitude
        {
            53. * units::Degrees
        },
        wgs84::Altitude
        {
            -2. * units::Meters
        }
    };

    // The reference velocity that is delivered in every update cycle.
    Velocity reference_velocity
    {
        9 * units::MetersPerSecond
    };

    // The reference heading that is delivered in every update cycle.
    Heading reference_heading
    {
        127 * units::Degrees
    };
};

class Provider : public location::Provider
{
  public:
    // For integration with the Provider factory.
    static std::string class_name();
    // Instantiates a new provider instance, populating the configuration object
    // from the provided property bundle. Please see dummy::Configuration::Keys
    // for the list of known options.
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    // Creates a new provider instance from the given configuration.
    Provider(const Configuration& config = Configuration{});
    // Cleans up all resources and stops the updates.
    ~Provider() noexcept;

    void on_new_event(const Event& event) override;

    void enable() override;
    void disable() override;
    void activate() override;
    void deactivate() override;

    Requirements requirements() const override;
    bool satisfies(const Criteria& criteria) override;
    const core::Signal<Update<Position>>& position_updates() const override;
    const core::Signal<Update<Heading>>& heading_updates() const override;
    const core::Signal<Update<Velocity>>& velocity_updates() const override;
    void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state);
    void on_reference_location_updated(const Update<Position>& position);
    void on_reference_velocity_updated(const Update<Velocity>& velocity);
    void on_reference_heading_updated(const Update<Heading>& heading);

  private:
    dummy::Configuration configuration;
    std::atomic<bool> stop_requested{false};
    std::thread worker{};

    struct
    {
        core::Signal<Update<Position>> position;
        core::Signal<Update<Heading>> heading;
        core::Signal<Update<Velocity>> velocity;
    } updates;
};
}
}
}

#endif // LOCATION_PROVIDERS_DUMMY_PROVIDER_H_

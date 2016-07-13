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

#include <location/logging.h>

#include <thread>

namespace dummy = location::providers::dummy;

struct dummy::Provider::Private
{
    enum class State
    {
        started,
        stopping,
        stopped
    };

    Private(const dummy::Configuration& configuration)
        : configuration(configuration),
          state(State::stopped)
    {
    }

    dummy::Configuration configuration;
    std::atomic<State> state;
    bool stop_requested;
    std::thread worker{};
};

std::string dummy::Provider::class_name()
{
    return "dummy::Provider";
}

location::Provider::Ptr dummy::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    dummy::Configuration provider_config;

    provider_config.update_period = std::chrono::milliseconds
    {
        config.get(dummy::Configuration::Keys::update_period, 500)
    };
    provider_config.reference_position.latitude = location::wgs84::Latitude
    {
        config.get(dummy::Configuration::Keys::reference_position_lat, 51.) * location::units::Degrees
    };
    provider_config.reference_position.longitude = location::wgs84::Longitude
    {
        config.get(dummy::Configuration::Keys::reference_position_lon, 7.) * location::units::Degrees
    };

    if (config.count(dummy::Configuration::Keys::reference_position_alt) > 0)
        provider_config.reference_position.altitude = location::wgs84::Altitude
        {
            config.get(dummy::Configuration::Keys::reference_position_alt, 0.) * location::units::Meters
        };

    if (config.count(dummy::Configuration::Keys::reference_horizontal_accuracy) > 0)
        provider_config.reference_position.accuracy.horizontal =
            config.get(dummy::Configuration::Keys::reference_horizontal_accuracy, 0.) * location::units::Meters;

    if (config.count(dummy::Configuration::Keys::reference_vertical_accuracy) > 0)
        provider_config.reference_position.accuracy.vertical =
            config.get(dummy::Configuration::Keys::reference_vertical_accuracy, 0.) * location::units::Meters;

    provider_config.reference_velocity = location::Velocity
    {
        config.get(dummy::Configuration::Keys::reference_velocity, 9.) * location::units::MetersPerSecond
    };
    provider_config.reference_heading = location::Heading
    {
        config.get(dummy::Configuration::Keys::reference_heading, 127.) * location::units::Degrees
    };

    return location::Provider::Ptr{new dummy::Provider{provider_config}};
}

dummy::Provider::Provider(const dummy::Configuration& config)
    : location::Provider(
          location::Provider::Features::position | location::Provider::Features::velocity | location::Provider::Features::heading,
          location::Provider::Requirements::none),
      d(new Private{config})
{
}

dummy::Provider::~Provider() noexcept
{
    stop_position_updates();

    if (d->worker.joinable())
        d->worker.join();
}

bool dummy::Provider::matches_criteria(const location::Criteria&)
{
    return true;
}

void dummy::Provider::start_position_updates()
{
    if (d->state.load() != Private::State::stopped)
        return;

    d->stop_requested = false;

    d->worker = std::move(std::thread([this]()
    {
        d->state.store(Private::State::started);
        VLOG(1) << __PRETTY_FUNCTION__ << ": started";

        location::Update<location::Position> position_update
        {
            d->configuration.reference_position,
            location::Clock::now()
        };

        location::Update<location::Heading> heading_update
        {
            d->configuration.reference_heading,
            location::Clock::now()
        };

        location::Update<location::Velocity> velocity_update
        {
            d->configuration.reference_velocity,
            location::Clock::now()
        };

        while (!d->stop_requested)
        {
            VLOG(10) << position_update;

            position_update.when = location::Clock::now();
            heading_update.when = location::Clock::now();
            velocity_update.when = location::Clock::now();

            mutable_updates().position(position_update);
            mutable_updates().heading(heading_update);
            mutable_updates().velocity(velocity_update);           

            std::this_thread::sleep_for(d->configuration.update_period);
        }

        d->state.store(Private::State::stopped);
    }));
}

void dummy::Provider::stop_position_updates()
{
    if (d->state.load() != Private::State::started)
        return;

    d->state.store(Private::State::stopping);
    VLOG(1) << __PRETTY_FUNCTION__ << ": stopping";

    d->stop_requested = true;

    if (d->worker.joinable())
        d->worker.join();
}


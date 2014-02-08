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

#include <com/ubuntu/location/logging.h>

#include <thread>

namespace location = com::ubuntu::location;
namespace dummy = com::ubuntu::location::providers::dummy;

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
            config.get(dummy::Configuration::key_update_period(), 500)
    };
    provider_config.reference_position.latitude(wgs84::Latitude
    {
            config.get(dummy::Configuration::key_reference_position_lat(), 51.) * units::Degrees
    });
    provider_config.reference_position.longitude(wgs84::Longitude
    {
            config.get(dummy::Configuration::key_reference_position_lon(), 7.) * units::Degrees
    });

    return location::Provider::Ptr{new dummy::Provider{provider_config}};
}

namespace
{
location::Provider::FeatureFlags features{"111"};
location::Provider::RequirementFlags requirements{"0000"};
}

dummy::Provider::Provider(const dummy::Configuration& config)
    : location::Provider(features, requirements),
      d(new Private{config})
{
}

dummy::Provider::~Provider() noexcept
{
    stop_position_updates();
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

        timespec ts {0, d->configuration.update_period.count() * 1000 * 1000};

        while (!d->stop_requested)
        {
            deliver_position_updates(d->configuration.reference_position);
            ::nanosleep(&ts, nullptr);
        }
    }));

    d->state.store(Private::State::stopped);
}

void dummy::Provider::stop_position_updates()
{
    if (d->state.load() != Private::State::started)
        return;

    d->state.store(Private::State::stopping);

    if (d->worker.joinable())
    {
        d->stop_requested = true;
        d->worker.join();
    }
}


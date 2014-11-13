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
#include <com/ubuntu/location/engine.h>

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/provider_selection_policy.h>

#include <stdexcept>

namespace cul = com::ubuntu::location;

cul::Engine::Engine(const std::shared_ptr<cul::ProviderSelectionPolicy>& provider_selection_policy)
          : provider_selection_policy(provider_selection_policy)
{
    if (!provider_selection_policy)
        std::runtime_error("Cannot construct an engine given a null ProviderSelectionPolicy");

    // Setup behavior in case of configuration changes.
    configuration.satellite_based_positioning_state.changed().connect([this](const SatelliteBasedPositioningState& state)
    {
        for_each_provider([state](const Provider::Ptr& provider)
        {
            if (provider->requires(cul::Provider::Requirements::satellites))
            {
                switch (state)
                {
                case SatelliteBasedPositioningState::on:
                    provider->state_controller()->state() = cul::Provider::Controller::State::enabled;
                    break;
                case SatelliteBasedPositioningState::off:
                    provider->state_controller()->state() = cul::Provider::Controller::State::disabled;
                    break;
                }
            }
        });
    });

    configuration.engine_state.changed().connect([this](const Engine::Status& status)
    {
        for_each_provider([status](const Provider::Ptr& provider)
        {
            switch (status)
            {
            case Engine::Status::on:
                provider->state_controller()->state() = cul::Provider::Controller::State::enabled;
                break;
            case Engine::Status::off:
                provider->state_controller()->state() = cul::Provider::Controller::State::disabled;
                break;
            default:
                break;
            }
        });
    });
}

cul::Engine::~Engine()
{
    for_each_provider([](const Provider::Ptr& provider)
    {
        provider->state_controller()->stop_position_updates();
        provider->state_controller()->stop_heading_updates();
        provider->state_controller()->stop_velocity_updates();
    });
}

cul::ProviderSelection cul::Engine::determine_provider_selection_for_criteria(const cul::Criteria& criteria)
{
    return provider_selection_policy->determine_provider_selection_for_criteria(criteria, *this);
}

bool cul::Engine::has_provider(const cul::Provider::Ptr& provider) noexcept
{
    return providers.count(provider) > 0;
}

void cul::Engine::add_provider(const cul::Provider::Ptr& provider)
{
    if (!provider)
        throw std::runtime_error("Cannot add null provider");

    // We wire up changes in the engine's configuration to the respective slots
    // of the provider.
    auto cp = updates.reference_location.changed().connect([provider](const cul::Update<cul::Position>& pos)
    {
        provider->on_reference_location_updated(pos);
    });

    auto cv = updates.reference_velocity.changed().connect([provider](const cul::Update<cul::Velocity>& velocity)
    {
        provider->on_reference_velocity_updated(velocity);
    });

    auto ch = updates.reference_heading.changed().connect([provider](const cul::Update<cul::Heading>& heading)
    {
        provider->on_reference_heading_updated(heading);
    });

    auto cr = configuration.wifi_and_cell_id_reporting_state.changed().connect([provider](cul::WifiAndCellIdReportingState state)
    {
        provider->on_wifi_and_cell_reporting_state_changed(state);
    });

    // And do the reverse: Satellite visibility updates are funneled via the engine's configuration.
    auto cs = provider->updates().svs.connect([this](const cul::Update<std::set<cul::SpaceVehicle>>& src)
    {
        updates.visible_space_vehicles.update([src](std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>& dest)
        {
            for(auto& sv : src.value)
            {
                dest[sv.key] = sv;
            }

            return true;
        });
    });

    // We are a bit dumb and just take any position update as new reference.
    // We should come up with a better heuristic here.
    auto cpr = provider->updates().position.connect([this](const cul::Update<cul::Position>& src)
    {
        updates.reference_location = src;
    });

    std::lock_guard<std::mutex> lg(guard);
    providers.emplace(provider, std::move(ProviderConnections{cp, ch, cv, cr, cs, cpr}));
}

void cul::Engine::remove_provider(const cul::Provider::Ptr& provider) noexcept
{
    std::lock_guard<std::mutex> lg(guard);

    auto it = providers.find(provider);
    if (it != providers.end())
    {
        providers.erase(it);
    }
}

void cul::Engine::for_each_provider(const std::function<void(const Provider::Ptr&)>& enumerator) const noexcept
{
    std::lock_guard<std::mutex> lg(guard);
    for (const auto& provider : providers)
    {
        try
        {
            enumerator(provider.first);
        }
        catch(const std::exception& e)
        {
            VLOG(1) << e.what();
        }
    }
}

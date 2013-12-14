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

#include <com/ubuntu/location/provider_selection_policy.h>

#include <stdexcept>

namespace cul = com::ubuntu::location;

cul::Engine::Engine(const std::shared_ptr<cul::ProviderSelectionPolicy>& provider_selection_policy)
          : provider_selection_policy(provider_selection_policy)
{
    if (!provider_selection_policy)
        std::runtime_error("Cannot construct an engine given a null ProviderSelectionPolicy");

    configuration.satellite_based_positioning_state = cul::SatelliteBasedPositioningState::on;
    configuration.wifi_and_cell_id_reporting_state = cul::WifiAndCellIdReportingState::off;
    configuration.engine_state = Engine::Status::on;
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
    auto c1 = configuration.reference_location.changed().connect([provider](const cul::Update<cul::Position>& pos)
    {
        provider->on_reference_location_updated(pos);
    });

    auto c2 = configuration.wifi_and_cell_id_reporting_state.changed().connect([provider](cul::WifiAndCellIdReportingState state)
    {
        provider->on_wifi_and_cell_reporting_state_changed(state);
    });

    // And do the reverse: Satellite visibility updates are funneled via the engine's configuration.
    auto c3 = provider->updates().svs.connect([this](const cul::Update<std::set<cul::SpaceVehicle>>& src)
    {
        configuration.visible_space_vehicles.update([src](std::set<cul::SpaceVehicle>& dest)
        {
            for(auto& sv : src.value)
            {
                // TODO: This is incredibly ugly and we should switch to a
                // std::map asap.
                dest.erase(sv);
                dest.insert(sv);
            }

            return true;
        });
    });

    providers.insert(std::make_pair(provider, ProviderConnections{c1, c2, c3}));
}

void cul::Engine::remove_provider(const cul::Provider::Ptr& provider) noexcept
{
    auto it = providers.find(provider);
    if (it != providers.end())
    {
        it->second.reference_location_updates.disconnect();
        it->second.wifi_and_cell_id_reporting_state_updates.disconnect();
        it->second.space_vehicle_visibility_updates.disconnect();

        providers.erase(it);
    }
}

void cul::Engine::for_each_provider(const std::function<void(const Provider::Ptr&)>& enumerator) const noexcept
{
    for (const auto& provider : providers)
        enumerator(provider.first);
}

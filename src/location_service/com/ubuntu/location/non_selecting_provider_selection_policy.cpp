/*
 * Copyright © 2014 Canonical Ltd.
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

#include <com/ubuntu/location/non_selecting_provider_selection_policy.h>

#include <set>

namespace location = com::ubuntu::location;

namespace
{

location::Provider::Features all_features()
{
    return location::Provider::Features::position |
           location::Provider::Features::heading |
           location::Provider::Features::velocity;
}

location::Provider::Requirements all_requirements()
{
    return location::Provider::Requirements::cell_network |
           location::Provider::Requirements::data_network |
           location::Provider::Requirements::monetary_spending |
           location::Provider::Requirements::satellites;
}

// A simple helper that takes a set of existing providers
// and maps calls to each of them.
struct BagOfProviders : public location::Provider
{
    BagOfProviders(const std::set<location::Provider::Ptr>& providers)
        : location::Provider{all_features(), all_requirements()},
          providers{providers}
    {
        // We connect to all updates of each provider.
        for (auto provider : BagOfProviders::providers)
        {
            event_connections.push_back(
                        provider->updates().position.connect([this](const location::Update<location::Position>& update)
                        {
                            mutable_updates().position(update);
                        }));

            event_connections.push_back(
                        provider->updates().heading.connect([this](const location::Update<location::Heading>& update)
                        {
                            mutable_updates().heading(update);
                        }));

            event_connections.push_back(
                        provider->updates().velocity.connect([this](const location::Update<location::Velocity>& update)
                        {
                            mutable_updates().velocity(update);
                        }));
        }

    }

    ~BagOfProviders()
    {
        for(auto& c : event_connections)
            c.disconnect();
    }

    // We always match :)
    bool matches_criteria(const location::Criteria&) override
    {
        return true;
    }

    // We forward all events to the other providers.
    void on_wifi_and_cell_reporting_state_changed(location::WifiAndCellIdReportingState state) override
    {
        for (auto provider : providers)
            provider->on_wifi_and_cell_reporting_state_changed(state);
    }

    void on_reference_location_updated(const location::Update<location::Position>& position) override
    {
        for (auto provider : providers)
            provider->on_reference_location_updated(position);
    }

    void on_reference_velocity_updated(const location::Update<location::Velocity>& velocity) override
    {
        for (auto provider : providers)
            provider->on_reference_velocity_updated(velocity);
    }

    void on_reference_heading_updated(const location::Update<location::Heading>& heading) override
    {
        for (auto provider : providers)
            provider->on_reference_heading_updated(heading);
    }

    void start_position_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->start_position_updates();
    }

    void stop_position_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->stop_position_updates();
    }

    void start_heading_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->start_heading_updates();
    }

    void stop_heading_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->stop_heading_updates();
    }

    void start_velocity_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->start_velocity_updates();
    }

    void stop_velocity_updates() override
    {
        for (auto provider : providers)
            provider->state_controller()->stop_velocity_updates();
    }

    std::set<location::Provider::Ptr> providers;
    std::vector<core::Connection> event_connections;
};
}

location::ProviderSelection location::NonSelectingProviderSelectionPolicy::determine_provider_selection_for_criteria(
        const location::Criteria&,
        const location::ProviderEnumerator& enumerator)
{
    // We put all providers in a set.
    std::set<location::Provider::Ptr> bag;
    enumerator.for_each_provider([&bag](const location::Provider::Ptr& provider)
    {
        bag.insert(provider);
    });

    auto bag_of_providers = std::make_shared<BagOfProviders>(bag);

    // Our bag of providers is responsible for delivering position/heading/velocity updates.
    return location::ProviderSelection
    {
        bag_of_providers, // position
        bag_of_providers, // heading
        bag_of_providers  // velocity
    };
}

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
#include <com/ubuntu/location/default_provider_selection_policy.h>

#include <com/ubuntu/location/engine.h>

namespace cul = com::ubuntu::location;

namespace
{
struct NullProvider : public cul::Provider
{
    NullProvider() = default;
};

std::shared_ptr<cul::Provider> null_provider_instance
{
    new NullProvider{}
};
}

const cul::Provider::Ptr& cul::ProviderSelectionPolicy::null_provider()
{
    return null_provider_instance;
}

cul::DefaultProviderSelectionPolicy::DefaultProviderSelectionPolicy()
{
}

cul::DefaultProviderSelectionPolicy::~DefaultProviderSelectionPolicy() noexcept
{
}

cul::ProviderSelection 
cul::DefaultProviderSelectionPolicy::determine_provider_selection_for_criteria(
    const cul::Criteria& criteria,
    const cul::ProviderEnumerator& enumerator)
{
    ProviderSelection selection
    {
        determine_position_updates_provider(criteria, enumerator),
        determine_heading_updates_provider(criteria, enumerator),
        determine_velocity_updates_provider(criteria, enumerator)
    };
    
    return selection;
}

cul::Provider::Ptr 
cul::DefaultProviderSelectionPolicy::determine_position_updates_provider(
    const cul::Criteria& criteria,
    const cul::ProviderEnumerator& enumerator)
{
    auto less = 
            [](const Provider::Ptr& lhs, const Provider::Ptr& rhs)
            {
                return
                lhs->state_controller()->are_position_updates_running() && !rhs->state_controller()->are_position_updates_running();
            };

    std::set<
        Provider::Ptr,
        std::function<bool(const Provider::Ptr&, const Provider::Ptr&)>> matching_providers(less);

    enumerator.for_each_provider(
        [&](const Provider::Ptr& provider)
        {
            if (provider->matches_criteria(criteria))
                matching_providers.insert(provider);
        });

    return matching_providers.empty() ? null_provider() : *matching_providers.begin();
}

cul::Provider::Ptr cul::DefaultProviderSelectionPolicy::determine_heading_updates_provider(
    const cul::Criteria& criteria,
    const cul::ProviderEnumerator& enumerator)
{
    auto less = 
            [](const Provider::Ptr& lhs, const Provider::Ptr& rhs)
            {
                return lhs->state_controller()->are_heading_updates_running() && !rhs->state_controller()->are_heading_updates_running();
            };
    
    std::set<
        Provider::Ptr, 
        std::function<bool(const Provider::Ptr&, const Provider::Ptr&)>> matching_providers(less);

    enumerator.for_each_provider(
        [&](const Provider::Ptr& provider)
        {
            if (provider->matches_criteria(criteria))
                matching_providers.insert(provider);
        });

    return matching_providers.empty() ? null_provider() : *matching_providers.begin();
}

cul::Provider::Ptr cul::DefaultProviderSelectionPolicy::determine_velocity_updates_provider(
    const cul::Criteria& criteria,
    const cul::ProviderEnumerator& enumerator)
{
    auto less = 
            [](const Provider::Ptr& lhs, const Provider::Ptr& rhs)
            {
                return lhs->state_controller()->are_velocity_updates_running() && !rhs->state_controller()->are_velocity_updates_running();
            };
    
    std::set<
        Provider::Ptr, 
        std::function<bool(const Provider::Ptr&, const Provider::Ptr&)>> matching_providers(less);

    enumerator.for_each_provider(
        [&](const Provider::Ptr& provider)
        {
            if (provider->matches_criteria(criteria))
                matching_providers.insert(provider);
        });

    return matching_providers.empty() ? null_provider() : *matching_providers.begin();
}

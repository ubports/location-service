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
#include "com/ubuntu/location/engine.h"

#include <stdexcept>

namespace cul = com::ubuntu::location;

cul::Engine::Engine(
    const std::set<cul::Provider::Ptr>& initial_providers,
    const cul::ProviderSelectionPolicy::Ptr& provider_selection_policy)
        : providers(initial_providers),
          provider_selection_policy(provider_selection_policy)
{
    if (!provider_selection_policy)
        std::runtime_error("Cannot construct an engine given a null ProviderSelectionPolicy");
}

cul::ProviderSelection cul::Engine::determine_provider_selection_for_criteria(const cul::Criteria& criteria)
{
    return provider_selection_policy->determine_provider_selection_from_set_for_criteria(criteria, providers);
}

bool cul::Engine::has_provider(const cul::Provider::Ptr& provider) noexcept
{
    return providers.count(provider) > 0;
}

void cul::Engine::add_provider(const cul::Provider::Ptr& provider)
{
    if (!provider)
        throw std::runtime_error("Cannot add null provider");
    
    providers.insert(provider);
}

void cul::Engine::remove_provider(const cul::Provider::Ptr& provider) noexcept
{
    providers.erase(provider);
}


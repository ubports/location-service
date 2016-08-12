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

#include <location/non_selecting_provider_selection_policy.h>
#include <location/providers/proxy.h>

#include <set>

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

    auto proxy = std::make_shared<providers::Proxy>(bag);

    // Our bag of providers is responsible for delivering position/heading/velocity updates.
    return location::ProviderSelection
    {
        proxy, // position
        proxy, // heading
        proxy  // velocity
    };
}

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

#ifndef LOCATION_NON_SELECTING_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_NON_SELECTING_PROVIDER_SELECTION_POLICY_H_

#include <location/provider_selection_policy.h>

namespace location
{
// A policy that makes sure that all providers are selected. It is entirely
// dumb and somewhat anniliates the idea of having a selection policy at all.
// However, it is a sensible choice until we devise a more clever heuristic
// to actually select providers based on requirements, features and criteria.
struct NonSelectingProviderSelectionPolicy : public ProviderSelectionPolicy
{
    ProviderSelection determine_provider_selection_for_criteria(const Criteria&, const ProviderEnumerator&) override;
};
}

#endif // NON_SELECTING_PROVIDER_SELECTION_POLICY_H_

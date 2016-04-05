/*
 * Copyright © 2016 Canonical Ltd.
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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com
 */
#ifndef COM_UBUNTU_LOCATION_FUSION_PROVIDER_SELECTION_POLICY_H
#define COM_UBUNTU_LOCATION_FUSION_PROVIDER_SELECTION_POILCY_H

#include <com/ubuntu/location/provider_selection_policy.h>

namespace com
{
namespace ubuntu
{
namespace location
{
// This policy selects all providers, supplying them in the form of a fusion
// provider for higher quality position updates
struct FusionProviderSelectionPolicy : public ProviderSelectionPolicy
{
    ProviderSelection determine_provider_selection_for_criteria(const Criteria &criteria, const ProviderEnumerator &enumerator);
};
}
}
}

#endif // COM_UBUNTU_LOCATION_FUSION_PROVIDER_SELECTION_POILCY_H

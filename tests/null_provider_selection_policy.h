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

#ifndef NULL_PROVIDER_SELECTION_POLICY_
#define NULL_PROVIDER_SELECTION_POLICY_

#include <location/provider_selection_policy.h>

struct NullProviderSelectionPolicy : public location::ProviderSelectionPolicy
{
    location::ProviderSelection
    determine_provider_selection_for_criteria(
            const location::Criteria&,
            const location::ProviderEnumerator&)
    {
        return location::ProviderSelection
        {
            location::Provider::Ptr{},
            location::Provider::Ptr{},
            location::Provider::Ptr{}
        };
    }
};

#endif // NULL_PROVIDER_SELECTION_POLICY_

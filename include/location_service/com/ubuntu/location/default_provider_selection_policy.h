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
#ifndef LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_

#include "com/ubuntu/location/provider_selection_policy.h"

namespace com
{
namespace ubuntu
{
namespace location
{
class DefaultProviderSelectionPolicy : public ProviderSelectionPolicy
{
public:
    DefaultProviderSelectionPolicy();
    ~DefaultProviderSelectionPolicy() noexcept;
    
    ProviderSelection determine_provider_selection_from_set_for_criteria(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_position_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_heading_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_velocity_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_

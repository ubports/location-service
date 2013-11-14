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
#ifndef LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_

#include "com/ubuntu/location/provider.h"

#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria;

struct ProviderSelection
{
    ProviderSelection(const Provider::Ptr position = Provider::Ptr{},
                      const Provider::Ptr heading = Provider::Ptr{},
                      const Provider::Ptr velocity = Provider::Ptr{}) : position_updates_provider(position),
                                                      heading_updates_provider(heading),
                                                      velocity_updates_provider(velocity)
    {
    }
    
    Provider::Features to_feature_flags() const
    {
        Provider::Features flags = Provider::Features::none;

        if (position_updates_provider)
            flags = flags | Provider::Features::position;
        if (heading_updates_provider)
            flags = flags | Provider::Features::heading;
        if(velocity_updates_provider)
            flags = flags | Provider::Features::velocity;

        return flags;
    }

    Provider::Ptr position_updates_provider;
    Provider::Ptr heading_updates_provider;
    Provider::Ptr velocity_updates_provider;
};

inline bool operator==(const ProviderSelection& lhs, const ProviderSelection& rhs)
{
    return lhs.position_updates_provider == rhs.position_updates_provider &&
            lhs.heading_updates_provider == rhs.heading_updates_provider &&
            lhs.velocity_updates_provider == rhs.velocity_updates_provider;
}

class ProviderSelectionPolicy
{
public:
    typedef std::shared_ptr<ProviderSelectionPolicy> Ptr;

    virtual ~ProviderSelectionPolicy() = default;

    virtual ProviderSelection determine_provider_selection_from_set_for_criteria(const Criteria& criteria, const std::set<Provider::Ptr>& providers) = 0;
protected:
    ProviderSelectionPolicy() = default;
private:
    ProviderSelectionPolicy(const ProviderSelectionPolicy&) = delete;
    ProviderSelectionPolicy& operator=(const ProviderSelectionPolicy&) = delete;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_

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
#ifndef PROVIDER_SELECTION_H_
#define PROVIDER_SELECTION_H_

#include <location/provider.h>

#include <memory>

namespace location
{
struct ProviderSelection
{
    inline Provider::Features to_feature_flags() const
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

    std::shared_ptr<Provider> position_updates_provider;
    std::shared_ptr<Provider> heading_updates_provider;
    std::shared_ptr<Provider> velocity_updates_provider;
};

inline bool operator==(const ProviderSelection& lhs, const ProviderSelection& rhs)
{
    return lhs.position_updates_provider == rhs.position_updates_provider &&
            lhs.heading_updates_provider == rhs.heading_updates_provider &&
            lhs.velocity_updates_provider == rhs.velocity_updates_provider;
}
}

#endif // PROVIDER_SELECTION_H_

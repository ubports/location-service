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
#ifndef LOCATION_PROXY_PROVIDER_H_
#define LOCATION_PROXY_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_selection_policy.h>

#include <bitset>
#include <memory>

namespace location
{
class ProxyProvider : public Provider
{
public:
    typedef std::shared_ptr<ProxyProvider> Ptr;

    ProxyProvider(const ProviderSelection& selection);
    ~ProxyProvider() noexcept;

    virtual void start_position_updates();
    virtual void stop_position_updates();

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates();

    virtual void start_heading_updates();
    virtual void stop_heading_updates();
    
private:
    ProviderSelection providers;

    struct
    {
        core::ScopedConnection position_updates;
        core::ScopedConnection heading_updates;
        core::ScopedConnection velocity_updates;
    } connections;
};
}

#endif // LOCATION_PROXY_PROVIDER_H_

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
#ifndef LOCATION_SERVICE_CONFIGURATION_H_
#define LOCATION_SERVICE_CONFIGURATION_H_

#include <location/service/permission_manager.h>

#include <location/provider.h>
#include <location/provider_selection_policy.h>

#include <set>

namespace location
{
class Engine;
namespace service
{
class Configuration
{
public:
    virtual ~Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    virtual std::shared_ptr<Engine> the_engine(
    	const std::set<Provider::Ptr>& provider_set,
        const ProviderSelectionPolicy::Ptr& provider_selection_policy) = 0;
    
    virtual ProviderSelectionPolicy::Ptr the_provider_selection_policy() = 0;
    virtual std::set<Provider::Ptr> the_provider_set() = 0;
    virtual PermissionManager::Ptr the_permission_manager() = 0;

protected:
    Configuration() = default;
};
}
}

#endif // LOCATION_SERVICE_CONFIGURATION_H_

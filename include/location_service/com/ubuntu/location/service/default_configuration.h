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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_

#include <com/ubuntu/location/service/configuration.h>

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class DefaultConfiguration
{
public:
    DefaultConfiguration();
    DefaultConfiguration(const DefaultConfiguration&) = delete;
    DefaultConfiguration& operator=(const DefaultConfiguration&) = delete;
    ~DefaultConfiguration() noexcept;

    virtual Engine::Ptr the_engine(
        const std::set<Provider::Ptr>& provider_set,
        const ProviderSelectionPolicy::Ptr& provider_selection_policy);

    ProviderSelectionPolicy::Ptr the_provider_selection_policy();

    std::set<Provider::Ptr> the_provider_set(
        const Provider::Ptr& seed = Provider::Ptr {});

    PermissionManager::Ptr the_permission_manager();
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_

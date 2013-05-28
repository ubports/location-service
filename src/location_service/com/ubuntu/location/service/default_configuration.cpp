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
#include "com/ubuntu/location/service/default_configuration.h"
#include "com/ubuntu/location/service/default_permission_manager.h"

#include "com/ubuntu/location/default_provider_selection_policy.h"
#include "com/ubuntu/location/default_provider_set.h"

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

culs::DefaultConfiguration::DefaultConfiguration()
{
}

culs::DefaultConfiguration::~DefaultConfiguration() noexcept
{
}

cul::Engine::Ptr culs::DefaultConfiguration::the_engine(
    const std::set<cul::Provider::Ptr>& provider_set,
    const cul::ProviderSelectionPolicy::Ptr& provider_selection_policy)
{
    return Engine::Ptr {new Engine{provider_set, provider_selection_policy}};
}

cul::ProviderSelectionPolicy::Ptr culs::DefaultConfiguration::the_provider_selection_policy()
{
    return ProviderSelectionPolicy::Ptr(new DefaultProviderSelectionPolicy());
}

std::set<cul::Provider::Ptr> culs::DefaultConfiguration::the_provider_set(
    const cul::Provider::Ptr& seed)
{
    return default_provider_set(seed);
}

culs::PermissionManager::Ptr culs::DefaultConfiguration::the_permission_manager()
{
    return DefaultPermissionManager::Ptr(new DefaultPermissionManager());
}


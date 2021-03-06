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
#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/default_permission_manager.h>
#include <com/ubuntu/location/service/system_configuration.h>

#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/fusion_provider_selection_policy.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

cul::Engine::Ptr culs::DefaultConfiguration::the_engine(
    const std::set<cul::Provider::Ptr>& provider_set,
    const cul::ProviderSelectionPolicy::Ptr& provider_selection_policy,
    const cul::Settings::Ptr& settings)
{
    auto engine = std::make_shared<cul::Engine>(provider_selection_policy, settings);
    for (const auto& provider : provider_set)
        engine->add_provider(provider);

    return engine;
}

cul::ProviderSelectionPolicy::Ptr culs::DefaultConfiguration::the_provider_selection_policy()
{
    return std::make_shared<cul::FusionProviderSelectionPolicy>();
}

std::set<cul::Provider::Ptr> culs::DefaultConfiguration::the_provider_set(
    const cul::Provider::Ptr& seed)
{
    return std::set<cul::Provider::Ptr>{seed};
}

culs::PermissionManager::Ptr culs::DefaultConfiguration::the_permission_manager(const core::dbus::Bus::Ptr& bus)
{
    return culs::SystemConfiguration::instance().create_permission_manager(bus);
}


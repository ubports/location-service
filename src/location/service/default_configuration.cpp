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
#include <location/service/default_configuration.h>
#include <location/service/default_permission_manager.h>
#include <location/service/trust_store_permission_manager.h>

#include <location/engine.h>
#include <location/fusion_provider_selection_policy.h>

location::Engine::Ptr location::service::DefaultConfiguration::the_engine(
    const std::set<location::Provider::Ptr>& provider_set,
    const location::ProviderSelectionPolicy::Ptr& provider_selection_policy,
    const location::Settings::Ptr& settings)
{
    auto engine = std::make_shared<location::Engine>(provider_selection_policy, settings);
    for (const auto& provider : provider_set)
        engine->add_provider(provider);

    return engine;
}

location::ProviderSelectionPolicy::Ptr location::service::DefaultConfiguration::the_provider_selection_policy()
{
    return std::make_shared<location::FusionProviderSelectionPolicy>();
}

std::set<location::Provider::Ptr> location::service::DefaultConfiguration::the_provider_set(
    const location::Provider::Ptr& seed)
{
    return std::set<location::Provider::Ptr>{seed};
}

location::service::PermissionManager::Ptr location::service::DefaultConfiguration::the_permission_manager(const core::dbus::Bus::Ptr& bus)
{
    return location::service::TrustStorePermissionManager::create_default_instance_with_bus(bus);
}


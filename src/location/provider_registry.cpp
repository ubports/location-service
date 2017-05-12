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
#include <location/provider_registry.h>
#include <location/provider.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>

location::ProviderRegistry& location::ProviderRegistry::instance()
{
    static location::ProviderRegistry pf;
    return pf;
}

void location::ProviderRegistry::add_provider_for_name(
        const std::string& name,
        const location::ProviderRegistry::Factory& factory,
        const Options& options)
{
    std::lock_guard<std::mutex> lg(guard);
    provider_store[name] = std::make_tuple(factory, options);
}

location::Provider::Ptr location::ProviderRegistry::create_provider_for_name_with_config(
        const std::string& name,
        const util::settings::Source& settings)
{
    std::lock_guard<std::mutex> lg(guard);
    if (provider_store.count(name) == 0)
        return Provider::Ptr{};
    
    return location::Provider::Ptr{std::get<0>(provider_store.at(name))(settings)};
}

void location::ProviderRegistry::enumerate(
        const std::function<void(const std::string&, const location::ProviderRegistry::Factory&, const Options&)>& enumerator)
{
    std::lock_guard<std::mutex> lg(guard);
    for (const auto& pair : provider_store)
        enumerator(pair.first, std::get<0>(pair.second), std::get<1>(pair.second));
}



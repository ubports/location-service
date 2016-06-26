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
#include <location/provider_factory.h>
#include <location/provider.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>

location::ProviderFactory& location::ProviderFactory::instance()
{
    static location::ProviderFactory pf;
    return pf;
}

void location::ProviderFactory::add_factory_for_name(
    const std::string& name, 
    const location::ProviderFactory::Factory& factory)
{
    std::lock_guard<std::mutex> lg(guard);
    factory_store[name] = factory;
}

location::Provider::Ptr location::ProviderFactory::create_provider_for_name_with_config(
    const std::string& name, 
    const location::ProviderFactory::Configuration& config)
{
    auto undecorated_name = extract_undecorated_name(name);

    std::lock_guard<std::mutex> lg(guard);
    if (factory_store.count(undecorated_name) == 0)
        return Provider::Ptr{};
    
    return location::Provider::Ptr{factory_store.at(undecorated_name)(config)};
}

void location::ProviderFactory::create_provider_for_name_with_config(
    const std::string& name,
    const location::ProviderFactory::Configuration& config,
    const std::function<void(Provider::Ptr)>& cb)
{
    auto undecorated_name = extract_undecorated_name(name);

    std::lock_guard<std::mutex> lg(guard);
    if (factory_store.count(undecorated_name) == 0)
        return;

    cb(location::Provider::Ptr{factory_store.at(undecorated_name)(config)});
}

void location::ProviderFactory::enumerate(
    const std::function<void(const std::string&, const location::ProviderFactory::Factory&)>& enumerator)
{
    std::lock_guard<std::mutex> lg(guard);
    std::for_each(
        factory_store.begin(), 
        factory_store.end(), 
        [enumerator](const std::map<std::string, location::ProviderFactory::Factory>::value_type& value)
        {
            enumerator(value.first, value.second);
        });
}

std::string location::ProviderFactory::extract_undecorated_name(const std::string& name)
{
    return name.substr(0, name.find("@"));
}



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
#include <com/ubuntu/location/provider_factory.h>
#include <com/ubuntu/location/provider.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace cul = com::ubuntu::location;

cul::ProviderFactory& cul::ProviderFactory::instance()
{
    static cul::ProviderFactory pf;
    return pf;
}

void cul::ProviderFactory::add_factory_for_name(
    const std::string& name, 
    const cul::ProviderFactory::Factory& factory)
{
    std::lock_guard<std::mutex> lg(guard);
    factory_store[name] = factory;
}

cul::Provider::Ptr cul::ProviderFactory::create_provider_for_name_with_config(
    const std::string& name, 
    const cul::ProviderFactory::Configuration& config)
{
    auto undecorated_name = name.substr(0, name.find("@"));

    std::lock_guard<std::mutex> lg(guard);
    if (factory_store.count(undecorated_name) == 0)
        return Provider::Ptr{};
    
    return cul::Provider::Ptr{factory_store.at(undecorated_name)(config)};
}

void cul::ProviderFactory::enumerate(
    const std::function<void(const std::string&, const cul::ProviderFactory::Factory&)>& enumerator)
{
    std::lock_guard<std::mutex> lg(guard);
    std::for_each(
        factory_store.begin(), 
        factory_store.end(), 
        [enumerator](const std::map<std::string, cul::ProviderFactory::Factory>::value_type& value)
        {
            enumerator(value.first, value.second);
        });
}



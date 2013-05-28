#include "com/ubuntu/location/provider_factory.h"
#include "com/ubuntu/location/provider.h"

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
    std::lock_guard<std::mutex> lg(guard);
    if (factory_store.count(name) == 0)
        return Provider::Ptr{};
    
    return cul::Provider::Ptr{factory_store.at(name)(config)};
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



#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_factory.h"

#include <map>

namespace cul = com::ubuntu::location;

namespace
{
struct FactoryInjector
{
    FactoryInjector(const std::string& name, const std::function<cul::Provider::Ptr(const cul::ProviderFactory::Configuration&)>& f)
    {
        com::ubuntu::location::ProviderFactory::instance().add_factory_for_name(name, f);
    }
};
}
#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GEOCLUE)
#include "com/ubuntu/location/providers/geoclue/provider.h"
static FactoryInjector geoclue_injector
{
    "geoclue::Provider", 
    com::ubuntu::location::providers::geoclue::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GEOCLUE

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS)
#include "com/ubuntu/location/providers/gps/provider.h"
static FactoryInjector gps_injector
{
    "gps::Provider", 
    com::ubuntu::location::providers::gps::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK)
#include "com/ubuntu/location/providers/skyhook/provider.h"
static FactoryInjector skyhook_injector
{
    "skyhook::Provider", 
    com::ubuntu::location::providers::skyhook::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK


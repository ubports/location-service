#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_DEFAULT_PROVIDER_SET_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_DEFAULT_PROVIDER_SET_H_

#include "com/ubuntu/location/provider.h"

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
std::set<Provider::Ptr> default_provider_set(const Provider::Ptr& seed = Provider::Ptr {});
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_DEFAULT_PROVIDER_SET_H_

#include "com/ubuntu/location/default_provider_set.h"

#include "com/ubuntu/location/provider.h"

#include <set>

namespace cul = com::ubuntu::location;

std::set<cul::Provider::Ptr> cul::default_provider_set(const cul::Provider::Ptr& seed)
{
    std::set<cul::Provider::Ptr> result{};
    if (seed)
        result.insert(seed);

    return result;
}

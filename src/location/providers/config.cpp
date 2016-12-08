/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTIlocationAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <location/provider.h>
#include <location/provider_factory.h>

#include "dummy/provider.h"
#include "mls/provider.h"
#include "ubx/provider.h"

#include <map>

namespace
{
struct FactoryInjector
{
    FactoryInjector(const std::string& name, const std::function<location::Provider::Ptr(const location::ProviderFactory::Configuration&)>& f)
    {
        location::ProviderFactory::instance().add_factory_for_name(name, f);
    }
};
}

static FactoryInjector dummy_injector
{
    "dummy::Provider",
    location::providers::dummy::Provider::create_instance
};

static FactoryInjector mls_injector
{
    "mls::Provider",
    location::providers::mls::Provider::create_instance
};

static FactoryInjector ubx_injector
{
    "ubx::Provider",
    location::providers::ubx::Provider::create_instance
};

#if defined(LOCATION_PROVIDERS_GPS)
#include <location/providers/gps/provider.h>
static FactoryInjector gps_injector
{
    "gps::Provider", 
    location::providers::gps::Provider::create_instance
};
#endif // LOCATION_PROVIDERS_GPS

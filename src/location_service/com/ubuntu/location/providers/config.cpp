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
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

#include "dummy/provider.h"
#include "dummy/delayed_provider.h"

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

static FactoryInjector dummy_injector
{
    "dummy::Provider",
    com::ubuntu::location::providers::dummy::Provider::create_instance
};

static FactoryInjector dummy_delayed_injector
{
    "dummy::DelayedProvider",
    com::ubuntu::location::providers::dummy::DelayedProvider::create_instance
};

#include <com/ubuntu/location/providers/remote/provider.h>
static FactoryInjector remote_injector
{
    "remote::Provider",
    com::ubuntu::location::providers::remote::Provider::Stub::create_instance
};

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GEOCLUE)
#include <com/ubuntu/location/providers/geoclue/provider.h>
static FactoryInjector geoclue_injector
{
    "geoclue::Provider", 
    com::ubuntu::location::providers::geoclue::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GEOCLUE

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_MLS)
#include <com/ubuntu/location/providers/mls/provider.h>
static FactoryInjector mls_injector
{
    "mls::Provider",
    com::ubuntu::location::providers::mls::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_MLS

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS)
#include <com/ubuntu/location/providers/gps/provider.h>
static FactoryInjector gps_injector
{
    "gps::Provider", 
    com::ubuntu::location::providers::gps::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK)
#include <com/ubuntu/location/providers/skyhook/provider.h>
static FactoryInjector skyhook_injector
{
    "skyhook::Provider", 
    com::ubuntu::location::providers::skyhook::Provider::create_instance
};
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK

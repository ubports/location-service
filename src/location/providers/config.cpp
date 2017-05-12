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
#include <location/provider_registry.h>

#include "dummy/provider.h"
#include "mls/provider.h"
#include "sirf/provider.h"
#include "ubx/provider.h"

#include <map>

namespace
{

template<typename T>
struct ProviderRegistrar
{
    ProviderRegistrar()
    {
        T::add_to_registry();
    }
};

}  // namespace

static ProviderRegistrar<location::providers::dummy::Provider> dummy_registrar{};
static ProviderRegistrar<location::providers::mls::Provider> mls_registrar{};
static ProviderRegistrar<location::providers::sirf::Provider> sirf_registrar{};
static ProviderRegistrar<location::providers::ubx::Provider> ubx_registrar{};

#if defined(LOCATION_PROVIDERS_GPS)
#include <location/providers/gps/provider.h>
static ProviderRegistrar<location::providers::gps::Provider> gps_registrar{};
#endif // LOCATION_PROVIDERS_GPS

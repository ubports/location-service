/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com>
 */

#ifndef LOCATION_PROVIDERS_DUMMY_DELAYED_PROVIDER_H_
#define LOCATION_PROVIDERS_DUMMY_DELAYED_PROVIDER_H_

#include "provider.h"

namespace location
{
namespace providers
{
namespace dummy
{
    struct DelayConfiguration
    {
        struct Keys
        {
            static constexpr const char* delay
            {
                "DelayInMs"
            };
        };
    };

class DelayedProvider : public Provider
{
    public:
        // For integration with the Provider factory
        static std::string class_name();
        // Waits for "DelayInMs" from the provided property bundle then
        // instantiates a new provider instance, populating the configuration object
        // from the provided property bundle.
        static DelayedProvider::Ptr create_instance(const ProviderFactory::Configuration&);
        // Creates a new provider instance from the given configuration.
        DelayedProvider(const Configuration config = Configuration{});
};
}
}
}

#endif // LOCATION_PROVIDERS_DUMMY_DELAYED_PROVIDER_H_

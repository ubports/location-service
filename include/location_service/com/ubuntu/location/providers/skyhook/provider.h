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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_

#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_factory.h"

#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace skyhook
{
class Provider : public com::ubuntu::location::Provider
{
  public:
    static Provider::Ptr create_instance(const ProviderFactory::Configuration& config);
    
    static const Provider::FeatureFlags& default_feature_flags();
    static const Provider::RequirementFlags& default_requirement_flags();

    struct Configuration
    {
        static std::string key_username() { return "username"; }
        static std::string key_realm() { return "realm"; }
        static std::string key_period() { return "period"; }

        std::string user_name;
        std::string realm;
        std::chrono::milliseconds period;
    };

    Provider(const Configuration& config);
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;
    ~Provider() noexcept;

    virtual bool matches_criteria(const Criteria&);

    virtual void start_position_updates();
    virtual void stop_position_updates();

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates();

    virtual void start_heading_updates();
    virtual void stop_heading_updates();

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_

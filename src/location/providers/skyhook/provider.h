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
 *              Manuel de la Pena <manuel.delapena@canonical.com>
 */
#ifndef LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_
#define LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_factory.h>

#include <memory>

namespace location
{
namespace providers
{
namespace skyhook
{
class Provider : public location::Provider
{
    enum class State
    {
        stopped,
        started,
        stop_requested
    };

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

    virtual bool matches_criteria(const Criteria&) override;

    virtual void start_position_updates() override;
    virtual void stop_position_updates() override;

    virtual void start_velocity_updates() override;
    virtual void stop_velocity_updates() override;

    virtual void start_heading_updates() override;
    virtual void stop_heading_updates() override;

 private:
    static WPS_Continuation periodic_callback(
            void* context,
            WPS_ReturnCode code,
            const WPS_Location* location,
            const void*);

    void start();
    void stop();

  private:

    Configuration config;
    State state;
    WPS_SimpleAuthentication authentication;
    std::thread worker;
};
}
}
}

#endif // LOCATION_PROVIDERS_SKYHOOK_PROVIDER_H_

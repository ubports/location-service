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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_ESPOO_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_ESPOO_PROVIDER_H_

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

namespace com
{

namespace ubuntu
{

namespace location
{

namespace providers
{

namespace espoo
{

class Provider : public com::ubuntu::location::Provider
{
  public:
    // For integration with the Provider factory.
    static std::string class_name();

    // Instantiates a new provider instance, populating the configuration object
    // from the provided property bundle.
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    // structure that represents the configuration used in the espoo provider
    struct Configuration
    {
        static std::string key_name() { return "name"; }
        static std::string key_path() { return "path"; }
        std::string name;
        std::string path;

        Provider::Features features = Provider::Features::none;
        Provider::Requirements requirements = Provider::Requirements::none;
    };

    Provider(const Configuration& config);
    ~Provider() noexcept;

    virtual bool matches_criteria(const Criteria&);

    virtual void start_position_updates();
    virtual void stop_position_updates();

  private:
    struct Private;
    std::unique_ptr<Private> d;
};

}  // espoo

}  // providers

}  // location

}  // ubuntu

}  // com
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GEOCLUE_PROVIDER_H_

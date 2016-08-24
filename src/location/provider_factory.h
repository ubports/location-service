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
#ifndef LOCATION_PROVIDER_FACTORY_H_
#define LOCATION_PROVIDER_FACTORY_H_

#include <location/configuration.h>
#include <location/provider.h>
#include <location/visibility.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace location
{
class Provider;

class LOCATION_DLL_PUBLIC ProviderFactory
{
  public:
    typedef location::Configuration Configuration;
    typedef std::function<Provider::Ptr(const Configuration&)> Factory;

    static ProviderFactory& instance();

    // Makes the given factory functor known for the given name.
    void add_factory_for_name(const std::string& name, const Factory& factory);

    // Tries to lookup the factory for the given name, and create a provider instance
    // by calling the factory and passing it the given config. Please note that the name
    // can be decorated with @ for distinguishing providers of the same type but different configs.
    // Eg.:
    // --provider=remote::Provider@espoo --remote::Provider@espoo::name="com.ubuntu.espoo.service.Provider" --remote::Provider@espoo::path="/espoo/service/Provider"
    // --provider=remote::Provider@gps --remote::Provider@gps::name="com.ubuntu.android.gps.Provider" --remote::Provider@gps::path="/android/gps/Provider"
    Provider::Ptr create_provider_for_name_with_config(const std::string& name, const Configuration& config);

    // Async version of above.
    void create_provider_for_name_with_config(const std::string& name, const Configuration& config, const std::function<void(Provider::Ptr)>& cb);

    void enumerate(const std::function<void(const std::string&, const Factory&)>& enumerator);

    static std::string extract_undecorated_name(const std::string& name);

  private:
    ProviderFactory() = default;
    ~ProviderFactory() = default;

    ProviderFactory(const ProviderFactory&) = delete;
    ProviderFactory& operator=(const ProviderFactory&) = delete;

    std::mutex guard;
    std::map<std::string, Factory> factory_store;
};
}

#endif // LOCATION_PROVIDER_FACTORY_H_

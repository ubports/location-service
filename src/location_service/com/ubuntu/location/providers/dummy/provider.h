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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_DUMMY_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_DUMMY_PROVIDER_H_

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
namespace dummy
{

struct Configuration
{
    inline static const char* key_update_period()
    {
        return "dummy::UpdatePeriodInMs";
    }

    inline static const char* key_reference_position_lat()
    {
        return "dummy::ReferenceLocationLat";
    }

    inline static const char* key_reference_position_lon()
    {
        return "dummy::ReferenceLocationLon";
    }

    std::chrono::milliseconds update_period{500};

    Position reference_position
    {
        wgs84::Latitude{9. * units::Degrees},
        wgs84::Longitude{53. * units::Degrees},
        wgs84::Altitude{-2. * units::Meters}
    };
};

class Provider : public com::ubuntu::location::Provider
{
  public:
    // For integration with the Provider factory.
    static std::string class_name();
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    Provider(const Configuration& config = Configuration{});
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;
    ~Provider() noexcept;

    // From Provider
    bool matches_criteria(const Criteria&);

    void start_position_updates();
    void stop_position_updates();

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_DUMMY_PROVIDER_H_

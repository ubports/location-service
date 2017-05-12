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
#ifndef LOCATION_PROVIDERS_GPS_PROVIDER_H_
#define LOCATION_PROVIDERS_GPS_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_registry.h>

#include "hardware_abstraction_layer.h"

namespace location
{
namespace providers
{
namespace gps
{

class Provider : public location::Provider
{
  public:
    // For integration with the Provider factory.
    static void add_to_registry();

    static Provider::Ptr create_instance(const util::settings::Source& source);

    Provider(const std::shared_ptr<HardwareAbstractionLayer>& hal = HardwareAbstractionLayer::create_default_instance());
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;
    ~Provider() noexcept;

    void on_new_event(const Event& event) override;

    void enable() override;
    void disable() override;
    void activate() override;
    void deactivate() override;

    Requirements requirements() const override;
    bool satisfies(const Criteria& criteria) override;
    const core::Signal<Update<Position>>& position_updates() const override;
    const core::Signal<Update<units::Degrees>>& heading_updates() const override;
    const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

    const core::Signal<Update<std::set<SpaceVehicle>>>& svs_updates() const;
    void on_reference_location_updated(const Update<Position>& position);

  private:
    std::shared_ptr<HardwareAbstractionLayer> hal;
    struct
    {
        core::Signal<Update<Position>> position;
        core::Signal<Update<units::Degrees>> heading;
        core::Signal<Update<units::MetersPerSecond>> velocity;
        core::Signal<Update<std::set<SpaceVehicle>>> svs;
    } updates;
};

}
}
}

#endif // LOCATION_PROVIDERS_GPS_PROVIDER_H_

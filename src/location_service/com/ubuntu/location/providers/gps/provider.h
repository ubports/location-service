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

#pragma once

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

#include "hardware_abstraction_layer.h"

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace gps
{

class Provider : public com::ubuntu::location::Provider
{
  public:
    // For integration with the Provider factory.
    static std::string class_name();
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    Provider(const std::shared_ptr<HardwareAbstractionLayer>& hal = HardwareAbstractionLayer::create_default_instance());
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;
    ~Provider() noexcept;

    bool matches_criteria(const Criteria&);

    void start_position_updates();
    void stop_position_updates();

    void start_velocity_updates();
    void stop_velocity_updates();

    void start_heading_updates();
    void stop_heading_updates();

    void on_reference_location_updated(const Update<Position>& position);

  private:
    std::shared_ptr<HardwareAbstractionLayer> hal;
};
}
}
}
}
}

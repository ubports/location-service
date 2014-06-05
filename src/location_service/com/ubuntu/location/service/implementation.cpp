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
#include <com/ubuntu/location/service/implementation.h>

#include <com/ubuntu/location/connectivity/manager.h>
#include <com/ubuntu/location/service/harvester.h>

#include <com/ubuntu/location/service/session/implementation.h>

#include <com/ubuntu/location/criteria.h>
#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/proxy_provider.h>

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

#include <chrono>
#include <functional>
#include <sstream>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

namespace dbus = core::dbus;

struct culs::Implementation::Private
{
    Private(const dbus::Bus::Ptr& connection,
            const cul::Engine::Ptr& engine,
            const culs::PermissionManager::Ptr& permission_manager,
            const culs::Harvester::Reporter::Ptr& reporter)
        : bus{connection},
          engine{engine},
          permission_manager{permission_manager},
          harvester
          {
              Harvester::Configuration
              {
                  engine,
                  connectivity::platform_default_manager(),
                  reporter
              }
          }
    {
        harvester.start();
    }

    dbus::Bus::Ptr bus;
    cul::Engine::Ptr engine;
    culs::PermissionManager::Ptr permission_manager;
    culs::Harvester harvester;
};

culs::Implementation::Implementation(
    const dbus::Bus::Ptr& bus,
    const cul::Engine::Ptr& engine,
    const culs::PermissionManager::Ptr& permission_manager,
    const Harvester::Reporter::Ptr& reporter)
    : Skeleton(bus, permission_manager),
      d{new Private{bus, engine, permission_manager, reporter}}
{
    if (!bus)
        throw std::runtime_error("Cannot create service for null bus.");
    if (!engine)
        throw std::runtime_error("Cannot create service for null engine.");
    if (!permission_manager)
        throw std::runtime_error("Cannot create service for null permission manager.");

    is_online() =
            engine->configuration.engine_state == Engine::Status::on ||
            engine->configuration.engine_state == Engine::Status::active;
    is_online().changed().connect(
                [this](bool value)
                {
                    d->engine->configuration.engine_state
                            = value ?
                                Engine::Status::on :
                                Engine::Status::off;
                });
    does_report_cell_and_wifi_ids() =
            d->engine->configuration.wifi_and_cell_id_reporting_state ==
            cul::WifiAndCellIdReportingState::on;
    does_report_cell_and_wifi_ids().changed().connect(
                [this](bool value)
                {
                    d->engine->configuration.wifi_and_cell_id_reporting_state
                            = value ?
                                cul::WifiAndCellIdReportingState::on :
                                cul::WifiAndCellIdReportingState::off;
                });
    does_satellite_based_positioning() =
            d->engine->configuration.satellite_based_positioning_state ==
            cul::SatelliteBasedPositioningState::on;
    does_satellite_based_positioning().changed().connect(
                [this](bool value)
                {
                    d->engine->configuration.satellite_based_positioning_state
                            = value ?
                                cul::SatelliteBasedPositioningState::on :
                                cul::SatelliteBasedPositioningState::off;
                });
    engine->configuration.engine_state.changed().connect(
                [this](Engine::Status status)
                {
                    is_online() =
                            status == Engine::Status::on ||
                            status == Engine::Status::active;
                });
    engine->configuration.satellite_based_positioning_state.changed().connect(
                [this](cul::SatelliteBasedPositioningState state)
                {
                    does_satellite_based_positioning() =
                            state == cul::SatelliteBasedPositioningState::on;
                });
    engine->updates.visible_space_vehicles.changed().connect(
                [this](const std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>&svs)
                {
                    VLOG(10) << "Receiving update on visible space vehicles.";
                    visible_space_vehicles() = svs;
                });
}

culs::Implementation::~Implementation() noexcept
{
}

culs::session::Interface::Ptr culs::Implementation::create_session_for_criteria(const cul::Criteria& criteria)
{
    auto provider_selection
            = d->engine->determine_provider_selection_for_criteria(criteria);
    auto proxy_provider = ProxyProvider::Ptr
    {
        new ProxyProvider{provider_selection}
    };

    return session::Interface::Ptr{new culs::session::Implementation(proxy_provider)};
}

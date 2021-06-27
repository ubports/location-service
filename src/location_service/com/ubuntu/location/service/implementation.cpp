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

culs::Implementation::Implementation(const culs::Implementation::Configuration& config)
    : Skeleton
      {
          Skeleton::Configuration
          {
              config.incoming,
              config.outgoing,
              culs::Skeleton::CredentialsResolver::Ptr
              {
                  new culs::Skeleton::DBusDaemonCredentialsResolver
                  {
                      config.outgoing,
                      culs::Skeleton::DBusDaemonCredentialsResolver::libapparmor_profile_resolver(),
                  }
              },
              culs::Skeleton::ObjectPathGenerator::Ptr
              {
                  new culs::Skeleton::ObjectPathGenerator{}
              },
              config.permission_manager
          }
      },
      configuration(config),
      harvester(config.harvester),
      connections
      {
          is_online().changed().connect(
              [this](bool value)
              {
                  configuration.engine->configuration.engine_state
                        = value ?
                            Engine::Status::on :
                            Engine::Status::off;
              }),
          does_report_cell_and_wifi_ids().changed().connect(
              [this](bool value)
              {
                  configuration.engine->configuration.wifi_and_cell_id_reporting_state
                      = value ?
                          cul::WifiAndCellIdReportingState::on :
                          cul::WifiAndCellIdReportingState::off;

                  if (value)
                      harvester.start();
                  else
                      harvester.stop();
              }),
          does_satellite_based_positioning().changed().connect(
              [this](bool value)
              {
                  configuration.engine->configuration.satellite_based_positioning_state
                          = value ?
                              cul::SatelliteBasedPositioningState::on :
                              cul::SatelliteBasedPositioningState::off;
              }),
          configuration.engine->configuration.engine_state.changed().connect(
              [this](Engine::Status status)
              {
                  switch (status)
                  {
                      case Engine::Status::off:
                          is_online() = false;
                          mutable_state() = State::disabled;
                          break;
                      case Engine::Status::on:
                          is_online() = true;
                          mutable_state() = State::enabled;
                          break;
                      case Engine::Status::active:
                          is_online() = true;
                          mutable_state() = State::active;
                          break;
                  }
              }),
          configuration.engine->configuration.satellite_based_positioning_state.changed().connect(
              [this](cul::SatelliteBasedPositioningState state)
              {
                  does_satellite_based_positioning() =
                          state == cul::SatelliteBasedPositioningState::on;
              }),
          configuration.engine->updates.visible_space_vehicles.changed().connect(
              [this](const std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle>&svs)
              {
                  visible_space_vehicles() = svs;
              }),
          configuration.engine->updates.last_known_location.changed().connect(
              [this](const cul::Optional<cul::Update<cul::Position>>& update)
              {
                  if (update)
                  {
                      harvester.report_position_update(update.get());
                  }
              })
      }
{
    if (!configuration.incoming)
        throw std::runtime_error("Cannot create service for null bus.");
    if (!configuration.outgoing)
        throw std::runtime_error("Cannot create service for null bus.");
    if (!configuration.engine)
        throw std::runtime_error("Cannot create service for null engine.");
    if (!configuration.permission_manager)
        throw std::runtime_error("Cannot create service for null permission manager.");

    switch (configuration.engine->configuration.engine_state)
    {
        case Engine::Status::off:
            is_online() = false;
            mutable_state() = State::disabled;
            break;
        case Engine::Status::on:
            is_online() = true;
            mutable_state() = State::enabled;
            break;
          case Engine::Status::active:
            is_online() = true;
            mutable_state() = State::active;
            break;
    };

    does_report_cell_and_wifi_ids() =
            configuration.engine->configuration.wifi_and_cell_id_reporting_state ==
            cul::WifiAndCellIdReportingState::on;
    does_satellite_based_positioning() =
            configuration.engine->configuration.satellite_based_positioning_state ==
            cul::SatelliteBasedPositioningState::on;
}

culs::session::Interface::Ptr culs::Implementation::create_session_for_criteria(const cul::Criteria& criteria)
{
    auto provider_selection
            = configuration.engine->determine_provider_selection_for_criteria(criteria);
    auto proxy_provider = ProxyProvider::Ptr
    {
        new ProxyProvider{provider_selection}
    };

    return session::Interface::Ptr{new culs::session::Implementation(proxy_provider)};
}

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
#include <location/service/implementation.h>

#include <location/connectivity/manager.h>
#include <location/service/harvester.h>

#include <location/service/session/implementation.h>

#include <location/criteria.h>
#include <location/engine.h>
#include <location/logging.h>
#include <location/proxy_provider.h>

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

#include <chrono>
#include <functional>
#include <sstream>

namespace dbus = core::dbus;

location::service::Implementation::Implementation(const location::service::Implementation::Configuration& config)
    : Skeleton
      {
          Skeleton::Configuration
          {
              config.incoming,
              config.outgoing,
              location::service::Skeleton::CredentialsResolver::Ptr
              {
                  new location::service::Skeleton::DBusDaemonCredentialsResolver
                  {
                      config.outgoing
                  }
              },
              location::service::Skeleton::ObjectPathGenerator::Ptr
              {
                  new location::service::Skeleton::ObjectPathGenerator{}
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
                          location::WifiAndCellIdReportingState::on :
                          location::WifiAndCellIdReportingState::off;

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
                              location::SatelliteBasedPositioningState::on :
                              location::SatelliteBasedPositioningState::off;
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
              [this](location::SatelliteBasedPositioningState state)
              {
                  does_satellite_based_positioning() =
                          state == location::SatelliteBasedPositioningState::on;
              }),
          configuration.engine->updates.visible_space_vehicles.changed().connect(
              [this](const std::map<location::SpaceVehicle::Key, location::SpaceVehicle>&svs)
              {
                  visible_space_vehicles() = svs;
              }),
          configuration.engine->updates.last_known_location.changed().connect(
              [this](const location::Optional<location::Update<location::Position>>& update)
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
            location::WifiAndCellIdReportingState::on;
    does_satellite_based_positioning() =
            configuration.engine->configuration.satellite_based_positioning_state ==
            location::SatelliteBasedPositioningState::on;
}

location::service::session::Interface::Ptr location::service::Implementation::create_session_for_criteria(const location::Criteria& criteria)
{
    auto provider_selection
            = configuration.engine->determine_provider_selection_for_criteria(criteria);
    auto proxy_provider = ProxyProvider::Ptr
    {
        new ProxyProvider{provider_selection}
    };

    session::Interface::Ptr session_iface{new session::Implementation(proxy_provider)};
    std::weak_ptr<session::Interface> session_weak{session_iface};
    session_iface->updates().position_status.changed().connect([this, session_weak](const session::Interface::Updates::Status& status)
    {
        location::Optional<location::Update<location::Position>> last_known_position = configuration.engine->updates.last_known_location.get();
        bool has_last_known_position = last_known_position ? true : false;
        bool is_session_enabled = status == location::service::session::Interface::Updates::Status::enabled;
        bool is_session_on_or_active = configuration.engine->configuration.engine_state != Engine::Status::off;

        if (has_last_known_position && is_session_enabled && is_session_on_or_active)
        {
            // Immediately send the last known position to the client
            if (auto session_iface = session_weak.lock())
            {
                session_iface->updates().position = last_known_position.get();
            }
        }
    });
    return session_iface;
}

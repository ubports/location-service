/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/proxy_provider.h>
#include <location/service_with_engine.h>
#include <location/session_with_provider.h>

location::ServiceWithEngine::ServiceWithEngine(const Engine::Ptr& engine)
    : engine{engine},
      connections
      {
          is_online_.changed().connect(
              [this](bool value)
              {
                  ServiceWithEngine::engine->configuration.engine_state
                        = value ?
                            Engine::Status::on :
                            Engine::Status::off;
              }),
          does_report_cell_and_wifi_ids_.changed().connect(
              [this](bool value)
              {
                  ServiceWithEngine::engine->configuration.wifi_and_cell_id_reporting_state
                      = value ?
                          location::WifiAndCellIdReportingState::on :
                          location::WifiAndCellIdReportingState::off;
              }),
          does_satellite_based_positioning_.changed().connect(
              [this](bool value)
              {
                  ServiceWithEngine::engine->configuration.satellite_based_positioning_state
                          = value ?
                              location::SatelliteBasedPositioningState::on :
                              location::SatelliteBasedPositioningState::off;
              }),
          engine->configuration.engine_state.changed().connect(
              [this](Engine::Status status)
              {
                  switch (status)
                  {
                      case Engine::Status::off:
                          is_online_ = false;
                          state_ = State::disabled;
                          break;
                      case Engine::Status::on:
                          is_online_ = true;
                          state_ = State::enabled;
                          break;
                      case Engine::Status::active:
                          is_online_ = true;
                          state_ = State::active;
                          break;
                  }
              }),
          engine->configuration.satellite_based_positioning_state.changed().connect(
              [this](location::SatelliteBasedPositioningState state)
              {
                  does_satellite_based_positioning_ =
                          state == location::SatelliteBasedPositioningState::on;
              }),
          engine->updates.visible_space_vehicles.changed().connect(
              [this](const std::map<location::SpaceVehicle::Key, location::SpaceVehicle>&svs)
              {
                  visible_space_vehicles_ = svs;
              })
      }
{
}

const core::Property<location::Service::State>& location::ServiceWithEngine::state() const
{
    return state_;
}

core::Property<bool>& location::ServiceWithEngine::does_satellite_based_positioning()
{
    return does_satellite_based_positioning_;
}

core::Property<bool>& location::ServiceWithEngine::is_online()
{
    return is_online_;
}

core::Property<bool>& location::ServiceWithEngine::does_report_cell_and_wifi_ids()
{
    return does_report_cell_and_wifi_ids_;
}

core::Property<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>& location::ServiceWithEngine::visible_space_vehicles()
{
    return visible_space_vehicles_;
}

location::Service::Session::Ptr location::ServiceWithEngine::create_session_for_criteria(const Criteria& criteria)
{
    auto proxy_provider = std::make_shared<ProxyProvider>(engine->determine_provider_selection_for_criteria(criteria));
    auto session = std::make_shared<SessionWithProvider>(proxy_provider);
    std::weak_ptr<Session> wp{session};

    session->updates().position_status.changed().connect([this, wp](const Session::Updates::Status& status)
    {
        location::Optional<location::Update<location::Position>> last_known_position = engine->updates.last_known_location.get();
        bool has_last_known_position = last_known_position ? true : false;
        bool is_session_enabled = status == Session::Updates::Status::enabled;
        bool is_session_on_or_active = engine->configuration.engine_state != Engine::Status::off;

        if (has_last_known_position && is_session_enabled && is_session_on_or_active)
            if (auto sp = wp.lock()) // Immediately send the last known position to the client
                sp->updates().position = last_known_position.get();
    });

    return session;
}

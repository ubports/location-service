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

#include <location/session_with_provider.h>

location::SessionWithProvider::SessionWithProvider(const Provider::Ptr& provider)
    : provider_{provider},
      connections_
      {
          provider_->updates().position.connect(
              [this](const Update<Position>& update)
              {
                  updates().position = update;
              }),
          provider_->updates().heading.connect(
              [this](const Update<Heading>& update)
              {
                  updates().heading = update;
              }),
          provider_->updates().velocity.connect(
              [this](const Update<Velocity>& update)
              {
                  updates().velocity = update;
              }),
          updates().position_status.changed().connect(
              [this](const Updates::Status& status)
              {
                  switch(status)
                  {
                  case Updates::Status::enabled:
                      start_position_updates(); break;
                  case Updates::Status::disabled:
                      stop_position_updates(); break;
                  }
              }),
          updates().velocity_status.changed().connect(
              [this](const Updates::Status& status)
              {
                  switch(status)
                  {
                  case Updates::Status::enabled:
                      start_velocity_updates(); break;
                  case Updates::Status::disabled:
                      stop_velocity_updates(); break;
                  }
              }),
          updates().heading_status.changed().connect(
              [this](const Updates::Status& status)
              {
                  switch(status)
                  {
                  case Updates::Status::enabled:
                      start_heading_updates(); break;
                  case Updates::Status::disabled:
                      stop_heading_updates(); break;
                  }
              })
      }
{
}

location::Service::Session::Updates& location::SessionWithProvider::updates()
{
    return updates_;
}

void location::SessionWithProvider::start_position_updates()
{
    provider_->state_controller()->start_position_updates();
}

void location::SessionWithProvider::stop_position_updates()
{
    provider_->state_controller()->stop_position_updates();
}

void location::SessionWithProvider::start_velocity_updates()
{
    provider_->state_controller()->start_velocity_updates();
}

void location::SessionWithProvider::stop_velocity_updates()
{
    provider_->state_controller()->stop_velocity_updates();
}

void location::SessionWithProvider::start_heading_updates()
{
    provider_->state_controller()->start_heading_updates();
}

void location::SessionWithProvider::stop_heading_updates()
{
    provider_->state_controller()->stop_heading_updates();
}


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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com
 */
 #include <com/ubuntu/location/fusion_provider.h>
 #include <com/ubuntu/location/logging.h>
 #include <com/ubuntu/location/update.h>

namespace cu = com::ubuntu;
namespace cul = com::ubuntu::location;

cul::Provider::Features all_features()
{
    return cul::Provider::Features::position |
           cul::Provider::Features::heading |
           cul::Provider::Features::velocity;
}

cul::Provider::Requirements all_requirements()
{
    return cul::Provider::Requirements::cell_network |
           cul::Provider::Requirements::data_network |
           cul::Provider::Requirements::monetary_spending |
           cul::Provider::Requirements::satellites;
}


cul::FusionProvider::FusionProvider(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector)
    : Provider{all_features(), all_requirements()},
      providers{providers}
{

    for (auto provider : providers)
    {
        connections.push_back(provider->updates().position.connect(
              [this, update_selector](const cul::Update<cul::Position>& u)
              {
                  // if this is the first update, use it
                  if (!last_position) {
                      mutable_updates().position(*(last_position = u));
                  } else {
                      try {
                          mutable_updates().position(*(last_position = update_selector->select(*last_position, u)));
                      } catch (const std::exception& e) {
                          LOG(WARNING) << "Error while updating position";
                      }
                  }
              }));
        connections.push_back(provider->updates().heading.connect(
              [this](const cul::Update<cul::Heading>& u)
              {
                  mutable_updates().heading(u);
              }));
        connections.push_back(provider->updates().velocity.connect(
              [this](const cul::Update<cul::Velocity>& u)
              {
                  mutable_updates().velocity(u);
              }));
    }

}

// We always match :)
bool cul::FusionProvider::matches_criteria(const cul::Criteria&)
{
    return true;
}

// We forward all events to the other providers.
void cul::FusionProvider::on_wifi_and_cell_reporting_state_changed(cul::WifiAndCellIdReportingState state)
{
    for (auto provider : providers)
        provider->on_wifi_and_cell_reporting_state_changed(state);
}

void cul::FusionProvider::on_reference_location_updated(const cul::Update<cul::Position>& position)
{
    for (auto provider : providers)
        provider->on_reference_location_updated(position);
}

void cul::FusionProvider::on_reference_velocity_updated(const cul::Update<cul::Velocity>& velocity)
{
    for (auto provider : providers)
        provider->on_reference_velocity_updated(velocity);
}

void cul::FusionProvider::on_reference_heading_updated(const cul::Update<cul::Heading>& heading)
{
    for (auto provider : providers)
        provider->on_reference_heading_updated(heading);
}

void cul::FusionProvider::start_position_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_position_updates();
}

void cul::FusionProvider::stop_position_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_position_updates();
}

void cul::FusionProvider::start_heading_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_heading_updates();
}

void cul::FusionProvider::stop_heading_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_heading_updates();
}

void cul::FusionProvider::start_velocity_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_velocity_updates();
}

void cul::FusionProvider::stop_velocity_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_velocity_updates();
}

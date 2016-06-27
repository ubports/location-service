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
 #include <location/fusion_provider.h>
 #include <location/logging.h>
 #include <location/update.h>

location::Provider::Features all_features()
{
    return location::Provider::Features::position |
           location::Provider::Features::heading |
           location::Provider::Features::velocity;
}

location::Provider::Requirements all_requirements()
{
    return location::Provider::Requirements::cell_network |
           location::Provider::Requirements::data_network |
           location::Provider::Requirements::monetary_spending |
           location::Provider::Requirements::satellites;
}

location::FusionProvider::FusionProvider(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector)
    : Provider{all_features(), all_requirements()},
      providers{providers}
{

    for (auto provider : providers)
    {
        connections.push_back(provider->updates().position.connect(
              [this, provider, update_selector](const location::Update<location::Position>& u)
              {
                  // if this is the first update, use it
                  if (!last_position) {
                      mutable_updates().position((*(last_position = WithSource<Update<Position>>{provider, u})).value);
                      
                  // otherwise use the selector
                  } else {
                      try {
                          mutable_updates().position((*(last_position = update_selector->select(*last_position, WithSource<Update<Position>>{provider, u}))).value);
                      } catch (const std::exception& e) {
                          LOG(WARNING) << "Error while updating position";
                      }
                  }
              }));
        connections.push_back(provider->updates().heading.connect(
              [this](const location::Update<location::Heading>& u)
              {
                  mutable_updates().heading(u);
              }));
        connections.push_back(provider->updates().velocity.connect(
              [this](const location::Update<location::Velocity>& u)
              {
                  mutable_updates().velocity(u);
              }));
    }

}

// We always match :)
bool location::FusionProvider::matches_criteria(const location::Criteria&)
{
    return true;
}

// We forward all events to the other providers.
void location::FusionProvider::on_wifi_and_cell_reporting_state_changed(location::WifiAndCellIdReportingState state)
{
    for (auto provider : providers)
        provider->on_wifi_and_cell_reporting_state_changed(state);
}

void location::FusionProvider::on_reference_location_updated(const location::Update<location::Position>& position)
{
    for (auto provider : providers)
        provider->on_reference_location_updated(position);
}

void location::FusionProvider::on_reference_velocity_updated(const location::Update<location::Velocity>& velocity)
{
    for (auto provider : providers)
        provider->on_reference_velocity_updated(velocity);
}

void location::FusionProvider::on_reference_heading_updated(const location::Update<location::Heading>& heading)
{
    for (auto provider : providers)
        provider->on_reference_heading_updated(heading);
}

void location::FusionProvider::start_position_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_position_updates();
}

void location::FusionProvider::stop_position_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_position_updates();
}

void location::FusionProvider::start_heading_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_heading_updates();
}

void location::FusionProvider::stop_heading_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_heading_updates();
}

void location::FusionProvider::start_velocity_updates()
{
    for (auto provider : providers)
        provider->state_controller()->start_velocity_updates();
}

void location::FusionProvider::stop_velocity_updates()
{
    for (auto provider : providers)
        provider->state_controller()->stop_velocity_updates();
}

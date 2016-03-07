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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_FUSION_PROVIDER_H
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_FUSION_PROVIDER_H

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_selection_policy.h>
#include <com/ubuntu/location/update_selector.h>

namespace com
{
namespace ubuntu
{
namespace location
{
class FusionProvider : public Provider
{
public:
    typedef std::shared_ptr<FusionProvider> Ptr;

    FusionProvider(const std::set<Provider::Ptr> &providers, const UpdateSelector::Ptr update_selector);
    ~FusionProvider() = default;

    bool matches_criteria(const Criteria &criteria) override;
    void on_wifi_and_cell_reporting_state_changed(location::WifiAndCellIdReportingState state) override;
    void on_reference_location_updated(const Update<Position>& position) override;
    void on_reference_velocity_updated(const Update<Velocity>& velocity) override;
    void on_reference_heading_updated(const Update<Heading>& heading) override;
    void start_position_updates() override;
    void stop_position_updates() override;
    void start_heading_updates() override;
    void stop_heading_updates() override;
    void start_velocity_updates() override;
    void stop_velocity_updates() override;

private:
    Optional<Update<Position>> last_position;
    std::set<Provider::Ptr> providers;
    std::vector<core::ScopedConnection> connections;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_FUSION_PROVIDER_H

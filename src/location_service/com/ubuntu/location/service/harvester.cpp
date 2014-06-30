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

#include <com/ubuntu/location/service/harvester.h>

#include <com/ubuntu/location/logging.h>

namespace location = com::ubuntu::location;

location::service::Harvester::Harvester(const location::service::Harvester::Configuration& configuration)
    : config(configuration),
      is_running{false}
{
}

location::service::Harvester::~Harvester()
{
    stop();
}

/** @brief Report updated position to the harvester instance. */
void location::service::Harvester::report_position_update(const location::Update<location::Position>& update)
{
    VLOG(10) << "Reference location changed: " << update;

    if (not is_running.load())
        return;

    std::vector<location::connectivity::WirelessNetwork::Ptr> visible_wifis;
    config.connectivity_manager->enumerate_visible_wireless_networks([&visible_wifis](location::connectivity::WirelessNetwork::Ptr wifi)
    {
        VLOG(10) << "Got a visible wifi: " << *wifi << std::endl;
        visible_wifis.push_back(wifi);
    });

    std::vector<location::connectivity::RadioCell::Ptr> connected_cells;
    config.connectivity_manager->enumerate_connected_radio_cells([&connected_cells](location::connectivity::RadioCell::Ptr cell)
    {
        VLOG(10) << "Got a cell: " << *cell << std::endl;
        connected_cells.push_back(cell);
    });

    config.reporter->report(update, visible_wifis, connected_cells);
}

void location::service::Harvester::start()
{
    if (is_running.load())
        return;

    is_running.exchange(true);

    config.reporter->start();
}

void location::service::Harvester::stop()
{
    if (not is_running.load())
        return;

    is_running.exchange(false);

    config.reporter->stop();
}

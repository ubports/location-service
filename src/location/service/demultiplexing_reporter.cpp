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

#include <location/service/demultiplexing_reporter.h>

location::service::DemultiplexingReporter::DemultiplexingReporter(const std::set<location::service::Harvester::Reporter::Ptr>& reporters)
    : reporters{reporters}
{

}

// Tell the reporters that it should start operating.
void location::service::DemultiplexingReporter::start()
{
    std::lock_guard<std::mutex> lg{reporters_guard};
    for(auto reporter : reporters)
        reporter->start();
}

// Tell the reporters to shut down its operation.
void location::service::DemultiplexingReporter::stop()
{
    std::lock_guard<std::mutex> lg{reporters_guard};
    for(auto reporter : reporters)
        reporter->stop();
}

// Triggers the reporters to send off the information.
void location::service::DemultiplexingReporter::report(
        const location::Update<location::Position>& update,
        const std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wifis,
        const std::vector<com::ubuntu::location::connectivity::RadioCell::Ptr>& cells)
{
    std::lock_guard<std::mutex> lg{reporters_guard};
    for(auto reporter : reporters)
        reporter->report(update, wifis, cells);
}

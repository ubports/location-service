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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEMULTIPLEXING_REPORTER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEMULTIPLEXING_REPORTER_H_

#include <com/ubuntu/location/service/harvester.h>

namespace com{namespace ubuntu{namespace location{namespace service
{

// A simple demultiplexer, distributing updates to a set of
// actual reporter implementations.
class DemultiplexingReporter : public Harvester::Reporter
{
public:
    DemultiplexingReporter(const std::set<Harvester::Reporter::Ptr>& reporters);

    // Tell the reporters that it should start operating.
    void start() override;

    // Tell the reporters to shut down its operation.
    void stop() override;

    // Triggers the reporters to send off the information.
    void report(const Update<Position>& update,
                const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
                const std::vector<connectivity::RadioCell::Ptr>& cells) override;
private:
    std::mutex reporters_guard;
    std::set<Harvester::Reporter::Ptr> reporters;
};
}}}}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEMULTIPLEXING_REPORTER_H_

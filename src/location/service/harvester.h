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
#ifndef LOCATION_SERVICE_HARVESTER_H_
#define LOCATION_SERVICE_HARVESTER_H_

#include <location/engine.h>
#include <location/update.h>
#include <location/connectivity/manager.h>

namespace location
{
namespace service
{
/** @brief Models a wifi- and cell-id harvester for crowd-sourcing purposes. */
class Harvester
{
public:

    /** @brief Models a reporter of position updates, augmented with wifi and cell ids. */
    struct Reporter
    {
        /** @cond */
        typedef std::shared_ptr<Reporter> Ptr;

        Reporter() = default;
        virtual ~Reporter() = default;
        /** @endcond */

        /** @brief Tell the reporter that it should start operating. */
        virtual void start() = 0;

        /** @brief Tell the reporter to shut down its operation. */
        virtual void stop() = 0;

        /**
         * @brief Triggers the reporter to send off the information.
         */
        virtual void report(const Update<Position>& update,
                            const std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wifis,
                            const std::vector<com::ubuntu::location::connectivity::RadioCell::Ptr>& cells) = 0;
    };

    /** @brief Configuration encapsulates all creation time options of class Harvester */
    struct Configuration
    {
        /** The connectivity manager that the harvester should use. */
        std::shared_ptr<com::ubuntu::location::connectivity::Manager> connectivity_manager;
        /** The reporter implementation */
        std::shared_ptr<Reporter> reporter;
    };

    /** @brief Creates a new instance and wires up to system components for receiving
     *  location updates, and wifi and cell id measurements.
     */
    Harvester(const Configuration& configuration);

    /** @brief Stops the data collection and frees all resources held by the instance. */
    virtual ~Harvester();

    /** @brief Report updated position to the harvester instance. */
    virtual void report_position_update(const Update<Position>& update);

    /** @brief Starts the harvester instance and its data collection. */
    virtual void start();

    /** @brief Stops the harvester instance and its data collection. */
    virtual void stop();

private:
    Configuration config;
    std::atomic<bool> is_running;
};
}
}

#endif // LOCATION_SERVICE_HARVESTER_H_

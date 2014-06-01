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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_HARVESTER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_HARVESTER_H_

#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/connectivity/manager.h>

#include <com/ubuntu/location/logging.h>

#include <atomic>

namespace com
{
namespace ubuntu
{
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
                            const std::vector<connectivity::WirelessNetwork::Ptr>& wifis,
                            const std::vector<connectivity::RadioCell>& cells) = 0;
    };

    /** @brief Configuration encapsulates all creation time options of class Harvester */
    struct Configuration
    {
        /** The position engine that the harvester should use. */
        std::shared_ptr<Engine> engine;
        /** The connectivity manager that the harvester should use. */
        std::shared_ptr<connectivity::Manager> connectivity_manager;
        /** The reporter implementation */
        std::shared_ptr<Reporter> reporter;
    };

    /** @brief Creates a new instance and wires up to system components for receiving
     *  location updates, and wifi and cell id measurements.
     */
    Harvester(const Configuration& configuration)
        : config(configuration),
          is_running{false}
    {
        config.engine->updates.reference_location.changed().connect([this](const Update<Position>& update)
        {
            VLOG(10) << "Reference location changed: " << update;

            if (not is_running.load())
                return;

            auto visible_wifis = config.connectivity_manager->visible_wireless_networks().get();
            auto connected_cells = config.connectivity_manager->connected_radio_cells().get();

            config.reporter->report(update, visible_wifis, connected_cells);
        });
    }

    virtual ~Harvester()
    {
        stop();
    }

    virtual void start()
    {
        if (is_running.load())
            return;

        is_running.exchange(true);

        config.reporter->start();
    }

    virtual void stop()
    {
        if (not is_running.load())
            return;

        is_running.exchange(false);

        config.reporter->stop();
    }

private:
    Configuration config;
    std::atomic<bool> is_running;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_HARVESTER_H_

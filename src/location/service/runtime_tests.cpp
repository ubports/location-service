/*
 * Copyright © 2015 Canonical Ltd.
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

#include <location/service/runtime.h>
#include <location/service/runtime_tests.h>

#include <location/clock.h>
#include <location/providers/gps/hardware_abstraction_layer.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <cstdlib>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace gps = location::providers::gps;

namespace
{
template<bool expected, typename Exception>
void expect(bool value, const std::string& what)
{
    if (value != expected) throw Exception{what};
}

// Poor man's testing fixture, taking care of shutting down the currently
// running service instance and restarting it after the tests have been run.
struct Fixture
{
    Fixture()
    {
        // We need to make sure that we are running as root. In addition, we will stop
        // any running location service instance prior to executing the test.
        expect<true, std::logic_error>(::getuid() == 0, "This test has to be run as root.");

        int rc = ::system("service ubuntu-location-service stop");
        // We consciously ignore any issues and assume that we are good to go.
        (void) rc;
    }

    ~Fixture()
    {
        int rc = ::system("service ubuntu-location-service start");
        // We consciously ignore any issues reported by the call to system
        // as we would make an otherwise fine test-suite failing just because
        // an "uninteresting" post-condition is not satisfied.
        (void) rc;
    }
};

#if defined(COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS)
int snr_and_ttff()
{
    typedef boost::accumulators::accumulator_set<
        double,
        boost::accumulators::stats<
            boost::accumulators::tag::mean,
            boost::accumulators::tag::variance
        >
    > Statistics;

    using boost::accumulators::mean;
    using boost::accumulators::variance;

    static const unsigned int trials = 3;

    Statistics stats;
    auto hal = gps::HardwareAbstractionLayer::create_default_instance();

    struct State
    {
        State() : fix_received(false)
        {
        }

        bool wait_for_fix_for(const std::chrono::seconds& seconds)
        {
            std::unique_lock<std::mutex> ul(guard);
            return wait_condition.wait_for(
                        ul,
                        seconds,
                        [this]() {return fix_received == true;});
        }

        void on_position_updated(const location::Position&)
        {
            fix_received = true;
            wait_condition.notify_all();
        }

        void reset()
        {
            fix_received = false;
        }

        std::mutex guard;
        std::condition_variable wait_condition;
        bool fix_received;
    } state;

    // We want to run in standalone mode
    hal->set_assistance_mode(gps::AssistanceMode::standalone);

    // We wire up our state to position updates from the hal.
    hal->position_updates().connect([&state](const location::Position& pos)
    {
        state.on_position_updated(pos);
    });

    // We report updates to the visible space vehicles here.
    hal->space_vehicle_updates().connect([](const std::set<location::SpaceVehicle>& svs)
    {
        std::cout << std::scientific;
        std::cout << "key snr has_almanac_data has_ephimeris_data used_in_fix azimuth elevation" << std::endl;
        for (const auto& sv : svs)
            std::cout << sv.key.id << " " << sv.snr << " " << sv.has_almanac_data << " " << sv.has_ephimeris_data << " " << sv.used_in_fix << " " << sv.azimuth.value() << " " << sv.elevation.value() << std::endl;
    });

    for (unsigned int i = 0; i < trials; i++)
    {
        // We want to force a cold start per trial.
        hal->delete_all_aiding_data();

        state.reset();
        auto start = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());
        {
            hal->start_positioning();
            // We expect a maximum cold start time of 15 minutes. The theoretical
            // limit is 12.5 minutes, and we add up some grace period to make the
            // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
            expect<true, std::runtime_error>(state.wait_for_fix_for(std::chrono::seconds{15 * 60}), "Wait for fix timed out.");
            hal->stop_positioning();
        }
        auto stop = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());

        stats((stop - start).count());
    }

    std::cout << "Mean time to first fix in [ms]: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::microseconds(
                         static_cast<std::uint64_t>(mean(stats)))).count()
              << std::endl;
    std::cout << "Variance in time to first fix in [ms]: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::microseconds(
                         static_cast<std::uint64_t>(variance(stats)))).count()
              << std::endl;

    return 0;
}
#else
int snr_and_ttff() { return 0; }
#endif // COM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS
}

int location::service::execute_runtime_tests()
{
    Fixture fixture; // This throws in case of issues.
    auto rc = snr_and_ttff(); if (rc != 0) return rc;
    // Other runtime tests go here;
    return rc;
}

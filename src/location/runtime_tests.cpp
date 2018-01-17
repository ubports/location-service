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

#include <location/runtime.h>
#include <location/runtime_tests.h>

#include <location/clock.h>
#include <location/glib/runtime.h>
#include <location/providers/gps/hardware_abstraction_layer.h>
//#include <location/providers/sirf/provider.h>
#include <location/providers/ubx/provider.h>
#include <location/util/benchmark.h>
#include <location/util/cli.h>

#include <core/posix/this_process.h>

#include <boost/lexical_cast.hpp>

#include <cstdlib>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace ba = boost::accumulators;
namespace cli = location::util::cli;
namespace env = core::posix::this_process::env;
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

#if defined(LOCATION_PROVIDERS_GPS)
int snr_and_ttff(std::ostream& cout, std::ostream& cerr)
{
    location::util::Benchmark benchmark{boost::lexical_cast<unsigned int>(env::get("SNR_AND_TTF_PROVIDER_TEST_TRIALS", "15")), "ttff in [µs]"};

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
    hal->space_vehicle_updates().connect([&cout](const std::set<location::SpaceVehicle>& svs)
    {
        cout << std::scientific;
        cout << "key snr has_almanac_data has_ephimeris_data used_in_fix azimuth elevation" << std::endl;
        for (const auto& sv : svs)
            cout << sv.key().id() << " " << sv.snr() << " " << sv.has_almanac_data() << " " << sv.has_ephimeris_data() << " " << sv.used_in_fix() << " " << sv.azimuth().value() << " " << sv.elevation().value() << std::endl;
    });

    {
        cli::ProgressBar pb(cout, "snr and ttf runtime test: ", 30);

        benchmark.run(
            [&](std::size_t)
            {
                hal->delete_all_aiding_data(); state.reset();
            },
            [&](std::size_t trial)
            {
                pb.update(trial / static_cast<double>(benchmark.trials()));

                hal->start_positioning();
                // We expect a maximum cold start time of 15 minutes. The theoretical
                // limit is 12.5 minutes, and we add up some grace period to make the
                // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
                expect<true, std::runtime_error>(state.wait_for_fix_for(std::chrono::seconds{15 * 60}), "Wait for fix timed out.");
                hal->stop_positioning();
            });
    }

    cout << benchmark << std::endl;

    return 0;
}
#else
int snr_and_ttff(const std::string& test_suite, std::ostream&, std::ostream&) { return 0; }
#endif // LOCATION_PROVIDERS_GPS

int ubx(std::ostream& cout, std::ostream&)
{
    location::util::Benchmark benchmark{boost::lexical_cast<unsigned int>(env::get("UBX_PROVIDER_TEST_TRIALS", "15")), "ttff in [µs]"};

    auto test_device = env::get_or_throw("UBX_PROVIDER_TEST_DEVICE");
    auto assist_now_enabled = env::get("UBX_PROVIDER_TEST_ASSIST_NOW_ENABLE", "false") == "true";
    auto assist_now_token = env::get("UBX_PROVIDER_TEST_ASSIST_NOW_TOKEN", "");
    auto assist_now_acquisition_timeout = boost::posix_time::seconds(
                boost::lexical_cast<std::uint64_t>(
                    env::get("UBX_PROVIDER_TEST_ASSIST_NOW_ACQUISITION_TIMEOUT", "5")));

    location::providers::ubx::Provider::Configuration configuration
    {
        location::providers::ubx::Provider::Protocol::ubx, test_device,
        {assist_now_enabled, assist_now_token, assist_now_acquisition_timeout}
    };

    auto provider = location::providers::ubx::Provider::create(configuration);

    struct State
    {
        State() : worker{[this]() { runtime.run(); }}, fix_received(false)
        {
        }

        ~State()
        {
            runtime.stop();
            if (worker.joinable())
                worker.join();
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

        location::glib::Runtime runtime{location::glib::Runtime::WithOwnMainLoop{}};
        std::thread worker;
        std::mutex guard;
        std::condition_variable wait_condition;
        bool fix_received;
    } state;

    provider->position_updates().connect([&state](const location::Update<location::Position>& update)
    {
        state.on_position_updated(update.value);
    });

    {
        cli::ProgressBar pb(cout, "ubx runtime test: ", 30);

        benchmark.run(
            [&](std::size_t)
            {
                provider->reset(); state.reset();
            },
            [&](std::size_t trial)
            {
                pb.update(trial / static_cast<double>(benchmark.trials()));

                provider->activate();
                // We expect a maximum cold start time of 15 minutes. The theoretical
                // limit is 12.5 minutes, and we add up some grace period to make the
                // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
                expect<true, std::runtime_error>(state.wait_for_fix_for(std::chrono::seconds{15 * 60}), "Wait for fix timed out.");
                provider->deactivate();
            });
    }

    cout << benchmark << std::endl;

    return 0;
}

/*
int sirf(std::ostream& cout, std::ostream&)
{
    location::util::Benchmark benchmark{boost::lexical_cast<unsigned int>(env::get("SIRF_PROVIDER_TEST_TRIALS", "15")), "ttff in [µs]"};

    auto test_device = env::get_or_throw("SIRF_PROVIDER_TEST_DEVICE");

    location::providers::sirf::Provider::Configuration configuration
    {
        location::providers::sirf::Provider::Protocol::sirf, test_device
    };

    auto provider = location::providers::sirf::Provider::create(configuration);

    struct State
    {
        State() : worker{[this]() { runtime.run(); }}, fix_received(false)
        {
        }

        ~State()
        {
            runtime.stop();
            if (worker.joinable())
                worker.join();
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

        location::glib::Runtime runtime{location::glib::Runtime::WithOwnMainLoop{}};
        std::thread worker;
        std::mutex guard;
        std::condition_variable wait_condition;
        bool fix_received;
    } state;

    provider->position_updates().connect([&state](const location::Update<location::Position>& update)
    {
        state.on_position_updated(update.value);
    });

    {
        cli::ProgressBar pb(cout, "sirf runtime test: ", 30);

        benchmark.run(
            [&](std::size_t)
            {
                provider->reset(); state.reset();
            },
            [&](std::size_t trial)
            {
                pb.update(trial / static_cast<double>(benchmark.trials()));

                provider->activate();
                // We expect a maximum cold start time of 15 minutes. The theoretical
                // limit is 12.5 minutes, and we add up some grace period to make the
                // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
                expect<true, std::runtime_error>(state.wait_for_fix_for(std::chrono::seconds{15 * 60}), "Wait for fix timed out.");
                provider->deactivate();
            });
    }

    cout << benchmark << std::endl;

    return 0;
}
*/

}  // namespace

int location::execute_runtime_tests(const std::string& test_suite, std::ostream& cout, std::ostream& cerr)
{
    if (test_suite == "android-gps")
        return snr_and_ttff(cout, cerr);
//    else if (test_suite == "sirf")
//        return sirf(cout, cerr);
    else if (test_suite == "ubx")
        return ubx(cout, cerr);
    return 0;
}

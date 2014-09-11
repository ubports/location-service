/*
 * Copyright © 2014 Canonical Ltd.
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

#include <com/ubuntu/location/providers/remote/provider.h>

#include <com/ubuntu/location/connectivity/manager.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/signal.h>
#include <core/posix/this_process.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;
namespace remote = com::ubuntu::location::providers::remote;
namespace statistics = boost::accumulators;

namespace
{
// A simple counter to keep track of events.
struct Counter
{
    Counter(const std::string& name) : name{name}, value{0}
    {
    }

    Counter& operator++()
    {
        value++; return *this;
    }

    Counter operator++(int)
    {
        auto copy = *this; ++*this; return copy;
    }

    Counter& operator--()
    {
        value--; return *this;
    }

    Counter operator--(int)
    {
        auto copy = *this; --*this; return copy;
    }

    std::uint64_t operator*() const
    {
        return value;
    }

    void reset()
    {
        value = 0;
    }

    std::string name;
    std::uint64_t value;
};

std::ostream& operator<<(std::ostream& out, const Counter& counter)
{
    return out << counter.name << ": " << counter.value;
}

// Helps in tracking time differences.
struct StopWatch
{
    StopWatch(const std::string& name)
        : start{std::chrono::system_clock::now()},
          name{name}
    {
    }

    void reset()
    {
        start = std::chrono::system_clock::now();
    }

    std::chrono::system_clock::duration stop()
    {
        return std::chrono::system_clock::now() - start;
    }

    std::chrono::system_clock::time_point start;
    std::string name;
};

struct Statistics
{
    // We accumulate size, mean, min, max and variance of a sample
    typedef statistics::accumulator_set
    <
        std::chrono::system_clock::time_point::rep,
        statistics::stats
        <
            statistics::tag::count,
            statistics::tag::mean,
            statistics::tag::min,
            statistics::tag::max,
            statistics::tag::variance
        >
    > Accumulator;

    Statistics(const std::string& name) : name{name}
    {
    }

    template<typename NumericType>
    void update(NumericType value)
    {
        accumulator(value);
    }

    void reset()
    {
        accumulator = Accumulator{};
    }

    Accumulator accumulator;
    std::string name;
};

std::ostream& operator<<(std::ostream& out, const Statistics& stats)
{
    return
            out << stats.name << ": "
                << "min: " << statistics::min(stats.accumulator) << ", "
                << "max: " << statistics::max(stats.accumulator) << ", "
                << "mean: " << statistics::mean(stats.accumulator) << ", "
                << "variance: " << statistics::variance(stats.accumulator);
}

struct EspooProviderTest : public ::testing::Test
{
    // All known environment variables.
    struct Parameters
    {
        struct Bus
        {
            static constexpr const char* name
            {
                "ESPOO_PROVIDER_TEST_BUS"
            };

            static constexpr const char* default_value
            {
                "session"
            };
        };
    };

    static core::dbus::Bus::Ptr bus_instance_according_to_env()
    {
        auto value = core::posix::this_process::env::get(Parameters::Bus::name, Parameters::Bus::default_value);

        core::dbus::WellKnownBus bus{core::dbus::WellKnownBus::starter};

        if (value == "session")
            bus = core::dbus::WellKnownBus::session;
        else if (value == "system")
            bus = core::dbus::WellKnownBus::system;

        auto result = std::make_shared<core::dbus::Bus>(bus);
        result->install_executor(core::dbus::asio::make_executor(result));

        return result;
    }

    static constexpr const char* service_name
    {
        "com.ubuntu.espoo.Service.Provider"
    };

    static constexpr const char* path
    {
        "/com/ubuntu/espoo/Service/Provider"
    };

    static std::uint64_t numeric_cell_id_from_cell(const cul::connectivity::RadioCell::Ptr& cell)
    {
        std::uint64_t cid;
        switch (cell->type())
        {
        case cul::connectivity::RadioCell::Type::gsm:
            cid = cell->gsm().id.get();
            break;
        case cul::connectivity::RadioCell::Type::umts:
            cid = cell->umts().id.get();
            break;
        case cul::connectivity::RadioCell::Type::lte:
            cid = cell->lte().id.get();
            break;
        default:
            cid = std::numeric_limits<std::uint64_t>::max();
            break;
        }
        return cid;
    }

    EspooProviderTest()
        : bus{bus_instance_according_to_env()},
          cm{cul::connectivity::platform_default_manager()}
    {
        // Bootstrap wifi stats
        cm->enumerate_visible_wireless_networks([this](const cul::connectivity::WirelessNetwork::Ptr&)
        {
            ++connectivity_stats.wifi_counter;
        });

        connectivity_stats.over_wifi_count.update(*connectivity_stats.wifi_counter);

        cm->wireless_network_added().connect([this](const cul::connectivity::WirelessNetwork::Ptr&)
        {
            connectivity_stats.over_wifi_count.update(*(++connectivity_stats.wifi_counter));
        });

        cm->wireless_network_removed().connect([this](const cul::connectivity::WirelessNetwork::Ptr&)
        {
            connectivity_stats.over_wifi_count.update(*(--connectivity_stats.wifi_counter));
        });

        // Bootstrap cell stats
        cm->enumerate_connected_radio_cells([this](const cul::connectivity::RadioCell::Ptr& cell)
        {
            on_radio_cell_added(cell);
        });

        cm->connected_cell_added().connect([this](const cul::connectivity::RadioCell::Ptr& cell)
        {
            on_radio_cell_added(cell);
        });

        cm->connected_cell_removed().connect([this](const cul::connectivity::RadioCell::Ptr& cell)
        {
            cells_to_ids.erase(cell);
        });
    }

    void on_radio_cell_added(const cul::connectivity::RadioCell::Ptr& cell)
    {
        // We store the cell characteristics here.
        cells_to_ids.insert(std::make_pair(cell, std::make_tuple(cell->type(), numeric_cell_id_from_cell(cell))));

        std::weak_ptr<cul::connectivity::RadioCell> wp{cell};

        // Subscribe to changes on the cell
        cell->changed().connect([wp, this]()
        {
            auto sp = wp.lock();

            if (not sp)
                return;

            if (cells_to_ids.count(sp) > 0)
            {
                auto tuple = cells_to_ids.at(sp);

                // The technology changed.
                if (std::get<0>(tuple) != sp->type())
                {
                    connectivity_stats.over_cell_changes.update(*(++connectivity_stats.cell_changes_counter));
                }
                else // No change in technology, comparing actual ids.
                {
                    if (std::get<1>(tuple) != numeric_cell_id_from_cell(sp))
                        connectivity_stats.over_cell_changes.update(*(++connectivity_stats.cell_changes_counter));
                }

                // Update last known values.
                cells_to_ids.at(sp) = std::make_tuple(sp->type(), numeric_cell_id_from_cell(sp));
            }
        });
    }

    // The bus connection for reaching out to the espoo provider.
    core::dbus::Bus::Ptr bus;
    // We monitor the connectivity subsystems.
    std::shared_ptr<cul::connectivity::Manager> cm;
    // We store individual cell ids together with their type.
    std::map
    <
        cul::connectivity::RadioCell::Ptr,
        std::tuple<cul::connectivity::RadioCell::Type, std::uint64_t>
    > cells_to_ids;
    struct
    {
        Counter wifi_counter{"Wifi counter"};
        Statistics over_wifi_count{"Wifi count stats"};
        Counter cell_changes_counter{"Cell changes"};
        Statistics over_cell_changes{"Cell changes stats"};
    } connectivity_stats;
};
}

TEST_F(EspooProviderTest, receives_position_updates_requires_daemons)
{
    // We shutdown the test on sigint.
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_int
    });

    trap->signal_raised().connect([trap](core::posix::Signal)
    {
        trap->stop();
    });

    remote::Provider::Configuration config;
    config.name = EspooProviderTest::service_name;
    config.path = EspooProviderTest::path;
    config.connection = bus;

    // We keep track of some characteristics.
    struct Stats
    {
        StopWatch execution_time{"Execution time of test"}; // The total execution time of the test.
        StopWatch position_updates_duration{"Position updates duration"}; // Time between position updates.
        Statistics position_updates_duration_stats{"Position updates duration stats"};
        Counter position_updates_counter{"Position updates"}; // The number of position updates we received.
    } stats;

    remote::Provider provider{config};

    provider.updates().position.connect([&stats](const cul::Update<cul::Position>& update)
    {
        // We track the number of position updates
        stats.position_updates_counter++;
        // And we update our statistics by querying our stopwatch.
        stats.position_updates_duration_stats.update(stats.position_updates_duration.stop().count());
        // And we reset it.
        stats.position_updates_duration.reset();
    });

    provider.start_position_updates();
    trap->run();
    provider.stop_position_updates();

    // Finally printing some statistics
    std::cout << "Total execution time: " << std::chrono::duration_cast<std::chrono::seconds>(stats.execution_time.stop()).count() << " [s]" << std::endl;
    std::cout << stats.position_updates_counter << std::endl;
    std::cout << stats.position_updates_duration_stats << std::endl;

    std::cout << connectivity_stats.over_wifi_count << std::endl;
    std::cout << connectivity_stats.over_cell_changes << std::endl;
}

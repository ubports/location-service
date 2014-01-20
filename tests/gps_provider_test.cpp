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
#include <com/ubuntu/location/providers/gps/provider.h>

#include <core/posix/this_process.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace gps = com::ubuntu::location::providers::gps;
namespace location = com::ubuntu::location;

namespace
{
struct UpdateTrap
{
    MOCK_METHOD1(on_position_updated, void(const location::Position&));
    MOCK_METHOD1(on_heading_updated, void(const location::Heading&));
    MOCK_METHOD1(on_velocity_updated, void(const location::Velocity&));
    MOCK_METHOD1(on_space_vehicles_updated, void(const std::set<location::SpaceVehicle>&));
};

struct MockSuplAssistant : public gps::HardwareAbstractionLayer::SuplAssistant
{
    MockSuplAssistant()
    {
        using namespace ::testing;

        ON_CALL(*this, status()).WillByDefault(ReturnRef(status_));
        ON_CALL(*this, server_ip()).WillByDefault(ReturnRef(ip_address_));
    }

    MOCK_CONST_METHOD0(status, const core::Property<Status>&());
    MOCK_CONST_METHOD0(server_ip, const core::Property<IpV4Address>&());
    MOCK_METHOD2(set_server, void(const std::string&, std::uint16_t));
    MOCK_METHOD1(notify_data_connection_open_via_apn, void(const std::string&));
    MOCK_METHOD0(notify_data_connection_closed, void());
    MOCK_METHOD0(notify_data_connection_not_available, void());

    core::Property<Status> status_;
    core::Property<IpV4Address> ip_address_;
};

struct MockHardwareAbstractionLayer : public gps::HardwareAbstractionLayer
{
    MockHardwareAbstractionLayer()
    {
        using namespace ::testing;

        ON_CALL(*this, supl_assistant()).WillByDefault(ReturnRef(supl_assistant_));
        ON_CALL(*this, start_positioning()).WillByDefault(Return(true));
        ON_CALL(*this, stop_positioning()).WillByDefault(Return(true));
        ON_CALL(*this, position_updates()).WillByDefault(ReturnRef(position_updates_));
        ON_CALL(*this, heading_updates()).WillByDefault(ReturnRef(heading_updates_));
        ON_CALL(*this, velocity_updates()).WillByDefault(ReturnRef(velocity_updates_));
        ON_CALL(*this, space_vehicle_updates()).WillByDefault(ReturnRef(space_vehicle_updates_));
        ON_CALL(*this, chipset_status()).WillByDefault(ReturnRef(chipset_status_));
    }

    MOCK_METHOD0(supl_assistant, gps::HardwareAbstractionLayer::SuplAssistant&());
    MOCK_CONST_METHOD0(position_updates, const core::Signal<location::Position>& ());
    MOCK_CONST_METHOD0(heading_updates, const core::Signal<location::Heading>&());
    MOCK_CONST_METHOD0(velocity_updates, const core::Signal<location::Velocity>& ());
    MOCK_CONST_METHOD0(space_vehicle_updates, const core::Signal<std::set<location::SpaceVehicle>>&());
    MOCK_METHOD0(delete_all_aiding_data, void());
    MOCK_CONST_METHOD0(chipset_status, const core::Property<gps::ChipsetStatus>&());
    MOCK_CONST_METHOD1(is_capable_of, bool(gps::AssistanceMode));
    MOCK_CONST_METHOD1(is_capable_of, bool(gps::PositionMode));
    MOCK_CONST_METHOD1(is_capable_of, bool(gps::Capability capability));
    MOCK_METHOD0(start_positioning, bool());
    MOCK_METHOD0(stop_positioning, bool());
    MOCK_METHOD1(set_assistance_mode, bool(gps::AssistanceMode));
    MOCK_METHOD1(set_position_mode, bool(gps::PositionMode));
    MOCK_METHOD1(inject_reference_position, bool(const location::Position&));
    MOCK_METHOD2(inject_reference_time,
                 bool(const std::chrono::microseconds&,
                      const std::chrono::microseconds&));

    MockSuplAssistant supl_assistant_;
    core::Signal<location::Position> position_updates_;
    core::Signal<location::Heading> heading_updates_;
    core::Signal<location::Velocity> velocity_updates_;
    core::Signal<std::set<location::SpaceVehicle>> space_vehicle_updates_;
    core::Property<gps::ChipsetStatus> chipset_status_;
};
}

TEST(GpsProvider, starting_updates_on_a_provider_instance_calls_into_the_hal)
{
    using namespace ::testing;

    NiceMock<MockHardwareAbstractionLayer> hal;
    std::shared_ptr<gps::HardwareAbstractionLayer> hal_ptr(&hal, [](gps::HardwareAbstractionLayer*){});

    gps::Provider provider(hal_ptr);

    EXPECT_CALL(hal, start_positioning()).Times(3);
    // Stop positioning will be called by the provider's dtor.
    // Thus 3 explicit stops and 1 implicit.
    EXPECT_CALL(hal, stop_positioning()).Times(4);

    provider.start_position_updates();
    provider.start_heading_updates();
    provider.start_velocity_updates();

    provider.stop_position_updates();
    provider.stop_heading_updates();
    provider.stop_velocity_updates();
}

TEST(GpsProvider, injecting_a_reference_position_calls_into_the_hal)
{
    using namespace ::testing;

    NiceMock<MockHardwareAbstractionLayer> hal;
    std::shared_ptr<gps::HardwareAbstractionLayer> hal_ptr(&hal, [](gps::HardwareAbstractionLayer*){});

    gps::Provider provider(hal_ptr);
    location::Position pos;
    EXPECT_CALL(hal, inject_reference_position(pos)).Times(1);

    provider.on_reference_location_updated(pos);
}

TEST(GpsProvider, updates_from_hal_are_passed_on_by_the_provider)
{
    using namespace ::testing;

    NiceMock<MockHardwareAbstractionLayer> hal;
    std::shared_ptr<gps::HardwareAbstractionLayer> hal_ptr(&hal, [](gps::HardwareAbstractionLayer*){});

    UpdateTrap update_trap;

    gps::Provider provider(hal_ptr);
    location::Position pos;
    location::Heading heading;
    location::Velocity velocity;
    std::set<location::SpaceVehicle> svs;

    provider.updates().position.connect([&update_trap](const location::Update<location::Position>& pos)
    {
        update_trap.on_position_updated(pos.value);
    });
    provider.updates().heading.connect([&update_trap](const location::Update<location::Heading>& heading)
    {
        update_trap.on_heading_updated(heading.value);
    });
    provider.updates().velocity.connect([&update_trap](const location::Update<location::Velocity>& velocity)
    {
        update_trap.on_velocity_updated(velocity.value);
    });
    provider.updates().svs.connect([&update_trap](const location::Update<std::set<location::SpaceVehicle>>& svs)
    {
        update_trap.on_space_vehicles_updated(svs.value);
    });

    EXPECT_CALL(update_trap, on_position_updated(pos)).Times(1);
    EXPECT_CALL(update_trap, on_heading_updated(heading)).Times(1);
    EXPECT_CALL(update_trap, on_velocity_updated(velocity)).Times(1);
    EXPECT_CALL(update_trap, on_space_vehicles_updated(svs)).Times(1);

    hal.position_updates_(pos);
    hal.heading_updates_(heading);
    hal.velocity_updates_(velocity);
    hal.space_vehicle_updates_(svs);
}

/*****************************************************************
 *                                                               *
 * All tests requiring hardware go here. They are named with     *
 * the suffix requires_hardware to indicate that they shouldn't  *
 * be executed as part of the usual build/test cycle. Instead    *
 * they are packaged up for later execution on an emulated or    *
 * real device.                                                  *
 *                                                               *
 ****************************************************************/
TEST(GpsProvider, accessing_starting_and_stopping_gps_provider_works_requires_hardware)
{
    com::ubuntu::location::providers::gps::Provider provider;
    EXPECT_NO_THROW(provider.start_position_updates());
    EXPECT_NO_THROW(provider.stop_position_updates());
    EXPECT_NO_THROW(provider.start_velocity_updates());
    EXPECT_NO_THROW(provider.stop_velocity_updates());
    EXPECT_NO_THROW(provider.start_heading_updates());
    EXPECT_NO_THROW(provider.stop_heading_updates());
}

namespace
{
struct HardwareAbstractionLayerFixture : public ::testing::Test
{
    void SetUp()
    {
        // We need to make sure that we are running as root. In addition, we will stop
        // any running location service instance prior to executing the test.
        if (!(::getuid() == 0))
            FAIL() << "This test has to be run as root.";

        int rc = ::system("service ubuntu-location-service stop");

        // We consciously ignore the return code of the command here.
        // The location service might not have been running before and with that
        // the command would return an error, although the precondition has been successfully
        // established.
        if (rc < 0)
            FAIL() << "Unable to stop the location service as part of the test setup.";
    }

    void TearDown()
    {
        int rc = ::system("service ubuntu-location-service start");
        (void) rc;
    }
};
}
// HardwareAbstractionLayerFixture.time_to_first_fix_cold_start_without_supl_benchmark_requires_hardware
TEST_F(HardwareAbstractionLayerFixture, time_to_first_fix_cold_start_without_supl_benchmark_requires_hardware)
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

    for (unsigned int i = 0; i < trials; i++)
    {
        std::cout << "Executing trial " << i << " of " << trials << " trials" << std::endl;
        // We want to force a cold start per trial.
        hal->delete_all_aiding_data();

        state.reset();
        auto start = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());
        {
            hal->start_positioning();
            // We expect a maximum cold start time of 15 minutes. The theoretical
            // limit is 12.5 minutes, and we add up some grace period to make the
            // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
            EXPECT_TRUE(state.wait_for_fix_for(std::chrono::seconds{15 * 60}));
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
}

// HardwareAbstractionLayerFixture.time_to_first_fix_cold_start_with_supl_benchmark_requires_hardware
TEST_F(HardwareAbstractionLayerFixture, time_to_first_fix_cold_start_with_supl_benchmark_requires_hardware)
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

    // We want to run in assisted mode
    EXPECT_TRUE(hal->set_assistance_mode(gps::AssistanceMode::mobile_station_based));

    // Let's see if we have a custom supl server configured via the environment
    try
    {
        auto server = core::posix::this_process::env::get_or_throw("GPS_SUPL_BENCHMARK_SERVER_ADDRESS");
        std::uint16_t port = 8799;

        try
        {
            std::stringstream ss(core::posix::this_process::env::get_or_throw("GPS_SUPL_BENCHMARK_SERVER_PORT"));
            ss >> port;
        } catch(...)
        {
            // Ignoring any issues and defaulting to 8799 for the port.
        }

        hal->supl_assistant().set_server(server, port);
    } catch(...)
    {
        // Ignoring exceptions here and defaulting to configuration provided
        // by the system.
    }

    // We wire up our state to position updates from the hal.
    hal->position_updates().connect([&state](const location::Position& pos)
    {
        state.on_position_updated(pos);
    });

    for (unsigned int i = 0; i < trials; i++)
    {
        std::cout << "Executing trial " << i << " of " << trials << " trials" << std::endl;
        // We want to force a cold start per trial.
        hal->delete_all_aiding_data();

        state.reset();
        auto start = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());
        {
            hal->start_positioning();
            // We expect a maximum cold start time of 15 minutes. The theoretical
            // limit is 12.5 minutes, and we add up some grace period to make the
            // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
            EXPECT_TRUE(state.wait_for_fix_for(std::chrono::seconds{15 * 60}));
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
}


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

#include <com/ubuntu/location/criteria.h>
#include <com/ubuntu/location/clock.h>
#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/implementation.h>
#include <com/ubuntu/location/service/stub.h>

#include <core/dbus/announcer.h>
#include <core/dbus/fixture.h>
#include <core/dbus/resolver.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/signal.h>

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>

namespace cul = com::ubuntu::location;
namespace culss = com::ubuntu::location::service::session;
namespace dbus = core::dbus;

namespace
{
struct LocationServiceStandalone : public core::dbus::testing::Fixture
{
};

template<typename T>
cul::Update<T> update_as_of_now(const T& value = T())
{
    return cul::Update<T>{value, cul::Clock::now()};
}

class DummyProvider : public cul::Provider
{
public:
    DummyProvider() : cul::Provider()
    {
    }

    ~DummyProvider() noexcept
    {
    }

    void inject_update(const cul::Update<cul::Position>& update)
    {
        mutable_updates().position(update);
    }

    void inject_update(const cul::Update<cul::Velocity>& update)
    {
        mutable_updates().velocity(update);
    }

    void inject_update(const cul::Update<cul::Heading>& update)
    {
        mutable_updates().heading(update);
    }

    bool matches_criteria(const cul::Criteria& /*criteria*/)
    {
        return true;
    }
};

struct AlwaysGrantingPermissionManager : public cul::service::PermissionManager
{
    PermissionManager::Result
    check_permission_for_credentials(const cul::Criteria&,
                                     const cul::service::Credentials&)
    {
        return PermissionManager::Result::granted;
    }
};

auto timestamp = cul::Clock::now();

// Create reference objects for injecting and validating updates.
cul::Update<cul::Position> reference_position_update
{
    {
        cul::wgs84::Latitude{9. * cul::units::Degrees},
        cul::wgs84::Longitude{53. * cul::units::Degrees},
        cul::wgs84::Altitude{-2. * cul::units::Meters}
    },
    timestamp
};

cul::Update<cul::Velocity> reference_velocity_update
{
    {5. * cul::units::MetersPerSecond},
    timestamp
};

cul::Update<cul::Heading> reference_heading_update
{
    {120. * cul::units::Degrees},
    timestamp
};
} // namespace

TEST_F(LocationServiceStandalone, SessionsReceiveUpdatesViaDBus)
{
    core::testing::CrossProcessSync sync_start;
    core::testing::CrossProcessSync sync_session_created;

    auto server = [this, &sync_start, &sync_session_created]()
    {
        SCOPED_TRACE("Server");

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));

        std::thread t{[bus](){bus->run();}};

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;

        cul::service::Implementation service(
                    bus,
                    config.the_engine(
                        config.the_provider_set(helper),
                        config.the_provider_selection_policy()),
                    config.the_permission_manager());

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        EXPECT_EQ(1, sync_session_created.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        dummy->inject_update(reference_position_update);
        dummy->inject_update(reference_velocity_update);
        dummy->inject_update(reference_heading_update);

        trap->run();

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start, &sync_session_created]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{15000}));

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));
        std::thread t{[bus](){bus->run();}};

        auto location_service = dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);
        
        auto s1 = location_service->create_session_for_criteria(cul::Criteria{});

        std::cout << "Successfully created session" << std::endl;

        cul::Update<cul::Position> position;
        auto c1 = s1->updates().position.changed().connect(
            [&](const cul::Update<cul::Position>& new_position) {
                std::cout << "On position updated: " << new_position << std::endl;
                position = new_position;
            });
        cul::Update<cul::Velocity> velocity;
        auto c2 = s1->updates().velocity.changed().connect(
            [&](const cul::Update<cul::Velocity>& new_velocity) {
                std::cout << "On velocity_changed " << new_velocity << std::endl;
                velocity = new_velocity;
            });
        cul::Update<cul::Heading> heading;
        auto c3 = s1->updates().heading.changed().connect(
            [&](const cul::Update<cul::Heading>& new_heading) {
                std::cout << "On heading changed: " << new_heading << std::endl;
                heading = new_heading;
                bus->stop();
            });
        
        std::cout << "Created event connections, starting updates...";

        s1->updates().position_status = culss::Interface::Updates::Status::enabled;
        s1->updates().heading_status = culss::Interface::Updates::Status::enabled;
        s1->updates().velocity_status = culss::Interface::Updates::Status::enabled;
        
        std::cout << "done" << std::endl;

        sync_session_created.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(reference_position_update, position);
        EXPECT_EQ(reference_velocity_update, velocity);
        EXPECT_EQ(reference_heading_update, heading);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

TEST_F(LocationServiceStandalone, EngineStatusCanBeQueriedAndAdjusted)
{
    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));
        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;
        auto engine = config.the_engine(
                    config.the_provider_set(helper),
                    config.the_provider_selection_policy());
        engine->configuration.engine_state = cul::Engine::Status::on;
        auto permission_manager = config.the_permission_manager();
        cul::service::Implementation service(
                    bus,
                    engine,
                    permission_manager);

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        std::thread t{[bus](){bus->run();}};

        trap->run();

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto bus = session_bus();
        auto location_service = dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);

        EXPECT_TRUE(location_service->is_online());
        location_service->is_online() = false;
        EXPECT_FALSE(location_service->is_online());

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

TEST_F(LocationServiceStandalone, SatellitePositioningStatusCanBeQueriedAndAdjusted)
{
    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;
        auto engine = config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy());
        engine->configuration.satellite_based_positioning_state.set(
                    cul::SatelliteBasedPositioningState::on);
        auto permission_manager = config.the_permission_manager();
        cul::service::Implementation service(
                    bus,
                    engine,
                    permission_manager);

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        std::thread t{[bus](){bus->run();}};

        trap->run();

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto bus = session_bus();
        auto location_service = dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);

        EXPECT_TRUE(location_service->does_satellite_based_positioning());
        location_service->does_satellite_based_positioning() = false;
        EXPECT_FALSE(location_service->does_satellite_based_positioning());

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

TEST_F(LocationServiceStandalone, WifiAndCellIdReportingStateCanBeQueriedAndAjdusted)
{
    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));
        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;
        auto engine = config.the_engine(
                    config.the_provider_set(helper),
                    config.the_provider_selection_policy());
        auto permission_manager = config.the_permission_manager();
        cul::service::Implementation service(
                    bus,
                    engine,
                    permission_manager);

        std::thread t{[bus](){bus->run();}};

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        trap->run();

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto bus = session_bus();
        auto location_service = dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);

        EXPECT_FALSE(location_service->does_report_cell_and_wifi_ids());
        location_service->does_report_cell_and_wifi_ids() = true;
        EXPECT_TRUE(location_service->does_report_cell_and_wifi_ids());

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

TEST_F(LocationServiceStandalone, VisibleSpaceVehiclesCanBeQueried)
{
    core::testing::CrossProcessSync sync_start;

    cul::SpaceVehicle sv;
    sv.key.type = cul::SpaceVehicle::Type::gps;
    sv.has_ephimeris_data = true;
    static const std::map<cul::SpaceVehicle::Key, cul::SpaceVehicle> visible_space_vehicles
    {
        {sv.key, sv}
    };

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));
        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;
        auto engine = config.the_engine(
                    config.the_provider_set(helper),
                    config.the_provider_selection_policy());
        auto permission_manager = config.the_permission_manager();
        cul::service::Implementation service(
                    bus,
                    engine,
                    permission_manager);
        engine->updates.visible_space_vehicles.set(visible_space_vehicles);

        std::thread t{[bus](){bus->run();}};

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        trap->run();

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto bus = session_bus();
        auto location_service = dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);

        auto svs = location_service->visible_space_vehicles().get();

        EXPECT_EQ(visible_space_vehicles, svs);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

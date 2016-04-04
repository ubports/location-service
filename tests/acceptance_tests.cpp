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
#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

#include <com/ubuntu/location/providers/dummy/provider.h>

#include <com/ubuntu/location/service/daemon.h>
#include <com/ubuntu/location/service/default_configuration.h>
#include <com/ubuntu/location/service/implementation.h>
#include <com/ubuntu/location/service/program_options.h>
#include <com/ubuntu/location/service/stub.h>

#include <core/dbus/announcer.h>
#include <core/dbus/bus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/resolver.h>

#include <core/dbus/asio/executor.h>

#include <core/posix/signal.h>
#include <core/posix/this_process.h>

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
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;
namespace dbus = core::dbus;

namespace
{

bool setup_trust_store_permission_manager_for_testing()
{
    core::posix::this_process::env::set_or_throw("TRUST_STORE_PERMISSION_MANAGER_IS_RUNNING_UNDER_TESTING", "1");
    return true;
}

static const bool trust_store_is_set_up_for_testing = setup_trust_store_permission_manager_for_testing();

struct NullReporter : public culs::Harvester::Reporter
{
    NullReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    void start() override
    {
    }

    /** @brief Tell the reporter to shut down its operation. */
    void stop()
    {
    }

    /**
     * @brief Triggers the reporter to send off the information.
     */
    void report(const cul::Update<cul::Position>&,
                const std::vector<cul::connectivity::WirelessNetwork::Ptr>&,
                const std::vector<cul::connectivity::RadioCell::Ptr>&)
    {
    }
};

struct NullSettings : public cul::Settings
{
    // Syncs the current settings to implementation-specific backends.
    void sync() override
    {
    }

    // Returns true iff a value is known for the given key.
    bool has_value_for_key(const std::string&) const override
    {
        return false;
    }

    // Gets an integer value known for the given key, or throws Error::NoValueForKey.
    std::string get_string_for_key_or_throw(const std::string& key) override
    {
        throw cul::Settings::Error::NoValueForKey{key};
    }

    // Sets values known for the given key.
    bool set_string_for_key(const std::string&, const std::string&) override
    {
        return false;
    }
};

cul::Settings::Ptr null_settings()
{
    return std::make_shared<NullSettings>();
}

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
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

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

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        EXPECT_EQ(1, sync_session_created.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        dummy->inject_update(reference_position_update);
        dummy->inject_update(reference_velocity_update);
        dummy->inject_update(reference_heading_update);

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

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
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        configuration.engine->configuration.engine_state = cul::Engine::Status::on;
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

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
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        configuration.engine->configuration.satellite_based_positioning_state.set(cul::SatelliteBasedPositioningState::on);
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

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
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    core::testing::CrossProcessSync sync_start;

    auto server = [this, &sync_start]()
    {
        SCOPED_TRACE("Server");
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            trap->stop();
        });

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

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
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

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

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        configuration.engine->updates.visible_space_vehicles.set(visible_space_vehicles);

        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

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

TEST_F(LocationServiceStandalone, NewSessionsGetLastKnownPosition)
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

        auto incoming = session_bus();
        auto outgoing = session_bus();

        incoming->install_executor(core::dbus::asio::make_executor(incoming));
        outgoing->install_executor(core::dbus::asio::make_executor(outgoing));

        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);

        cul::service::DefaultConfiguration config;
        cul::service::Implementation::Configuration configuration
        {
            incoming,
            outgoing,
            config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy(), null_settings()),
            config.the_permission_manager(incoming),
            cul::service::Harvester::Configuration
            {
                cul::connectivity::platform_default_manager(),
                std::make_shared<NullReporter>()
            }
        };
        auto location_service = std::make_shared<cul::service::Implementation>(configuration);

        configuration.engine->updates.last_known_location.set(reference_position_update);
        std::thread t1{[incoming](){incoming->run();}};
        std::thread t2{[outgoing](){outgoing->run();}};

        sync_start.try_signal_ready_for(std::chrono::milliseconds{500});

        trap->run();

        incoming->stop();
        outgoing->stop();

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &sync_start]()
    {
        SCOPED_TRACE("Client");

        EXPECT_EQ(1, sync_start.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

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

        std::cout << "Created event connections, starting updates..." << std::endl;

        s1->updates().position_status = culss::Interface::Updates::Status::enabled;

        std::cout << "done" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        bus->stop();

        if (t.joinable())
            t.join();

        EXPECT_EQ(reference_position_update, position);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(server, client));
}

namespace
{
struct LocationServiceStandaloneLoad : public LocationServiceStandalone
{
    struct Keys
    {
        // Reference Latitude value for dummy::Provider setup.
        static constexpr const char* ref_lat{"ref_lat"};
        // Reference Longitude value for dummy::Provider setup.
        static constexpr const char* ref_lon{"ref_lon"};
        // Reference velocity value for dummy::Provider setup.
        static constexpr const char* ref_velocity{"ref_velocity"};
        // Reference heading value for dummy::Provider setup.
        static constexpr const char* ref_heading{"ref_heading"};
        // Update period length in [ms] for dummy::Provider setup.
        static constexpr const char* update_period{"update_period"};
        // Number of clients that should be fired up.
        static constexpr const char* client_count{"client_count"};
        // Test duration in [s]
        static constexpr const char* test_duration{"test_duration"};
    };
    static cul::ProgramOptions init_options()
    {
        cul::ProgramOptions options;

        options.environment_prefix("ACCEPTANCE_TEST_");

        options.add(Keys::ref_lat,
                    "Reference Latitude value for dummy::Provider setup.",
                    51.444670);

        options.add(Keys::ref_lon,
                    "Reference Longitude value for dummy::Provider setup.",
                    7.210852);

        options.add(Keys::ref_velocity,
                    "Reference velocity value for dummy::Provider setup.",
                    7.);

        options.add(Keys::ref_heading,
                    "Reference heading value for dummy::Provider setup.",
                    80.);

        options.add(Keys::update_period,
                    "Update period length for dummy::Provider setup.",
                    std::uint32_t{10});

        options.add(Keys::client_count,
                    "Number of clients that should be fired up.",
                    std::uint32_t{10});

        options.add(Keys::test_duration,
                    "Test duration in [s]",
                    std::uint32_t{30});

        return options;
    }

    // Initialize our options pack.
    cul::ProgramOptions options = init_options();
    // And force parsing of the environment.
    bool options_parsed_from_env
    {
        options.parse_from_environment()
    };
    // Reference values used in setting expectations
    const double ref_lat
    {
        options.value_for_key<double>(Keys::ref_lat)
    };
    const double ref_lon
    {
        options.value_for_key<double>(Keys::ref_lon)
    };
    const double ref_velocity
    {
        options.value_for_key<double>(Keys::ref_velocity)
    };
    const double ref_heading
    {
       options.value_for_key<double>(Keys::ref_heading)
    };
    const std::uint32_t update_period
    {
        options.value_for_key<std::uint32_t>(Keys::update_period)
    };
    const std::uint32_t client_count
    {
        options.value_for_key<std::uint32_t>(Keys::client_count)
    };
    const std::chrono::seconds test_duration
    {
        options.value_for_key<std::uint32_t>(Keys::test_duration)
    };

};
}

#include "did_finish_successfully.h"

TEST_F(LocationServiceStandaloneLoad, MultipleClientsConnectingAndDisconnectingWorks)
{
    EXPECT_TRUE(trust_store_is_set_up_for_testing);

    options.print(LOG(INFO));

    auto server = core::posix::fork([this]()
    {
        SCOPED_TRACE("Server");

        VLOG(1) << "Server started: " << getpid();

        cul::Configuration dummy_provider_config;

        dummy_provider_config.add(
                    cul::providers::dummy::Configuration::Keys::reference_position_lat,
                    ref_lat);

        dummy_provider_config.add(
                    cul::providers::dummy::Configuration::Keys::reference_position_lon,
                    ref_lon);

        dummy_provider_config.add(
                    cul::providers::dummy::Configuration::Keys::reference_heading,
                    ref_heading);

        dummy_provider_config.add(
                    cul::providers::dummy::Configuration::Keys::reference_velocity,
                    ref_velocity);

        dummy_provider_config.add(
                    cul::providers::dummy::Configuration::Keys::update_period,
                    update_period);

        std::map<std::string, cul::Configuration> provider_config
        {
            {cul::providers::dummy::Provider::class_name(), dummy_provider_config}
        };

        cul::service::Daemon::Configuration config;
        config.incoming = std::make_shared<core::dbus::Bus>(core::posix::this_process::env::get_or_throw("DBUS_SESSION_BUS_ADDRESS"));
        config.outgoing = std::make_shared<core::dbus::Bus>(core::posix::this_process::env::get_or_throw("DBUS_SESSION_BUS_ADDRESS"));
        config.is_testing_enabled = false;
        config.providers =
        {
            cul::providers::dummy::Provider::class_name()
        };
        config.provider_options = provider_config;
        config.settings = null_settings();

        core::posix::exit::Status status{core::posix::exit::Status::failure};

        try
        {
            status = static_cast<core::posix::exit::Status>(cul::service::Daemon::main(config));
        } catch(const std::exception& e)
        {
            ADD_FAILURE() << e.what();
        } catch(...)
        {
            ADD_FAILURE() << "Caught exception while executing daemon";
        }

        return ::testing::Test::HasFailure() ?
                    core::posix::exit::Status::failure :
                    status;
    }, core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::seconds{15});

    auto client = [this]()
    {
        SCOPED_TRACE("Client");

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap](core::posix::Signal)
        {
            VLOG(1) << "Received core::posix::Signal::sig_term";
            trap->stop();
        });

        auto bus = session_bus();
        bus->install_executor(dbus::asio::make_executor(bus));
        std::thread t{[bus](){bus->run();}};

        try
        {
            auto location_service = dbus::resolve_service_on_bus<
                cul::service::Interface,
                cul::service::Stub>(bus);

            auto s1 = location_service->create_session_for_criteria(cul::Criteria{});

            VLOG(1) << "Successfully created session: " << s1 << std::flush;

            auto c1 = s1->updates().position.changed().connect(
                [this](const cul::Update<cul::Position>& new_position)
                {
                    VLOG(100) << new_position;

                    EXPECT_DOUBLE_EQ(ref_lat, new_position.value.latitude.value.value());
                    EXPECT_DOUBLE_EQ(ref_lon, new_position.value.longitude.value.value());
                });

            auto c2 = s1->updates().velocity.changed().connect(
                [this](const cul::Update<cul::Velocity>& new_velocity)
                {
                    VLOG(100) << new_velocity;
                    EXPECT_DOUBLE_EQ(ref_velocity, new_velocity.value.value());
                });

            auto c3 = s1->updates().heading.changed().connect(
                [this](const cul::Update<cul::Heading>& new_heading)
                {
                    VLOG(100) << new_heading;
                    EXPECT_DOUBLE_EQ(ref_heading, new_heading.value.value());
                });

            VLOG(100) << "Created event connections, starting updates..." << std::flush;

            s1->updates().position_status = culss::Interface::Updates::Status::enabled;
            s1->updates().heading_status = culss::Interface::Updates::Status::enabled;
            s1->updates().velocity_status = culss::Interface::Updates::Status::enabled;

            trap->run();
        } catch(const std::exception& e)
        {
            LOG(ERROR) << e.what();
        } catch(...)
        {
            LOG(ERROR) << "Something else happened";
        }

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    std::vector<core::posix::ChildProcess> clients;

    for (unsigned int i = 0; i < client_count; i++)
    {
        clients.push_back(core::posix::fork(client, core::posix::StandardStream::empty));
    }

    bool running = true;

    std::thread chaos
    {
        [&clients, client, &running]()
        {
            std::this_thread::sleep_for(std::chrono::seconds{2});

            std::default_random_engine generator;
            std::uniform_int_distribution<int> dice(1, 6);

            while (running && clients.size() > 1)
            {

                std::uniform_int_distribution<int> client_dist(0,clients.size()-1);

                // Sample a client index
                auto idx = client_dist(generator);
                // Pick a client at random
                auto& c = clients.at(idx);
                // Kill that client
                c.send_signal_or_throw(core::posix::Signal::sig_kill);
                // Wait for the child to avoid zombies
                c.wait_for(core::posix::wait::Flags::untraced);
                // And remove it from the list
                if (dice(generator) <= 3)
                    clients.erase(clients.begin() + idx);
                else
                    clients.at(idx) = core::posix::fork(client, core::posix::StandardStream::empty);

                // And we pause for 500ms
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
            }
        }
    };

    // We let the setup spin for 30 seconds.
    std::this_thread::sleep_for(test_duration);

    running = false;

    if (chaos.joinable())
        chaos.join();

    for (auto& client : clients)
    {
        VLOG(1) << "Stopping client...: " << client.pid();
        client.send_signal_or_throw(core::posix::Signal::sig_term);
        EXPECT_TRUE(did_finish_successfully(client.wait_for(core::posix::wait::Flags::untraced)));
    }

    VLOG(1) << "Cleaned up clients, shutting down the service...";

    server.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(server.wait_for(core::posix::wait::Flags::untraced)));
}

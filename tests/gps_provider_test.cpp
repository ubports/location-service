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
#include <com/ubuntu/location/providers/gps/android_hardware_abstraction_layer.h>

#include <com/ubuntu/location/providers/gps/net_cpp_gps_xtra_downloader.h>

#include <com/ubuntu/location/logging.h>
#include <com/ubuntu/location/service/program_options.h>

#include <core/posix/fork.h>
#include <core/posix/this_process.h>

#include <core/testing/cross_process_sync.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "web_server.h"

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

auto gps_conf =
R"foo(
# XTRA_SERVER_QUERY (1=on, 0=off)
# If XTRA_SERVER_QUERY is on, the XTRA_SERVERs listed
# below will be ignored, and instead the servers will
# be queried from the modem.
XTRA_SERVER_QUERY=0
# XTRA_SERVERs below are used only if XTRA_SERVER_QUERY
# is off.
XTRA_SERVER_1=http://xtra1.gpsonextra.net/xtra2.bin
XTRA_SERVER_2=http://xtra2.gpsonextra.net/xtra2.bin
XTRA_SERVER_3=http://xtra3.gpsonextra.net/xtra2.bin

# Error Estimate
# _SET = 1
# _CLEAR = 0
ERR_ESTIMATE=0

#Test
NTP_SERVER=time.gpsonextra.net
#Asia
# NTP_SERVER=asia.pool.ntp.org
#Europe
# NTP_SERVER=europe.pool.ntp.org
#North America
# NTP_SERVER=north-america.pool.ntp.org

# DEBUG LEVELS: 0 - none, 1 - Error, 2 - Warning, 3 - Info
#               4 - Debug, 5 - Verbose
# If DEBUG_LEVEL is commented, Android's logging levels will be used
DEBUG_LEVEL = 2

# Intermediate position report, 1=enable, 0=disable
INTERMEDIATE_POS=0

# supl version 1.0
SUPL_VER=0x10000

# GPS Capabilities bit mask
# SCHEDULING = 0x01
# MSB = 0x02
# MSA = 0x04
# ON_DEMAND_TIME = 0x10
# GEOFENCE = 0x20
# default = ON_DEMAND_TIME | MSA | MSB | SCHEDULING | GEOFENCE
CAPABILITIES=0x33

# Accuracy threshold for intermediate positions
# less accurate positions are ignored, 0 for passing all positions
# ACCURACY_THRES=5000

################################
##### AGPS server settings #####
################################

# FOR SUPL SUPPORT, set the following
# SUPL_HOST=supl.host.com or IP
# SUPL_PORT=1234
SUPL_HOST=supl.google.com
SUPL_PORT=7275

# FOR C2K PDE SUPPORT, set the following
# C2K_HOST=c2k.pde.com or IP
# C2K_PORT=1234

####################################
#  LTE Positioning Profile Settings
####################################
# 0: Enable RRLP on LTE(Default)
# 1: Enable LPP_User_Plane on LTE
# 2: Enable LPP_Control_Plane
# 3: Enable both LPP_User_Plane and LPP_Control_Plane
LPP_PROFILE = 0

################################
# EXTRA SETTINGS
################################
# NMEA provider (1=Modem Processor, 0=Application Processor)
NMEA_PROVIDER=0

##################################################
# Select Positioning Protocol on A-GLONASS system
##################################################
# 0x1: RRC CPlane
# 0x2: RRLP UPlane
# 0x4: LLP Uplane
A_GLONASS_POS_PROTOCOL_SELECT = 0
)foo";

}

TEST(AndroidGpsXtraDownloader, reading_configuration_from_valid_conf_file_works)
{
    std::stringstream ss{gps_conf};

    auto config = gps::android::GpsXtraDownloader::Configuration::from_gps_conf_ini_file(ss);

    EXPECT_EQ(3, config.xtra_hosts.size());

    EXPECT_EQ("http://xtra1.gpsonextra.net/xtra2.bin", config.xtra_hosts.at(0));
    EXPECT_EQ("http://xtra2.gpsonextra.net/xtra2.bin", config.xtra_hosts.at(1));
    EXPECT_EQ("http://xtra3.gpsonextra.net/xtra2.bin", config.xtra_hosts.at(2));
}

TEST(GpsProvider, starting_updates_on_a_provider_instance_calls_into_the_hal)
{
    using namespace ::testing;

    NiceMock<MockHardwareAbstractionLayer> hal;
    std::shared_ptr<gps::HardwareAbstractionLayer> hal_ptr(&hal, [](gps::HardwareAbstractionLayer*){});

    gps::Provider provider(hal_ptr);

    // The Android HAL should only ever be called at most once for starting and stopping.
    // It seems like some HAL implementations otherwise get stuck or expose undefined
    // behavior later down the road.
    EXPECT_CALL(hal, start_positioning()).Times(1);
    EXPECT_CALL(hal, stop_positioning()).Times(1);

    provider.start_position_updates();
    provider.stop_position_updates();
    provider.start_heading_updates();
    provider.stop_heading_updates();
    provider.start_velocity_updates();
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

TEST(GpsXtraDownloader, throws_for_missing_xtra_hosts)
{
    gps::android::NetCppGpsXtraDownloader downloader;
    EXPECT_ANY_THROW(downloader.download_xtra_data(gps::android::GpsXtraDownloader::Configuration{}));
}

TEST(GpsXtraDownloader, downloading_xtra_data_from_known_host_works)
{
    static const int data_size{200};

    core::testing::CrossProcessSync cps; // server - ready -> client

    testing::web::server::Configuration configuration
    {
        5000,
        [](mg_connection* conn)
        {
            static char data[data_size];
            for (int i = 0; i < data_size; i++)
                data[i] = i%2;

            std::map<std::string, std::set<std::string>> header;

            for (int i = 0; i < conn->num_headers; i++)
            {
                header[conn->http_headers[i].name].insert(conn->http_headers[i].value);
            }

            EXPECT_TRUE(header.at("Accept").count("*/*") == 1);
            EXPECT_TRUE(header.at("Accept").count("application/vnd.wap.mms-message") == 1);
            EXPECT_TRUE(header.at("Accept").count("application/vnd.wap.sic") == 1);
            EXPECT_TRUE(header.at("X-Wap-Profile")
                        .count(gps::android::GpsXtraDownloader::x_wap_profile_value) == 1);

            mg_send_status(conn, 200);
            mg_send_data(conn, &data, data_size);

            return MG_TRUE;
        }
    };

    auto server = core::posix::fork(
                std::bind(testing::a_web_server(configuration), cps),
                core::posix::StandardStream::empty);

    cps.wait_for_signal_ready_for(std::chrono::seconds{2});

    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    gps::android::GpsXtraDownloader::Configuration config;
    config.xtra_hosts.push_back("http://127.0.0.1:5000");

    gps::android::NetCppGpsXtraDownloader downloader;
    auto result = downloader.download_xtra_data(config);

    EXPECT_EQ(data_size, result.size());
    for (int i = 0; i < data_size; i++)
        EXPECT_EQ(i%2, result[i]);
}

TEST(GpsXtraDownloader, throws_for_unreachable_host)
{
    gps::android::GpsXtraDownloader::Configuration config;
    config.xtra_hosts.push_back("http://does_not_exist.host.com/");

    gps::android::NetCppGpsXtraDownloader downloader;
    EXPECT_ANY_THROW(downloader.download_xtra_data(config));
}

TEST(GpsXtraDownloader, download_attempt_throws_if_timeout_is_reached)
{
    testing::web::server::Configuration web_serverconfiguration
    {
        5000,
        [](mg_connection*)
        {
            return MG_TRUE;
        }
    };

    core::testing::CrossProcessSync cps; // server - ready -> client

    auto server = core::posix::fork(
                std::bind(testing::a_web_server(web_serverconfiguration), cps),
                core::posix::StandardStream::empty);

    cps.wait_for_signal_ready_for(std::chrono::seconds{2});

    gps::android::GpsXtraDownloader::Configuration download_config;
    download_config.xtra_hosts.push_back("http://127.0.0.1:5000");

    gps::android::NetCppGpsXtraDownloader downloader;
    EXPECT_ANY_THROW(downloader.download_xtra_data(download_config));
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
TEST(GpsProvider, DISABLED_accessing_starting_and_stopping_gps_provider_works_requires_hardware)
{
    com::ubuntu::location::providers::gps::Provider provider;
    EXPECT_NO_THROW(provider.start_position_updates());
    EXPECT_NO_THROW(provider.stop_position_updates());
    EXPECT_NO_THROW(provider.start_velocity_updates());
    EXPECT_NO_THROW(provider.stop_velocity_updates());
    EXPECT_NO_THROW(provider.start_heading_updates());
    EXPECT_NO_THROW(provider.stop_heading_updates());
}

// We are carrying out quite some positioning here and leverage that fact for feeding location
// and wifi/cell data to Mozilla location service instances. Please note that we feed to the mozilla location service
// in the general case.
#include <com/ubuntu/location/service/harvester.h>
#include <com/ubuntu/location/service/ichnaea_reporter.h>

namespace
{
struct NullReporter : public location::service::Harvester::Reporter
{
    NullReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    void start()
    {
    }

    /** @brief Tell the reporter to shut down its operation. */
    void stop()
    {
    }

    /**
     * @brief Triggers the reporter to send off the information.
     */
    void report(const location::Update<location::Position>&,
                const std::vector<location::connectivity::WirelessNetwork::Ptr>&,
                const std::vector<location::connectivity::RadioCell::Ptr>&)
    {
    }
};

struct HardwareAbstractionLayerFixture : public ::testing::Test
{
    // If this key is set to any value in the environment, we send off data to Mozilla location
    // service instances. ENABLE_HARVESTING_DURING_TESTS
    static constexpr const char* enable_harvesting{"enable_harvesting_during_tests"};
    // The host name of the Mozilla location service instance.
    static constexpr const char* ichnaea_host{"ichnaea_host"};
    // The API key to submit under.
    static constexpr const char* ichnaea_api_key{"ichnaea_api_key"};
    // The API key to submit under.
    static constexpr const char* ichnaea_nickname{"ichnaea_nickname"};
    // Reference latitude value.
    static constexpr const char* ref_lat{"ref_lat"};
    // Reference longitude value.
    static constexpr const char* ref_lon{"ref_lon"};
    // Reference horizontal accuracy value.
    static constexpr const char* ref_accuracy{"ref_accuracy"};
    // SUPL host address
    static constexpr const char* supl_host{"supl_host"};
    // SUPL port
    static constexpr const char* supl_port{"supl_port"};

    static location::ProgramOptions init_options()
    {
        location::ProgramOptions options;

        options.environment_prefix("GPS_TEST_");

        options.add(enable_harvesting,
                    "If this key is set to any value in the environment, "
                    "we send off data to Mozilla location service instances.",
                    false);

        options.add(ichnaea_host,
                    "The host name of the Mozilla location service instance.",
                    std::string{"https://location.services.mozilla.com"});

        options.add(ichnaea_api_key,
                    "The API key to submit under.",
                    std::string{"ubuntu_location_service_test_cases"});

        options.add(ichnaea_nickname,
                    "The nickname to submit under.",
                    std::string{"ubuntu_location_service"});

        options.add(ref_lat,
                    "Reference latitude value.",
                    double{51.444670});

        options.add(ref_lon,
                    "Reference longitude value.",
                    double{7.210852});

        options.add(ref_accuracy,
                    "Reference horizontal accuracy value.",
                    double{10});

        options.add(supl_host,
                    "SUPL host to use for positioning benchmarks.",
                    std::string{"supl.google.com"});

        options.add(supl_port,
                    "SUPL port to use for positioning benchmarks.",
                    std::uint16_t{7476});

        return options;
    }

    HardwareAbstractionLayerFixture()
    {
        options.print(LOG(INFO));
        harvester.start();
    }

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

    bool IsHarvestingDuringTestsEnabled() const
    {
        return options.value_for_key<bool>(enable_harvesting);
    }

    location::ProgramOptions options = init_options();
    bool options_parsed_from_env
    {
        options.parse_from_environment()
    };
    // The Ichnaea instance and its configuration.
    location::service::ichnaea::Reporter::Configuration reporter_configuration
    {
        options.value_for_key<std::string>(ichnaea_host),
        options.value_for_key<std::string>(ichnaea_api_key),
        options.value_for_key<std::string>(ichnaea_nickname)
    };
    std::shared_ptr<location::service::ichnaea::Reporter> reporter
    {
        new location::service::ichnaea::Reporter{reporter_configuration}
    };
    // The harvester instance and its configuration.
    location::service::Harvester::Configuration harvester_configuration
    {
        location::connectivity::platform_default_manager(),
        reporter
    };
    location::service::Harvester harvester
    {
        harvester_configuration
    };
    // Reference position for SUPL benchmarks.
    location::Position ref_pos
    {
        location::wgs84::Latitude
        {
            options.value_for_key<double>(ref_lat) * location::units::Degrees
        },
        location::wgs84::Longitude
        {
            options.value_for_key<double>(ref_lon) * location::units::Degrees
        },
        location::wgs84::Altitude
        {
            0 * location::units::Meter
        },
        location::units::Quantity<location::units::Length>
        {
            options.value_for_key<double>(ref_accuracy) * location::units::Meters
        }
    };
};
}

TEST_F(HardwareAbstractionLayerFixture, DISABLED_provider_construction_works_requires_hardware)
{
    {
        location::ProviderFactory::instance().create_provider_for_name_with_config("gps::Provider", location::Configuration{});
    }

    {
        gps::Provider provider;
    }

    {
        gps::Provider::create_instance(location::Configuration{});
    }

    {
        location::ProviderFactory::instance().create_provider_for_name_with_config("gps::Provider", location::Configuration{});
    }
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
    hal->position_updates().connect([this, &state](const location::Position& pos)
    {
        if (IsHarvestingDuringTestsEnabled())
            harvester.report_position_update(location::Update<location::Position>
            {
                pos, location::Clock::now()
            });
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
    using namespace core::posix;

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

    // We wire up our state to position updates from the hal.
    hal->position_updates().connect([this, &state](const location::Position& pos)
    {
        if (IsHarvestingDuringTestsEnabled())
            harvester.report_position_update(location::Update<location::Position>
            {
                pos, location::Clock::now()
            });
        state.on_position_updated(pos);
    });

    for (unsigned int i = 0; i < trials; i++)
    {
        std::cout << "Executing trial " << i << " of " << trials << " trials" << std::endl;

        // We want to force a cold start per trial.
        hal->delete_all_aiding_data();
        state.reset();

        // We want to run in assisted mode
        EXPECT_TRUE(hal->set_assistance_mode(gps::AssistanceMode::mobile_station_based));

        auto supl_host = options.value_for_key<std::string>(HardwareAbstractionLayerFixture::supl_host);
        auto supl_port = options.value_for_key<std::uint16_t>(HardwareAbstractionLayerFixture::supl_port);

        hal->supl_assistant().set_server(supl_host, supl_port);

        auto start = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());
        {
            bool running = true;

            hal->start_positioning();

            std::thread injector([this, hal, &running]()
            {
                while (running)
                {
                    hal->inject_reference_position(ref_pos);
                    std::this_thread::sleep_for(std::chrono::seconds{1});
                }
            });

            // We expect a maximum cold start time of 15 minutes. The theoretical
            // limit is 12.5 minutes, and we add up some grace period to make the
            // test more robust (see http://en.wikipedia.org/wiki/Time_to_first_fix).
            EXPECT_TRUE(state.wait_for_fix_for(std::chrono::seconds{15 * 60}));
            hal->stop_positioning();

            running = false;
            if (injector.joinable())
                injector.join();

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


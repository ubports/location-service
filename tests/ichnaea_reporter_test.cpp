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

#include <com/ubuntu/location/service/ichnaea_reporter.h>

#include "mongoose.h"

#include <core/posix/fork.h>
#include <core/posix/signal.h>
#include <core/posix/wait.h>
#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <json/json.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <condition_variable>

namespace json = Json;
namespace location = com::ubuntu::location;

namespace
{
struct MockWirelessNetwork : public location::connectivity::WirelessNetwork
{
    /** @brief Timestamp when the network became visible. */
    MOCK_CONST_METHOD0(timestamp, const core::Property<std::chrono::system_clock::time_point>&());

    /** @brief Returns the BSSID of the network */
    MOCK_CONST_METHOD0(bssid, const core::Property<std::string>&());

    /** @brief Returns the SSID of the network. */
    MOCK_CONST_METHOD0(ssid, const core::Property<std::string>&());

    /** @brief Returns the mode of the network. */
    MOCK_CONST_METHOD0(mode, const core::Property<Mode>&());

    /** @brief Returns the frequency that the network/AP operates upon. */
    MOCK_CONST_METHOD0(frequency, const core::Property<Frequency>&());

    /** @brief Returns the signal quality of the network/AP in percent. */
    MOCK_CONST_METHOD0(signal_strength, const core::Property<SignalStrength>&());
};

location::Update<location::Position> reference_position_update
{
    {
        location::wgs84::Latitude{9. * location::units::Degrees},
        location::wgs84::Longitude{53. * location::units::Degrees},
        location::wgs84::Altitude{-2. * location::units::Meters}
    },
    location::Clock::now()
};

}

TEST(IchnaeaReporter, issues_correct_posts_requests)
{
    static const location::connectivity::RadioCell ref_cell
    {
        location::connectivity::RadioCell::Gsm
        {
            location::connectivity::RadioCell::Gsm::MCC{42},
            location::connectivity::RadioCell::Gsm::MNC{42},
            location::connectivity::RadioCell::Gsm::LAC{42},
            location::connectivity::RadioCell::Gsm::ID{42},
            location::connectivity::RadioCell::Gsm::SignalStrength{21}
        }
    };
    static const core::Property<std::chrono::system_clock::time_point> ref_timestamp
    {
        std::chrono::system_clock::now()
    };

    static const core::Property<std::string> ref_bssid
    {
        "bssid:42"
    };

    static const core::Property<std::string> ref_ssid
    {
        "ssid:42"
    };

    static const core::Property<location::connectivity::WirelessNetwork::Mode> ref_mode
    {
        location::connectivity::WirelessNetwork::Mode::infrastructure
    };

    static const core::Property<location::connectivity::WirelessNetwork::Frequency> ref_frequency
    {
        location::connectivity::WirelessNetwork::Frequency{4242}
    };

    static const core::Property<location::connectivity::WirelessNetwork::SignalStrength> ref_strength
    {
        location::connectivity::WirelessNetwork::SignalStrength{80}
    };

    static const std::string api_key
    {
        "test_key"
    };

    core::posix::ChildProcess server = core::posix::fork([]()
    {
        bool terminated = false;

        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        trap->signal_raised().connect([trap, &terminated](core::posix::Signal)
        {
            trap->stop();
            terminated = true;
        });

        struct Handler
        {
            static int on_request(mg_connection* conn, mg_event ev)
            {
                auto thiz = static_cast<Handler*>(conn->server_param);

                switch (ev)
                {
                case MG_REQUEST:
                    return thiz->handle_request(conn);
                case MG_AUTH:
                    return MG_TRUE;
                default:
                    return MG_FALSE;
                }

                return MG_FALSE;
            }

            int handle_request(mg_connection* conn)
            {

                EXPECT_STREQ("/v1/submit", conn->uri);
                EXPECT_STREQ("POST", conn->request_method);
                EXPECT_EQ("key=" + api_key, conn->query_string);
                EXPECT_NE(nullptr, conn->content);
                EXPECT_GT(conn->content_len, 0);

                using namespace location::service::ichnaea;

                json::Reader reader;
                json::Value result;

                EXPECT_TRUE(reader.parse(conn->content, result));

                auto items = result[Reporter::Json::items];
                EXPECT_EQ(1u, items.size());

                auto item = items[0];
                EXPECT_EQ("gsm", item[Reporter::Json::radio].asString());

                EXPECT_DOUBLE_EQ(
                            reference_position_update.value.latitude.value.value(),
                            item[Reporter::Json::lat].asDouble());
                EXPECT_DOUBLE_EQ(
                            reference_position_update.value.longitude.value.value(),
                            item[Reporter::Json::lon].asDouble());

                auto wifis = item[Reporter::Json::wifi];
                EXPECT_EQ(1u, wifis.size());

                auto wifi = wifis[0];
                EXPECT_EQ(ref_bssid.get(), wifi[Reporter::Json::Wifi::key].asString());
                EXPECT_EQ(ref_frequency->get(), wifi[Reporter::Json::Wifi::frequency].asInt());

                auto cells = item[Reporter::Json::cell];
                EXPECT_EQ(1u, cells.size());

                auto cell = cells[0];
                EXPECT_EQ(ref_cell.gsm().mobile_country_code.get(), cell[Reporter::Json::Cell::mcc].asInt());
                EXPECT_EQ(ref_cell.gsm().mobile_network_code.get(), cell[Reporter::Json::Cell::mnc].asInt());
                EXPECT_EQ(ref_cell.gsm().location_area_code.get(), cell[Reporter::Json::Cell::lac].asInt());
                EXPECT_EQ(ref_cell.gsm().id.get(), cell[Reporter::Json::Cell::cid].asInt());
                EXPECT_EQ(ref_cell.gsm().strength.get(), cell[Reporter::Json::Cell::asu].asInt());

                mg_send_status(conn, static_cast<int>(submit::success));
                return MG_TRUE;
            }
        } handler;

        std::thread trap_worker
        {
            [trap]()
            {
                trap->run();
            }
        };

        auto server = mg_create_server(&handler, Handler::on_request);
        mg_set_option(server, "listening_port", "5000");

        for (;;) {
            mg_poll_server(server, 200);

            if (terminated)
                break;
        }

        // Cleanup, and free server instance
        mg_destroy_server(&server);

        if (trap_worker.joinable())
            trap_worker.join();

        return HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    using namespace ::testing;

    auto wireless_network = std::make_shared<NiceMock<MockWirelessNetwork> >();
    ON_CALL(*wireless_network, timestamp()).WillByDefault(ReturnRef(ref_timestamp));
    ON_CALL(*wireless_network, bssid()).WillByDefault(ReturnRef(ref_bssid));
    ON_CALL(*wireless_network, ssid()).WillByDefault(ReturnRef(ref_ssid));
    ON_CALL(*wireless_network, mode()).WillByDefault(ReturnRef(ref_mode));
    ON_CALL(*wireless_network, frequency()).WillByDefault(ReturnRef(ref_frequency));
    ON_CALL(*wireless_network, signal_strength()).WillByDefault(ReturnRef(ref_strength));

    location::service::ichnaea::Reporter::Configuration config
    {
        "http://127.0.0.1:5000",
        api_key
    };

    location::service::ichnaea::Reporter reporter{config};

    reporter.start();
    reporter.report(reference_position_update, {wireless_network}, {ref_cell});

    std::this_thread::sleep_for(std::chrono::milliseconds{500});

    server.send_signal_or_throw(core::posix::Signal::sig_term);
    auto result = server.wait_for(core::posix::wait::Flags::untraced);

    EXPECT_EQ(core::posix::wait::Result::Status::exited, result.status);
    EXPECT_EQ(core::posix::exit::Status::success, result.detail.if_exited.status);
}

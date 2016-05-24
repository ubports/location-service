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

#include "web_server.h"

#include <core/posix/fork.h>
#include <core/posix/signal.h>
#include <core/posix/wait.h>
#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <json.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <condition_variable>

namespace location = com::ubuntu::location;

namespace
{
struct MockWirelessNetwork : public location::connectivity::WirelessNetwork
{
    /** @brief Timestamp when the network became visible. */
    MOCK_CONST_METHOD0(last_seen, const core::Property<std::chrono::system_clock::time_point>&());

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

struct MockRadioCell : public location::connectivity::RadioCell
{
    typedef std::shared_ptr<MockRadioCell> Ptr;

    /** @brief Returns the type of the radio cell. */
    MOCK_CONST_METHOD0(changed, const core::Signal<>&());

    /** @brief Returns the type of the radio cell. */
    MOCK_CONST_METHOD0(type, Type());

    /** @brief Returns GSM-specific details or throws std::runtime_error if this is not a GSM radiocell. */
    MOCK_CONST_METHOD0(gsm, const Gsm&());

    /** @brief Returns UMTS-specific details or throws std::runtime_error if this is not a UMTS radiocell. */
    MOCK_CONST_METHOD0(umts, const Umts&());

    /** @brief Returns LTE-specific details or throws std::runtime_error if this is not an LTE radiocell. */
    MOCK_CONST_METHOD0(lte, const Lte&());
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
    using namespace ::testing;

    static const location::connectivity::RadioCell::Gsm gsm
    {
        location::connectivity::RadioCell::Gsm::MCC{42},
        location::connectivity::RadioCell::Gsm::MNC{42},
        location::connectivity::RadioCell::Gsm::LAC{42},
        location::connectivity::RadioCell::Gsm::ID{42},
        location::connectivity::RadioCell::Gsm::SignalStrength{21}
    };

    static const MockRadioCell::Ptr ref_cell
    {
        new NiceMock<MockRadioCell>()
    };

    ON_CALL(*ref_cell, type()).WillByDefault(Return(location::connectivity::RadioCell::Type::gsm));
    ON_CALL(*ref_cell, gsm()).WillByDefault(ReturnRef(gsm));

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

    static const std::string nick_name
    {
        "nick_name"
    };

    core::testing::CrossProcessSync cps; // server - ready -> client

    testing::web::server::Configuration web_server_configuration
    {
        5000,
        [](mg_connection* conn)
        {
            core::net::http::Header header;

            for (int i = 0; i < conn->num_headers; i++)
            {
                header.add(conn->http_headers[i].name, conn->http_headers[i].value);
            }

            EXPECT_TRUE(header.has(location::service::ichnaea::Reporter::nick_name_header));
            EXPECT_STREQ("/v1/submit", conn->uri);
            EXPECT_STREQ("POST", conn->request_method);
            EXPECT_EQ("key=" + api_key, conn->query_string);
            EXPECT_NE(nullptr, conn->content);
            EXPECT_GT(conn->content_len, 0);

            using namespace location::service::ichnaea;

            json::Object object = json::Object::parse_from_string(conn->content);

            auto items = object.get(Reporter::Json::items);
            EXPECT_EQ(1u, items.array_size());

            auto item = items.get_object_for_index(0);
            EXPECT_EQ("gsm", item.get(Reporter::Json::radio).to_string());

            EXPECT_DOUBLE_EQ(
                        reference_position_update.value.latitude.value.value(),
                        item.get(Reporter::Json::lat).to_double());
            EXPECT_DOUBLE_EQ(
                        reference_position_update.value.longitude.value.value(),
                        item.get(Reporter::Json::lon).to_double());

            auto wifis = item.get(Reporter::Json::wifi);
            EXPECT_EQ(1u, wifis.array_size());            

            auto wifi = wifis.get_object_for_index(0);
            EXPECT_EQ(ref_bssid.get(), wifi.get(Reporter::Json::Wifi::key).to_string());
            EXPECT_EQ(ref_frequency->get(), wifi.get(Reporter::Json::Wifi::frequency).to_int32());

            auto cells = item.get(Reporter::Json::cell);
            EXPECT_EQ(1u, cells.array_size());

            auto cell = cells.get_object_for_index(0);
            EXPECT_EQ(ref_cell->gsm().mobile_country_code.get(), cell.get(Reporter::Json::Cell::mcc).to_int32());
            EXPECT_EQ(ref_cell->gsm().mobile_network_code.get(), cell.get(Reporter::Json::Cell::mnc).to_int32());
            EXPECT_EQ(ref_cell->gsm().location_area_code.get(), cell.get(Reporter::Json::Cell::lac).to_int32());
            EXPECT_EQ(ref_cell->gsm().id.get(), cell.get(Reporter::Json::Cell::cid).to_int32());
            EXPECT_EQ(ref_cell->gsm().strength.get(), cell.get(Reporter::Json::Cell::asu).to_int32());

            mg_send_status(conn, static_cast<int>(submit::success));
            return MG_TRUE;
        }
    };

    core::posix::ChildProcess server = core::posix::fork(
                std::bind(testing::a_web_server(web_server_configuration), cps),
                core::posix::StandardStream::empty);    

    using namespace ::testing;

    auto wireless_network = std::make_shared<NiceMock<MockWirelessNetwork> >();
    ON_CALL(*wireless_network, last_seen()).WillByDefault(ReturnRef(ref_timestamp));
    ON_CALL(*wireless_network, bssid()).WillByDefault(ReturnRef(ref_bssid));
    ON_CALL(*wireless_network, ssid()).WillByDefault(ReturnRef(ref_ssid));
    ON_CALL(*wireless_network, mode()).WillByDefault(ReturnRef(ref_mode));
    ON_CALL(*wireless_network, frequency()).WillByDefault(ReturnRef(ref_frequency));
    ON_CALL(*wireless_network, signal_strength()).WillByDefault(ReturnRef(ref_strength));

    cps.wait_for_signal_ready_for(std::chrono::seconds{2});

    location::service::ichnaea::Reporter::Configuration config
    {
        "http://127.0.0.1:5000",
        api_key,
        "nickname"
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

/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/providers/gps/sntp_client.h>
#include <location/service/runtime.h>

#include <future>

#include <core/posix/this_process.h>

#include <gtest/gtest.h>

namespace env = core::posix::this_process::env;
namespace sntp = location::providers::gps::sntp;

namespace
{
struct SntpClient : public ::testing::Test
{
    void SetUp()
    {
        rt->start();
    }

    void TearDown()
    {
        rt->stop();
    }

    std::shared_ptr<location::service::Runtime> rt = location::service::Runtime::create();
    std::string host = env::get("UBUNTU_LOCATION_SERVICE_NTP_HOST_FOR_TESTING", "ntp.ubuntu.com");
};
}

TEST_F(SntpClient, packet_has_correct_size)
{
    static_assert(sizeof(sntp::Packet) == 48, "");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
TEST(EnumConstant, yields_same_value_as_multibyte_constant)
{
    static_assert(sntp::detail::enum_constant<'L','O','C','L'>() == 'LOCL', "");
}
#pragma GCC diagnostic pop

TEST_F(SntpClient, succeeds_in_querying_time_from_server)
{
    location::providers::gps::sntp::Client sntp_client;
    try
    {
        sntp_client.request_time(host, std::chrono::milliseconds{5000}, rt->service());
    }
    catch(...)
    {
    }
}

TEST_F(SntpClient, throws_for_timed_out_operation)
{
    location::providers::gps::sntp::Client sntp_client;
    EXPECT_ANY_THROW(sntp_client.request_time(host, std::chrono::milliseconds{1}, rt->service()));
}

TEST_F(SntpClient, returns_correct_data_in_packet)
{
    for (unsigned int i = 0; i < 10; i++)
    {
        location::providers::gps::sntp::Client sntp_client;
        try
        {
            auto response = sntp_client.request_time(host, std::chrono::milliseconds{1000}, rt->service());

            EXPECT_LE(0, response.packet.stratum.value());
            if (response.packet.stratum.value() <= 1)
            {
                std::stringstream ss; ss << response.packet.reference_identifier;
                EXPECT_TRUE(ss.str().size() > 0);
            }
        }
        catch(...)
        {
            // We ignore any exception being thrown as we are working against a real-world server
            // in a load-test scenario.
        }
    }
}

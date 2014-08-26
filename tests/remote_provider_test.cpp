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
 * Authored by: Manuel de la Peña <manuel.delapena@canonical.com>
 */

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/proxy_provider.h>
#include <com/ubuntu/location/providers/remote/provider.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;
namespace remote = com::ubuntu::location::providers::remote;

using namespace ::testing;


MATCHER_P(postion_equals_tuple, value, "Returns if the string maps are equal.") {
    auto pos = arg.value;

    // convert the tuple to the correct units
    cul::wgs84::Longitude longitude {std::get<0>(value)* cul::units::Degrees};
    cul::wgs84::Latitude latitude {std::get<1>(value)* cul::units::Degrees};
    cul::wgs84::Altitude altitude {std::get<2>(value)* cul::units::Meters};

    return longitude == pos.longitude && latitude == pos.latitude && altitude == pos.altitude;
}

class MockEventConsumer
{
 public:
    MockEventConsumer() {}

    MOCK_METHOD1(on_new_position, void(const cul::Update<cul::Position>&));
};

TEST(RemoteProvider, matches_criteria)
{
    auto conf = remote::Provider::Configuration{};
    conf.name = "com.ubuntu.espoo.Service.Provider";
    conf.path = "/com/ubuntu/espoo/Service/Provider";

    remote::Provider provider(conf);

    EXPECT_FALSE(provider.requires(com::ubuntu::location::Provider::Requirements::satellites));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::cell_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::data_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::monetary_spending));
}

TEST(RemoteProvider, updates_are_fwd)
{
    // update received from the remote provider in a tuple
    std::tuple<double, double, double, double, uint32_t> update{3, 4, 4, 4, 0}; 

    auto conf = remote::Provider::Configuration{};
    conf.name = "com.ubuntu.espoo.Service.Provider";
    conf.path = "/com/ubuntu/espoo/Service/Provider";

    remote::Provider provider{conf};

    cul::Provider::Ptr p1{std::addressof(provider), [](cul::Provider*){}};

    cul::ProviderSelection selection{p1, p1, p1};

    cul::ProxyProvider pp{selection};

    MockEventConsumer mec;
    EXPECT_CALL(mec, on_new_position(postion_equals_tuple(update))).Times(1);

    pp.updates().position.connect([&mec](const cul::Update<cul::Position>& p){mec.on_new_position(p);});

    // create an update to be processed
    provider.on_position_changed(update);
}

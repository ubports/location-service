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
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/session.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
struct MockProvider : public com::ubuntu::location::Provider
{
    MOCK_METHOD1(subscribe_to_position_updates,
                 com::ubuntu::location::ChannelConnection(std::function<void(const com::ubuntu::location::Update<com::ubuntu::location::Position>&)>));

    MOCK_METHOD1(subscribe_to_heading_updates,
                 com::ubuntu::location::ChannelConnection(std::function<void(const com::ubuntu::location::Update<com::ubuntu::location::Heading>&)>));

    MOCK_METHOD1(subscribe_to_velocity_updates,
                 com::ubuntu::location::ChannelConnection(std::function<void(const com::ubuntu::location::Update<com::ubuntu::location::Velocity>&)>));
};
}

TEST(LocationSession, constructing_with_a_null_provider_throws)
{
    EXPECT_ANY_THROW(com::ubuntu::location::Session session(com::ubuntu::location::Provider::Ptr {}));
}

TEST(LocationSession, changing_provider_association_of_session_results_in_connections_to_new_provider)
{
    using namespace ::testing;
    NiceMock<MockProvider> mock_provider1;
    ON_CALL(mock_provider1, subscribe_to_position_updates(_))
    .WillByDefault(Return(com::ubuntu::location::ChannelConnection()));
    ON_CALL(mock_provider1, subscribe_to_heading_updates(_))
    .WillByDefault(Return(com::ubuntu::location::ChannelConnection()));
    ON_CALL(mock_provider1, subscribe_to_velocity_updates(_))
    .WillByDefault(Return(com::ubuntu::location::ChannelConnection()));

    NiceMock<MockProvider> mock_provider2;
    EXPECT_CALL(mock_provider2, subscribe_to_position_updates(_))
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));
    EXPECT_CALL(mock_provider2, subscribe_to_heading_updates(_))
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));
    EXPECT_CALL(mock_provider2, subscribe_to_velocity_updates(_))
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));

    com::ubuntu::location::Session session(com::ubuntu::location::Provider::Ptr {&mock_provider1, [](com::ubuntu::location::Provider*) {}});
    session.change_provider_assocation_to(com::ubuntu::location::Provider::Ptr {&mock_provider2, [](com::ubuntu::location::Provider*) {}});
}

TEST(LocationSession, installing_updates_handlers_connects_them_to_the_provider_associated_to_the_session)
{
    using namespace ::testing;

    NiceMock<MockProvider> mock_provider;

    EXPECT_CALL(mock_provider, subscribe_to_position_updates(_))
    .Times(1)
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));
    EXPECT_CALL(mock_provider, subscribe_to_heading_updates(_))
    .Times(1)
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));
    EXPECT_CALL(mock_provider, subscribe_to_velocity_updates(_))
    .Times(1)
    .WillRepeatedly(Return(com::ubuntu::location::ChannelConnection()));

    com::ubuntu::location::Session session(com::ubuntu::location::Provider::Ptr {&mock_provider, [](com::ubuntu::location::Provider*) {}});

    session.install_position_updates_handler([](const com::ubuntu::location::Update<com::ubuntu::location::Position>&)
    {
    });
    session.install_heading_updates_handler([](const com::ubuntu::location::Update<com::ubuntu::location::Heading>&)
    {
    });
    session.install_velocity_updates_handler([](const com::ubuntu::location::Update<com::ubuntu::location::Velocity>&)
    {
    });
}

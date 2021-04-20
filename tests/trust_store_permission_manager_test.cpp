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

#include <com/ubuntu/location/service/trust_store_permission_manager.h>

#include <com/ubuntu/location/criteria.h>

#include <core/posix/fork.h>
#include <core/testing/cross_process_sync.h>

#include <gmock/gmock.h>

#include <thread>

#include <sys/apparmor.h>

namespace location = com::ubuntu::location;
namespace service = com::ubuntu::location::service;

namespace
{
struct MockAgent : public core::trust::Agent
{
    /**
     * @brief Presents the given request to the user, returning the user-provided answer.
     * @param request The trust request that a user has to answer.
     * @param description Extended description of the trust request.
     */
    MOCK_METHOD1(authenticate_request_with_parameters, core::trust::Request::Answer(const core::trust::Agent::RequestParameters&));
};

location::Criteria default_criteria;
}

TEST(TrustStorePermissionManager, calls_out_to_agent)
{
    using namespace ::testing;

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(1)
            .WillRepeatedly(Return(core::trust::Request::Answer::denied));

    service::TrustStorePermissionManager pm
    {
        mock_agent,
    };

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(
                  default_criteria,
                  location::service::Credentials{::getpid(), ::getuid(), "some.profile"}));
}

TEST(TrustStorePermissionManager, returns_rejected_for_throwing_agent)
{
    using namespace ::testing;

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(1)
            .WillRepeatedly(Throw(std::runtime_error{"Thrown from mock agent"}));

    service::TrustStorePermissionManager pm{mock_agent};

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(default_criteria,
                                                  location::service::Credentials{::getpid(), ::getuid(), "unconfined"}));
}

TEST(TrustStorePermissionManager, resolves_app_id)
{
    using namespace ::testing;

    const pid_t pid = ::getpid();
    const uid_t uid = ::getuid();
    const std::string profile = "does.not.exist";

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(1)
            .WillRepeatedly(Return(core::trust::Request::Answer::denied));

    service::TrustStorePermissionManager pm
    {
        mock_agent,
    };

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(default_criteria,
                                                  location::service::Credentials{pid, uid, profile}));
}

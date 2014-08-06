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

struct MockAppArmorProfileResolver
{
    MOCK_METHOD1(resolve_pid_to_app_armor_profile, std::string(const core::trust::Pid&));

    service::TrustStorePermissionManager::AppArmorProfileResolver to_functional()
    {
        return [this](const core::trust::Pid& pid)
        {
            return resolve_pid_to_app_armor_profile(pid);
        };
    }
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
        service::TrustStorePermissionManager::libapparmor_profile_resolver()
    };

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(
                  default_criteria,
                  location::service::Credentials{::getpid(), ::getuid()}));
}

TEST(TrustStorePermissionManager, returns_rejected_for_throwing_agent)
{
    using namespace ::testing;

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(1)
            .WillRepeatedly(Throw(std::runtime_error{"Thrown from mock agent"}));

    service::TrustStorePermissionManager pm{mock_agent, service::TrustStorePermissionManager::libapparmor_profile_resolver()};

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(default_criteria, location::service::Credentials{::getpid(), ::getuid()}));
}

TEST(TrustStorePermissionManager, resolves_app_id)
{
    using namespace ::testing;

    const pid_t pid = ::getpid();
    const uid_t uid = ::getuid();

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(1)
            .WillRepeatedly(Return(core::trust::Request::Answer::denied));

    MockAppArmorProfileResolver resolver;
    EXPECT_CALL(resolver, resolve_pid_to_app_armor_profile(core::trust::Pid{pid}))
            .Times(1)
            .WillRepeatedly(Return(std::string{"does.not.exist"}));

    service::TrustStorePermissionManager pm
    {
        mock_agent,
        resolver.to_functional()
    };

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(default_criteria, location::service::Credentials{pid, uid}));
}

TEST(TrustStorePermissionManager, returns_rejected_for_throwing_app_id_resolver)
{
    using namespace ::testing;

    const pid_t pid = ::getpid();
    const uid_t uid = ::getuid();

    auto mock_agent = std::make_shared<MockAgent>();

    EXPECT_CALL(*mock_agent, authenticate_request_with_parameters(_))
            .Times(0); // This should never be called if we cannot resolve an apparmor profile.

    MockAppArmorProfileResolver resolver;
    EXPECT_CALL(resolver, resolve_pid_to_app_armor_profile(core::trust::Pid{pid}))
            .Times(1)
            .WillRepeatedly(Throw(std::runtime_error{"Thrown from MockAppArmorProfileResolver"}));

    service::TrustStorePermissionManager pm
    {
        mock_agent,
        resolver.to_functional()
    };

    EXPECT_EQ(service::PermissionManager::Result::rejected,
              pm.check_permission_for_credentials(default_criteria, location::service::Credentials{pid, uid}));
}

// We should be provided with this kind of functionality by the trust-store.
// The respective request is captured here:
//   https://bugs.launchpad.net/trust-store/+bug/1350736
TEST(AppArmorProfileResolver, libapparmor_profile_resolver_returns_correct_profile_for_unconfined_process)
{
    auto child = core::posix::fork(
                []() { while (true); return core::posix::exit::Status::success;},
                core::posix::StandardStream::empty);

    EXPECT_EQ("unconfined",
              service::TrustStorePermissionManager::libapparmor_profile_resolver()(core::trust::Pid{child.pid()}));
}

TEST(AppArmorProfileResolver, libapparmor_profile_resolver_throws_for_apparmor_error)
{
    // Passing -1 as the pid value results in the underlying apparmor call failing
    // and the implementation translating to a std::system_error.
    EXPECT_THROW(service::TrustStorePermissionManager::libapparmor_profile_resolver()(core::trust::Pid{-1}),
                 std::system_error);
}

// We disabel this test by default as it requires the developer to take some manual preparations.
// Specifically:
//   sudo apparmor-parser -n for_testing --add tests/app_armor_testing_profile
namespace for_testing
{
    static constexpr const char* an_empty_profile_for_testing_purposes
    {
        "an_empty_profile_for_testing_purposes"
    };
}
TEST(AppArmorProfileResolver, DISABLED_libapparmor_profile_resolver_returns_correct_profile_for_confined_process)
{
    core::testing::CrossProcessSync cps; // child --| aa_profile_changed |--> parent
    auto child = core::posix::fork(
                [&cps]()
                {
                    aa_change_profile(for_testing::an_empty_profile_for_testing_purposes);
                    cps.try_signal_ready_for(std::chrono::milliseconds{500});
                    while (true);
                    return core::posix::exit::Status::success;
                },
                core::posix::StandardStream::empty);

    cps.wait_for_signal_ready_for(std::chrono::milliseconds{500});

    EXPECT_EQ(for_testing::an_empty_profile_for_testing_purposes,
              service::TrustStorePermissionManager::libapparmor_profile_resolver()(core::trust::Pid{child.pid()}));
}

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
#include "cross_process_sync.h"
#include "fork_and_run.h"

#include <com/ubuntu/location/accuracy.h>
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

#include <org/freedesktop/dbus/announcer.h>
#include <org/freedesktop/dbus/resolver.h>

#include <org/freedesktop/dbus/asio/executor.h>

#include <gtest/gtest.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>

namespace cuc = com::ubuntu::connectivity;
namespace cul = com::ubuntu::location;
namespace culss = com::ubuntu::location::service::session;

namespace
{
org::freedesktop::dbus::Bus::Ptr the_session_bus()
{
    org::freedesktop::dbus::Bus::Ptr bus{
        new org::freedesktop::dbus::Bus{org::freedesktop::dbus::WellKnownBus::session}};
    return bus;
}

template<typename T>
cul::Update<T> update_as_of_now(const T& value = T())
{
    return cul::Update<T>{value, cul::Clock::now()};
}

class DummyProvider : public cul::Provider
{
public:
    DummyProvider() : cul::Provider(std::shared_ptr<cuc::Manager>{})
    {
    }

    ~DummyProvider() noexcept
    {
    }

    void inject_update(const cul::Update<cul::Position>& update)
    {
        mutable_updates().position = update;
    }

    void inject_update(const cul::Update<cul::Velocity>& update)
    {
        mutable_updates().velocity = update;
    }

    void inject_update(const cul::Update<cul::Heading>& update)
    {
        mutable_updates().heading = update;
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
}

TEST(LocationServiceStandalone, SessionsReceiveUpdatesViaDBus)
{
    test::CrossProcessSync sync_start;
    test::CrossProcessSync sync_session_created;

    auto server = [&sync_start, &sync_session_created]()
    {
        SCOPED_TRACE("Server");
        auto bus = the_session_bus();
        bus->install_executor(org::freedesktop::dbus::Executor::Ptr(new org::freedesktop::dbus::asio::Executor{bus}));
        auto dummy = new DummyProvider();
        cul::Provider::Ptr helper(dummy);
        cul::service::DefaultConfiguration config;
        
        cul::service::Implementation service(
                    bus,
                    config.the_engine(config.the_provider_set(helper), config.the_provider_selection_policy()),
                    config.the_permission_manager());


        sync_start.signal_ready();

        std::thread t{[bus](){bus->run();}};

        sync_session_created.wait_for_signal_ready();

        dummy->inject_update(reference_position_update);
        dummy->inject_update(reference_velocity_update);
        dummy->inject_update(reference_heading_update);

        if (t.joinable())
            t.join();
    };

    auto client = [&sync_start, &sync_session_created]()
    {
        SCOPED_TRACE("Client");

        sync_start.wait_for_signal_ready();

        auto bus = the_session_bus();
        bus->install_executor(org::freedesktop::dbus::Executor::Ptr(new org::freedesktop::dbus::asio::Executor{bus}));
        std::thread t{[bus](){bus->run();}};
        auto location_service = org::freedesktop::dbus::resolve_service_on_bus<
            cul::service::Interface,
            cul::service::Stub>(bus);
        
        auto s1 = location_service->create_session_for_criteria(cul::Criteria{});

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
        
        s1->updates().position_status = culss::Interface::Updates::Status::enabled;
        s1->updates().heading_status = culss::Interface::Updates::Status::enabled;
        s1->updates().velocity_status = culss::Interface::Updates::Status::enabled;
        
        sync_session_created.signal_ready();

        if (t.joinable())
            t.join();

        EXPECT_EQ(reference_position_update, position);
        EXPECT_EQ(reference_velocity_update, velocity);
        EXPECT_EQ(reference_heading_update, heading);
    };

    EXPECT_NO_FATAL_FAILURE(test::fork_and_run(server, client));
}

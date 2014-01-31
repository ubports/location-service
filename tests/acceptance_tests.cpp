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

#include <core/dbus/announcer.h>
#include <core/dbus/resolver.h>

#include <core/dbus/asio/executor.h>

#include <gtest/gtest.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>

namespace
{

core::dbus::Bus::Ptr the_session_bus()
{
    core::dbus::Bus::Ptr bus{
        new core::dbus::Bus{core::dbus::WellKnownBus::session}};
    return bus;
}

class DummyProvider : public com::ubuntu::location::Provider
{
public:
    DummyProvider()
    {
    }

    ~DummyProvider() noexcept
    {
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Position>& update)
    {
        deliver_position_updates(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Velocity>& update)
    {
        deliver_velocity_updates(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Heading>& update)
    {
        deliver_heading_updates(update);
    }

    bool matches_criteria(const com::ubuntu::location::Criteria& /*criteria*/)
    {
        return true;
    }
};

struct AlwaysGrantingPermissionManager : public com::ubuntu::location::service::PermissionManager
{
    PermissionManager::Result
    check_permission_for_credentials(const com::ubuntu::location::Criteria&, 
                                     const com::ubuntu::location::service::Credentials&)
    {
        return PermissionManager::Result::granted;
    }
};

auto timestamp = com::ubuntu::location::Clock::now();

com::ubuntu::location::Update<com::ubuntu::location::Position> reference_position_update
{
    {
        com::ubuntu::location::wgs84::Latitude{9. * com::ubuntu::location::units::Degrees},
        com::ubuntu::location::wgs84::Longitude{53. * com::ubuntu::location::units::Degrees},
        com::ubuntu::location::wgs84::Altitude{-2. * com::ubuntu::location::units::Meters}
    },
    timestamp
};

com::ubuntu::location::Update<com::ubuntu::location::Velocity> reference_velocity_update
{
    {5. * com::ubuntu::location::units::MetersPerSecond},
    timestamp
};

com::ubuntu::location::Update<com::ubuntu::location::Heading> reference_heading_update
{
    {120. * com::ubuntu::location::units::Degrees},
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
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto dummy = new DummyProvider();
        com::ubuntu::location::Provider::Ptr helper(dummy);
        com::ubuntu::location::service::DefaultConfiguration config;
        
        com::ubuntu::location::service::Implementation service(bus,
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
        bus->install_executor(core::dbus::asio::make_executor(bus));
        std::thread t{[bus](){bus->run();}};
        auto location_service = core::dbus::resolve_service_on_bus<
            com::ubuntu::location::service::Interface, 
            com::ubuntu::location::service::Stub>(bus);
        
        auto s1 = location_service->create_session_for_criteria(com::ubuntu::location::Criteria{});
        
        com::ubuntu::location::Update<com::ubuntu::location::Position> position;
        s1->install_position_updates_handler(
            [&](const com::ubuntu::location::Update<com::ubuntu::location::Position>& new_position) {
                std::cout << "On position updated: " << new_position << std::endl;
                position = new_position;
            });
        com::ubuntu::location::Update<com::ubuntu::location::Velocity> velocity;
        s1->install_velocity_updates_handler(
            [&](const com::ubuntu::location::Update<com::ubuntu::location::Velocity>& new_velocity) {
                std::cout << "On velocity_changed " << new_velocity << std::endl;
                velocity = new_velocity;
            });
        com::ubuntu::location::Update<com::ubuntu::location::Heading> heading;
        s1->install_heading_updates_handler(
            [&](const com::ubuntu::location::Update<com::ubuntu::location::Heading>& new_heading) {
                std::cout << "On heading changed: " << new_heading << std::endl;
                heading = new_heading;
                bus->stop();
            });
        
        s1->start_position_updates();
        s1->start_velocity_updates();
        s1->start_heading_updates();
        
        sync_session_created.signal_ready();

        if (t.joinable())
            t.join();

        EXPECT_EQ(reference_position_update, position);
        EXPECT_EQ(reference_velocity_update, velocity);
        EXPECT_EQ(reference_heading_update, heading);
    };

    EXPECT_NO_FATAL_FAILURE(test::fork_and_run(server, client));
}

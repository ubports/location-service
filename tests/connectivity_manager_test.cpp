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

#include <com/ubuntu/location/connectivity/manager.h>

#include <com/ubuntu/location/connectivity/ofono_nm_connectivity_manager.h>

#include "did_finish_successfully.h"
#include "mock_network_manager.h"
#include "mock_ofono.h"

#include <core/dbus/fixture.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/fork.h>
#include <core/posix/signal.h>

#include <gtest/gtest.h>

namespace location = com::ubuntu::location;
namespace connectivity = com::ubuntu::location::connectivity;

namespace
{
std::shared_ptr<core::posix::SignalTrap> a_trap_stopping_itself()
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_term, core::posix::Signal::sig_int
    });

    trap->signal_raised().connect([trap](core::posix::Signal)
    {
        trap->stop();
    });

    return trap;
}

struct ConnectivityManager : public core::dbus::testing::Fixture
{
    // A functional for setting up mock Ofono and NetworkManager instances.
    typedef std::function<
        void(
            mock::NetworkManager&, // The mock network manager instance
            mock::Ofono::Manager&, // The mock ofono instance
            std::shared_ptr<core::posix::SignalTrap> // The signal trap
            )
    > ServiceSetup;

    std::function<core::posix::exit::Status()> create_service_with_setup(const ServiceSetup& setup)
    {
        return [this, setup]()
        {
            auto trap = a_trap_stopping_itself();

            auto bus = session_bus(); bus->install_executor(core::dbus::asio::make_executor(bus));
            std::thread worker{[bus]() { bus->run(); }};

            auto nm = core::dbus::Service::add_service(bus, xdg::NetworkManager::name());
            auto ofono = core::dbus::Service::add_service(bus, org::Ofono::name());

            auto nm_manager = nm->add_object_for_path(core::dbus::types::ObjectPath{"/org/freedesktop/NetworkManager"});
            auto ofono_manager = ofono->add_object_for_path(core::dbus::types::ObjectPath{"/"});

            ::testing::NiceMock<mock::NetworkManager> nm_mock(bus, nm, nm_manager);
            ::testing::NiceMock<mock::Ofono::Manager> ofono_mock(bus, ofono_manager);

            setup(nm_mock, ofono_mock, trap);

            bus->stop();

            if(worker.joinable())
                worker.join();

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                                   core::posix::exit::Status::success;
        };
    }
};
}
/*
TEST(RadioCell, explicit_construction_yields_correct_type)
{
    {
        location::connectivity::RadioCell::Gsm gsm
        {
            location::connectivity::RadioCell::Gsm::MCC{42},
            location::connectivity::RadioCell::Gsm::MNC{42},
            location::connectivity::RadioCell::Gsm::LAC{42},
            location::connectivity::RadioCell::Gsm::ID{42},
            location::connectivity::RadioCell::Gsm::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{gsm};

        EXPECT_EQ(location::connectivity::RadioCell::Type::gsm, cell.type());
    }

    {
        location::connectivity::RadioCell::Umts umts
        {
            location::connectivity::RadioCell::Umts::MCC{42},
            location::connectivity::RadioCell::Umts::MNC{42},
            location::connectivity::RadioCell::Umts::LAC{42},
            location::connectivity::RadioCell::Umts::ID{42},
            location::connectivity::RadioCell::Umts::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{umts};

        EXPECT_EQ(location::connectivity::RadioCell::Type::umts, cell.type());
    }

    {
        location::connectivity::RadioCell::Lte lte
        {
            location::connectivity::RadioCell::Lte::MCC{42},
            location::connectivity::RadioCell::Lte::MNC{42},
            location::connectivity::RadioCell::Lte::TAC{42},
            location::connectivity::RadioCell::Lte::ID{42},
            location::connectivity::RadioCell::Lte::PID{42},
            location::connectivity::RadioCell::Lte::SignalStrength{21}
        };

        location::connectivity::RadioCell cell{lte};

        EXPECT_EQ(location::connectivity::RadioCell::Type::lte, cell.type());
    }
}*/

TEST(ConnectivityManagerOnDevice, repeatedly_requesting_network_scans_works_requires_hardware)
{
    auto manager = location::connectivity::platform_default_manager();

    for (unsigned int i = 0; i < 100; i++)
        manager->request_scan_for_wireless_networks();
}

TEST(ConnectivityManagerOnDevice, repeatedly_querying_the_connected_cell_works_requires_hardware)
{
    auto manager = location::connectivity::platform_default_manager();

    for (unsigned int i = 0; i < 100; i++)
    {
        manager->enumerate_connected_radio_cells([](const location::connectivity::RadioCell::Ptr& cell)
        {
            std::cout << *cell << std::endl;
        });
    }
}

TEST(ConnectivityManagerOnDevice, default_implementation_is_queryable_for_wifis_and_radio_cells_requires_hardware)
{
    auto manager = location::connectivity::platform_default_manager();

    manager->enumerate_connected_radio_cells([](const location::connectivity::RadioCell::Ptr& cell)
    {
        std::cout << *cell << std::endl;
    });

    manager->enumerate_visible_wireless_networks([](location::connectivity::WirelessNetwork::Ptr wifi)
    {
        std::cout << *wifi << std::endl;
    });
}

TEST(ConnectivityManagerOnDevice, default_implementation_is_queryable_for_wifi_and_wwan_status_requires_hardware)
{
    auto manager = location::connectivity::platform_default_manager();

    std::cout << std::boolalpha << manager->is_wifi_enabled().get() << std::endl;
    std::cout << std::boolalpha << manager->is_wwan_enabled().get() << std::endl;
    std::cout << std::boolalpha << manager->is_wifi_hardware_enabled().get() << std::endl;
    std::cout << std::boolalpha << manager->is_wwan_hardware_enabled().get() << std::endl;
}

TEST_F(ConnectivityManager, queries_devices_and_modems_when_initialized)
{
    auto service_proc = core::posix::fork(create_service_with_setup([](mock::NetworkManager& nm, mock::Ofono::Manager& ofono, std::shared_ptr<core::posix::SignalTrap> trap)
    {
        EXPECT_CALL(nm, get_devices()).Times(1);
        EXPECT_CALL(ofono, get_modems()).Times(1);

        trap->run();
    }), core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::seconds{1});

    auto client_proc = core::posix::fork([this]()
    {
        auto bus = session_bus(); bus->install_executor(core::dbus::asio::make_executor(bus));
        std::thread worker{[bus]() { bus->run(); }};

        connectivity::OfonoNmConnectivityManager cm{bus};

        bus->stop();

        if (worker.joinable())
            worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    EXPECT_TRUE(did_finish_successfully(client_proc.wait_for(core::posix::wait::Flags::untraced)));
    service_proc.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(service_proc.wait_for(core::posix::wait::Flags::untraced)));
}

TEST_F(ConnectivityManager, correctly_handles_wifi_devices_and_aps_on_init)
{
    using namespace ::testing;

    auto service_proc = core::posix::fork(create_service_with_setup([](mock::NetworkManager& nm, mock::Ofono::Manager& ofono, std::shared_ptr<core::posix::SignalTrap> trap)
    {
        std::vector<mock::NetworkManager::AccessPoint> aps;
        for (unsigned int i = 0; i < 10; i++)
        {
            mock::NetworkManager::AccessPoint ap
            {
                nm.service->add_object_for_path(core::dbus::types::ObjectPath
                {
                    "/ap_" + std::to_string(i)
                })
            };
            aps.push_back(ap);
        }

        auto aps_to_paths = [&aps]() -> std::vector<core::dbus::types::ObjectPath>
        {
            std::vector<core::dbus::types::ObjectPath> paths;
            for(const auto& ap : aps)
                paths.push_back(ap.object->path());
            return paths;
        };

        mock::NetworkManager::Device wifi
        {
            nm.bus,
            nm.service->add_object_for_path(core::dbus::types::ObjectPath{"/wifi"})
        };
        wifi.properties.device_type->set((std::uint32_t)xdg::NetworkManager::Device::Type::wifi);

        EXPECT_CALL(wifi, get_access_points()).Times(1).WillOnce(Return(aps_to_paths()));
        EXPECT_CALL(nm, get_devices())
            .Times(1)
            .WillOnce(
                Return([&wifi]()
                {
                    return std::vector<core::dbus::types::ObjectPath>({wifi.object->path()});
                }()));
        EXPECT_CALL(ofono, get_modems()).Times(1);

        trap->run();
    }), core::posix::StandardStream::empty);

    std::this_thread::sleep_for(std::chrono::seconds{1});

    auto client_proc = core::posix::fork([this]()
    {
        auto bus = session_bus(); bus->install_executor(core::dbus::asio::make_executor(bus));
        std::thread worker{[bus]() { bus->run(); }};

        connectivity::OfonoNmConnectivityManager cm{bus};

        std::uint32_t wifi_counter{0};
        cm.enumerate_visible_wireless_networks([&wifi_counter](const connectivity::WirelessNetwork::Ptr&)
        {
            wifi_counter++;
        });
        EXPECT_EQ(10, wifi_counter);

        bus->stop();

        if (worker.joinable())
            worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure :
                                               core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    EXPECT_TRUE(did_finish_successfully(client_proc.wait_for(core::posix::wait::Flags::untraced)));
    service_proc.send_signal_or_throw(core::posix::Signal::sig_term);
    EXPECT_TRUE(did_finish_successfully(service_proc.wait_for(core::posix::wait::Flags::untraced)));
}

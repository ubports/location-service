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
#include <com/ubuntu/location/providers/remote/provider.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/object.h>
#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace culpr = com::ubuntu::location::providers::remote;

namespace dbus = core::dbus;

namespace
{
dbus::Bus::Ptr the_system_bus()
{
    static dbus::Bus::Ptr system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    return system_bus;
}
}

struct culpr::Provider::Private
{
    typedef core::dbus::Signal<
        com::ubuntu::remote::RemoteInterface::Signals::PositionChanged,
	com::ubuntu::remote::RemoteInterface::Signals::PositionChanged::ArgumentType
    > PositionChanged;

    Private(const culpr::Provider::Configuration& config)
            : bus(the_system_bus()),
              service(dbus::Service::use_service(bus, config.name)),
              object(service->object_for_path(config.path)),
              signal_position_changed(object->get_signal<com::ubuntu::remote::RemoteInterface::Signals::PositionChanged>())
    {
    }

    void start()
    {
        VLOG(10) << __PRETTY_FUNCTION__;
        bus->install_executor(core::dbus::asio::make_executor(bus));
        if (!worker.joinable())
            worker = std::move(std::thread{std::bind(&dbus::Bus::run, bus)});
    }

    void stop()
    {
        VLOG(10) << __PRETTY_FUNCTION__;
        try
        {
            bus->stop();
        }
        catch(...)
        {
            // can happen if the start method was not called
            VLOG(10) << "Stopping not started remote provider.";
        }

        if (worker.joinable())
            worker.join();
    }

    dbus::Bus::Ptr bus;
    dbus::Service::Ptr service;
    dbus::Object::Ptr object;
    PositionChanged::Ptr signal_position_changed;
    PositionChanged::SubscriptionToken position_updates_connection;

    std::thread worker;
};

std::string culpr::Provider::class_name()
{
    return "remote::Provider";
}

cul::Provider::Ptr culpr::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culpr::Provider::Configuration pConfig;
    pConfig.name = config.count(Configuration::key_name()) > 0 ? config.get<std::string>(Configuration::key_name()) : throw std::runtime_error("Missing bus-name");
    pConfig.path = config.count(Configuration::key_path()) > 0 ? config.get<std::string>(Configuration::key_path()) : throw std::runtime_error("Missing bus-path");
    return cul::Provider::Ptr{new culpr::Provider{pConfig}};
}

culpr::Provider::Provider(const culpr::Provider::Configuration& config)
        : com::ubuntu::location::Provider(config.features, config.requirements),
          d(new Private(config))
{
    d->position_updates_connection =
        d->signal_position_changed->connect(
            [this](const com::ubuntu::remote::RemoteInterface::Signals::PositionChanged::ArgumentType& arg)
            {
                this->on_position_changed(arg);
            });
}

culpr::Provider::~Provider() noexcept
{
    d->stop();
}

void culpr::Provider::on_position_changed(const com::ubuntu::remote::RemoteInterface::Signals::PositionChanged::ArgumentType& arg)
{
    auto longitude = std::get<0>(arg);
    auto latitude = std::get<1>(arg);
    auto altitude = std::get<2>(arg);
    VLOG(10) << "New update received with longitude: " << longitude
        << "latitude: " << latitude << "altitude: " << altitude;

    cul::Position pos
    {
        cul::wgs84::Latitude{latitude* cul::units::Degrees},
        cul::wgs84::Longitude{longitude* cul::units::Degrees}
    };

    pos.altitude = cul::wgs84::Altitude{altitude* cul::units::Meters};
    cul::Update<cul::Position> update(pos);
    VLOG(10) << "Position updated added";
    mutable_updates().position(update);
}

bool culpr::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culpr::Provider::start_position_updates()
{
    VLOG(10) << "Starting remote provider\n";
    d->start();
}

void culpr::Provider::stop_position_updates()
{
    VLOG(10) << "Stopping remote provider\n";
    d->stop();
}

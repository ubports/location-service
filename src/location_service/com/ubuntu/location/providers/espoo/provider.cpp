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
#include <com/ubuntu/location/providers/espoo/provider.h>
#include <com/ubuntu/location/providers/espoo/espoo.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/object.h>
#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>

#include "core/dbus/object.h"
#include "core/dbus/signal.h"

#include <thread>

namespace cul = com::ubuntu::location;
namespace culpe = com::ubuntu::location::providers::espoo;

namespace dbus = core::dbus;

namespace
{
dbus::Bus::Ptr the_system_bus()
{
    static dbus::Bus::Ptr system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    return system_bus;
}
}

struct culpe::Provider::Private
{
    typedef core::dbus::Signal<
        com::ubuntu::espoo::Espoo::Signals::PositionChanged,
	com::ubuntu::espoo::Espoo::Signals::PositionChanged::ArgumentType
    > PositionChanged;

    Private(const culpe::Provider::Configuration& config)
            : bus(the_system_bus()),
              service(dbus::Service::use_service(bus, config.name)),
              object(service->object_for_path(config.path)),
              signal_position_changed(object->get_signal<com::ubuntu::espoo::Espoo::Signals::PositionChanged>())
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
        bus->stop();
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

std::string culpe::Provider::class_name()
{
    return "espoo::Provider";
}

cul::Provider::Ptr culpe::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culpe::Provider::Configuration pConfig;
    pConfig.name = config.count(Configuration::key_name()) > 0 ? config.get<std::string>(Configuration::key_name()) : throw std::runtime_error("Missing bus-name");
    pConfig.path = config.count(Configuration::key_path()) > 0 ? config.get<std::string>(Configuration::key_path()) : throw std::runtime_error("Missing bus-path");
    return cul::Provider::Ptr{new culpe::Provider{pConfig}};
}

culpe::Provider::Provider(const culpe::Provider::Configuration& config) 
        : com::ubuntu::location::Provider(config.features, config.requirements),
          d(new Private(config))
{
    d->position_updates_connection = 
            d->signal_position_changed->connect(
                [this](const com::ubuntu::espoo::Espoo::Signals::PositionChanged::ArgumentType& arg)
                {
		    std::cout << "Got new update!\n"; 
                    auto longitude = std::get<0>(arg);
		    auto latitude = std::get<1>(arg);
		    auto altitude = std::get<2>(arg);

                    VLOG(10) << "New update received with longitude: " << longitude  << "latitude: " << latitude << "altitude: " << altitude;

                    cul::Position pos
                    {
                        cul::wgs84::Latitude{latitude* cul::units::Degrees},
                        cul::wgs84::Longitude{longitude* cul::units::Degrees}
                    };

                    pos.altitude = cul::wgs84::Altitude{altitude* cul::units::Meters};
                    cul::Update<cul::Position> update(pos);
                    VLOG(10) << "Position updated added";
                    this->mutable_updates().position(update);
                });

}

culpe::Provider::~Provider() noexcept
{
    d->stop();
}

bool culpe::Provider::matches_criteria(const cul::Criteria&)
{
    return true;

}

void culpe::Provider::start_position_updates()
{
    VLOG(10) << "Starting espoo provider\n";
    d->start();
}

void culpe::Provider::stop_position_updates()
{
    VLOG(10) << "Stopping espoo provider\n";
    d->stop();
}

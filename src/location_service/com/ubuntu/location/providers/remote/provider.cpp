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
#include <com/ubuntu/location/providers/remote/provider.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/object.h>
#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace culpr = com::ubuntu::location::providers::remote;
namespace cur = com::ubuntu::remote;
namespace dbus = core::dbus;

namespace
{
template<typename T>
void throw_if_error(const dbus::Result<T>& result)
{
    if (result.is_error()) throw std::runtime_error
    {
        result.error().print()
    };
}

dbus::Bus::Ptr the_system_bus()
{
    dbus::Bus::Ptr system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    system_bus->install_executor(core::dbus::asio::make_executor(system_bus));
    return system_bus;
}
}

struct culpr::Provider::Private
{
    Private(const culpr::Provider::Configuration& config)
            : bus(config.connection),
              service(dbus::Service::use_service(bus, config.name)),
              object(service->object_for_path(config.path)),
              stub(object),
              worker([this]() { bus->run(); })
    {
    }

    ~Private()
    {
        bus->stop();

        if (worker.joinable())
            worker.join();
    }

    dbus::Bus::Ptr bus;
    dbus::Service::Ptr service;
    dbus::Object::Ptr object;

    com::ubuntu::remote::RemoteInterface::Stub stub;

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

    pConfig.connection = the_system_bus();

    return cul::Provider::Ptr{new culpr::Provider{pConfig}};
}

culpr::Provider::Provider(const culpr::Provider::Configuration& config)
        : com::ubuntu::location::Provider(config.features, config.requirements),
          d(new Private(config))
{
    d->stub.signals.position_changed->connect(
        [this](const cur::RemoteInterface::Signals::PositionChanged::ArgumentType& arg)
        {
            VLOG(50) << "culpr::Provider::PositionChanged: " << arg;
            mutable_updates().position(arg);
        });
    d->stub.signals.heading_changed->connect(
        [this](const cur::RemoteInterface::Signals::HeadingChanged::ArgumentType& arg)
        {
            VLOG(50) << "culpr::Provider::HeadingChanged: " << arg;
            mutable_updates().heading(arg);
        });
    d->stub.signals.velocity_changed->connect(
        [this](const cur::RemoteInterface::Signals::VelocityChanged::ArgumentType& arg)
        {
            VLOG(50) << "culpr::Provider::VelocityChanged: " << arg;
            mutable_updates().velocity(arg);
        });
}

culpr::Provider::~Provider() noexcept
{
}

bool culpr::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culpr::Provider::start_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StartPositionUpdates, void>());
}

void culpr::Provider::stop_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StopPositionUpdates, void>());
}

void culpr::Provider::start_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StartHeadingUpdates, void>());
}

void culpr::Provider::stop_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StopHeadingUpdates, void>());
}

void culpr::Provider::start_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StartVelocityUpdates, void>());
}

void culpr::Provider::stop_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<cur::RemoteInterface::StopVelocityUpdates, void>());
}

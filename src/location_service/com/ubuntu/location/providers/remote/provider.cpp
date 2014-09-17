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

#include <com/ubuntu/location/providers/remote/interface.h>

#include <com/ubuntu/location/logging.h>

#include <core/dbus/object.h>
#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>

#include <thread>

namespace cul = com::ubuntu::location;
namespace remote = com::ubuntu::location::providers::remote;

namespace dbus = core::dbus;

namespace
{
void throw_if_error(const dbus::Result<void>& result)
{
    if (result.is_error()) throw std::runtime_error
    {
        result.error().print()
    };
}

template<typename T>
T throw_if_error_or_return(const dbus::Result<T>& result)
{
    if (result.is_error()) throw std::runtime_error
    {
        result.error().print()
    };
    return result.value();
}

dbus::Bus::Ptr the_system_bus()
{
    dbus::Bus::Ptr system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    system_bus->install_executor(core::dbus::asio::make_executor(system_bus));
    return system_bus;
}
}

struct remote::Provider::Stub::Private
{
    Private(const remote::stub::Configuration& config)
            : object(config.object),
              stub(object)
    {
    }

    ~Private()
    {
    }

    dbus::Object::Ptr object;
    remote::Interface::Stub stub;
};

std::string remote::Provider::Stub::class_name()
{
    return "remote::Provider";
}

cul::Provider::Ptr remote::Provider::Stub::create_instance(const cul::ProviderFactory::Configuration& config)
{
    auto name = config.count(Stub::key_name) > 0 ? config.get<std::string>(Stub::key_name) :
                                                   throw std::runtime_error("Missing bus-name");
    auto path = config.count(Stub::key_path) > 0 ? config.get<std::string>(Stub::key_path) :
                                                   throw std::runtime_error("Missing bus-path");

    auto bus = the_system_bus();
    auto service = dbus::Service::use_service(bus, name);
    auto object = service->object_for_path(path);

    return cul::Provider::Ptr{new remote::Provider::Stub{remote::stub::Configuration{object}}};
}

remote::Provider::Stub::Stub(const remote::stub::Configuration& config)
        : com::ubuntu::location::Provider(/* TODO(tvoss) Features should be all initially*/),
          d(new Private(config))
{
    d->stub.signals.position_changed->connect(
        [this](const remote::Interface::Signals::PositionChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::PositionChanged: " << arg;
            mutable_updates().position(arg);
        });
    d->stub.signals.heading_changed->connect(
        [this](const remote::Interface::Signals::HeadingChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::HeadingChanged: " << arg;
            mutable_updates().heading(arg);
        });
    d->stub.signals.velocity_changed->connect(
        [this](const remote::Interface::Signals::VelocityChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::VelocityChanged: " << arg;
            mutable_updates().velocity(arg);
        });
}

remote::Provider::Stub::~Stub() noexcept
{
}

bool remote::Provider::Stub::matches_criteria(const cul::Criteria& criteria)
{
    VLOG(10) << __PRETTY_FUNCTION__ << std::endl;
    return throw_if_error_or_return(d->stub.object->transact_method<remote::Interface::MatchesCriteria, bool>(criteria));
}

bool remote::Provider::Stub::supports(const cul::Provider::Features& f) const
{
    VLOG(10) << __PRETTY_FUNCTION__;
    return throw_if_error_or_return(d->stub.object->transact_method<remote::Interface::Supports, bool>(f));
}

bool remote::Provider::Stub::requires(const cul::Provider::Requirements& r) const
{
    VLOG(10) << __PRETTY_FUNCTION__;
    return throw_if_error_or_return(d->stub.object->transact_method<remote::Interface::Requires, bool>(r));
}

void remote::Provider::Stub::on_wifi_and_cell_reporting_state_changed(cul::WifiAndCellIdReportingState state)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::OnWifiAndCellIdReportingStateChanged, void>(state));
}

void remote::Provider::Stub::on_reference_location_updated(const cul::Update<cul::Position>& position)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::OnReferenceLocationChanged, void>(position));
}

void remote::Provider::Stub::on_reference_velocity_updated(const cul::Update<cul::Velocity>& velocity)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::OnReferenceVelocityChanged, void>(velocity));
}

void remote::Provider::Stub::on_reference_heading_updated(const cul::Update<cul::Heading>& heading)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::OnReferenceHeadingChanged, void>(heading));
}

void remote::Provider::Stub::start_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StartPositionUpdates, void>());
}

void remote::Provider::Stub::stop_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StopPositionUpdates, void>());
}

void remote::Provider::Stub::start_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StartHeadingUpdates, void>());
}

void remote::Provider::Stub::stop_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StopHeadingUpdates, void>());
}

void remote::Provider::Stub::start_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StartVelocityUpdates, void>());
}

void remote::Provider::Stub::stop_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;
    throw_if_error(d->stub.object->transact_method<remote::Interface::StopVelocityUpdates, void>());
}

struct remote::Provider::Skeleton::Private
{
    Private(const remote::skeleton::Configuration& config)
            : bus(config.bus),
              skeleton(config.object),
              impl(config.provider),
              connections
              {
                  impl->updates().position.connect([this](const cul::Update<cul::Position>& position)
                  {
                      skeleton.signals.position_changed->emit(position.value);
                  }),
                  impl->updates().heading.connect([this](const cul::Update<cul::Heading>& heading)
                  {
                      skeleton.signals.heading_changed->emit(heading.value);
                  }),
                  impl->updates().velocity.connect([this](const cul::Update<cul::Velocity>& velocity)
                  {
                      skeleton.signals.velocity_changed->emit(velocity.value);
                  })
              }
    {
    }

    core::dbus::Bus::Ptr bus;
    remote::Interface::Skeleton skeleton;
    cul::Provider::Ptr impl;

    // All connections to signals go here.
    struct
    {
        core::ScopedConnection position_changed;
        core::ScopedConnection heading_changed;
        core::ScopedConnection velocity_changed;
    } connections;
};

remote::Provider::Skeleton::Skeleton(const remote::skeleton::Configuration& config)
    : cul::Provider(),
      d(new Private(config))
{
    // Wire up to updates.
    d->impl->updates().position.connect([this](const cul::Update<cul::Position>& position)
    {
        d->skeleton.signals.position_changed->emit(position.value);
    });

    d->impl->updates().heading.connect([this](const cul::Update<cul::Heading>& heading)
    {
        d->skeleton.signals.heading_changed->emit(heading.value);
    });

    d->impl->updates().velocity.connect([this](const cul::Update<cul::Velocity>& velocity)
    {
        d->skeleton.signals.velocity_changed->emit(velocity.value);
    });

    // And install method handlers.
    d->skeleton.object->install_method_handler<remote::Interface::MatchesCriteria>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "MatchesCriteria";

        cul::Criteria criteria; msg->reader() >> criteria;
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << matches_criteria(criteria);

        d->bus->send(reply);
    });

    d->skeleton.object->install_method_handler<remote::Interface::Supports>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Supports";

        cul::Provider::Features f; msg->reader() >> f;
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << supports(f);

        d->bus->send(reply);
    });

    d->skeleton.object->install_method_handler<remote::Interface::Requires>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Requires";

        cul::Provider::Requirements r; msg->reader() >> r;
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << requires(r);

        d->bus->send(reply);
    });

    d->skeleton.object->install_method_handler<remote::Interface::OnWifiAndCellIdReportingStateChanged>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "OnWifiAndCellIdReportingStateChanged";

        cul::WifiAndCellIdReportingState s; msg->reader() >> s;
        d->bus->send(dbus::Message::make_method_return(msg));

        on_wifi_and_cell_reporting_state_changed(s);
    });

    d->skeleton.object->install_method_handler<remote::Interface::OnReferenceLocationChanged>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "OnReferenceLocationChanged";

        cul::Update<cul::Position> u; msg->reader() >> u;
        d->bus->send(dbus::Message::make_method_return(msg));

        on_reference_location_updated(u);
    });

    d->skeleton.object->install_method_handler<remote::Interface::OnReferenceHeadingChanged>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "OnReferenceHeadingChanged";

        cul::Update<cul::Heading> u; msg->reader() >> u;
        d->bus->send(dbus::Message::make_method_return(msg));

        on_reference_heading_updated(u);
    });

    d->skeleton.object->install_method_handler<remote::Interface::OnReferenceVelocityChanged>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "OnReferenceVelocityChanged";

        cul::Update<cul::Velocity> u; msg->reader() >> u;
        d->bus->send(dbus::Message::make_method_return(msg));

        on_reference_velocity_updated(u);
    });

    d->skeleton.object->install_method_handler<remote::Interface::StartPositionUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StartPositionUpdates";
        start_position_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::StopPositionUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StopPositionUpdates";
        stop_position_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::StartHeadingUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StartHeadingUpdates";
        start_heading_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::StopHeadingUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StopHeadingUpdates";
        stop_heading_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::StartVelocityUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StartVelocityUpdates";
        start_velocity_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::StopVelocityUpdates>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "StartVelocityUpdates";
        stop_velocity_updates();
        d->bus->send(dbus::Message::make_method_return(msg));
    });
}

remote::Provider::Skeleton::~Skeleton() noexcept
{
    d->skeleton.object->uninstall_method_handler<remote::Interface::MatchesCriteria>();

    d->skeleton.object->uninstall_method_handler<remote::Interface::StartPositionUpdates>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::StopPositionUpdates>();

    d->skeleton.object->uninstall_method_handler<remote::Interface::StartHeadingUpdates>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::StopHeadingUpdates>();

    d->skeleton.object->uninstall_method_handler<remote::Interface::StartVelocityUpdates>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::StopVelocityUpdates>();
}

// We just forward calls to the actual implementation
bool remote::Provider::Skeleton::matches_criteria(const cul::Criteria& criteria)
{
    return d->impl->matches_criteria(criteria);
}

bool remote::Provider::Skeleton::supports(const cul::Provider::Features& f) const
{
    return d->impl->supports(f);
}

bool remote::Provider::Skeleton::requires(const cul::Provider::Requirements& r) const
{
    return d->impl->requires(r);
}

void remote::Provider::Skeleton::on_wifi_and_cell_reporting_state_changed(cul::WifiAndCellIdReportingState state)
{
    d->impl->on_wifi_and_cell_reporting_state_changed(state);
}

void remote::Provider::Skeleton::on_reference_location_updated(const cul::Update<cul::Position>& position)
{
    d->impl->on_reference_location_updated(position);
}

void remote::Provider::Skeleton::on_reference_velocity_updated(const cul::Update<cul::Velocity>& velocity)
{
    d->impl->on_reference_velocity_updated(velocity);
}

void remote::Provider::Skeleton::on_reference_heading_updated(const cul::Update<cul::Heading>& heading)
{
    d->impl->on_reference_heading_updated(heading);
}

void remote::Provider::Skeleton::start_position_updates()
{
    d->impl->state_controller()->start_position_updates();
}

void remote::Provider::Skeleton::stop_position_updates()
{
    d->impl->state_controller()->stop_position_updates();
}

void remote::Provider::Skeleton::start_heading_updates()
{
    d->impl->state_controller()->start_heading_updates();
}

void remote::Provider::Skeleton::stop_heading_updates()
{
    d->impl->state_controller()->stop_heading_updates();
}

void remote::Provider::Skeleton::start_velocity_updates()
{
    d->impl->state_controller()->start_velocity_updates();
}

void remote::Provider::Skeleton::stop_velocity_updates()
{
    d->impl->state_controller()->stop_velocity_updates();
}

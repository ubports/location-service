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

#include <core/posix/this_process.h>

#include <boost/asio.hpp>

#include <thread>

namespace cul = com::ubuntu::location;
namespace remote = com::ubuntu::location::providers::remote;

namespace dbus = core::dbus;

namespace
{
struct Runtime
{
    static Runtime& instance()
    {
        static Runtime runtime;
        return runtime;
    }

    Runtime()
        : running{true},
          worker1{std::bind(&Runtime::run, this, std::ref(io.service))},
          worker2{std::bind(&Runtime::run, this, std::ref(task.service))}
    {
    }

    ~Runtime()
    {
        stop();
    }

    void run(boost::asio::io_service& service)
    {
        while (running)
        {
            try
            {
                service.run();
            }
            catch (const std::exception& e)
            {
                LOG(WARNING) << e.what();
            }
            catch (...)
            {
                LOG(WARNING) << "Caught exception from event loop, restarting.";
            }
        }
    }

    void stop()
    {
        running = false;

        task.service.stop();

        if (worker2.joinable())
            worker2.join();

        io.service.stop();

        if (worker1.joinable())
            worker1.join();
    }

    bool running;
    struct
    {
        boost::asio::io_service service;
        boost::asio::io_service::work keep_alive
        {
            service
        };
    } io;
    struct
    {
        boost::asio::io_service service;
        boost::asio::io_service::work keep_alive
        {
            service
        };
    } task;
    std::thread worker1;
    std::thread worker2;
};

core::dbus::Bus::Ptr bus_from_name(const std::string& bus_name)
{
    core::dbus::Bus::Ptr bus;

    if (bus_name == "system")
        bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);
    else if (bus_name == "session")
        bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    else if (bus_name == "system_with_address_from_env")
        bus = std::make_shared<core::dbus::Bus>(core::posix::this_process::env::get_or_throw("DBUS_SYSTEM_BUS_ADDRESS"));
    else if (bus_name == "session_with_address_from_env")
        bus = std::make_shared<core::dbus::Bus>(core::posix::this_process::env::get_or_throw("DBUS_SESSION_BUS_ADDRESS"));

    if (not bus) throw std::runtime_error
    {
        "Could not create bus for name: " + bus_name
    };

    bus->install_executor(core::dbus::asio::make_executor(bus, Runtime::instance().io.service));

    return bus;
}

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
    auto bus_name = config.count(Stub::key_bus) > 0 ? config.get<std::string>(Stub::key_bus) :
                                                       "system";
    auto name = config.count(Stub::key_name) > 0 ? config.get<std::string>(Stub::key_name) :
                                                   throw std::runtime_error("Missing bus-name");
    auto path = config.count(Stub::key_path) > 0 ? config.get<std::string>(Stub::key_path) :
                                                   throw std::runtime_error("Missing bus-path");

    auto bus = bus_from_name(bus_name);
    auto service = dbus::Service::use_service(bus, name);
    auto object = service->object_for_path(path);

    return create_instance_with_config(remote::stub::Configuration{object});
}

cul::Provider::Ptr remote::Provider::Stub::create_instance_with_config(const remote::stub::Configuration& config)
{
    std::shared_ptr<remote::Provider::Stub> result{new remote::Provider::Stub{config}};
    result->setup_event_connections();
    return result;
}

remote::Provider::Stub::Stub(const remote::stub::Configuration& config)
        : com::ubuntu::location::Provider(/* TODO(tvoss) Features should be all initially*/),
          d(new Private(config))
{
}

void remote::Provider::Stub::setup_event_connections()
{
    std::weak_ptr<remote::Provider::Stub> wp{shared_from_this()};

    d->stub.signals.position_changed->connect(
        [wp](const remote::Interface::Signals::PositionChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::PositionChanged: " << arg;
            Runtime::instance().task.service.post([wp, arg]()
            {
                auto sp = wp.lock();

                if (not sp)
                    return;

                sp->mutable_updates().position(arg);
            });
        });

    d->stub.signals.heading_changed->connect(
        [wp](const remote::Interface::Signals::HeadingChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::HeadingChanged: " << arg;
            Runtime::instance().task.service.post([wp, arg]()
            {
                auto sp = wp.lock();

                if (not sp)
                    return;

                sp->mutable_updates().heading(arg);
            });
        });

    d->stub.signals.velocity_changed->connect(
        [wp](const remote::Interface::Signals::VelocityChanged::ArgumentType& arg)
        {
            VLOG(50) << "remote::Provider::Stub::VelocityChanged: " << arg;
            Runtime::instance().task.service.post([wp, arg]()
            {
                auto sp = wp.lock();

                if (not sp)
                    return;

                sp->mutable_updates().velocity(arg);
            });
        });
}

remote::Provider::Stub::~Stub() noexcept
{
    VLOG(10) << __PRETTY_FUNCTION__;
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
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp, position]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        try
        {
            throw_if_error(sp->stub.object->transact_method<remote::Interface::OnReferenceLocationChanged, void>(position));
        } catch(const std::exception& e)
        {
            // We drop the error and just log it for post-mortem inspection.
            LOG(WARNING) << "Transaction<remote::Interface::OnReferenceLocationChanged>: " << e.what();
        }
    });
}

void remote::Provider::Stub::on_reference_velocity_updated(const cul::Update<cul::Velocity>& velocity)
{
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp, velocity]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        try
        {
            throw_if_error(sp->stub.object->transact_method<remote::Interface::OnReferenceVelocityChanged, void>(velocity));
        } catch(const std::exception& e)
        {
            // We drop the error and just log it for post-mortem inspection.
            LOG(WARNING) << "Transaction<remote::Interface::OnReferenceVelocityChanged>: " << e.what();
        }
    });
}

void remote::Provider::Stub::on_reference_heading_updated(const cul::Update<cul::Heading>& heading)
{
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp, heading]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        try
        {
            throw_if_error(sp->stub.object->transact_method<remote::Interface::OnReferenceHeadingChanged, void>(heading));
        } catch(const std::exception& e)
        {
            // We drop the error and just log it for post-mortem inspection.
            LOG(WARNING) << "Transaction<remote::Interface::OnReferenceHeadingChanged>: " << e.what();
        }
    });
}

void remote::Provider::Stub::start_position_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StartPositionUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;
}

void remote::Provider::Stub::stop_position_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StopPositionUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;
}

void remote::Provider::Stub::start_heading_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StartHeadingUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;
}

void remote::Provider::Stub::stop_heading_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StopHeadingUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;
}

void remote::Provider::Stub::start_velocity_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StartVelocityUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;

}

void remote::Provider::Stub::stop_velocity_updates()
{
    VLOG(10) << "> " << __PRETTY_FUNCTION__;
    std::weak_ptr<Private> wp{d};
    Runtime::instance().task.service.post([wp]()
    {
        auto sp = wp.lock();

        if (not sp)
            return;

        throw_if_error(sp->stub.object->transact_method<remote::Interface::StopVelocityUpdates, void>());
    });
    VLOG(10) << "< " << __PRETTY_FUNCTION__;
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
                      VLOG(100) << "Position changed reported by impl: " << position;                      
                      skeleton.signals.position_changed->emit(position.value);
                  }),
                  impl->updates().heading.connect([this](const cul::Update<cul::Heading>& heading)
                  {
                      VLOG(100) << "Heading changed reported by impl: " << heading;
                      skeleton.signals.heading_changed->emit(heading.value);
                  }),
                  impl->updates().velocity.connect([this](const cul::Update<cul::Velocity>& velocity)
                  {
                      VLOG(100) << "Velocity changed reported by impl: " << velocity;
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

#include "com/ubuntu/location/service/implementation.h"

#include "com/ubuntu/location/service/session/implementation.h"

#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/engine.h"
#include "com/ubuntu/location/proxy_provider.h"

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

#include <chrono>
#include <functional>
#include <sstream>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

namespace dbus = core::dbus;

struct culs::Implementation::Private
{
    dbus::types::ObjectPath make_session_path()
    {
        static std::size_t counter{0};
        std::stringstream ss; ss << "/sessions/" << counter;
        counter++;

        return dbus::types::ObjectPath{ss.str()};
    }

    dbus::Bus::Ptr bus;
    cul::Engine::Ptr engine;
    culs::PermissionManager::Ptr permission_manager;
};

culs::Implementation::Implementation(
    const dbus::Bus::Ptr& bus,
    const cul::Engine::Ptr& engine,
    const culs::PermissionManager::Ptr& permission_manager)
    : Skeleton(bus, permission_manager),
      d{new Private{bus, engine, permission_manager}}
{
    if (!bus)
        throw std::runtime_error("Cannot create service for null bus.");
    if (!engine)
        throw std::runtime_error("Cannot create service for null engine.");
    if (!permission_manager)
        throw std::runtime_error("Cannot create service for null permission manager.");
}

culs::Implementation::~Implementation() noexcept
{
}

culs::session::Interface::Ptr culs::Implementation::create_session_for_criteria(const cul::Criteria& criteria)
{
    auto provider_selection
            = d->engine->determine_provider_selection_for_criteria(criteria);
    auto proxy_provider = ProxyProvider::Ptr
            {
                new ProxyProvider{provider_selection}
            };
    
    return session::Interface::Ptr{new session::Implementation(d->bus, d->make_session_path(), proxy_provider)};
}

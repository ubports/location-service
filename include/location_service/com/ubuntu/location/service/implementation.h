#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_

#include "com/ubuntu/location/service/skeleton.h"

#include "com/ubuntu/location/service/session/implementation.h"

#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/engine.h"
#include "com/ubuntu/location/proxy_provider.h"

#include <org/freedesktop/dbus/bus.h>
#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/traits/service.h>
#include <org/freedesktop/dbus/types/object_path.h>

#include <chrono>
#include <functional>
#include <sstream>

namespace dbus = org::freedesktop::dbus;

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Implementation : public Skeleton
{
  public:
    typedef std::shared_ptr<Implementation> Ptr;
    
    Implementation(
        const org::freedesktop::dbus::Bus::Ptr& bus,
        const Engine::Ptr& engine, 
        const PermissionManager::Ptr& permission_manager)
        : Skeleton(bus, permission_manager),
          bus(bus),
          engine(engine),
          permission_manager(permission_manager)
    {
        if (!engine)
            throw std::runtime_error("Cannot create service for null engine.");
        if (!permission_manager)
            throw std::runtime_error("Cannot create service for null permission manager.");
    }

    virtual ~Implementation() noexcept
    {        
    }

    virtual session::Interface::Ptr create_session_for_criteria(const Criteria& criteria)
    {
        auto provider_selection 
                = engine->determine_provider_selection_for_criteria(criteria);
        auto proxy_provider = ProxyProvider::Ptr
                {
                    new ProxyProvider{provider_selection}
                };
        return session::Interface::Ptr{new session::Implementation(bus, make_session_path(), proxy_provider)};
    }

  private:   
    org::freedesktop::dbus::types::ObjectPath make_session_path()
    {
        static std::size_t counter{0};
        std::stringstream ss; ss << "/sessions/" << counter;
        counter++;
        
        return org::freedesktop::dbus::types::ObjectPath{ss.str()};
    }

    org::freedesktop::dbus::Bus::Ptr bus;
    Engine::Ptr engine;
    PermissionManager::Ptr permission_manager;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_H_

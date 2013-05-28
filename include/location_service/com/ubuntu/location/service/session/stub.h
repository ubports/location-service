#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_STUB_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SESSION_SESSION_STUB_H_

#include "com/ubuntu/location/service/session/interface.h"

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"

#include <org/freedesktop/dbus/stub.h>

#include <functional>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
namespace session
{
class Stub : public org::freedesktop::dbus::Stub<Interface>
{
  public:
    Stub(
        const org::freedesktop::dbus::Bus::Ptr& bus, 
        const org::freedesktop::dbus::types::ObjectPath& session_path); 

    virtual const org::freedesktop::dbus::types::ObjectPath& path() const;
    
    virtual void start_position_updates();
    virtual void stop_position_updates() noexcept;

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates() noexcept;

    virtual void start_heading_updates();
    virtual void stop_heading_updates() noexcept;

  private:
    void update_heading(DBusMessage* msg);
    void update_position(DBusMessage* msg);
    void update_velocity(DBusMessage* msg);

    org::freedesktop::dbus::types::ObjectPath session_path;
    org::freedesktop::dbus::Object::Ptr object;
};
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_STUB_H_

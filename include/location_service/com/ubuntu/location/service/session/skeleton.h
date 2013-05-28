#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_

#include "com/ubuntu/location/service/session/interface.h"

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/skeleton.h>

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
class Skeleton : public org::freedesktop::dbus::Skeleton<Interface>              
{
  public:
    Skeleton(
        const org::freedesktop::dbus::Bus::Ptr& bus, 
        const org::freedesktop::dbus::types::ObjectPath& session_path);
    
    virtual ~Skeleton() noexcept;
    
    virtual const org::freedesktop::dbus::types::ObjectPath& path() const;

  private:
    void handle_start_position_updates(DBusMessage* msg);
    void handle_stop_position_updates(DBusMessage* msg);

    void handle_start_velocity_updates(DBusMessage* msg);
    void handle_stop_velocity_updates(DBusMessage* msg);

    void handle_start_heading_updates(DBusMessage* msg);
    void handle_stop_heading_updates(DBusMessage* msg);

    org::freedesktop::dbus::types::ObjectPath session_path;
    org::freedesktop::dbus::Object::Ptr object;
};
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_SKELETON_H_

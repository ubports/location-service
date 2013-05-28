#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_

#include "com/ubuntu/location/service/interface.h"
#include "com/ubuntu/location/service/session/interface.h"

#include <org/freedesktop/dbus/stub.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Stub : public org::freedesktop::dbus::Stub<Interface>
{
  public:
    Stub(const dbus::Bus::Ptr& connection);
    ~Stub() noexcept;

    session::Interface::Ptr create_session_for_criteria(const Criteria& criteria);

  private:
    org::freedesktop::dbus::Bus::Ptr bus;
    org::freedesktop::dbus::Object::Ptr object;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_STUB_H_

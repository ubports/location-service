#include "com/ubuntu/location/service/stub.h"
#include "com/ubuntu/location/service/session/stub.h"

#include "com/ubuntu/location/logging.h"

#include <org/freedesktop/dbus/stub.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

culs::Stub::Stub(const dbus::Bus::Ptr& connection) 
    : dbus::Stub<culs::Interface>(connection),
      bus(connection),
      object(access_service()->object_for_path(culs::Interface::path()))
{    	
}

culs::Stub::~Stub() noexcept
{    	
}

culss::Interface::Ptr culs::Stub::create_session_for_criteria(const cul::Criteria& criteria)
{
	auto op = object->invoke_method_synchronously<
        culs::Interface::CreateSessionForCriteria,
        culs::Interface::CreateSessionForCriteria::ResultType
    >(criteria);

    if (op.is_error())
        throw std::runtime_error(op.error());

    LOG(INFO) << "Successfully created session with path: " << op.value().as_string();

    return culss::Interface::Ptr(new culss::Stub{bus, op.value()});
}

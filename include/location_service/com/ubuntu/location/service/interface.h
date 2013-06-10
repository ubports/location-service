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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_

#include "com/ubuntu/location/service/session/interface.h"

#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/traits/service.h>
#include <org/freedesktop/dbus/types/object_path.h>

#include <chrono>
#include <functional>

namespace dbus = org::freedesktop::dbus;

namespace com
{
namespace ubuntu
{
namespace location
{

struct Criteria;

namespace service
{
class Interface
{
  protected:
    struct Errors
    {
        struct InsufficientPermissions { inline static std::string name() { return "com.ubuntu.location.Service.Error.InsufficientPermissions"; } };
        struct CreatingSession { inline static std::string name() { return "com.ubuntu.location.Service.Error.CreatingSession"; } };
    };

    struct CreateSessionForCriteria
    {
        typedef com::ubuntu::location::service::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "CreateSessionForCriteria"
            };
            return s;
        }

        typedef dbus::types::ObjectPath ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    Interface() = default;

  public:
    typedef std::shared_ptr<Interface> Ptr;

    inline static const std::string& path()
    {
        static const std::string s{"/com/ubuntu/location/Service"};
        return s;
    }

    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;
    virtual ~Interface() = default;

    virtual session::Interface::Ptr create_session_for_criteria(const Criteria& criteria) = 0;
};
}
}
}
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace traits
{
template<>
struct Service<com::ubuntu::location::service::Interface>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            "com.ubuntu.location.Service"
        };
        return s;
    }
};
}
}
}
}

#include "com/ubuntu/location/codec.h"

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_INTERFACE_H_

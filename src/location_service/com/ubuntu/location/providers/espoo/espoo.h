/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Manuel de la Pena <manuel.delapena@canonical.com>
 */

#ifndef CORE_UBUNTU_ESPOO_PROVIDER_P_H_
#define CORE_UBUNTU_ESPOO_PROVIDER_P_H_

#include <com/ubuntu/location/providers/espoo/macros.h>
#include <core/dbus/traits/service.h>

namespace com
{

namespace ubuntu
{

namespace espoo
{

struct Espoo
{

    static const std::string& name()
    {
        static const std::string s{"com.ubuntu.espoo.Service.Provider"};
        return s;
    }

    METHOD(StartPositionUpdates, Espoo, void)
    METHOD(StopPositionUpdates, Espoo, void)
    METHOD(StartHeadingUpdates, Espoo, void)
    METHOD(StopHeadingUpdates, Espoo, void)
    METHOD(StartVelocityUpdates, Espoo, void)
    METHOD(StopvVelocityUpdates, Espoo, void)

    struct Signals
    {
        struct PositionChanged
        {
            inline static std::string name()
            {
                return "PositionChanged";
            };
            typedef Espoo Interface;
            typedef std::tuple<double, double, double, double, uint32_t> ArgumentType;
        };

        SIGNAL(HeadingChanged, Espoo, double)
        SIGNAL(VelocityChanged, Espoo, double)
    };

    struct Properties
    {
        READABLE_PROPERTY(HasPosition, Espoo, bool)
        READABLE_PROPERTY(HasVelocity, Espoo, bool)
        READABLE_PROPERTY(HasHeading, Espoo, bool)
        READABLE_PROPERTY(RequiresSatellites, Espoo, bool)
        READABLE_PROPERTY(RequiresCellNetwork, Espoo, bool)
        READABLE_PROPERTY(RequiresDataNetwork, Espoo, bool)
        READABLE_PROPERTY(RequiresMonetarySpending, Espoo, bool)
        READABLE_PROPERTY(ArePositionUpdatesRunning, Espoo, bool)
        READABLE_PROPERTY(AreHeadingUpdatesRunning, Espoo, bool)
        READABLE_PROPERTY(AreVelocityUpdatesRunning, Espoo, bool)
    };

};


} // espoo

} // ubuntu

}  // core

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<com::ubuntu::espoo::Espoo>
{
    static const std::string& interface_name()
    {
        static const std::string s{"com.ubuntu.espoo.Service.Provider"};
        return s;
    }

    inline static const std::string& object_path()
    {
        static const std::string s{"/com/ubuntu/espoo/Service/Provider"};
        return s;
    }

};
}
}
}

#endif

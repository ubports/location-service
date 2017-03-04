/*
 * Copyright © 2016 Canonical Ltd.
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
#ifndef LOCATION_SESSION_WITH_PROVIDER_H_
#define LOCATION_SESSION_WITH_PROVIDER_H_

#include <location/provider.h>
#include <location/service.h>

namespace location
{
class SessionWithProvider : public Service::Session
{
public:
    SessionWithProvider(const Provider::Ptr& provider);
    virtual ~SessionWithProvider();

    // From Session::Service.
    Updates& updates() override;

private:
    void start_position_updates();
    void stop_position_updates();

    void start_velocity_updates();
    void stop_velocity_updates();

    void start_heading_updates();
    void stop_heading_updates();

    Updates updates_;
    Provider::Ptr provider_;

    struct
    {
        core::ScopedConnection position_updates;
        core::ScopedConnection velocity_updates;
        core::ScopedConnection heading_updates;

        core::ScopedConnection position_status_updates;
        core::ScopedConnection heading_status_updates;
        core::ScopedConnection velocity_status_updates;
    } connections_;
};
}

#endif // LOCATION_SESSION_WITH_PROVIDER_H_

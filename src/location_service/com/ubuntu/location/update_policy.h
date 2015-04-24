/*
 * Copyright © 2015 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_UPDATE_POLICY_H_
#define LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_UPDATE_POLICY_H_

#include <memory>

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>

namespace com
{
namespace ubuntu
{
namespace location
{

// An interface that can be implemented to add heuristics on how heading, position or velocity updateswill be chosen.
// This class allows developers to inject different heuristics in the engine to perform the update selection
// so that the app developers can take advantage of it.
class UpdatePolicy {
 public:
    typedef std::shared_ptr<UpdatePolicy> Ptr;

    UpdatePolicy(const UpdatePolicy&) = delete;
    UpdatePolicy(UpdatePolicy&&) = delete;
    UpdatePolicy& operator=(const UpdatePolicy&) = delete;
    virtual ~UpdatePolicy() = default;

    // Return if the given position update will be verified as the new position in the engine.
    virtual const location::Update<location::Position>& verify_update(const location::Update<location::Position>& update) = 0;
    // Return if the given heading update will be verified as the new heading in the engine.
    virtual const location::Update<location::Heading>& verify_update(const location::Update<location::Heading>& update) = 0;
    // Return if the given velocity update will be verified as the new velocity in the engine.
    virtual const location::Update<location::Velocity>& verify_update(const location::Update<location::Velocity>& update) = 0;
 protected:
    UpdatePolicy() = default;

    template <class T> bool is_significantly_newer(const location::Update<T> last_update, const location::Update<T> update, std::chrono::minutes limit) const
    {
       auto delta = update.when - last_update.when;
       return delta > limit;
    }

    template <class T> bool is_significantly_older(const location::Update<T> last_update, const location::Update<T> update, std::chrono::minutes limit) const
    {
       auto delta = update.when - last_update.when;
       return delta < (-1 * limit);
    }


};
}
}
}

#endif // LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_UPDATE_POLICY_H_


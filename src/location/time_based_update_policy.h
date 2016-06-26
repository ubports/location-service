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
#ifndef LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_TIMES_BASED_UPDATE_POLICY_H_
#define LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_TIMES_BASED_UPDATE_POLICY_H_

#include <chrono>
#include <mutex>

#include "update_policy.h"

namespace location
{

// An interface that can be implemented to add heuristics on how heading, position or velocity updates will be chosen.
// This class ensures that the best update possible is chosen within a reasonable time bracket.
class TimeBasedUpdatePolicy : public UpdatePolicy {

 public:
    TimeBasedUpdatePolicy(std::chrono::minutes mins=default_timeout());
    TimeBasedUpdatePolicy(const TimeBasedUpdatePolicy&) = delete;
    ~TimeBasedUpdatePolicy() = default;

    // Return if the given position update will be verified as the new position in the engine.
    const location::Update<location::Position>& verify_update(const location::Update<location::Position>& update) override;
    // Return if the given heading update will be verified as the new heading in the engine.
    const location::Update<location::Heading>& verify_update(const location::Update<location::Heading>& update) override;
    // Return if the given velocity update will be verified as the new velocity in the engine.
    const location::Update<location::Velocity>& verify_update(const location::Update<location::Velocity>& update) override;

    static std::chrono::minutes default_timeout();

 protected:
    // not private to simplify the testing but should be private
    location::Update<location::Position> last_position_update;
    location::Update<location::Heading> last_heading_update;
    location::Update<location::Velocity> last_velocity_update;

 private:
    // callbacks can happen in diff threads, make sure multi-threading will work in this class
    std::mutex position_update_mutex;
    std::mutex heading_update_mutex;
    std::mutex velocity_update_mutex;
    // used to calculate the time accepted bracket
    std::chrono::minutes limit;
};

}

#endif //LOCATION_SERVICE_UBUNTU_LOCATION_SERVICE_TIMES_BASED_UPDATE_POLICY_H_

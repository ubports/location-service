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

#include "time_based_update_policy.h"

namespace {
    int DEFAULT_TIME_LIMIT = 2;
}
namespace com
{
namespace ubuntu
{
namespace location
{

TimeBasedUpdatePolicy::TimeBasedUpdatePolicy()
    : TimeBasedUpdatePolicy(DEFAULT_TIME_LIMIT)
{
}

TimeBasedUpdatePolicy::TimeBasedUpdatePolicy(int mins)
    : limit(mins)
{

}
const location::Update<location::Position>& TimeBasedUpdatePolicy::verify_update(const location::Update<location::Position>& update)
{
    std::lock_guard<std::mutex> guard(position_update_mutex);
    bool use_new_update;
    if (is_significantly_newer(update))
    {
        use_new_update = true;
    }
    else if (is_significantly_older(update))
    {
        use_new_update = false;
    }
    else
    {
        use_new_update = last_position_update.value.accuracy.horizontal && update.value.accuracy.horizontal
            && *last_position_update.value.accuracy.horizontal >= *update.value.accuracy.horizontal;
    }

    if (use_new_update)
    {
        last_position_update = update;
        return update;
    }
    else
    {
        return last_position_update;
    }
}


const location::Update<location::Heading>& TimeBasedUpdatePolicy::verify_update(const location::Update<location::Heading>& update)
{
    std::lock_guard<std::mutex> guard(heading_update_mutex);
    bool use_new_update;
    if (is_significantly_newer(update))
    {
        use_new_update = true;
    }
    else if (is_significantly_older(update))
    {
        use_new_update = false;
    }
    if (use_new_update)
    {
        last_heading_update = update;
        return update;
    }
    else
    {
        return last_heading_update;
    }
}

const location::Update<location::Velocity>& TimeBasedUpdatePolicy::verify_update(const location::Update<location::Velocity>& update)
{
    std::lock_guard<std::mutex> guard(velocity_update_mutex);
    bool use_new_update;
    if (is_significantly_newer(update))
    {
        use_new_update = true;
    }
    else if (is_significantly_older(update))
    {
        use_new_update = false;
    }

    if (use_new_update)
    {
        last_velocity_update = update;
        return update;
    }
    else
    {
        return last_velocity_update;
    }
}

}
}
}
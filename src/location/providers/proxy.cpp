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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com
 */

#include <location/providers/proxy.h>

location::providers::Proxy::Proxy(const std::set<Provider::Ptr>& providers)
      : providers{providers}
{
    for (auto provider : providers)
    {
        connections.emplace_back(provider->position_updates().connect([this](const location::Update<location::Position>& u)
        {
            updates.position(u);
        }));

        connections.emplace_back(provider->heading_updates().connect([this](const location::Update<location::units::Degrees>& u)
        {
            updates.heading(u);
        }));

        connections.emplace_back(provider->velocity_updates().connect([this](const location::Update<location::units::MetersPerSecond>& u)
        {
            updates.velocity(u);
        }));
    }
}

void location::providers::Proxy::on_new_event(const Event& event)
{
    for (auto provider: providers) provider->on_new_event(event);
}

void location::providers::Proxy::enable()
{
    for (auto provider : providers) provider->enable();
}

void location::providers::Proxy::disable()
{
    for (auto provider : providers) provider->disable();
}

void location::providers::Proxy::activate()
{
    for (auto provider : providers) provider->activate();
}

void location::providers::Proxy::deactivate()
{
    for (auto provider : providers) provider->deactivate();
}

location::Provider::Requirements location::providers::Proxy::requirements() const
{
    location::Provider::Requirements reqs{location::Provider::Requirements::none};
    for (auto provider : providers) reqs = reqs | provider->requirements();
    return reqs;
}

bool location::providers::Proxy::satisfies(const Criteria& criteria)
{
    bool result{true};
    for (auto provider : providers) result &= provider->satisfies(criteria);
    return result;
}

const core::Signal<location::Update<location::Position>>& location::providers::Proxy::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& location::providers::Proxy::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& location::providers::Proxy::velocity_updates() const
{
    return updates.velocity;
}

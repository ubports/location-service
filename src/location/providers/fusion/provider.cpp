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

#include <location/providers/fusion/provider.h>

#include <location/logging.h>
#include <location/update.h>

namespace fusion = location::providers::fusion;

fusion::Provider::Provider(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector)
      : providers{providers}
{
}

void fusion::Provider::on_new_event(const Event& event)
{
    for (auto provider : providers) provider->on_new_event(event);
}

void fusion::Provider::enable()
{
    for (auto provider : providers) provider->enable();
}

void fusion::Provider::disable()
{
    for (auto provider : providers) provider->disable();
}

void fusion::Provider::activate()
{
    for (auto provider : providers) provider->activate();
}

void fusion::Provider::deactivate()
{
    for (auto provider : providers) provider->deactivate();
}

location::Provider::Requirements fusion::Provider::requirements() const
{
    location::Provider::Requirements reqs{location::Provider::Requirements::none};
    for (auto provider : providers) reqs = reqs | provider->requirements();
    return reqs;
}

bool fusion::Provider::satisfies(const Criteria& criteria)
{
    bool result{true};
    for (auto provider : providers) result &= provider->satisfies(criteria);
    return result;
}

const core::Signal<location::Update<location::Position>>& fusion::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::Heading>>& fusion::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::Velocity>>& fusion::Provider::velocity_updates() const
{
    return updates.velocity;
}

fusion::Provider::Ptr fusion::Provider::finalize(const UpdateSelector::Ptr& update_selector)
{
    auto sp = shared_from_this(); std::weak_ptr<fusion::Provider> wp{sp};

    for (auto provider : providers)
    {
        provider->position_updates().connect(
              [wp, provider, update_selector](const location::Update<location::Position>& u)
              {
                  if (auto sp = wp.lock())
                  {
                      // if this is the first update, use it
                      if (!sp->last_position) {
                          sp->updates.position((*(sp->last_position = WithSource<Update<Position>>{provider, u})).value);

                      // otherwise use the selector
                      } else {
                          try {
                              sp->updates.position((*(sp->last_position = update_selector->select(*sp->last_position, WithSource<Update<Position>>{provider, u}))).value);
                          } catch (const std::exception& e) {
                              LOG(WARNING) << "Error while updating position";
                          }
                      }
                  }
              });
        provider->heading_updates().connect(
              [wp](const location::Update<location::Heading>& u)
              {
                  if (auto sp = wp.lock())
                    sp->updates.heading(u);
              });
        provider->velocity_updates().connect(
              [wp](const location::Update<location::Velocity>& u)
              {
                  if (auto sp = wp.lock())
                    sp->updates.velocity(u);
              });
    }

    return sp;
}

fusion::Provider::Ptr fusion::Provider::create(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector)
{
    auto sp = fusion::Provider::Ptr{new fusion::Provider{providers, update_selector}};
    return sp->finalize(update_selector);
}

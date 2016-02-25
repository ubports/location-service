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
 #include <com/ubuntu/location/fusion_provider.h>
 #include <com/ubuntu/location/update.h>

namespace cu = com::ubuntu;
namespace cul = com::ubuntu::location;

bool is_better_than(cul::Update<cul::Position> a, cul::Update<cul::Position> b)
{

    // Basically copied this value from the Android fusion provider
    const std::chrono::seconds cutoff(11);

    // if b is a bogus (initial) position return true
    if (b.value == cul::Position()) {
        return true;
    }

    // If the new position is newer by a significant margin then just use it
    if (a.when > b.when + cutoff) {
        return true;
    }

    // Choose the position with the smaller accuracy circle
    if (!a.value.accuracy.horizontal)
        return false;
    if(!b.value.accuracy.horizontal)
        return true;
    return a.value.accuracy.horizontal < b.value.accuracy.horizontal;
}

cul::FusionProvider::FusionProvider(const std::set<location::Provider::Ptr>& providers)
    : BagOfProviders(providers)
{
    // XXX: BoP constructor sets up these connections, which leads to duplicated
    // events
    connections.clear();

    for (auto provider : providers)
    {
        connections.push_back(provider->updates().position.connect(
              [this](const cul::Update<cul::Position>& u)
              {
                  if (is_better_than(u, last_position)) {
                      mutable_updates().position(u);
                      last_position = u;
                  }
              }));
        connections.push_back(provider->updates().heading.connect(
              [this](const cul::Update<cul::Heading>& u)
              {
                  mutable_updates().heading(u);
              }));
        connections.push_back(provider->updates().velocity.connect(
              [this](const cul::Update<cul::Velocity>& u)
              {
                  mutable_updates().velocity(u);
              }));
    }

}

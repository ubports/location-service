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
#ifndef LOCATION_PROVIDERS_PROXY_H
#define LOCATION_PROVIDERS_PROXY_H

#include <location/provider.h>

#include <set>
#include <vector>

namespace location
{
namespace providers
{
class Proxy : public Provider
{
public:
    explicit Proxy(const std::set<Provider::Ptr>& providers);

    // From Provider
    void on_new_event(const Event&) override;
    void enable() override;
    void disable() override;
    void activate() override;
    void deactivate() override;

    Requirements requirements() const override;
    bool satisfies(const Criteria& criteria) override;
    const core::Signal<Update<Position>>& position_updates() const override;
    const core::Signal<Update<units::Degrees>>& heading_updates() const override;
    const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

private:
    std::set<Provider::Ptr> providers;
    std::vector<core::ScopedConnection> connections;
    struct
    {
        core::Signal<Update<Position>> position;
        core::Signal<Update<units::Degrees>> heading;
        core::Signal<Update<units::MetersPerSecond>> velocity;
    } updates;
};
}
}
#endif // LOCATION_PROVIDERS_PROXY_H

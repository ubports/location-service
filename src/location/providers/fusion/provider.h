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
#ifndef LOCATION_PROVIDERS_FUSION_PROVIDER_H
#define LOCATION_PROVIDERS_FUSION_PROVIDER_H

#include <location/provider.h>
#include <location/providers/fusion/update_selector.h>

#include <vector>

namespace location
{
namespace providers
{
namespace fusion
{
class Provider : public std::enable_shared_from_this<Provider>, public location::Provider
{
public:
    typedef std::shared_ptr<Provider> Ptr;

    static Ptr create(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector);

    // From Provider
    void on_new_event(const Event& event) override;

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
    Provider(const std::set<location::Provider::Ptr>& providers, const UpdateSelector::Ptr& update_selector);

    std::shared_ptr<Provider> finalize(const UpdateSelector::Ptr& update_selector);

    Optional<WithSource<Update<Position>>> last_position;
    std::set<location::Provider::Ptr> providers;
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
}

#endif // LOCATION_PROVIDERS_FUSION_PROVIDER_H

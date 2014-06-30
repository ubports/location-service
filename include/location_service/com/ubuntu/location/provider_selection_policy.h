/*
 * Copyright © 2012-2013 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_

#include <com/ubuntu/location/provider_enumerator.h>
#include <com/ubuntu/location/provider_selection.h>

#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria;
class Engine;

class ProviderSelectionPolicy
{
public:
    typedef std::shared_ptr<ProviderSelectionPolicy> Ptr;

    static const Provider::Ptr& null_provider();

    ProviderSelectionPolicy(const ProviderSelectionPolicy&) = delete;
    ProviderSelectionPolicy& operator=(const ProviderSelectionPolicy&) = delete;
    virtual ~ProviderSelectionPolicy() = default;

    virtual ProviderSelection determine_provider_selection_for_criteria(
            const Criteria& criteria,
            const ProviderEnumerator& enumerator) = 0;

protected:
    ProviderSelectionPolicy() = default;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_

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
#ifndef PROVIDER_ENUMERATOR_H_
#define PROVIDER_ENUMERATOR_H_

#include <functional>
#include <memory>

namespace location
{
class Provider;
class ProviderEnumerator
{
public:
    ProviderEnumerator(const ProviderEnumerator&) = delete;
    virtual ~ProviderEnumerator() = default;

    ProviderEnumerator& operator=(const ProviderEnumerator&) = delete;
    bool operator==(const ProviderEnumerator&) const = delete;

    virtual void for_each_provider(const std::function<void(const std::shared_ptr<Provider>&)>&) const = 0;

protected:
    ProviderEnumerator() = default;
};
}

#endif // PROVIDER_ENUMERATOR_H_

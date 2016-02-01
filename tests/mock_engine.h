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
#ifndef MOCK_ENGINE_H
#define MOCK_ENGINE_H

#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/provider.h>

#include <gmock/gmock.h>

struct MockEngine : public com::ubuntu::location::Engine
{
    MockEngine(
            const com::ubuntu::location::ProviderSelectionPolicy::Ptr& provider_selection_policy,
            const com::ubuntu::location::Settings::Ptr& settings)
            : com::ubuntu::location::Engine(provider_selection_policy, settings)
    {
    }

    // has_provider and remove_provider cannot be mocked because they are marked noexcept
    MOCK_METHOD1(determine_provider_selection_for_criteria, com::ubuntu::location::ProviderSelection(const com::ubuntu::location::Criteria&));
    MOCK_METHOD1(add_provider, void(const com::ubuntu::location::Provider::Ptr&));
    MOCK_METHOD1(for_each_provider, void(const std::function<void(const com::ubuntu::location::Provider::Ptr&)>& enumerator));
};

#endif // MOCK_ENGINE_H

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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com>
 */

#include <com/ubuntu/location/logging.h>

#include <chrono>
#include <thread>

#include "delayed_provider.h"

namespace location = com::ubuntu::location;
namespace dummy = com::ubuntu::location::providers::dummy;

std::string dummy::DelayedProvider::class_name()
{
    return "dummy::DelayedProvider";
}

location::Provider::Ptr dummy::DelayedProvider::create_instance(const ProviderFactory::Configuration& config)
{
    int delay = config.get(dummy::DelayConfiguration::Keys::delay, 0);
    VLOG(1) << __PRETTY_FUNCTION__ << ": delay for " << delay << "ms";

    std::this_thread::sleep_for(std::chrono::milliseconds{delay});

    return dummy::Provider::create_instance(config);
}

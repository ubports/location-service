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

#include <location/providers/gps/hardware_abstraction_layer.h>

#include <iostream>

namespace gps = location::providers::gps;

std::ostream& gps::operator<<(std::ostream& out, const gps::HardwareAbstractionLayer::ReferenceTimeSample& sample)
{
    return out << "[since epoch: "  << sample.since_epoch.count() << " [ms], since boot: "
                                    << sample.since_boot.count() << " [ms], uncertainty: "
                                    << sample.uncertainty.count() << " [ms]";
}

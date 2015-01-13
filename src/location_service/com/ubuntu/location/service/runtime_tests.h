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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_RUNTIME_TESTS_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_RUNTIME_TESTS_H_

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
// execute_runtime_tests runs all configured runtime tests.
// Returns 0 if successful.
int execute_runtime_tests();
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_RUNTIME_TESTS_H_

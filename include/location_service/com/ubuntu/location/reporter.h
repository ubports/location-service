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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_REPORTER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_REPORTER_H_

#include <core/property.h>

#include <memory>

namespace com
{
namespace ubuntu
{
namespace connectivity
{
class Manager;
}
namespace location
{
class Engine;
/**
 * @brief The Reporter class is the abstract base of all positioning reporters.
 */
class Reporter
{
public:
    typedef std::shared_ptr<Reporter> Ptr;

    virtual ~Reporter() = default;

    Reporter(const Reporter&) = delete;
    Reporter& operator=(const Reporter&) = delete;

protected:
    /**
     * @brief Constructs the object and makes the engine and manager available to implementations.
     * @param engine The location engine to be used by implementations.
     * @param manager The connectivity manager to be used by implementations.
     */
    explicit Reporter(
        const std::shared_ptr<com::ubuntu::location::Engine>& engine,
        const std::shared_ptr<com::ubuntu::connectivity::Manager>& manager);

    /**
     * @brief Provides access to the location engine instance.
     */
    const std::shared_ptr<com::ubuntu::location::Engine>& location_engine();

    /**
     * @brief Provides access to the connectivity manager instance.
     */
    const std::shared_ptr<com::ubuntu::connectivity::Manager>& connectivity_manager();

private:
    struct
    {
        /** The location engine instance. */
        std::shared_ptr<com::ubuntu::location::Engine> location_engine;
        /** The connectivity manager instance. */
        std::shared_ptr<com::ubuntu::connectivity::Manager> connectivity_manager;
    } d;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_REPORTER_H_

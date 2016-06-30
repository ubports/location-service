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
#ifndef LOCATION_PERMISSION_MANAGER_H_
#define LOCATION_PERMISSION_MANAGER_H_

#include <memory>

namespace location
{
struct Criteria;
/** @brief Credentials of a remote session. */
struct Credentials
{
    /** @brief The process id of the remote peer. */
    pid_t pid;
    /** @brief The user id the remote peer runs under. */
    uid_t uid;
};

/**
 * @brief The PermissionManager class is an interface to check whether an application
 * is allowed to access the location services.
 */
class PermissionManager
{
public:
    /** Manager pointer type. */
    typedef std::shared_ptr<PermissionManager> Ptr;

    /**
     * @brief The Result enum summarizes the results of a query for permissions.
     */
    enum class Result
    {
        granted, ///< The app is allowed to access the location service.
        rejected ///< The app is not allowed to access the location service.
    };

    virtual ~PermissionManager() = default;
    PermissionManager(const PermissionManager&) = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;

    /**
     * @brief Checks whether the app with the given credentials is allowed to access the service for the given criteria.
     * @param criteria The requirements of the remote peer.
     * @param credentials The credentials identifying the remote peer.
     * @return Result::granted if the remote peer is allowed to access the location service, Result::rejected otherwise.
     */
    virtual Result check_permission_for_credentials(
        const Criteria& criteria,
        const Credentials& credentials) = 0;
    
protected:
    PermissionManager() = default;
};
}

#endif // LOCATION_PERMISSION_MANAGER_H_

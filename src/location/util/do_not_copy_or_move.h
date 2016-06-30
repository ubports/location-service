/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
 *
 */

#ifndef LOCATION_UTIL_DO_NOT_COPY_OR_MOVE_H_
#define LOCATION_UTIL_DO_NOT_COPY_OR_MOVE_H_

namespace location
{
namespace util
{
/// @cond
class DoNotCopyOrMove
{
public:
    DoNotCopyOrMove(const DoNotCopyOrMove&) = delete;
    DoNotCopyOrMove(DoNotCopyOrMove&&) = delete;
    virtual ~DoNotCopyOrMove() = default;
    DoNotCopyOrMove& operator=(const DoNotCopyOrMove&) = delete;
    DoNotCopyOrMove& operator=(DoNotCopyOrMove&&) = delete;

protected:
    DoNotCopyOrMove() = default;
};
/// @endcond
}
}

#endif // LOCATION_UTIL_DO_NOT_COPY_OR_MOVE_H_

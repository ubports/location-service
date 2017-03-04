/*
 * Copyright © 2017 Canonical Ltd.
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
#ifndef LOCATION_GLIB_SHARED_OBJECT_H_
#define LOCATION_GLIB_SHARED_OBJECT_H_

#include <glib-object.h>

#include <iostream>
#include <memory>

namespace location
{
namespace glib
{

template<typename T>
struct ObjectDeleter
{
    void operator()(T* object) const
    {
        if (object)
        {
            g_object_unref(object);
        }
    }
};

template<typename T>
using SharedObject = std::shared_ptr<T>;

template<typename T>
SharedObject<T> make_shared_object(T* object)
{
    return SharedObject<T>{object, ObjectDeleter<T>{}};
}

template<typename T>
T* ref_object(T* object)
{
    return static_cast<T*>(g_object_ref(object));
}

}  // namespace glib
}  // namespace location

#endif  // LOCATION_GLIB_SHARED_OBJECT_H_

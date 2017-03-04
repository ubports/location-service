/*
 * Copyright © 2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTIlocationAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef LOCATION_RESULT_H_
#define LOCATION_RESULT_H_

#include <boost/variant.hpp>

#include <stdexcept>

namespace location
{

template<typename T>
class Result {
public:
    Result(const T& value) : variant{value}
    {
    }

    Result(const std::exception_ptr& exception)
        : variant{exception}
    {
    }

    explicit operator bool() const
    {
        return variant.which() == 0;
    }

    const T& value() const
    {
        if (variant.which() != 0)
            rethrow();
        return boost::get<T>(variant);
    }

    void rethrow() const
    {
        std::rethrow_exception(boost::get<std::exception_ptr>(variant));
    }

private:
    using Variant = boost::variant<T, std::exception_ptr>;
    Variant variant;
};

template<typename T>
inline Result<T> make_result(const T& value)
{
    return Result<T>{value};
}

template<typename T>
inline Result<T> make_error_result(std::exception_ptr exception)
{
    return Result<T>{exception};
}

}  // namespace location

#endif  // LOCATION_RESULT_H_

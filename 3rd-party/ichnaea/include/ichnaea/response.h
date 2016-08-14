// Copyright (C) 2016 Canonical Ltd.
// 
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ICHNAEA_RESPONSE_H_
#define ICHNAEA_RESPONSE_H_

#include <ichnaea/error.h>

#include <boost/variant.hpp>

#include <type_traits>

namespace ichnaea
{
/// @brief Repsonse models a typed variant that either contains the Result of
/// an operation xor an error if the operation fails.
template<typename Result>
class Response
{
  public:
    /// @brief NotAnError is thrown if error() is invoked on a Response<T>
    /// instance that contains a result.
    struct NotAnError : public std::runtime_error
    {
        NotAnError() : std::runtime_error{"Not an error"}
        {
        }
    };

    /// @brief Response initializes a new instance with result such
    /// that subsequent invocations of is_error() return false.
    explicit Response(const Result& result)
            : result_or_error{result}
    {
    }

    /// @brief Response initializes a new instance with error such
    /// that subsequent invocations of is_error() return true.
    explicit Response(const Error& error)
            : result_or_error{error}
    {
    }

    /// @brief is_error returns true if the instance contains an error.
    bool is_error() const
    {
        return 1 == result_or_error.which();
    }

    /// @brief error returns the error response or throws NotAnError
    /// if no error is contained in the instance.
    const Error& error() const
    {
        if (not is_error())
            throw NotAnError{};

        return boost::get<Error>(result_or_error);
    }

    /// @brief result returns the Result instance. Throws the contained
    /// error if this is an error response.
    const Result& result() const
    {
        if (is_error())
            throw error();

        return boost::get<Result>(result_or_error);
    }

  private:
    boost::variant<Result, Error> result_or_error;    
};
}

#endif // ICHNAEA_RESPONSE_H_

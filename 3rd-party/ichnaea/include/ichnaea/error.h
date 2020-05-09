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
#ifndef ICHNAEA_ERROR_H_
#define ICHNAEA_ERROR_H_

#include <core/net/http/status.h>

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

namespace ichnaea
{
/// @brief Error models an error response.
struct Error : public std::runtime_error
{
    /// @brief Detail provides further details describing
    /// an error condition.
    struct Detail
    {
        std::string domain;
        std::string reason;
        std::string message;
    };

    /// @brief Initializes a new instance with code and message.
    ///
    /// A human-readable message is generated from code and message,
    /// handing it to the std::runtime_error ctor such that what invocations
    /// provide a meaningful summary of the exception.
    Error(core::net::http::Status code, const std::string& message);
    
    std::vector<Detail> errors;   ///< errors provides further details about the error condition.
    core::net::http::Status code; ///< code describes the top-level class of the error condition.
    std::string message;          ///< message provides a human-readable error message.
    
};

/// @brief operator<< inserts error into out.
std::ostream& operator<<(std::ostream& out, const Error& error);
}

#endif // ICHNAEA_ERROR_H_

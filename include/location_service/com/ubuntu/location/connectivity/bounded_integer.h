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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_BOUNDED_INTEGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_BOUNDED_INTEGER_H_

#include <iostream>
#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace connectivity
{
/**
 * @brief A helper class to handle bounded integer values, with an optional domain
 * for tagging domain-specific types.
 */
template<int min, int max, int domain = 0>
class BoundedInteger
{
public:
    static_assert(min < max, "min >= max");

    /**
     * @brief Access the minimum value of the integer.
     */
    inline static int minimum()
    {
        return min;
    }

    /**
     * @brief Access the maximum value of the integer.
     */
    inline static int maximum()
    {
        return max;
    }

    /**
     * @brief Returns max - min.
     */
    inline static int range()
    {
        return max - min;
    }

    /**
     * @brief Constructs an invalid instance.
     */
    BoundedInteger() : value(min-1)
    {
    }

    /**
     * @brief Constructs an instance from a raw value
     * @param value The raw value.
     * @throw std::runtime_error if value is not in [min, max].
     */
    explicit BoundedInteger(int value) : value(value)
    {
        if (value < min || value > max)
            throw std::runtime_error(
                    std::to_string(value) + " is not in " + "[" +
                    std::to_string(min) + ", " + std::to_string(max) + "]");
    }

    /**
     * @brief Copy c'tor.
     * @param rhs The instance to copy from.
     */
    BoundedInteger(const BoundedInteger<min, max, domain>& rhs) : value(rhs.value)
    {
    }

    /**
     * @brief Assignment operator.
     * @param rhs The instance to assign from.
     * @return A mutable reference to this instance.
     */
    BoundedInteger<min, max, domain>& operator=(const BoundedInteger<min, max, domain>& rhs)
    {
        value = rhs.value;
        return *this;
    }

    /**
     * @brief Equality comparison operator.
     * @param rhs The instance to compare to.
     * @return true iff both instances' value are equal.
     */
    bool operator==(const BoundedInteger<min, max, domain>& rhs) const
    {
        return value == rhs.value;
    }

    /**
     * @brief Implicit casting operator to a raw integer value.
     * @return The raw integer value.
     */
    inline operator int() const
    {
        return get();
    }

    /**
     * @brief is_valid checks whether the contained value is in [min, max].
     * @return true iff the contained integer value is in [min, max].
     */
    inline bool is_valid() const
    {
        return min <= value && value <= max;
    }

    /**
     * @brief Returns the raw integer value contained in this instance.
     * @throw std::runtime_error if is_valid() returns false.
     */
    inline int get() const
    {
        if (!is_valid())
            throw std::runtime_error("BoundedInteger::get: Contained value is not valid.");

        return value;
    }

    /**
     * @brief Assigns a new raw integer value
     * @param new_value The new value.
     * @throw std::runtime_error if new_value is not in [min, max].
     */
    inline void set(int new_value)
    {
        if (new_value < min || new_value > max)
            throw std::runtime_error(
                    std::to_string(new_value) + " is not in " + "[" +
                    std::to_string(min) + ", " + std::to_string(max) + "]");

        value = new_value;
    }

    /**
     * @brief operator << pretty prints an instance of BoundedInteger.
     * @param out The stream to print to.
     * @param bi The instance to print.
     * @return The stream that has been printed to.
     */
    inline friend std::ostream& operator<<(std::ostream& out, const BoundedInteger<min, max, domain>& bi)
    {
        out << bi.value;

        if (!bi.is_valid())
            out << " -> invalid";

        return out;
    }

private:
    int value;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_BOUNDED_INTEGER_H_

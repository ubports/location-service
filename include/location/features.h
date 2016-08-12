/*
 * Copyright © 2016 Canonical Ltd.
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

#ifndef LOCATION_FEATURES_H_
#define LOCATION_FEATURES_H_

#include <iosfwd>

namespace location
{
/**
 * @brief Enumerates the known features that can be supported by providers.
 */
enum class Features : std::size_t
{
    none        = 0,        ///< The provider does not support any feature.
    position    = 1 << 0,   ///< The provider features position updates.
    velocity    = 1 << 1,   ///< The provider features velocity updates.
    heading     = 1 << 2    ///< The provider features heading updates.
};

/** @brief operator| returns the bitwise or of lhs and rhs. */
Features operator|(Features lhs, Features rhs);
/** @brief operator& returns the bitwise and of lhs and rhs. */
Features operator&(Features lhs, Features rhs);

/** @brief operator<< inserts features into out. */
std::ostream& operator<<(std::ostream& out, Features features);

/** @brief operator>> extracts features from in. */
std::istream& operator>>(std::istream& in, Features& features);
}

#endif // LOCATION_FEATURES_H_

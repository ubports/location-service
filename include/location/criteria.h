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
#ifndef LOCATION_CRITERIA_H_
#define LOCATION_CRITERIA_H_

#include <location/features.h>
#include <location/optional.h>
#include <location/visibility.h>
#include <location/units/units.h>

namespace location
{
/// @brief Summarizes criteria of a client session with respect to functionality
/// and accuracy for position, velocity and heading measurements.
///
/// Criteria implements a named parameter interface to gradually refine
/// Criteria instances as in:
///   Criteria{}.requirements(Features::positions);
struct LOCATION_DLL_PUBLIC Criteria
{
    /// @brief Accuracy bundles up requirements on the quality of
    /// position, velocity and heading estimates.
    ///
    /// Accuracy implements a named parameter interface to gradually
    /// refine Accuracy instances as in:
    ///   Accuracy{}.horizontal(10 * units::meters)
    ///             .vertical(15 * units::meters);
    class Accuracy
    {
    public:
        /// @cond
        Accuracy();
        ~Accuracy();
        Accuracy(const Accuracy&);
        Accuracy(Accuracy&&);
        Accuracy& operator=(const Accuracy&);
        Accuracy& operator=(Accuracy&&);
        /// @endcond

        /// @brief horizontal returns the optional horizontal accuracy.
        const Optional<units::Meters>& horizontal() const;
        /// @brief horizontal returns the optional horizontal accuracy.
        Optional<units::Meters>& horizontal();
        /// @brief horizontal adjusts the optional horizontal accuracy to value.
        Accuracy horizontal(const units::Meters& value) const;
        /// @brief horizontal adjusts the optional horizontal accuracy to value.
        Accuracy& horizontal(const units::Meters& value);

        /// @brief vertical returns the optional vertical accuracy.
        const Optional<units::Meters>& vertical() const;
        /// @brief vertical returns the optional vertical accuracy.
        Optional<units::Meters>& vertical();
        /// @brief vertical adjusts the optional vertical accuracy to value.
        Accuracy vertical(const units::Meters& value) const;
        /// @brief vertical adjusts the optional vertical accuracy to value.
        Accuracy& vertical(const units::Meters& value);

        /// @brief velocity returns the optional accuracy of the velocity estimate.
        const Optional<units::MetersPerSecond>& velocity() const;
        /// @brief velocity returns the optional accuracy of the velocity estimate.
        Optional<units::MetersPerSecond>& velocity();
        /// @brief velocity adjusts the optional accuracy of the velocity estimate to value.
        Accuracy velocity(const units::MetersPerSecond& value) const;
        /// @brief velocity adjusts the optional accuracy of the velocity estimate to value.
        Accuracy& velocity(const units::MetersPerSecond& value);

        /// @brief heading returns the optional accuracy of the heading estimate.
        const Optional<units::Degrees>& heading() const;
        /// @brief heading returns the optional accuracy of the heading estimate.
        Optional<units::Degrees>& heading();
        /// @brief heading adjusts the optional accuracy of the heading estimate to value.
        Accuracy heading(const units::Degrees& value) const;
        /// @brief heading adjusts the optional accuracy of the heading estimate to value.
        Accuracy& heading(const units::Degrees& value);

    private:
        /// @cond
        struct Private;
        std::unique_ptr<Private> d;
        /// @endcond
    };

    /// @brief Criteria initializes a new instance with requirements.
    explicit Criteria(Features requirements = Features::position | Features::heading | Features::velocity);
    /// @cond
    Criteria(const Criteria&);
    Criteria(Criteria&&);
    Criteria& operator=(const Criteria&);
    Criteria& operator=(Criteria&&);
    ~Criteria();
    /// @endcond

    /// @brief requirements returns the configured requirements.
    Features requirements() const;
    /// @brief requirements returns the configured requirements.
    Features& requirements();
    /// @brief requirements sets the configured requirements to value.
    Criteria requirements(Features value) const;
    /// @brief requirements sets the configured requirements to value.
    Criteria& requirements(Features value);

    /// @brief accuracy returns an immutable reference to the requested accuracy.
    const Accuracy& accuracy() const;
    /// @brief accuracy returns a mutable reference to the requested accuracy.
    Accuracy& accuracy();

private:
    /// @cond
    struct Private;
    std::unique_ptr<Private> d;
    /// @endcond
};
}

#endif // LOCATION_CRITERIA_H_

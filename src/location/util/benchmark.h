/*
 * Copyright (C) 2017 Canonical, Ltd.
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
#ifndef LOCATION_UTIL_BENCHMARK_H_
#define LOCATION_UTIL_BENCHMARK_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>

namespace location
{
namespace util
{

class Benchmark
{
public:
    explicit Benchmark(std::size_t trials, const std::string& title);

    std::size_t trials() const;

    void run(std::function<void(std::size_t)> prepare,
             std::function<void(std::size_t)> run);

    std::ostream& print(std::ostream& out) const;

private:
    typedef boost::accumulators::accumulator_set<
        double,
        boost::accumulators::stats<
            boost::accumulators::tag::min,
            boost::accumulators::tag::max,
            boost::accumulators::tag::median
        >
    > MinMedianMax;

    typedef boost::accumulators::accumulator_set<
        double,
        boost::accumulators::stats<
            boost::accumulators::tag::p_square_quantile
        >
    > Quantile;

    std::size_t trials_;
    std::string title_;
    MinMedianMax min_median_max;
    Quantile first_quartile{boost::accumulators::quantile_probability = .25};
    Quantile third_quartile{boost::accumulators::quantile_probability = .75};
};

std::ostream& operator<<(std::ostream& out, const Benchmark& benchmark);

}  // namespace util
}  // namespace location

#endif  // LOCATION_UTIL_BENCHMARK_H_

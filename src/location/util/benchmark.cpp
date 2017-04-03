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

#include <location/util/benchmark.h>

#include <location/clock.h>

#include <iomanip>
#include <iostream>

namespace ba = boost::accumulators;

location::util::Benchmark::Benchmark(std::size_t trials, const std::string& title)
    : trials_{trials}, title_{title}
{
}

std::size_t location::util::Benchmark::trials() const
{
    return trials_;
}

void location::util::Benchmark::run(std::function<void(std::size_t)> prepare,
                                    std::function<void(std::size_t)> run)
{
    for (std::size_t trial = 1; trial <= trials_; trial++)
    {
        prepare(trial);

        auto start = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());
        run(trial);
        auto stop = std::chrono::duration_cast<std::chrono::microseconds>(location::Clock::now().time_since_epoch());

        auto diff = stop - start;

        min_median_max(diff.count());
        first_quartile(diff.count());
        third_quartile(diff.count());
    }
}

std::ostream& location::util::Benchmark::print(std::ostream& out) const
{
    static constexpr std::size_t column_width{15};

    return out << std::setw(column_width) << std::setfill(' ') << std::left << title_ << " | "
               << std::setw(column_width) << std::setfill(' ') << std::left << "min" << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << "1st quartile" << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << "median" << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << "3rd quartile" << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << "max" << std::endl
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << "---"
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << "-"
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << "-"
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << "-"
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << "-"
               << std::setw(column_width) << std::setfill('-') << std::left << "-" << std::endl
               << std::setw(column_width) << std::setfill(' ') << std::left << " " << "| "
               << std::setw(column_width) << std::setfill(' ') << std::left << ba::min(min_median_max) << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << ba::p_square_quantile(first_quartile) << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << ba::median(min_median_max) << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << ba::p_square_quantile(third_quartile) << " "
               << std::setw(column_width) << std::setfill(' ') << std::left << ba::max(min_median_max) << " ";
}

std::ostream& location::util::operator<<(std::ostream& out, const Benchmark& benchmark)
{
    return benchmark.print(out);
}

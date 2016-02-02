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

#include <com/ubuntu/location/time_since_boot.h>
#include <com/ubuntu/location/logging.h>

#if defined(COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API)
#include <ubuntu/hardware/alarm.h>
#endif // COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace location = com::ubuntu::location;

namespace
{
bool have_ubuntu_hardware_alarm()
{
#if defined(COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API)
    return true;
#else
    return false;
#endif // COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API
}

std::chrono::nanoseconds from_ubuntu_hardware_alarm()
{
#if defined(COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API)
    struct Scope
    {
        Scope() : alarm{u_hardware_alarm_create()}
        {
        }

        ~Scope()
        {
            u_hardware_alarm_unref(alarm);
        }

        UHardwareAlarm alarm{nullptr};
    };

    static Scope scope;

    ::timespec ts;
    if (not scope.alarm)
        throw std::runtime_error{"Failed to access UHardwareAlarm subsystem."};

    if (U_STATUS_SUCCESS != u_hardware_alarm_get_elapsed_real_time(scope.alarm, &ts))
        throw std::runtime_error{"Failed to query time since boot from UHardwareAlarm."};

    return std::chrono::seconds{ts.tv_sec} + std::chrono::nanoseconds{ts.tv_nsec};
#else
    throw std::logic_error{"not implemented"};
#endif // COM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API
}


std::chrono::nanoseconds from_clock_boottime()
{
    ::timespec ts;
    if (-1 == clock_gettime(CLOCK_BOOTTIME, &ts))
        throw std::system_error(errno, std::system_category());

    return std::chrono::seconds{ts.tv_sec} + std::chrono::nanoseconds{ts.tv_nsec};
}

std::chrono::nanoseconds boot_time()
{
    std::ifstream in{"/proc/stat"};
    std::string line;

    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss{line};
        std::string name; ss >> name;

        if (name != "btime")
            continue;

        std::uint64_t ts; ss >> ts;
        return std::chrono::nanoseconds{std::chrono::seconds{ts}};
    }

    throw std::runtime_error{"Failed to determine system boot time."};
}

std::chrono::nanoseconds from_proc_stat()
{
    static const std::chrono::nanoseconds bt{boot_time()};
    return std::chrono::system_clock::now().time_since_epoch() - bt;
}
}

std::chrono::nanoseconds location::time_since_boot()
{
    if (have_ubuntu_hardware_alarm())
    {
        try
        {
            return from_ubuntu_hardware_alarm();
        }
        catch (const std::exception& e)
        {
            VLOG(1) << e.what();
        }
    }

    try
    {
        return from_clock_boottime();
    }
    catch (const std::exception& e)
    {
        VLOG(1) << e.what();
    }

    return from_proc_stat();
}

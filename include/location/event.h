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
#ifndef LOCATION_EVENT_H_
#define LOCATION_EVENT_H_

#include <location/visibility.h>

#include <memory>

namespace location
{
/// @brief Event models the base of an event hierarchy used to pass
/// system- and session-wide events to providers and sessions.
class LOCATION_DLL_PUBLIC Event
{
public:
    // Safe us some typing
    typedef std::shared_ptr<Event> Ptr;

    /// @brief Receiver models handling of incoming messages.
    class Receiver
    {
    public:
        // Safe us some typing.
        typedef std::shared_ptr<Receiver> Ptr;

        /// @cond
        virtual ~Receiver() = default;
        Receiver(const Receiver&) = delete;
        Receiver(Receiver&&) = delete;
        Receiver& operator=(const Receiver&) = delete;
        Receiver& operator=(Receiver&&) = delete;
        /// @endcond

        /// @brief on_new_event is invoked for every new event.
        virtual void on_new_event(const Event& event) = 0;

    protected:
        Receiver() = default;
    };

    /// @brief Type enumerates all known types.
    ///
    /// Not an enum class on purpose.
    enum Type
    {
        first_user_defined_type = 1024
    };

    /// @brief next_user_defined_type returns the next available slot for a user-defined type.
    static Type next_user_defined_type(const std::string& name);

    /// @brief register_type makes T known to the bus and assigns it a unique Type.
    template<typename T>
    static Type register_type(const std::string& name);

    /// @cond
    virtual ~Event() = default;

    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;
    /// @endcond

    /// @brief type returns the Type of the message.
    virtual Type type() const = 0;

    /// @cond
    Event() = default;
    /// @endcond
};

/// @brief TypeOf enables calling code to check for a specific event type.
template<typename T>
struct LOCATION_DLL_PUBLIC TypeOf
{
    /// @brief Query returns the Event::Type registered for T.
    ///
    /// Left unimplemented on purpose, to provoke linker errors.
    /// in the non-specialized case.
    static Event::Type query();
};

template<typename T>
Event::Type Event::register_type(const std::string& name)
{
    static Event::Type type{Event::next_user_defined_type(name)};
    return type;
}
}

#endif // LOCATION_EVENT_H_

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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"

#include <com/ubuntu/property.h>

#include <atomic>
#include <bitset>
#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
class Provider
{
public:
    typedef std::shared_ptr<Provider> Ptr;

    enum class Features : std::size_t
    {
        none = 0,
        position = 1 << 0,
        velocity = 1 << 1,
        heading = 1 << 2
    };    

    enum class Requirements : std::size_t
    {
        none = 0,
        satellites = 1 << 0,
        cell_network = 1 << 1,
        data_network = 1 << 2,
        monetary_spending = 1 << 4
    };    

    class Controller
    {
    public:
        typedef std::shared_ptr<Controller> Ptr;

        template<typename T> 
        class Cache
        {
          public:
            Cache() : d{ T{}, false }
            {
            }
            const T& value() const { return d.value; }
            void update(const T& new_value) { d.value = new_value; d.is_valid = true; }
            bool is_valid() const { return d.is_valid; }
            void invalidate() { d.is_valid = false; }

          private:
            struct
            {
                T value;
                bool is_valid;
            } d;
        };

        virtual ~Controller() = default;
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;

        virtual void start_position_updates();
        virtual void stop_position_updates();
        bool are_position_updates_running() const;

        virtual void start_heading_updates();
        virtual void stop_heading_updates();
        bool are_heading_updates_running() const;

        virtual void start_velocity_updates();
        virtual void stop_velocity_updates();
        bool are_velocity_updates_running() const;

        const Cache<Update<Position>>& cached_position_update() const;
        const Cache<Update<Heading>>& cached_heading_update() const;
        const Cache<Update<Velocity>>& cached_velocity_update() const;

    protected:
        friend class Provider;
        explicit Controller(Provider& instance);

    private:
        void on_position_updated(const Update<Position>& position);
        void on_velocity_updated(const Update<Velocity>& velocity);
        void on_heading_updated(const Update<Heading>& heading);

        Provider& instance;
        std::atomic<int> position_updates_counter;
        std::atomic<int> heading_updates_counter;
        std::atomic<int> velocity_updates_counter;
        ScopedChannelConnection position_update_connection;
        ScopedChannelConnection velocity_update_connection;
        ScopedChannelConnection heading_update_connection;
        struct
        {
            Cache<Update<Position>> position;
            Cache<Update<Velocity>> velocity;
            Cache<Update<Heading>> heading;            
        } cached;
    };

    virtual ~Provider() = default;

    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;

    virtual const Controller::Ptr& state_controller() const;

    virtual ChannelConnection subscribe_to_position_updates(std::function<void(const Update<Position>&)> f);
    virtual ChannelConnection subscribe_to_heading_updates(std::function<void(const Update<Heading>&)> f);
    virtual ChannelConnection subscribe_to_velocity_updates(std::function<void(const Update<Velocity>&)> f);

    virtual bool supports(const Features& f) const;
    virtual bool requires(const Requirements& r) const;

    virtual bool matches_criteria(const Criteria&);
    
protected:
    explicit Provider(
        const Features& features = Features::none,
        const Requirements& requirements = Requirements::none);

    void deliver_position_updates(const Update<Position>& update);
    void deliver_heading_updates(const Update<Heading>& update);
    void deliver_velocity_updates(const Update<Velocity>& update);
    
    virtual void start_position_updates();
    virtual void stop_position_updates();

    virtual void start_heading_updates();
    virtual void stop_heading_updates();

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates();

private:
    Features features;
    Requirements requirements;
    Channel<Update<Position>> position_updates_channel;
    Channel<Update<Heading>> heading_updates_channel;
    Channel<Update<Velocity>> velocity_updates_channel;
    Controller::Ptr controller;
};

Provider::Features operator|(Provider::Features lhs, Provider::Features rhs);
Provider::Features operator&(Provider::Features lhs, Provider::Features rhs);

Provider::Requirements operator|(Provider::Requirements lhs, Provider::Requirements rhs);
Provider::Requirements operator&(Provider::Requirements lhs, Provider::Requirements rhs);
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_

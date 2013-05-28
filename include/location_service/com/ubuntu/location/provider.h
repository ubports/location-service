#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"

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

    enum class Feature : std::size_t
    {
        position,
        velocity,
        heading
    };

    typedef std::bitset<3> FeatureFlags;

    enum class Requirement : std::size_t
    {
        satellites,
        cell_network,
        data_network,
        monetary_spending
    };

    typedef std::bitset<4> RequirementFlags;

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

    virtual bool supports(const Feature& f) const;
    virtual bool requires(const Requirement& r) const;

    virtual bool matches_criteria(const Criteria&);
    
protected:
    explicit Provider(
        const FeatureFlags& feature_flags = FeatureFlags(),
        const RequirementFlags& requirement_flags = RequirementFlags());

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
    FeatureFlags feature_flags;
    RequirementFlags requirement_flags;
    Channel<Update<Position>> position_updates_channel;
    Channel<Update<Heading>> heading_updates_channel;
    Channel<Update<Velocity>> velocity_updates_channel;
    Controller::Ptr controller;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_

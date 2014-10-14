/*
 * Copyright © 2014 Canonical Ltd.
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
#ifndef MOCK_EVENT_CONSUMER_H_
#define MOCK_EVENT_CONSUMER_H_

#include <gmock/gmock.h>

#include <condition_variable>
#include <mutex>

class MockEventConsumer
{
 public:
    MockEventConsumer()
    {
        using namespace ::testing;

        ON_CALL(*this, on_new_position(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_position_update_arrived));
        ON_CALL(*this, on_new_heading(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_heading_update_arrived));
        ON_CALL(*this, on_new_velocity(_))
                .WillByDefault(
                    InvokeWithoutArgs(
                        this,
                        &MockEventConsumer::notify_velocity_update_arrived));
    }

    bool wait_for_position_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{position.guard};
        return position.wait_condition.wait_for(ul, timeout, [this] { return position.update_arrived; });
    }

    bool wait_for_heading_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{heading.guard};
        return heading.wait_condition.wait_for(ul, timeout, [this] { return heading.update_arrived; });
    }

    bool wait_for_velocity_update_for(const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul{velocity.guard};
        return velocity.wait_condition.wait_for(ul, timeout, [this] { return velocity.update_arrived; });
    }

    MOCK_METHOD1(on_new_position, void(const com::ubuntu::location::Update<com::ubuntu::location::Position>&));
    MOCK_METHOD1(on_new_heading, void(const com::ubuntu::location::Update<com::ubuntu::location::Heading>&));
    MOCK_METHOD1(on_new_velocity, void(const com::ubuntu::location::Update<com::ubuntu::location::Velocity>&));

private:
    // Notes down the arrival of a position update
    // and notifies any waiting threads about the event.
    void notify_position_update_arrived()
    {
        position.update_arrived = true;
        position.wait_condition.notify_all();
    }

    // Notes down the arrival of a heading update
    // and notifies any waiting threads about the event.
    void notify_heading_update_arrived()
    {
        heading.update_arrived = true;
        heading.wait_condition.notify_all();
    }

    // Notes down the arrival of a heading update
    // and notifies any waiting threads about the event.
    void notify_velocity_update_arrived()
    {
        velocity.update_arrived = true;
        velocity.wait_condition.notify_all();
    }

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } position;

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } heading;

    struct
    {
        std::mutex guard;
        std::condition_variable wait_condition;
        bool update_arrived{false};
    } velocity;
};

#endif // MOCK_EVENT_CONSUMER_H_

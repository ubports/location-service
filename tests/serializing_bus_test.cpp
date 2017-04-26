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

#include <location/glib/runtime.h>
#include <location/glib/serializing_bus.h>

#include <gmock/gmock.h>

namespace
{
struct IndexedEvent : public location::Event
{
    IndexedEvent(std::size_t idx) : idx{idx}
    {
    }

    Event::Type type() const override
    {
        return Event::Type{Event::Type::first_user_defined_type};
    }

    std::size_t idx;
};

struct MockEventReceiver : public location::Event::Receiver
{
    MOCK_METHOD1(on_new_event, void(const location::Event&));
};
}

TEST(SerializingBus, ctor_and_dtor_work_for_valid_runtime)
{
    auto sb = location::glib::SerializingBus::create();
}

TEST(SerializingBus, dispatches_events_serially)
{
    using namespace ::testing;

    location::glib::Runtime runtime{location::glib::Runtime::WithOwnMainLoop{}};

    auto sb = location::glib::SerializingBus::create();
    auto receiver = std::make_shared<NiceMock<MockEventReceiver>>();

    auto verifier = [](const location::Event& e)
    {
        const IndexedEvent& indexed_event = dynamic_cast<const IndexedEvent&>(e);
        if (indexed_event.idx == 100000)
            location::glib::Runtime::instance()->stop();
    };

    ON_CALL(*receiver, on_new_event(_)).WillByDefault(Invoke(verifier));

    sb->subscribe(receiver);

    for (std::size_t idx = 1; idx <= 100000; idx++)
        sb->dispatch(std::make_shared<IndexedEvent>(idx));

    runtime.run();
}

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

#include <com/ubuntu/location/dispatching_provider.h>

#include <com/ubuntu/location/logging.h>

#include <future>

namespace location = com::ubuntu::location;

location::DispatchingProvider::DispatchingProvider(const location::DispatchingProvider::Dispatcher& dispatcher, const location::Provider::Ptr& fwd)
    : dispatcher{dispatcher},
      fwd{fwd},
      connections
      {
          fwd->updates().position.connect([this](const location::Update<location::Position>& update)
          {
              auto sp = shared_from_this();
              this->dispatcher([sp, update]()
              {
                  try
                  {
                      sp->mutable_updates().position(update);
                  } catch(const std::exception& e)
                  {
                      LOG(WARNING) << e.what();
                  } catch(...)
                  {
                  }
              });
          }),
          fwd->updates().heading.connect([this](const location::Update<location::Heading>& update)
          {
              auto sp = shared_from_this();
              this->dispatcher([sp, update]()
              {
                  try
                  {
                      sp->mutable_updates().heading(update);
                  } catch(const std::exception& e)
                  {
                      LOG(WARNING) << e.what();
                  } catch(...)
                  {
                  }
              });
          }),
          fwd->updates().velocity.connect([this](const location::Update<location::Velocity>& update)
          {
              auto sp = shared_from_this();
              this->dispatcher([sp, update]()
              {
                  try
                  {
                     sp->mutable_updates().velocity(update);
                  } catch(const std::exception& e)
                  {
                      LOG(WARNING) << e.what();
                  } catch(...)
                  {
                  }
              });
          })
      }
{
    if (not dispatcher) throw std::logic_error
    {
        "com::ubuntu::location::DispatchingProvider: Cannot operate without valid dispatcher"
    };

    if (not fwd) throw std::logic_error
    {
        "com::ubuntu::location::DispatchingProvider: Cannot operate without valid Provider"
    };
}

location::DispatchingProvider::~DispatchingProvider()
{
}

bool location::DispatchingProvider::supports(const location::Provider::Features& f) const
{
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    dispatcher([&]()
    {
        try
        {
            promise.set_value(fwd->supports(f));
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
            promise.set_exception(std::current_exception());
        } catch(...)
        {
            promise.set_exception(std::current_exception());
        }
    });

    return future.get();
}

bool location::DispatchingProvider::requires(const location::Provider::Requirements& r) const
{
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    dispatcher([&]()
    {
        try
        {
            promise.set_value(fwd->requires(r));
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
            promise.set_exception(std::current_exception());
        } catch(...)
        {
            promise.set_exception(std::current_exception());
        }
    });

    return future.get();
}

bool location::DispatchingProvider::matches_criteria(const location::Criteria& criteria)
{
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    dispatcher([&]()
    {
        try
        {
            promise.set_value(fwd->matches_criteria(criteria));
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
            promise.set_exception(std::current_exception());
        } catch(...)
        {
            promise.set_exception(std::current_exception());
        }
    });

    return future.get();
}

// We forward all events to the other providers.
void location::DispatchingProvider::on_wifi_and_cell_reporting_state_changed(location::WifiAndCellIdReportingState state)
{
    auto sp = shared_from_this();
    dispatcher([sp, state]()
    {
        try
        {
            sp->fwd->on_wifi_and_cell_reporting_state_changed(state);
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::on_reference_location_updated(const location::Update<location::Position>& position)
{
    auto sp = shared_from_this();
    dispatcher([sp, position]()
    {
        try
        {
            sp->fwd->on_reference_location_updated(position);
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::on_reference_velocity_updated(const location::Update<location::Velocity>& velocity)
{
    auto sp = shared_from_this();
    dispatcher([sp, velocity]()
    {
        try
        {
            sp->fwd->on_reference_velocity_updated(velocity);
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::on_reference_heading_updated(const location::Update<location::Heading>& heading)
{
    auto sp = shared_from_this();
    dispatcher([sp, heading]()
    {
        try
        {
            sp->fwd->on_reference_heading_updated(heading);
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

// As well as the respective state change requests.
void location::DispatchingProvider::start_position_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->start_position_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::stop_position_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->stop_position_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::start_heading_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->start_heading_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::stop_heading_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->stop_heading_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::start_velocity_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->start_velocity_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

void location::DispatchingProvider::stop_velocity_updates()
{
    auto sp = shared_from_this();
    dispatcher([sp]()
    {
        try
        {
            sp->fwd->state_controller()->stop_velocity_updates();
        } catch(const std::exception& e)
        {
            LOG(WARNING) << e.what();
        } catch(...)
        {
        }
    });
}

/*
 * Copyright © 2020 UBports foundation
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
 * Authored by: Marius Gripsgard <marius@ubports.com>
 */

#pragma once

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

#include <libgpsmm.h>

#include <thread>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace gpsd
{
class Provider : public com::ubuntu::location::Provider
{
  public:
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    struct Configuration
    {
        static std::string key_host() { return "host"; }
        static std::string key_port() { return "port"; }
        std::string host;
        std::string port;

        Provider::Features features = Provider::Features::none;
        Provider::Requirements requirements = Provider::Requirements::none;
    };

    Provider(const Configuration& config);
    ~Provider() noexcept;

    virtual bool matches_criteria(const Criteria&);

    virtual void start_position_updates() override;
    virtual void stop_position_updates() override;

    virtual void start_velocity_updates() override;
    virtual void stop_velocity_updates() override;

    virtual void start_heading_updates() override;
    virtual void stop_heading_updates() override;

  private:
    void start();
    void stop();
    void loop();
    void on_data(struct gps_data_t *gpsdata);

  private:
    std::shared_ptr<gpsmm> m_gpsd;
    std::thread m_worker;
    bool m_run = false;
};
}
}
}
}
}

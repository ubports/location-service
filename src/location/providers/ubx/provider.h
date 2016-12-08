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
#ifndef LOCATION_PROVIDERS_UBX_PROVIDER_H_
#define LOCATION_PROVIDERS_UBX_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_factory.h>
#include <location/runtime.h>

#include <location/providers/ubx/_8/serial_port_receiver.h>

#include <boost/filesystem.hpp>

#include <thread>

namespace location
{
namespace providers
{
namespace ubx
{
// ubx::Provider integrates GNSS receivers relying on
// ublox chipsets with locationd.
//
// In this version, only receivers connected to a serial port
// are supported. More to this, this version only reads NMEA sentences
// and does not support the proprietary ublox protocol.
//
// Configuration parameters:
//   - device[=/dev/ttyUSB1] serial device connecting to the receiver.
class Provider : public location::Provider
{
public:
    // For integration with the Provider factory.
    static std::string class_name();
    // Instantiates a new provider instance, populating the configuration object
    // from the provided property bundle. Please see dummy::Configuration::Keys
    // for the list of known options.
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    // Creates a new provider instance talking via device to the ubx chipset.
    Provider(const boost::filesystem::path& device);
    // Cleans up all resources and stops the updates.
    ~Provider() noexcept;

    // From Provider
    void on_new_event(const Event& event) override;

    void enable() override;
    void disable() override;
    void activate() override;
    void deactivate() override;

    Requirements requirements() const override;
    bool satisfies(const Criteria& criteria) override;
    const core::Signal<Update<Position>>& position_updates() const override;
    const core::Signal<Update<units::Degrees>>& heading_updates() const override;
    const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

private:
    // Relays incoming sentences to a provider instance.
    struct Monitor : public _8::Receiver::Monitor, public boost::static_visitor<>
    {
        explicit Monitor(Provider* provider);

        // From Receiver::Monitor
        void on_new_chunk(_8::Receiver::Buffer::iterator it, _8::Receiver::Buffer::iterator itE) override;
        void on_new_nmea_sentence(const _8::nmea::Sentence& sentence) override;

        void operator()(const _8::nmea::Gga& gga) const;
        void operator()(const _8::nmea::Gsa& gsa) const;
        void operator()(const _8::nmea::Gll& gll) const;
        void operator()(const _8::nmea::Gsv& gsv) const;
        void operator()(const _8::nmea::Rmc& rmc) const;
        void operator()(const _8::nmea::Txt& txt) const;
        void operator()(const _8::nmea::Vtg& vtg) const;

        Provider* provider;
    };

    std::shared_ptr<location::Runtime> runtime;
    std::shared_ptr<Monitor> monitor;
    std::shared_ptr<_8::SerialPortReceiver> receiver;
    struct
    {
        core::Signal<Update<Position>> position;
        core::Signal<Update<units::Degrees>> heading;
        core::Signal<Update<units::MetersPerSecond>> velocity;
    } updates;
};
}
}
}

#endif // LOCATION_PROVIDERS_UBX_PROVIDER_H_

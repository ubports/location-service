/*
 * Copyright © 2012-2014 Canonical Ltd.
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
#ifndef CACHED_RADIO_CELL_H_
#define CACHED_RADIO_CELL_H_

#include <com/ubuntu/location/connectivity/radio_cell.h>

#include "ofono.h"

#include <boost/asio.hpp>

namespace detail
{

class CachedRadioCell : public com::ubuntu::location::connectivity::RadioCell
{
public:
    // To save some typing.
    typedef std::shared_ptr<CachedRadioCell> Ptr;

    // Translates ofono technologies to radio-cell types.
    static const std::map<std::string, com::ubuntu::location::connectivity::RadioCell::Type>& type_lut();

    // Creates an instance of a cached radio cell, deduced from the network registration
    // associated to the modem.
    CachedRadioCell(const org::Ofono::Manager::Modem& modem, boost::asio::io_service& io_service);

    // Frees all resources and cuts all event connections.
    ~CachedRadioCell();

    // Returns true iff the network registration is roaming.
    const core::Property<bool>& is_roaming() const;

    // Returns true iff the instance represents a valid cell.
    const core::Property<bool>& is_valid() const;

    // Emitted when the cell details change.
    const core::Signal<>& changed() const override;

    // Returns the type of the radio cell.
    com::ubuntu::location::connectivity::RadioCell::Type type() const override;

    // Returns GSM-specific details or throws std::runtime_error if this is not a GSM radiocell.
    const com::ubuntu::location::connectivity::RadioCell::Gsm& gsm() const override;

    // Returns UMTS-specific details or throws std::runtime_error if this is not a UMTS radiocell.
    const com::ubuntu::location::connectivity::RadioCell::Umts& umts() const override;

    // Returns LTE-specific details or throws std::runtime_error if this is not an LTE radiocell.
    const com::ubuntu::location::connectivity::RadioCell::Lte& lte() const override;

    // Invoked whenever a modem property changes remotely in ofono.
    void on_modem_property_changed(const std::tuple<std::string, core::dbus::types::Variant>& tuple);

    // Invoked whenever a property specific to a network registration changes remotely.
    void on_network_registration_property_changed(const std::tuple<std::string, core::dbus::types::Variant>& tuple);

private:
    // All members required for implementing the
    // cell change heuristics go here.
    struct CellChangeHeuristics
    {
        CellChangeHeuristics(boost::asio::io_service& io_service,
                             bool needed);
        // True if the heuristic is needed
        const bool needed;
        // The io_service for setting up timeouts.
        boost::asio::io_service& io_service;
        // We might experience a race on construction, if a change
        // of a cell attribute arrives prior to setting up the timeout.
        std::mutex guard;
        // Our timer for invalidating cells.
        boost::asio::deadline_timer invalidation_timer;
        // Property to indicate whether the current cell is
        // still valid according to the cell change heuristics.
        core::Property<bool> valid;
    } cell_change_heuristics;

    // Executes the cell change heuristics if precondition is met.
    void execute_cell_change_heuristics_if_appropriate();

    core::Property<bool> roaming;
    core::Signal<> on_changed;
    Type radio_type;
    org::Ofono::Manager::Modem modem;

    // Encapsulates all event connections that have to be cut on destruction.
    struct
    {
        core::dbus::Signal
        <
            org::Ofono::Manager::Modem::PropertyChanged,
            org::Ofono::Manager::Modem::PropertyChanged::ArgumentType
        >::SubscriptionToken modem_properties_changed;

        core::dbus::Signal
        <
            org::Ofono::Manager::Modem::NetworkRegistration::PropertyChanged,
            org::Ofono::Manager::Modem::NetworkRegistration::PropertyChanged::ArgumentType
        >::SubscriptionToken network_registration_properties_changed;
    } connections;

    // Marks the unset type in a variant.
    struct None {};

    // Our custom variant handling the different known cell identity details.
    union Detail
    {
        // Constructs an empty instance
        Detail();
        // Constructs an instance holding details about a gsm cell.
        Detail(const com::ubuntu::location::connectivity::RadioCell::Gsm& gsm);
        // Constructs an instance holding details about a umtscell.
        Detail(const com::ubuntu::location::connectivity::RadioCell::Umts& umts);
        // Constructs an instance holding details about an lte cell.
        Detail(const com::ubuntu::location::connectivity::RadioCell::Lte& lte);

        // Our union members.
        None none;
        Gsm gsm;
        Umts umts;
        Lte lte;
    } detail;
};
}

#endif // CACHED_RADIO_CELL_H_

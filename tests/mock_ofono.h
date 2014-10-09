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
#ifndef MOCK_OFONO_H_
#define MOCK_OFONO_H_

#include <com/ubuntu/location/connectivity/ofono.h>

#include <core/dbus/types/variant.h>

#include <gmock/gmock.h>

namespace mock
{
struct Ofono
{
    struct Modem
    {
        struct NetworkRegistration
        {
            NetworkRegistration(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object)
                : bus{bus}, object{object}
            {
                // Saves us some typing.
                using namespace ::testing;

                object->install_method_handler<org::Ofono::Manager::Modem::NetworkRegistration::GetProperties>([this](const core::dbus::Message::Ptr& msg)
                {
                    auto reply = core::dbus::Message::make_method_return(msg);
                    reply->writer() << get_properties();

                    this->bus->send(reply);
                });

                ON_CALL(*this, get_properties()).WillByDefault(Invoke(this, &NetworkRegistration::get_properties_impl));

                ON_CALL(*this, status()).WillByDefault(Return(org::Ofono::Manager::Modem::NetworkRegistration::Status::unknown));
                ON_CALL(*this, technology()).WillByDefault(Return(org::Ofono::Manager::Modem::NetworkRegistration::Technology::gsm()));
                ON_CALL(*this, location_area_code()).WillByDefault(Return(0));
                ON_CALL(*this, cell_id()).WillByDefault(Return(0));
                ON_CALL(*this, mcc()).WillByDefault(Return("0"));
                ON_CALL(*this, mnc()).WillByDefault(Return("0"));
            }

            virtual org::Ofono::Manager::Modem::NetworkRegistration::GetProperties::ValueType get_properties_impl()
            {
                org::Ofono::Manager::Modem::NetworkRegistration::GetProperties::ValueType result;

                result[org::Ofono::Manager::Modem::NetworkRegistration::Status::name()] = core::dbus::types::Variant::encode(status());
                result[org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode::name()] = core::dbus::types::Variant::encode(location_area_code());
                result[org::Ofono::Manager::Modem::NetworkRegistration::CellId::name()] = core::dbus::types::Variant::encode(cell_id());
                result[org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode::name()] = core::dbus::types::Variant::encode(mcc());
                result[org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode::name()] = core::dbus::types::Variant::encode(mnc());
                result[org::Ofono::Manager::Modem::NetworkRegistration::Technology::name()] = core::dbus::types::Variant::encode(technology());
                result[org::Ofono::Manager::Modem::NetworkRegistration::Strength::name()] = core::dbus::types::Variant::encode(strength());

                return result;
            }

            MOCK_METHOD0(get_properties, org::Ofono::Manager::Modem::NetworkRegistration::GetProperties::ValueType());

            MOCK_METHOD0(status, org::Ofono::Manager::Modem::NetworkRegistration::Status::ValueType());
            MOCK_METHOD0(location_area_code, org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode::ValueType());
            MOCK_METHOD0(cell_id, org::Ofono::Manager::Modem::NetworkRegistration::CellId::ValueType());
            MOCK_METHOD0(mcc, org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode::ValueType());
            MOCK_METHOD0(mnc, org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode::ValueType());
            MOCK_METHOD0(technology, org::Ofono::Manager::Modem::NetworkRegistration::Technology::ValueType());
            MOCK_METHOD0(strength, org::Ofono::Manager::Modem::NetworkRegistration::Strength::ValueType());

            core::dbus::Bus::Ptr bus;
            core::dbus::Object::Ptr object;
        };

        // Constructs a modem without a network registration
        Modem(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object)
            : bus{bus}, object{object}
        {
            // Saves us some typing.
            using namespace ::testing;

            object->install_method_handler<org::Ofono::Manager::Modem::GetProperties>([this](const core::dbus::Message::Ptr& msg)
            {
                auto reply = core::dbus::Message::make_method_return(msg);
                reply->writer() << get_properties();

                this->bus->send(reply);
            });

            ON_CALL(*this, get_properties()).WillByDefault(Invoke(this, &Modem::get_properties_impl));

            ON_CALL(*this, is_powered()).WillByDefault(Return(true));
            ON_CALL(*this, is_online()).WillByDefault(Return(true));
            ON_CALL(*this, is_locked_down()).WillByDefault(Return(true));
            ON_CALL(*this, name()).WillByDefault(Return(__PRETTY_FUNCTION__));
            ON_CALL(*this, manufacturer()).WillByDefault(Return(__PRETTY_FUNCTION__));
            ON_CALL(*this, model()).WillByDefault(Return(__PRETTY_FUNCTION__));
            ON_CALL(*this, revision()).WillByDefault(Return(__PRETTY_FUNCTION__));
            ON_CALL(*this, serial()).WillByDefault(Return(__PRETTY_FUNCTION__));
            ON_CALL(*this, features()).WillByDefault(Return(std::vector<std::string>()));
            ON_CALL(*this, interfaces()).WillByDefault(Return(std::vector<std::string>()));
            ON_CALL(*this, type()).WillByDefault(Return(org::Ofono::Manager::Modem::Properties::Type::test));
        }

        // Constructs a modem without a network registration
        Modem(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object, const std::shared_ptr<mock::Ofono::Modem::NetworkRegistration>& net_reg)
            : Modem{bus, object}
        {
            // Saves us some typing.
            using namespace ::testing;

            network_registration = net_reg;

            object->install_method_handler<org::Ofono::Manager::Modem::GetProperties>([this](const core::dbus::Message::Ptr& msg)
            {
                auto reply = core::dbus::Message::make_method_return(msg);
                reply->writer() << get_properties();

                this->bus->send(reply);
            });

            ON_CALL(*this, features()).WillByDefault(Return(std::vector<std::string>{{org::Ofono::Manager::Modem::Properties::Features::net}}));
            ON_CALL(*this, interfaces()).WillByDefault(Return(std::vector<std::string>{{org::Ofono::Manager::Modem::Properties::Interfaces::network_registration}}));
        }

        virtual ~Modem() = default;

        virtual org::Ofono::Manager::Modem::GetProperties::ValueType get_properties_impl()
        {
            org::Ofono::Manager::Modem::GetProperties::ValueType result;

            result[org::Ofono::Manager::Modem::Properties::Powered::name()] = core::dbus::types::Variant::encode(is_powered());
            result[org::Ofono::Manager::Modem::Properties::Online::name()] = core::dbus::types::Variant::encode(is_online());
            result[org::Ofono::Manager::Modem::Properties::Lockdown::name()] = core::dbus::types::Variant::encode(is_locked_down());
            result[org::Ofono::Manager::Modem::Properties::Name::name()] = core::dbus::types::Variant::encode(name());
            result[org::Ofono::Manager::Modem::Properties::Manufacturer::name()] = core::dbus::types::Variant::encode(manufacturer());
            result[org::Ofono::Manager::Modem::Properties::Model::name()] = core::dbus::types::Variant::encode(model());
            result[org::Ofono::Manager::Modem::Properties::Revision::name()] = core::dbus::types::Variant::encode(revision());
            result[org::Ofono::Manager::Modem::Properties::Serial::name()] = core::dbus::types::Variant::encode(serial());
            result[org::Ofono::Manager::Modem::Properties::Features::name()] = core::dbus::types::Variant::encode(features());
            result[org::Ofono::Manager::Modem::Properties::Interfaces::name()] = core::dbus::types::Variant::encode(interfaces());
            result[org::Ofono::Manager::Modem::Properties::Type::name()] = core::dbus::types::Variant::encode(type());

            return result;
        }

        MOCK_METHOD0(get_properties, org::Ofono::Manager::Modem::GetProperties::ValueType());

        MOCK_METHOD0(is_powered, org::Ofono::Manager::Modem::Properties::Powered::ValueType());
        MOCK_METHOD0(is_online, org::Ofono::Manager::Modem::Properties::Online::ValueType());
        MOCK_METHOD0(is_locked_down, org::Ofono::Manager::Modem::Properties::Lockdown::ValueType());
        MOCK_METHOD0(name, org::Ofono::Manager::Modem::Properties::Name::ValueType());
        MOCK_METHOD0(manufacturer, org::Ofono::Manager::Modem::Properties::Manufacturer::ValueType());
        MOCK_METHOD0(model, org::Ofono::Manager::Modem::Properties::Model::ValueType());
        MOCK_METHOD0(revision, org::Ofono::Manager::Modem::Properties::Revision::ValueType());
        MOCK_METHOD0(serial, org::Ofono::Manager::Modem::Properties::Serial::ValueType());
        MOCK_METHOD0(features, org::Ofono::Manager::Modem::Properties::Features::ValueType());
        MOCK_METHOD0(interfaces, org::Ofono::Manager::Modem::Properties::Interfaces::ValueType());
        MOCK_METHOD0(type, org::Ofono::Manager::Modem::Properties::Type::ValueType());
        // We are adding implementing the modem interface on this object.
        core::dbus::Bus::Ptr bus;
        core::dbus::Object::Ptr object;

        // A modem might have a network registration.
        std::shared_ptr<NetworkRegistration> network_registration;
    };

    struct Manager
    {
        Manager(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object)
            : bus{bus}, manager{object}
        {
            using namespace ::testing;

            ON_CALL(*this, get_modems()).WillByDefault(Return(std::vector<core::dbus::types::ObjectPath>{}));

            manager->install_method_handler<org::Ofono::Manager::GetModems>([this](const core::dbus::Message::Ptr& msg)
            {
                auto reply = core::dbus::Message::make_method_return(msg);
                reply->writer() << get_modems();
                this->bus->send(reply);
            });
        }

        MOCK_METHOD0(get_modems, std::vector<core::dbus::types::ObjectPath>());

        core::dbus::Bus::Ptr bus;
        core::dbus::Object::Ptr manager;
    };
};
}
#endif // MOCK_OFONO_H_

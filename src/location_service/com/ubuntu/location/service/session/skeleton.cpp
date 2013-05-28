#include "com/ubuntu/location/service/session/skeleton.h"

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/skeleton.h>

#include <functional>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

culss::Skeleton::Skeleton(
    const dbus::Bus::Ptr& bus, 
    const dbus::types::ObjectPath& session_path)
        : dbus::Skeleton<Interface>{bus},
          session_path{session_path},
          object{access_service()->add_object_for_path(session_path)}
{
    object->install_method_handler<Interface::StartPositionUpdates>(std::bind(&Skeleton::handle_start_position_updates, this, std::placeholders::_1));
    object->install_method_handler<Interface::StopPositionUpdates>(std::bind(&Skeleton::handle_stop_position_updates, this, std::placeholders::_1));
    object->install_method_handler<Interface::StartVelocityUpdates>(std::bind(&Skeleton::handle_start_velocity_updates, this, std::placeholders::_1));
    object->install_method_handler<Interface::StopVelocityUpdates>(std::bind(&Skeleton::handle_stop_velocity_updates, this, std::placeholders::_1));
    object->install_method_handler<Interface::StartHeadingUpdates>(std::bind(&Skeleton::handle_start_heading_updates, this, std::placeholders::_1));
    object->install_method_handler<Interface::StopHeadingUpdates>(std::bind(&Skeleton::handle_stop_heading_updates, this, std::placeholders::_1));
}

culss::Skeleton::~Skeleton() noexcept
{
    object->uninstall_method_handler<Interface::StartPositionUpdates>();
    object->uninstall_method_handler<Interface::StopPositionUpdates>();
    object->uninstall_method_handler<Interface::StartVelocityUpdates>();
    object->uninstall_method_handler<Interface::StopVelocityUpdates>();
    object->uninstall_method_handler<Interface::StartHeadingUpdates>();
    object->uninstall_method_handler<Interface::StopHeadingUpdates>();
}

const dbus::types::ObjectPath& culss::Skeleton::path() const
{
    return session_path;
}

void culss::Skeleton::handle_start_position_updates(DBusMessage* msg)
{
    try
    {
        start_position_updates();
        auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
        access_bus()->send(reply->get());
    } catch(const std::runtime_error& e)
    {
        auto error = org::freedesktop::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
        access_bus()->send(error->get());
    }
}

void culss::Skeleton::handle_stop_position_updates(DBusMessage* msg)
{
    stop_position_updates();
    auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
    access_bus()->send(reply->get());
}

void culss::Skeleton::handle_start_velocity_updates(DBusMessage* msg)
{
    try
    {
        start_velocity_updates();
        auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
        access_bus()->send(reply->get());
    } catch(const std::runtime_error& e)
    {
        auto error = org::freedesktop::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
        access_bus()->send(error->get());
    }
}

void culss::Skeleton::handle_stop_velocity_updates(DBusMessage* msg)
{
    stop_velocity_updates();
    auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
    access_bus()->send(reply->get());
}

void culss::Skeleton::handle_start_heading_updates(DBusMessage* msg)
{
    try
    {
        start_heading_updates();
        auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
        access_bus()->send(reply->get());
    } catch(const std::runtime_error& e)
    {
        auto error = org::freedesktop::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
        access_bus()->send(error->get());
    }
}

void culss::Skeleton::handle_stop_heading_updates(DBusMessage* msg)
{
    stop_heading_updates();
    auto reply = org::freedesktop::dbus::Message::make_method_return(msg);
    access_bus()->send(reply->get());
}
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
#include "com/ubuntu/location/service/skeleton.h"

#include "com/ubuntu/location/logging.h"

#include <core/dbus/dbus.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/types/object_path.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

namespace
{

template<typename SessionType>
struct SessionStore
{
    typedef std::shared_ptr<SessionStore> Ptr;

    SessionStore() = default;
    SessionStore(const SessionStore&) = delete;
    SessionStore& operator=(const SessionStore&) = delete;
    virtual ~SessionStore() = default;

    virtual void remove_session(const std::shared_ptr<SessionType>& session) = 0;
};

struct SessionWrapper : public std::enable_shared_from_this<SessionWrapper>
{
    typedef std::shared_ptr<SessionWrapper> Ptr;

    SessionWrapper(const SessionStore<SessionWrapper>::Ptr& session_store,
		   const culss::Interface::Ptr& session,
		   core::dbus::Service::Ptr service,
		   core::dbus::Object::Ptr object)
	    : session_store(session_store),
	      session{session},
	      remote(service, object)
    {
	session->install_position_updates_handler(std::bind(&SessionWrapper::on_position_update, this, std::placeholders::_1));
	session->install_velocity_updates_handler(std::bind(&SessionWrapper::on_velocity_update, this, std::placeholders::_1));
	session->install_heading_updates_handler(std::bind(&SessionWrapper::on_heading_update, this, std::placeholders::_1));
    }

    const core::dbus::types::ObjectPath& path() const
    {
	return session->path();
    }

    void on_session_died() noexcept
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
	VLOG(1) << "Session died, removing from store and stopping all updates.";

	auto thiz = shared_from_this();
	try
	{
	    session_store->remove_session(thiz);
	    session->stop_position_updates();
	    session->stop_heading_updates();
	    session->stop_velocity_updates();
	} catch(const std::runtime_error& e)
	{
	    LOG(ERROR) << "Error while stopping updates for died session: " << e.what();
	}
    }

    void on_position_update(const cul::Update<cul::Position>& position)
    {
	try
	{
	    remote.session->invoke_method_synchronously<culs::session::Interface::UpdatePosition, void>(position);
	} catch(const std::runtime_error& e)
	{
	    // We consider the session to be dead once we hit an exception here.
	    // We thus remove it from the central and end its lifetime.
        //on_session_died();
	}
    }

    void on_velocity_update(const cul::Update<cul::Velocity>& velocity)
    {
	try
	{
	    remote.session->invoke_method_synchronously<culs::session::Interface::UpdateVelocity, void>(velocity);
	} catch(const std::runtime_error& e)
	{
	    // We consider the session to be dead once we hit an exception here.
	    // We thus remove it from the central and end its lifetime.
        //on_session_died();
	}
    }

    void on_heading_update(const cul::Update<cul::Heading>& heading)
    {
	try
	{
	    remote.session->invoke_method_synchronously<culs::session::Interface::UpdateHeading, void>(heading);
	} catch(const std::runtime_error& e)
	{
	    // We consider the session to be dead once we hit an exception here.
	    // We thus remove it from the central and end its lifetime.
        //on_session_died();
	}
    }

    SessionStore<SessionWrapper>::Ptr session_store;
    culs::session::Interface::Ptr session;
    struct Remote
    {
	explicit Remote(const dbus::Service::Ptr& service,
			const dbus::Object::Ptr& session)
		: service(service),
		  session(session)
	{
	}
	
	dbus::Service::Ptr service;
	dbus::Object::Ptr session;
    } remote;
};
}

struct culs::Skeleton::Private : public SessionStore<SessionWrapper>, std::enable_shared_from_this<culs::Skeleton::Private>
{
    Private(Skeleton* parent, const dbus::Bus::Ptr& connection, const culs::PermissionManager::Ptr& permission_manager)
	: parent(parent),
	  permission_manager(permission_manager),
	  daemon(connection),
	  object(parent->access_service()->add_object_for_path(culs::Interface::path()))
    {
	object->install_method_handler<culs::Interface::CreateSessionForCriteria>(
	    std::bind(&culs::Skeleton::Private::handle_create_session_for_criteria, this, std::placeholders::_1));
    }

    ~Private() noexcept {}

    void handle_create_session_for_criteria(const core::dbus::Message::Ptr& in);
    void remove_session(const std::shared_ptr<SessionWrapper>& session);

    Skeleton* parent;
    PermissionManager::Ptr permission_manager;
    dbus::DBus daemon;
    dbus::Object::Ptr object;
    std::mutex guard;
    std::map<dbus::types::ObjectPath, std::shared_ptr<SessionWrapper>> session_store;
};

culs::Skeleton::Skeleton(const dbus::Bus::Ptr& connection, const culs::PermissionManager::Ptr& permission_manager)
	: dbus::Skeleton<culs::Interface>(connection), d{new Private{this, connection, permission_manager}}
{
}

culs::Skeleton::~Skeleton() noexcept
{
}

void culs::Skeleton::Private::handle_create_session_for_criteria(const core::dbus::Message::Ptr& in)
{
    auto sender = in->sender();

    try
    {
	Criteria criteria;
	in->reader() >> criteria;

	Credentials credentials
	{
	    static_cast<pid_t>(daemon.get_connection_unix_process_id(sender)),
	    static_cast<uid_t>(daemon.get_connection_unix_user(sender))
	};

	if (PermissionManager::Result::rejected == permission_manager->check_permission_for_credentials(criteria, credentials))
	    throw std::runtime_error("Client lacks permissions to access the service with the given criteria");

	auto session = parent->create_session_for_criteria(criteria);

    auto service = dbus::Service::use_service(parent->access_bus(), in->sender());
    auto object = service->object_for_path(session->path());

    {
        std::lock_guard<std::mutex> lg(guard);

        auto wrapper = SessionWrapper::Ptr{new SessionWrapper{shared_from_this(), session, service, object}};

        bool inserted = false;
        std::tie(std::ignore, inserted) = session_store.insert(std::make_pair(session->path(), wrapper));

        auto reply = dbus::Message::make_method_return(in);
        reply->writer() << session->path();
        parent->access_bus()->send(reply);

        if (!inserted)
            throw std::runtime_error("Could not insert duplicate session into store.");
    }

    } catch(const std::runtime_error& e)
    {
    parent->access_bus()->send(
        dbus::Message::make_error(
            in,
            culs::Interface::Errors::CreatingSession::name(),
            e.what()));

    LOG(ERROR) << "Error creating session: " << e.what();
    }
}

void culs::Skeleton::Private::remove_session(const SessionWrapper::Ptr& session)
{
    std::lock_guard<std::mutex> lg(guard);
    session_store.erase(session->path());

    VLOG(1) << "# of session in session store: " << session_store.size() << std::endl;
}

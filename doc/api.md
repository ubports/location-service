# API

Locationd offers multiple different ways of interfacing with the
service. The primary protocol is using DBus. We offer a plain C++ API,
too, that abstracts away the protocol details.

All APIs we expose are guaranteed to remain ABI stable within any
given major release.

## DBus

The DBus introspection files are available in
[`${SOURCE}/data/location/dbus`](http://bazaar.launchpad.net/~thomas-voss/location-service/next/files/head:/data/) ready
for consumption by static and dynamic binding generators. Methods,
properties and signals are documented within the respective
introspection files.

## C++

A client application then uses the API to establish a session with a
service, register observers to receive updates and to control the
status of updates. The following snippet illustrates basic usage of
the client API:

```{cpp}
#include <location/service.h>
#include <location/session.h>

#include <location/glib/context.h>

auto context = location::glib::Context::create_for_system_bus();

context->connect_to_service([context](const location::Service::Ptr& service)
{
	service->create_session_for_criteria(location::Criteria{}, [context, service](const location::Session::Ptr& session)
	{
		session->updates().position.changed().connect([this](const location::Update<location::Position>& pos)
		{
			std::cout << pos << std::endl;
		});

	    session->updates().heading.changed().connect([this](const location::Update<location::units::Degrees>& heading)
	    {
		    std::cout << pos << std::endl;
	    });

        session->updates().velocity.changed().connect([this](const location::Update<location::units::MetersPerSecond>& velocity)
	    {
		    std::cout << pos << std::endl;
	    });

	    session->updates().position_status = location::Service::Session::Updates::Status::enabled;
	    session->updates().heading_status = location::Service::Session::Updates::Status::enabled;
	    session->updates().velocity_status = location::Service::Session::Updates::Status::enabled;
	});
});

```

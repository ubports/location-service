# API

Location service exposes a DBus API to interact with a service instance.
We do not expose introspection for the API, yet. Instead, we provide a C++ client API that 
abstracts away from the underlying IPC mechanism.

A client application then uses the API to establish a session with a
service, register observers to receive updates and to control the
status of updates. The following snippet illustrates basic usage of the client API:

```{cpp}
auto service = location::connect_to_service(...);
auto session = service->create_session_for_criteria(location::Criteria{});

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
```

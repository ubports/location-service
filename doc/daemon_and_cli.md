# Service Daemon and CLI

The location service offers a daemon executable and a corresponding
command-line interface for interacting with it. The daemon does not
necessarily require root privileges, but might so depending on your
configuration.

Run the following command to receive an overview of the arguments to
the daemon:

    ubuntu-location-serviced --help

An example invocation of the daemon, configuring a GPS provider that
relies on the Android HAL to talk to the chipset, exposing the service
on the system DBus instance:

    ubuntu-location-serviced --bus system --provider gps::Provider

The cli allows for querying properties of a running service instance, e.g.:

    ubuntu-location-serviced-cli --bus system --get --property is_online

## Configuring an Out-Of-Process Provider

If you want to run a provider out of process, the daemon executable
allows you to do so by instantiating a so-called remote provider. The
following invocation of the service tries to connect to the provider
instance described by the given unique DBus name and path.

    ubuntu-location-serviced \
	  --bus system \
	  --provider remote::Provider \
          --remote::Provider::bus=system \
	  --remote::Provider::name=com.ubuntu.location.provider.Gps \
	  --remote::Provider::path=/

Please note that the service allows for decorating provider names to
uniquely identify per provider configuration options and to allow for
loading more than one provider of a certain kind. The following
configuration configures two remote providers, one relying on GPS
(decorated with @gps) and another one relying on network-based
positioning (decorated with @network):

    ubuntu-location-serviced \
	  --bus system \
	  --provider remote::Provider@gps \
          --remote::Provider@gps::bus=system \
	  --remote::Provider@gps::name=com.ubuntu.location.provider.Gps \
	  --remote::Provider@gps::path=/ \
	  --provider remote::Provider@network \
          --remote::Provider@network::bus=system \
	  --remote::Provider@network::name=com.ubuntu.location.provider.Network \
	  --remote::Provider@network::path=/

    

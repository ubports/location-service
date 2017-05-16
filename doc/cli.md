# CLI

locationd offers a command-line interface for controlling
and monitoring the service. The following commands are available:

 - `list`: Lists all provider implementations known to the service.
 - `monitor`: Connects to a locationd instance, monitoring its activity. Supports KML or tabular data output of position/heading/velocity data.
 - `provider`: Executes a known provider implementation in an out-of-process sandbox.
 - `run`: Executes the service.
 - `set`: Persist the given key/value pair.
 - `status`: Queries the status of a service instance.
 - `test`: Executes standalone runtime tests against individual provider implementations.

For all of the commands, an exit status of 0 indicates success. An exit status of 1 indicates
an error. Normal output goes to stdout, while all errors/warnings are output to stderr.

## Snap-Specific Command Names

If you are using the cli from a snap (`snap install locationd --channel edge`), the commands will 
be wrapped up for you in a convenient way, following the pattern locationd.$COMMAND. With that, if 
you want to check on the status of the service, simply run:
~~~bash
$ locationd.status
~~~

## Hotplug Support

locationd itself does not interface with udev directly but instead delegates to the host
system's udev setup and the user/administrator to define reasonable udev rules.

## Testing Scenarios

For testing purposes, it is often handy to inspect position/velocity/heading estimates on the command line.
The `monitor` command helps here. It connects to the service, starts the positioning engine and outputs 
position estimates to stdout until it receives a SIGTERM.

~~~bash
$ locationd.monitor
I0516 08:36:56.752629  8124 monitor.cpp:226] Enabled position/heading/velocity updates...
51.44483       7.21064        13.27          n/a            n/a            n/a            n/a
51.44483       7.21064        13.26          n/a            n/a            n/a            n/a
51.44483       7.21069        16.36          n/a            n/a            n/a            n/a
51.44483       7.21069        16.36          n/a            n/a            n/a            n/a
51.44483       7.21068        15.94          n/a            n/a            n/a            n/a
51.44483       7.21064        12.66          n/a            n/a            n/a            n/a
51.44484       7.21063        12.26          n/a            n/a            n/a            n/a
51.44484       7.21063        12.26          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
51.44485       7.21058        10.08          n/a            n/a            n/a            n/a
51.44485       7.21058        10.08          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
51.44485       7.21059        10.00          n/a            n/a            n/a            n/a
~~~

## Standalone Runtime Tests

Sometimes it is convenient to be able to test specific provider
implementations in isolation, i.e., without the respective provider
running in the context of locationd. To this end, the `test` command
is available.  Right now, two test suites `sirf` and `ubx` are
available. Please make sure that the underlying serial devices are not
in use by any other process before executing the test suite. 
The behavior of the test-suites can be adjusted by the following environment variables:

 * `ubx`:
   * `UBX_PROVIDER_TEST_DEVICE`: Mandatory, path to the serial device connecting to the uBlox receiver.
   * `UBX_PROVIDER_TEST_TRIALS`: Defaults to 15, number of independent positioning attempts from cold start.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_ENABLE`: Defaults to `false`, toggles usage of uBlox AssistNow Online for assisted GNSS.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_TOKEN`: Defaults to the empty string, token used to verify access to the uBlox AssistNow services.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_ACQUISITION_TIMEOUT`: Defaults to `5` seconds, the provider waits this long for an initial fix before reaching out for assistance data.
 * `sirf`:
   * `SIRF_PROVIDER_TEST_DEVICE`: Mandatory, path to the serial device connecting to the SiRF receiver.
   * `SIRF_PROVIDER_TEST_TRIALS`: Defaults to 15, number of independent positioning attempts from cold start.

An example invocation for testing the uBlox provider would look like:

~~~bash
$ sudo UBX_PROVIDER_TEST_DEVICE=/dev/ttyACM1 UBX_PROVIDER_TEST_TRIALS=50 locationd.test --test-suite=ubx
~~~

## Verbose Logging

locationd and all of its commands can be switched to verbose mode by
setting the environment variable `GLOG_v` to `1`, e.g.:

~~~bash
$ sudo GLOG_v=1 locationd.provide --id=mls::Provider
I0516 08:40:15.010741  8200 service.cpp:190] static void location::dbus::stub::Service::on_bus_acquired(GObject*, GAsyncResult*, gpointer)
I0516 08:40:15.015348  8200 service.cpp:221] static void location::dbus::stub::Service::on_name_appeared_for_creation(GDBusConnection*, const gchar*, const gchar*, gpointer)
I0516 08:40:15.022554  8200 service.cpp:165] static void location::dbus::stub::Service::on_proxy_ready(GObject*, GAsyncResult*, gpointer)
I0516 08:40:15.029153  8200 w11t_manager.cpp:452] static void location::connectivity::w11t::Supplicant::on_bus_ready(GObject*, GAsyncResult*, gpointer)
I0516 08:40:15.033778  8200 service.cpp:244] static void location::dbus::stub::Service::on_provider_added(GObject*, GAsyncResult*, gpointer)
I0516 08:40:15.034494  8200 w11t_manager.cpp:477] static void location::connectivity::w11t::Supplicant::on_proxy_ready(GObject*, GAsyncResult*, gpointer)
I0516 08:40:15.036355  8200 w11t_manager.cpp:285] static void location::connectivity::w11t::Interface::on_proxy_ready(GObject*, GAsyncResult*, gpointer)
I0516 08:40:21.915056  8200 provider.cpp:467] static bool location::providers::remote::Provider::Skeleton::handle_activate(ComUbuntuLocationServiceProvider*, GDBusMethodInvocation*, gpointer)
I0516 08:40:26.832139  8200 w11t_manager.cpp:310] static void location::connectivity::w11t::Interface::handle_scan_done(FiW1Wpasupplicant1WirelessInterface*, gboolean, gpointer)
I0516 08:40:26.832207  8200 provider.cpp:91] Wireless network scan finished.
I0516 08:40:33.293536  8200 w11t_manager.cpp:325] static void location::connectivity::w11t::Interface::handle_bss_added(FiW1Wpasupplicant1WirelessInterface*, const char*, GVariant*, gpointer)
I0516 08:40:33.294180  8200 w11t_manager.cpp:310] static void location::connectivity::w11t::Interface::handle_scan_done(FiW1Wpasupplicant1WirelessInterface*, gboolean, gpointer)
I0516 08:40:33.294219  8200 provider.cpp:91] Wireless network scan finished.
I0516 08:40:33.297814  8200 w11t_manager.cpp:98] static void location::connectivity::w11t::BSS::on_proxy_ready(GObject*, GAsyncResult*, gpointer)
I0516 08:40:43.278961  8200 w11t_manager.cpp:354] static void location::connectivity::w11t::Interface::handle_bss_removed(FiW1Wpasupplicant1WirelessInterface*, const char*, gpointer)
I0516 08:40:43.279033  8200 w11t_manager.cpp:310] static void location::connectivity::w11t::Interface::handle_scan_done(FiW1Wpasupplicant1WirelessInterface*, gboolean, gpointer)
I0516 08:40:43.279059  8200 provider.cpp:91] Wireless network scan finished.
I0516 08:40:43.284925  8200 w11t_manager.cpp:169] static void location::connectivity::w11t::BSS::on_age_changed(GObject*, GParamSpec*, gpointer)
~~~

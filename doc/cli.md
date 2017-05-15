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

## Standalone Runtime Tests

Sometimes it is convenient to be able to test specific provider
implementations in isolation, i.e., without the respective provider
running in the context of locationd. To this end, the `test` command
is available.  Right now, two test suites `sirf` and `ubx` are
available. Please make sure that the underlying serial devices are not
in use by any other process before executing the test suite. The behavior of the test-suites can be adjusted by the following environment variables:

 * `ubx`:
   * `UBX_PROVIDER_TEST_DEVICE`: Mandatory, path to the serial device connecting to the uBlox receiver.
   * `UBX_PROVIDER_TEST_TRIALS`: Defaults to 15, number of independent positioning attempts from cold start.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_ENABLE`: Defaults to `false`, toggles usage of uBlox AssistNow Online for assisted GNSS.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_TOKEN`: Defaults to the empty string, token used to verify access to the uBlox AssistNow services.
   * `UBX_PROVIDER_TEST_ASSIST_NOW_ACQUISITION_TIMEOUT`: Defaults to `5` seconds, the provider waits this long for an initial fix before reaching out for assistance data.
 * `sirf`:
   * `SIRF_PROVIDER_TEST_DEVICE`: Mandatory, path to the serial device connecting to the SiRF receiver.
   * `SIRF_PROVIDER_TEST_TRIALS`: Defaults to 15, number of independent positioning attempts from cold start.

## Verbose Logging

locationd and all of its commands can be switched to verbose mode by
setting the environment variable `GLOG_v` to `1`.

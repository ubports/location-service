ubuntu-location-service(2) -- A location service aggregating position/velocity/heading updates and exporting them over dbus.
============================================================================================================================

## SYNOPSIS

`ubuntu-location-service` `--bus` <well known bus name> `--provider` <provider> [`--provider` <provider>] [provider specific options]

## DESCRIPTION

A location service aggregating position/velocity/heading updates and exporting them over dbus.

## OPTIONS

These options control which bus the service is exposed upon and which providers are instantiated:

  * `--bus`=<bus>:
  Specify the bus that the service should be exposed upon, valid values are "session" or "system".
  
  * `--provider`=<provider>:
  Specify the provider that should be instantiated and configured for this service instance.
  Valid values are: geoclue::Provider, gps::Provider and skyhook::Provider.
    
These options control the behavior of the geoclue provider:

  * `--geoclue::Provider::name`=<name>:
  The name of the provider on the bus.
  
  * `--geoclue::Provider::path`=<path>:
  The object path of the provider on the bus.
  
These options control the behavior of the Skyhook provider:

  * `--skyhook::Provider::username`=<username>:
  Username for authenicating with the Skyhook service.
  
  * `--skyhook::Provider::realm`=<realm>:
  Realm for authenticating with the Skyhook service.
  
  * `--skyhook::Provider::period`=<period>:
  Delay between updates in [ms].
  
These options control the logging behavior of the service:

  * `--logtostderr`=<boolean, default: false>:
  Log messages to stderr instead of logfiles, defaults to false.
  
  * `--stderrthreshold`=<threshold, default: 2>:
  Copy log messages at or above this level to stderr in addition to logfiles. The numbers of severity levels INFO, WARNING, ERROR, and FATAL are 0, 1, 2, and 3, respectively. Defa

  * `--minloglevel`=<level, default: 0>:
  Log messages at or above this level. Again, the numbers of severity levels INFO, WARNING, ERROR, and FATAL are 0, 1, 2, and 3, respectively.
  
  * `--log_dir`=<path, default: "">:
  If specified, logfiles are written into this directory instead of the default logging directory.

  * `--v`=<level, default: 0>:
  Show all VLOG(m) messages for m less or equal the value of this flag.

## EXAMPLES

Starting the service with a gps and a skyhook provider, exposing it on the system bus:
    $ /usr/bin/ubuntu-location-service/examples/service
        --bus system 
        --provider gps::Provider 
        --provider skyhook::Provider 
        --skyhook::Provider::username=ut_testing 
        --skyhook::Provider::realm=ut_testing.com

A slightly more verbose instance:
    $ /usr/bin/ubuntu-location-service/examples/service 
      --logtostderr=true
      --v=5
      --bus system
      --provider gps::Provider 
      --provider skyhook::Provider 
      --skyhook::Provider::username=ut_testing 
      --skyhook::Provider::realm=ut_testing.com

## COPYRIGHT

Ubuntu Location Service is Copyright (C) 2012 Canonical Ltd.

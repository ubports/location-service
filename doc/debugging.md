# Debugging

Location not working? Here's how to debug.

## Layers

Test in OSMTouch (QML app using Qt API) before testing in webapps or
webbrowser app. Different results? File a bug where it doesn't
work. Same result of no location? Next step.

## Check that stack works with dummy provider

Edit /etc/init/ubuntu-location-provider.override to start
location-serviced with just the dummy provider; this should
work. Doesn't work? File a bug against location-service. Works? Reset
config to defaults and try the next thing.

## Hardware GPS breaking all of location-service

GPS provider is built-in into location-service and might break all of
it if it goes south (working on splitting it out); try enabling only
the HERE provider on the location-serviced command-line and see if
that works. Works? File a bug against location-service. Doesn't work?
Move on.

## HERE test

To test whether the low-level HERE stack gets a location, put
http://people.canonical.com/~lool/espoo-cli on your phone (will be
included along HERE bits in the future) and run with:

    chmod a+x espoo-cli
    GLOG_logtostderr=1 GLOG_v=100 LD_LIBRARY_PATH=/custom/vendor/here/location-provider/lib/arm-linux-gnueabihf ./espoo-cli 5

NB: 5 is the number of location updates after which the tool exits;
updates should come in at approx 15s interval. Output looks like:

    I1101 21:30:01.285964 4403 cli.cpp:117] Requested number of updates is 2
    I1101 21:30:01.299002 4403 cli.cpp:133] Starting location updates
    I1101 21:30:01.301888 4403 cli.cpp:141] Starting GLib main loop
    I1101 21:30:11.304612 4403 cli.cpp:158] Location: tstamp=1414891811 lat=xyz long=foo hor. acc.=2569 alt=nan vert. acc.=nan tech=cell
    I1101 21:30:11.306061 4403 cli.cpp:170] Remaining updates: 1
    I1101 21:30:26.736821 4403 cli.cpp:158] Location: tstamp=1414891826 lat=xyz long=foo hor. acc.=2824 alt=nan vert. acc.=nan tech=cell
    I1101 21:30:26.738348 4403 cli.cpp:148] Stopping location updates

Low-level HERE stack works but location-serviced with just HERE
provider doesn't work? File a bug against espoo projet (HERE) and/or
location-service. Low-level HERE stack doesn't work? Move on

## location-service and espoo-service debug

Collect some debug data by editing /etc/init/ubuntu-espoo-service.conf
and /etc/init/ubuntu-location-service.override and changing the start
sequence to add some env vars:

    export GLOG_v=200
    export GLOG_logtostderr=1

before the exec. Reboot, and start some app. You should have some log
files under /var/log/upstart/ubuntu-espoo-service.log and
/var/log/upstart/ubuntu-location-service.log to attach to a bug
report; e.g. a working espoo log looks like this:

    WARNING: Logging before InitGoogleLogging() is written to STDERR
    I1105 16:30:10.221474  1620 provider.cpp:568] StartPositionUpdates
    I1105 16:30:10.224901  1620 provider.cpp:122] Successfully started position updates.
    I1105 16:30:10.228739  1620 provider.cpp:596] StartVelocityUpdates
    I1105 16:30:13.046851  1621 provider.cpp:83] Received location: Position(lat: Coordinate(12.34 deg), lon: Coordinate(12.34 deg), alt: Coordinate(nan m), hor.acc.: 1430 m, ver.acc.: nan m)

No position there? check connectivity API works by running:

    cd /tmp
    wget http://people.ubuntu.com/~lool/connectivity
    GLOG_v=200 GLOG_logtostderr=1 ./connectivity

you should see something like:

    I1105 16:47:26.431466 11140 cached_radio_cell.cpp:160] (mcc: 123, mnc: 2, lac: 1234, id: 123456, asu: 1)
    I1105 16:47:26.533818 11140 connectivity.cpp:47] Is wifi enabled: true
    I1105 16:47:26.533963 11140 connectivity.cpp:48] Is wifi hw enabled: true
    I1105 16:47:26.534010 11140 connectivity.cpp:49] Is wwan enabled: true
    I1105 16:47:26.534050 11140 connectivity.cpp:50] Is wwan hw enabled: true
    I1105 16:47:26.534442 11140 connectivity.cpp:122] umts(mcc: 123, mnc: 2, lac: 1234, id: 123456, asu: 1)
    I1105 16:47:26.534633 11140 connectivity.cpp:155] (bssid: 12:12:12:12:12:12, ssid: xyz, last seen: 1415224046, mode: Mode::infrastructure, frequency: 2442, strength: 63)
    I1105 16:47:26.534828 11140 connectivity.cpp:155] (bssid: 12:12:12:12:12:12, ssid: boing, last seen: 1415224046, mode: Mode::infrastructure, frequency: 2467, strength: 57)

Also, please attach output of /usr/share/ofono/scripts/list-modems > list-modems-output.txt
Please note that the command might take ~1 minute to complete.

TODO: document dbus-monitor / d-feet capturing of client / system traffic with snooping config.



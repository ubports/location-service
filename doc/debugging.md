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

## location-service debug

Collect some debug data by editing /etc/init/ubuntu-location-service.override
and changing the start sequence to add some env vars:

    export GLOG_v=200

before the exec. Reboot, and start some app. You should have some log
files under /var/log/upstart/ubuntu-location-service.log to attach to a bug
report; e.g. a working log looks like this:

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



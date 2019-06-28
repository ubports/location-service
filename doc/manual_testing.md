# Manual Testplan

[TOC]

While the automatic test suite of the location service is
comprehensive and covers large parts of the functionality of the
service itself, we still provide an additional level of acceptance
testing covering the entire location stack/experience as a part of
this document.

## Dependents/Clients

 - qtubuntu-sensors
 - Qt/QML applications:
   - Browser
   - osmTouch

## Test Plan

This test plan is not supposed to be complete; use it to guide your
manual testing so you don't miss big functional areas that are part of
the component; also this should be used as guideline to inspire the
exploratory testing which should be adapted smartly based on the real
content of a MP.

Please note that if you're testing the GPS provider, the location
service relies on GPS hardware to obtain a location fix. For that, it
might be required that you execute the manual steps listed before
close to a window or ideally outside, with good satellite visibility
conditions.

__Note: It can take up to 15 minutes for the GPS device to get a lock, due to lack of assisted GPS__

 - Install latest image on phone
 - Install freshly built MPs that are needed for landing

Depending on the default configuration of location-service on the
image, you may skip parts of this test plan. E.g. if GPS hardware is
disabled, skip this part. You can see which providers are enabled by
looking at the list of providers on the location-serviced command-line
(`ps fauxw | grep location-service`, then look at the `--provider`
flags).

### Dummy provider

This tests forces location-service to use only the dummy provider;
this providers a baseline test for the app to trust-store to
location-service path.

 - phablet-shell into the phone:
   - `sudo service ubuntu-location-service stop && sudo /usr/bin/ubuntu-location-serviced --bus system --provider dummy::Provider --dummy::Provider::ReferenceLocationLat=48.857503 --dummy::Provider::ReferenceLocationLon=2.295072`
 - As phablet, start the trust store again (it stops when location-service is stopped) with: `start ubuntu-location-service-trust-stored`
 - Ensure that all AP tests for the webbrowser pass as expected
 - Point the browser to maps.google.com (alternatively: here.com, maps.bing.fr).
 - Request centering the map on current position and observe if it works correctly (should show the Eiffel tower)
 - Install osmTouch from the app store
 - Launch osmTouch and check if it centers on the Eiffel tower.
 - Install a maps webapp such as HERE or Google Maps webapp from the app store
 - Launch maps webapp and check if it centers on the Eiffel tower.

### GPS Test Plan

This applies only if GPS provider is enabled.

 - (If applicable: Remember to add the silo you are testing)
 - `sudo apt-get install ubuntu-location-service-tests`
 - If you want to send off crowdsourced information, i.e., information about visible wifis and visible radio cells for the obtained location fixes to Mozilla's location service and our own instance: 
   - `sudo GLOG_v=40 GLOG_logtostderr=1 GPS_TEST_ENABLE_HARVESTING_DURING_TESTS=1  /usr/bin/uls-tests/gps_provider_test --gtest_filter=*.time_to_first_fix_cold_start_without_supl_benchmark_requires_hardware`
 - If you '''don't''' want to send off crowdsourced information:
   - `sudo GLOG_v=40 GLOG_logtostderr=1 /usr/bin/uls-tests/gps_provider_test --gtest_filter=*.time_to_first_fix_cold_start_without_supl_benchmark_requires_hardware`

 - The test will output a lot of diagnostic information to the
   terminal and will take ~30 minutes. If satellite visibility is
   limited, it can take even longer. The test will automatically
   report success or failure.

### Preliminary AGPS Test Plan

**Does not apply to Krillin**

Please note that the Krillin GPS chipset driver and its integration
within Ubuntu does not support vanilla AGPS (i.e., SUPL) right
now. For that, this test case is irrelevant for Krillin and is likely
to fail.

This applied only if GPS provider and some other provider (giving
_A_ssistance) are enabled.

 - Add the silo.
 - `sudo apt-get install ubuntu-location-service-tests`
 - Obtain a (rough) location estimate for your current location on Google maps.
 - Make sure to replace INSERT_ESTIMATE_HERE with the respective
   values obtained from Google maps.
 - If you want to send off crowdsourced information, i.e., information
   about visible wifis and visible radio cells for the obtained
   location fixes to Mozilla's location service and our own instance:
   - `sudo GLOG_v=40 GLOG_logtostderr=1 GPS_TEST_ENABLE_HARVESTING_DURING_TESTS=1  GPS_TEST_REF_LAT=INSERT_ESTIMATE_HERE GPS_TEST_REF_LON=INSERT_ESTIMATE_HERE /usr/bin/uls-tests/gps_provider_test --gtest_filter=*.time_to_first_fix_cold_start_with_supl_benchmark_requires_hardware`
 - If you '''don't''' want to send off crowdsourced information:
   - `sudo GLOG_v=40 GLOG_logtostderr=1 GPS_TEST_REF_LAT=INSERT_ESTIMATE_HERE GPS_TEST_REF_LON=INSERT_ESTIMATE_HERE /usr/bin/uls-tests/gps_provider_test --gtest_filter=*.time_to_first_fix_cold_start_with_supl_benchmark_requires_hardware`

 - The test will output a lot of diagnostic information to the
   terminal and will take ~10 minutes or less. The test will
   automatically report success or failure.

# Connectivity API

For integration of network-based positioning providers, the location
service offers a connectivity API that provides access to wifi and
cell measurements as well as information on the current overall
connectivity status of the device. Please execute the following
commands on a newly flashed device with a writable image:

  - `sudo apt-get update && sudo apt-get build-dep location-service && sudo apt-get install libubuntu-location-service-dev ubuntu-location-service-examples`
  - `mkdir /tmp/build && cd /tmp/build && cmake /usr/share/ubuntu-location-service/examples/standalone/connectivity/ && make`
  - `GLOG_logtostderr=1 ./connectivity`

Verify that the output looks similar to:

    phablet@ubuntu-phablet:/tmp/build$ ./connectivity 
    Is wifi enabled: true
    Is wifi hw enabled: true
    Is wwan enabled: false
    Is wwan hw enabled: true
    umts(mcc: 262, mnc: 2, lac: 5313, id: 131948771, asu: 7)
    (bssid: BC:F2:AF:AF:19:A2, ssid: devolo-bcf2afaf19a2, last seen: 1408955086, mode: Mode::infrastructure, frequency: 2462, strength: 72)
    (bssid: 00:22:3F:35:43:58, ssid: JustAnotherWLAN, last seen: 1408955086, mode: Mode::infrastructure, frequency: 2412, strength: 24)
    (bssid: 82:C7:A6:40:8C:4E, ssid: EasyBox-44D054, last seen: 1408955206, mode: Mode::infrastructure, frequency: 2417, strength: 17)
    (bssid: 00:24:01:B8:32:8D, ssid: gra, last seen: 1408955086, mode: Mode::infrastructure, frequency: 2412, strength: 12)
    (bssid: C0:25:06:3C:28:22, ssid: FRITZ!Box 6360 Cable, last seen: 1408954966, mode: Mode::infrastructure, frequency: 2412, strength: 17)
    (bssid: 00:1C:4A:A5:B7:59, ssid: FRITZ!Box Fon WLAN 7170, last seen: 1408954966, mode: Mode::infrastructure, frequency: 2437, strength: 10)
    Last seen changed for wifi (bssid: BC:F2:AF:AF:19:A2, ssid: devolo-bcf2afaf19a2, last seen: 1408955257, mode: Mode::infrastructure, frequency: 2462, strength: 72)
    Last seen changed for wifi (bssid: 00:22:3F:35:43:58, ssid: JustAnotherWLAN, last seen: 1408955257, mode: Mode::infrastructure, frequency: 2412, strength: 24)
    Signal strength changed for wifi: (bssid: BC:F2:AF:AF:19:A2, ssid: devolo-bcf2afaf19a2, last seen: 1408955257, mode: Mode::infrastructure, frequency: 2462, strength: 73)

# Trust Store Integration

Please note that we are assuming a freshly wiped system for testing
here. If you cannot fulfill that pre-condition, please run `rm -rf
/home/phablet/.local/share/UbuntuLocationService && sudo shutdown -r` prior to running the
tests:

## Unconfined

 - Open the browser, go to maps.google.com
 - Observe the in-browser dialog asking for granting access to location.

## Confined Web-App
 
 - Open the Nokia Here web app, observe the trust dialog appearing.

## Confined Application

 - Open osmtouch and observe the osmtouch surface sliding up, presenting you with a trust dialog.

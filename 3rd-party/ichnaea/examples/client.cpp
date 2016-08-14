#include <ichnaea/client.h>

#include <com/ubuntu/location/connectivity/manager.h>

#include <core/dbus/macros.h>
#include <core/dbus/object.h>
#include <core/dbus/signal.h>

#include <core/net/http/client.h>

#include <thread>
#include <vector>

namespace connectivity = com::ubuntu::location::connectivity;

int main()
{
    auto manager = connectivity::platform_default_manager();
    auto http_client = core::net::http::make_client();

    std::thread worker1{[http_client]() {http_client->run();}};

    auto client = std::make_shared<ichnaea::Client>("test", http_client);

    manager->wireless_network_scan_finished().connect([manager, client]
    {
        std::cout << "Wireless network scan finished" << std::endl;

        std::vector<connectivity::WirelessNetwork::Ptr> wifis;

        manager->enumerate_visible_wireless_networks([&wifis](const connectivity::WirelessNetwork::Ptr& wifi)
        {
            wifis.push_back(wifi);
        });

        ichnaea::geolocate::Parameters params;
        params.consider_ip = true;

        for (auto wifi : wifis)
        {
            ichnaea::WifiAccessPoint ap;
            ap.bssid = wifi->bssid().get();
            ap.ssid = wifi->ssid().get();
            ap.frequency = wifi->frequency().get();
            ap.signal_strength = wifi->signal_strength().get();

            params.wifi_access_points.insert(ap);
        }

        try
        {
            client->geolocate(params, [client, params](const ichnaea::Response<ichnaea::geolocate::Result>& response)
            {
                if (not response.is_error())
                {
                    std::cout << "Submitting to the service again." << std::endl;
                    ichnaea::geosubmit::Parameters submission;
                    ichnaea::geosubmit::Report report;
                    report.timestamp = std::chrono::system_clock::now();
                    report.position.latitude = response.result().location.lat;
                    report.position.longitude = response.result().location.lon;
                    report.position.accuracy = response.result().accuracy;
                    report.wifi_access_points = params.wifi_access_points;
                    submission.reports.push_back(report);

                    client->geosubmit(submission, [](const ichnaea::Response<ichnaea::geosubmit::Result>& response)
                    {
                        if (not response.is_error())
                            std::cout << "Successfully submitted to service." << std::endl;
                        else
                            std::cout << "Error submitting to service: " << response.error() << std::endl;
                    });
                }
                else
                {
                    std::cout << "Error querying service for location: " << response.error() << std::endl;
                }
            });

            client->region(params, [](const ichnaea::Response<ichnaea::region::Result>& response)
            {
                if (not response.is_error())
                    std::cout << response.result().country_code << ", " << response.result().country_name << std::endl;
                else
                    std::cout << "Error querying service for region information: " << response.error() << std::endl;
            });
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    });

    while (true)
    {
        manager->request_scan_for_wireless_networks();
        std::this_thread::sleep_for(std::chrono::seconds{15});
    }

    return 0;
}

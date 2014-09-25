#include <com/ubuntu/location/connectivity/manager.h>

#include <core/posix/signal.h>

#include <glog/logging.h>

#include <cstdlib>

#include <thread>

namespace location = com::ubuntu::location;

namespace
{
// Just convenience to safe some typing.
typedef std::vector<location::connectivity::RadioCell> RadioCells;
typedef std::vector<location::connectivity::WirelessNetwork::Ptr> WirelessNetworks;
}

// This example illustrates the usage of the location-service-specific connectivity API.
// The setup for obtaining measurements of wifis and radio cells is as follows:
//
//   (1.) Obtain an instance of location::connectivity::Manager.
//   (2.) Connect to the changed signals of the wifi and cell properties and react according to your component's requirements.
//   (3.) Bootstrap your own setup by explicitly getting all visible wifis and connected cells.
int main(int argc, char** argv)
{
    google::InitGoogleLogging("com::ubuntu::location::connectivity");

    // We catch sig-term to exit cleanly.
    auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
    trap->signal_raised().connect([trap](core::posix::Signal)
                                  {
                                      trap->stop();
                                  });

    // Trying to acquire a default implementation.
    auto cm = location::connectivity::platform_default_manager();

    if (not cm)
    {
        LOG(ERROR) << "Could not get hold of a connectivity::Manager implementation, aborting...";
        std::exit(1);
    }

    // Let's query some properties about wifi and wwan capabilities
    LOG(INFO) << "Is wifi enabled: " << std::boolalpha << cm->is_wifi_enabled().get();
    LOG(INFO) << "Is wifi hw enabled: " << std::boolalpha << cm->is_wifi_hardware_enabled().get();
    LOG(INFO) << "Is wwan enabled: " << std::boolalpha << cm->is_wwan_enabled().get();
    LOG(INFO) << "Is wwan hw enabled: " << std::boolalpha << cm->is_wwan_hardware_enabled().get();

    // Subscribe to state changes
    cm->state().changed().connect([](location::connectivity::State state)
    {
        LOG(INFO) << "Connectivity state changed: " << state;
    });

    // Subscribe to wifi/wwan state changes
    cm->is_wifi_enabled().changed().connect([](bool enabled)
    {
        LOG(INFO) << "Wifi is " << (enabled ? "" : "not") << " enabled";
    });

    cm->is_wwan_enabled().changed().connect([](bool enabled)
    {
        LOG(INFO) << "Wwan is " << (enabled ? "" : "not") << " enabled";
    });

    cm->is_wifi_hardware_enabled().changed().connect([](bool enabled)
    {
        LOG(INFO) << "Wifi h/w is " << (enabled ? "" : "not") << " enabled";
    });

    cm->is_wwan_hardware_enabled().changed().connect([](bool enabled)
    {
        LOG(INFO) << "Wwan h/w is " << (enabled ? "" : "not") << " enabled";
    });

    // Subscribe to connection characteristics changes
    cm->active_connection_characteristics().changed().connect([](location::connectivity::Characteristics flags)
    {
        LOG(INFO) << "Characteristics for the primary network connection have changed: " << flags;
    });

    // Subscribe to wifi added/removed signals.
    cm->wireless_network_added().connect([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        LOG(INFO) << "Visible wireless network was added: " << *wifi;

        // We don't want to keep the object alive
        std::weak_ptr<location::connectivity::WirelessNetwork> wp
        {
            wifi
        };

        // Subscribe to signal strength and last_seen updates. Please note that this is not considering
        // the case of subscribing to already known wifis. We leave this up
        // to consumers of the api.
        wifi->last_seen().changed().connect([wp](const std::chrono::system_clock::time_point& tp)
        {
            auto sp = wp.lock();
            if (sp)
                LOG(INFO) << "Signal strength changed for wifi " << sp->ssid().get() << ": " << tp.time_since_epoch().count();
        });

        wifi->signal_strength().changed().connect([wp](const location::connectivity::WirelessNetwork::SignalStrength& s)
        {
            auto sp = wp.lock();
            if (sp)
                LOG(INFO) << "Signal strength changed for wifi " << sp->ssid().get() << ": " << s;
        });
    });

    cm->wireless_network_removed().connect([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        LOG(INFO) << "Visible wireless network was removed: " << wifi->ssid().get();
    });

    // Iterate over all radio cells that the device is connected with.
    cm->enumerate_connected_radio_cells([](const location::connectivity::RadioCell::Ptr& cell)
    {
        LOG(INFO) << *cell;

        std::weak_ptr<location::connectivity::RadioCell> wp{cell};

        // Subscribe to changes on the cell
        cell->changed().connect([wp]()
        {
            auto sp = wp.lock();

            if (sp)
                LOG(INFO) << "Something changed on a radio cell: " << *sp;
        });
    });

    cm->connected_cell_added().connect([](const location::connectivity::RadioCell::Ptr& cell)
    {
        LOG(INFO) << "Connected cell added: " << *cell;

        // Subscribe to changes on the cell
        cell->changed().connect([]()
        {
            LOG(INFO) << "Something changed on a radio cell.";
        });
    });

    cm->connected_cell_removed().connect([](const location::connectivity::RadioCell::Ptr& cell)
    {
        LOG(INFO) << "Connected cell removed: " << *cell;
    });

    // Iterate over all networks that are visible right now.
    cm->enumerate_visible_wireless_networks([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        LOG(INFO) << *wifi;

        // We don't want to keep the object alive
        std::weak_ptr<location::connectivity::WirelessNetwork> wp
        {
            wifi
        };

        // Subscribe to last-seen updates.
        wifi->last_seen().changed().connect([wp](const std::chrono::system_clock::time_point& tp)
        {
            auto sp = wp.lock();
            if (sp)
                LOG(INFO) << "Last seen changed for wifi " << *sp;
        });

        // Subscribe to signal strength updates. Please note that this is not considering
        // the case of subscribing to already known wifis. We leave this up
        // to consumers of the api.
        wifi->signal_strength().changed().connect([wp](const location::connectivity::WirelessNetwork::SignalStrength& s)
        {
            auto sp = wp.lock();
            if (sp)
                LOG(INFO) << "Signal strength changed for wifi: " << *sp;
        });
    });
    
    // Subscribe to end-of-scan signals
    cm->wireless_network_scan_finished().connect([]()
    {
        LOG(INFO) << "A wireless network scan finished.";
    });

    // Request a scan for wireless networks.
    try
    {
        cm->request_scan_for_wireless_networks();
    } catch(const std::runtime_error& e)
    {
        LOG(ERROR) << e.what();
    }

    bool cancelled = false;

    std::thread t1
    {
        [cm, &cancelled]()
        {
            cm->request_scan_for_wireless_networks();

            while (not cancelled)
            {
                cm->enumerate_visible_wireless_networks([](const location::connectivity::WirelessNetwork::Ptr&)
                {
                    // We do nothing with the actual values and just keep the thread running
                    // to put some load on the infrastructure.
                });

                // Let's query some properties about wifi and wwan capabilities
                try
                {
                    cm->is_wifi_enabled().get();
                    cm->is_wifi_hardware_enabled().get();
                    cm->is_wwan_enabled().get();
                    cm->is_wwan_hardware_enabled().get();
                } catch(const std::exception& e)
                {
                    LOG(ERROR) << e.what();
                }

            }
        }
    };

    std::thread t2
    {
        [cm, &cancelled]()
        {
            while (not cancelled)
            {
                cm->enumerate_connected_radio_cells([](const location::connectivity::RadioCell::Ptr&)
                {
                    // We do nothing with the actual values and just keep the thread running
                    // to put some load on the infrastructure.
                });                               
            }
        }
    };

    trap->run();

    cancelled = true;

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    return 0;
}    



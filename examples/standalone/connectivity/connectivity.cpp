#include <com/ubuntu/location/connectivity/manager.h>

#include <core/posix/signal.h>

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
        std::cerr << "Could not get hold of a connectivity::Manager implementation, aborting..." << std::endl;
        std::exit(1);
    }

    // Subscribe to wifi added/removed signals.
    cm->wireless_network_added().connect([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        std::cout << "Visible wireless network was added: " << *wifi << std::endl;

        // We don't want to keep the object alive
        std::weak_ptr<location::connectivity::WirelessNetwork> wp
        {
            wifi
        };

        // Subscribe to signal strength updates. Please note that this is not considering
        // the case of subscribing to already known wifis. We leave this up
        // to consumers of the api.
        wifi->signal_strength().changed().connect([wp](const location::connectivity::WirelessNetwork::SignalStrength& s)
        {
            auto sp = wp.lock();
            if (sp)
                std::cout << "Signal strength changed for wifi " << sp->ssid().get() << ": " << s << std::endl;
        });
    });

    cm->wireless_network_removed().connect([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        std::cout << "Visible wireless network was removed: " << wifi->ssid().get() << std::endl;
    });

    // Iterate over all radio cells that the device is connected with.
    cm->enumerate_connected_radio_cells([](const location::connectivity::RadioCell::Ptr& cell)
    {
        std::cout << *cell << std::endl;

        std::weak_ptr<location::connectivity::RadioCell> wp{cell};

        // Subscribe to changes on the cell
        cell->changed().connect([wp]()
        {
            auto sp = wp.lock();

            if (sp)
                std::cout << "Something changed on a radio cell: " << *sp << std::endl;
        });
    });

    cm->connected_cell_added().connect([](const location::connectivity::RadioCell::Ptr& cell)
    {
        std::cout << *cell << std::endl;

        // Subscribe to changes on the cell
        cell->changed().connect([]()
        {
            std::cout << "Something changed on a radio cell." << std::endl;
        });
    });

    cm->connected_cell_removed().connect([](const location::connectivity::RadioCell::Ptr& cell)
    {
        std::cout << *cell << std::endl;
    });

    // Iterate over all networks that are visible right now.
    cm->enumerate_visible_wireless_networks([](const location::connectivity::WirelessNetwork::Ptr& wifi)
    {
        std::cout << wifi->ssid().get() << ", timestamp: ";
        auto ts = std::chrono::system_clock::to_time_t(wifi->last_seen().get());
        std::cout << std::ctime(&ts);

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
            {
                auto ts = std::chrono::system_clock::to_time_t(tp);
                std::cout << "Last seen changed for wifi " << sp->ssid().get() << ": " << std::ctime(&ts) << std::endl;
            }
        });

        // Subscribe to signal strength updates. Please note that this is not considering
        // the case of subscribing to already known wifis. We leave this up
        // to consumers of the api.
        wifi->signal_strength().changed().connect([wp](const location::connectivity::WirelessNetwork::SignalStrength& s)
        {
            auto sp = wp.lock();
            if (sp)
                std::cout << "Signal strength changed for wifi " << sp->ssid().get() << ": " << s << std::endl;
        });

        std::cout << "  " << *wifi << std::endl;
    });
    
    // Subscribe to end-of-scan signals
    cm->wireless_network_scan_finished().connect([]()
    {
        std::cout << "A wireless network scan finished." << std::endl;
    });

    // Request a scan for wireless networks.
    try
    {
        cm->request_scan_for_wireless_networks();
    } catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }

    bool cancelled = false;

    std::thread t1
    {
        [cm, &cancelled]()
        {
            while (not cancelled)
            {
                cm->enumerate_visible_wireless_networks([](const location::connectivity::WirelessNetwork::Ptr&)
                {
                    // We do nothing with the actual values and just keep the thread running
                    // to put some load on the infrastructure.
                });
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



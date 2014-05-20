#include <com/ubuntu/location/connectivity/manager.h>

#include <core/posix/signal.h>

#include <cstdlib>

namespace location = com::ubuntu::location;

// Just some convenience to safe some typing.
typedef std::vector<location::connectivity::RadioCell> RadioCells;
typedef std::vector<location::connectivity::WirelessNetwork> WirelessNetworks;

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

    // We subscribe to changes for visible wireless networks.
    cm->visible_wireless_networks().changed().connect([](const WirelessNetworks& networks)
                                                  {
                                                      std::cout << "Visible wireless networks changed:" << std::endl;
                                                      
                                                      for (const auto& wifi : networks)
                                                          std::cout << "  " << wifi << std::endl;
                                                  });

    // We subscribe to changes for connected radio cells.
    cm->connected_radio_cells().changed().connect([](const RadioCells& cells)
                                                  {
                                                      std::cout << "Connected radio cells changed:" << std::endl;
                                                      
                                                      for (const auto& cell : cells)
                                                          std::cout << "  " << cell << std::endl;
                                                  });

    // Iterate over all radio cells that the device is connected with.
    for (const auto& cell : cm->connected_radio_cells().get())
        std::cout << cell << std::endl;

    // Iterate over all visible wireless networks.
    for (const auto& wifi: cm->visible_wireless_networks().get())
            std::cout << wifi << std::endl;
    
    cm->request_scan_for_wireless_networks();

    trap->run();

    return 0;
}    



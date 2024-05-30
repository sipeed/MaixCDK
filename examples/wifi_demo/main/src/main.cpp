
#include "maix_basic.hpp"
#include "main.h"
#include "maix_wifi.hpp"

using namespace maix;
using namespace maix::network;


int _main(int argc, char* argv[])
{
    std::vector<std::string> wifi_ifaces = wifi::list_devices();
    for(auto &iface : wifi_ifaces)
    {
        log::info("wifi iface: %s", iface.c_str());
    }
    if(wifi_ifaces.empty())
    {
        log::error("no wifi iface found");
        return -1;
    }
    wifi::Wifi wifi(wifi_ifaces[0]);

    wifi.start_scan();
    log::info("wait for 3s to scan wifi");
    time::sleep(3); // sleep 3s to wait for scan result

    std::vector<wifi::AP_Info> scan_result = wifi.get_scan_result();
    log::info("ssid, bssid, channel, rssi, security");
    for(auto &ap : scan_result)
    {
        log::info("%-20s, %s, %-10d, %-10d, %s",
            ap.ssid_str().c_str(), ap.bssid.c_str(), ap.channel, ap.rssi, ap.security.c_str());
    }

    wifi.stop_scan();

    log::info("IP: %s", wifi.get_ip().c_str());
    log::info("MAC: %s", wifi.get_mac().c_str());
    log::info("Gateway: %s", wifi.get_gateway().c_str());

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}



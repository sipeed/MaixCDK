/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.4.8: add wifi support
 */

#pragma once
#include "maix_basic.hpp"
#include <string>
#include <vector>
#include <functional>

namespace maix::network::wifi
{

    class AP_Info
    {
    public:
        std::vector<uint8_t> ssid;
        std::string bssid;
        std::string security;
        int channel;
        int frequency;
        int rssi;

        std::string ssid_str() const
        {
            return std::string(ssid.begin(), ssid.end());
        }
    };

    std::vector<std::string> get_wifi_iface_list();

    class Wifi
    {
    public:
        Wifi(std::string iface = "wlan0");
        ~Wifi();

        // common
        std::string get_ip();
        std::string get_mac();
        std::string get_gateway();
        // std::string get_channel();
        // std::string get_netmask();
        // std::string get_ssid();
        // std::string get_bssid();
        // std::string get_dns();

        // STA mode
        err::Err start_scan();
        std::vector<wifi::AP_Info> get_scan_result();
        void stop_scan();
        err::Err connect(const std::string &ssid, const std::string &password, bool wait = true, int timeout = 60);
        err::Err disconnect();
        bool is_connected();
        // std::string get_rssi();

        // AP mode
        err::Err start_ap(const std::string &ssid, const std::string &password,
                        const std::string &ip = "192.168.66.1", const std::string &netmask = "255.255.255.0",
                        int channel = 1, bool hidden = false,
                        const std::string &ssid_5g = "", const std::string &password_5g = "",
                        int channel_5g = 36, bool hidden_5g = false,
                        int bandwidth = 20, int bandwidth_5g = 20
                        );
        err::Err stop_ap();
        bool is_ap_mode();

    private:
        std::string _iface;
        bool _ap_mode;
    };

} // namespace maix::wifi


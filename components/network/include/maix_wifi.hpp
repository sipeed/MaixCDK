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

    /**
     * WiFi AP info
     * @maixpy maix.network.wifi.AP_Info
     */
    class AP_Info
    {
    public:
        /**
         * WiFi AP info SSID
         * @maixpy maix.network.wifi.AP_Info.ssid
         */
        std::vector<uint8_t> ssid;

        /**
         * WiFi AP info BSSID
         * @maixpy maix.network.wifi.AP_Info.bssid
         */
        std::string bssid;

        /**
         * WiFi AP info security
         * @maixpy maix.network.wifi.AP_Info.security
         */
        std::string security;

        /**
         * WiFi AP info channel
         * @maixpy maix.network.wifi.AP_Info.channel
         */
        int channel;

        /**
         * WiFi AP info frequency
         * @maixpy maix.network.wifi.AP_Info.frequency
         */
        int frequency;

        /**
         * WiFi AP info rssi
         * @maixpy maix.network.wifi.AP_Info.rssi
         */
        int rssi;

        /**
         * WiFi AP info ssid_str
         * @maixpy maix.network.wifi.AP_Info.ssid_str
         */
        std::string ssid_str()
        {
            return std::string(ssid.begin(), ssid.end());
        }
    };

    /**
     * List WiFi interfaces
     * @return WiFi interface list, string type
     * @maixpy maix.network.wifi.list_devices
     */
    std::vector<std::string> list_devices();

    /**
     * Wifi class
     * @maixpy maix.network.wifi.Wifi
     */
    class Wifi
    {
    public:
        /**
         * Wifi class
         * @param iface wifi interface name, default is wlan0
         * @maixpy maix.network.wifi.Wifi.__init__
         * @maixcdk maix.network.wifi.Wifi.Wifi
         */
        Wifi(std::string iface = "wlan0");
        ~Wifi();

        /**
         * Get current WiFi ip
         * @return ip, string type, if network not connected, will return empty string.
         * @maixpy maix.network.wifi.Wifi.get_ip
         */
        std::string get_ip();

        /**
         * Get current WiFi MAC address
         * @return ip, string type.
         * @maixpy maix.network.wifi.Wifi.get_mac
         */
        std::string get_mac();

        /**
         * Get current WiFi SSID
         * @param from_cache if true, will not read config from file, direct use ssid in cache.
         *          attention, first time call this method will auto matically read config from file, and if call connect method will set cache.
         * @return SSID, string type.
         * @maixpy maix.network.wifi.Wifi.get_ssid
         */
        std::string get_ssid(bool from_cache = true);

        /**
         * Get current WiFi ip
         * @return ip, string type, if network not connected, will return empty string.
         * @maixpy maix.network.wifi.Wifi.get_gateway
         */
        std::string get_gateway();

        // std::string get_channel();
        // std::string get_netmask();
        // std::string get_ssid();
        // std::string get_bssid();
        // std::string get_dns();

        // STA mode
        /**
         * WiFi start scan AP info around in background.
         * @return If success, return err.Err.ERR_NONE, else means failed.
         * @maixpy maix.network.wifi.Wifi.start_scan
         */
        err::Err start_scan();

        /**
         * Get WiFi scan AP info.
         * @return wifi.AP_Info list.
         * @maixpy maix.network.wifi.Wifi.get_scan_result
         */
        std::vector<network::wifi::AP_Info> get_scan_result();

        /**
         * Stop WiFi scan AP info.
         * @maixpy maix.network.wifi.Wifi.stop_scan
         */
        void stop_scan();

        /**
         * Connect to WiFi AP.
         * @param ssid SSID of AP
         * @param password password of AP, if no password, leave it empty.
         * @param wait wait for got IP or failed or timeout.
         * @param timeout connect timeout internal, unit second.
         * @return If success, return err.Err.ERR_NONE, else means failed.
         * @maixpy maix.network.wifi.Wifi.connect
         */
        err::Err connect(const std::string &ssid, const std::string &password, bool wait = true, int timeout = 60);

        /**
         * Disconnect from WiFi AP.
         * @return If success, return err.Err.ERR_NONE, else means failed.
         * @maixpy maix.network.wifi.Wifi.disconnect
         */
        err::Err disconnect();

        /**
         * See if WiFi is connected to AP.
         * @return If connected return true, else false.
         * @maixpy maix.network.wifi.Wifi.is_connected
         */
        bool is_connected();
        // std::string get_rssi();

        /**
         * Start WiFi AP.
         * @param ssid SSID of AP.
         * @param password password of AP, if no password, leave it empty.
         * @param ip ip address of hostap, default empty string means auto generated one according to hardware.
         * @param netmask netmask, default 255.255.255.0, now only support 255.255.255.0 .
         * @param mode WiFi mode, default g(IEEE 802.11g (2.4 GHz)), a = IEEE 802.11a (5 GHz), b = IEEE 802.11b (2.4 GHz).
         * @param channel WiFi channel number, 0 means auto select. MaixCAM not support auto, will default channel 1.
         * @param hidden hidden SSID or not.
         * @return If success, return err.Err.ERR_NONE, else means failed.
         * @maixpy maix.network.wifi.Wifi.start_ap
         */
        err::Err start_ap(const std::string &ssid, const std::string &password,
                          std::string mode = "g", int channel = 0,
                          const std::string &ip = "192.168.66.1", const std::string &netmask = "255.255.255.0",
                          bool hidden = false);

        /**
         * Stop WiFi AP.
         * @return If success, return err.Err.ERR_NONE, else means failed.
         * @maixpy maix.network.wifi.Wifi.stop_ap
         */
        err::Err stop_ap();

        /**
         * Whether WiFi is AP mode
         * @return True if AP mode now, or False.
         * @maixpy maix.network.wifi.Wifi.is_ap_mode
         */
        bool is_ap_mode();

    private:
        std::string _iface;
        std::string _ssid;
        bool _ssid_cached;
    };

} // namespace maix::wifi

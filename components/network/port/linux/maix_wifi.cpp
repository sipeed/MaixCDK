#include "maix_wifi.hpp"
#include "maix_basic.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <net/if_arp.h>

namespace maix::network::wifi
{

    static std::string split_ip3(const std::string &ip)
    {
        // Find the last dot in the string
        size_t lastDot = ip.rfind('.');

        // Check if a dot was found
        if (lastDot != std::string::npos)
        {
            // Return the substring from the beginning to the last dot
            return ip.substr(0, lastDot);
        }

        // If no dot is found, return the original string (or handle it as an error)
        return ip;
    }

    static int wifi_freq_to_channel(int freq)
    {
        if (freq >= 2412 && freq <= 2484)
        {
            return (freq - 2412) / 5 + 1;
        }
        else if (freq >= 5170 && freq <= 5825)
        {
            return (freq - 5170) / 5 + 34;
        }
        else
        {
            return 0;
        }
    }

    static std::vector<uint8_t> bytes_str_to_bytes(const std::string &str)
    {
        // 1\xe4\xb8\xad\xe7\xba\xa2\xe9\x9b\x86\xe5\x9b\xa2\\xhjk
        // char or byte start with '\x'
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < str.size(); i++)
        {
            if (str[i] == '\\' && i + 1 < str.size() && str[i + 1] == '\\')
            {
                bytes.push_back('\\');
                i++;
            }
            else if (str[i] == '\\' && i + 1 < str.size() && str[i + 1] == 'x')
            {
                if (i + 3 < str.size())
                {
                    char c = 0;
                    for (int j = 0; j < 2; j++)
                    {
                        c <<= 4;
                        if (str[i + 2 + j] >= '0' && str[i + 2 + j] <= '9')
                        {
                            c |= str[i + 2 + j] - '0';
                        }
                        else if (str[i + 2 + j] >= 'a' && str[i + 2 + j] <= 'f')
                        {
                            c |= str[i + 2 + j] - 'a' + 10;
                        }
                        else if (str[i + 2 + j] >= 'A' && str[i + 2 + j] <= 'F')
                        {
                            c |= str[i + 2 + j] - 'A' + 10;
                        }
                    }
                    bytes.push_back(c);
                    i += 3;
                }
            }
            else
            {
                bytes.push_back(str[i]);
            }
        }
        return bytes;
    }

    std::vector<std::string> list_devices()
    {
        std::vector<std::string> wifi_ifaces;
        struct ifaddrs *ifaddr, *ifa;
        int family;

        if (getifaddrs(&ifaddr) == -1)
        {
            log::error("getifaddrs failed: %s", strerror(errno));
            return wifi_ifaces;
        }

        /* Walk through linked list, maintaining head pointer so we can free list later */
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;

            /* Check if this is a packet socket */
            if (family == AF_PACKET)
            {
                /* Check if this is a WiFi interface */
                int sock = socket(AF_INET, SOCK_DGRAM, 0);
                struct iwreq req;
                strncpy(req.ifr_name, ifa->ifa_name, IFNAMSIZ);
                if (strstr(ifa->ifa_name, "wlan") != NULL || ioctl(sock, SIOCGIWNAME, &req) != -1)
                {
                    wifi_ifaces.push_back(ifa->ifa_name);
                }
                close(sock);
            }
        }

        freeifaddrs(ifaddr);
        return wifi_ifaces;
    }

    Wifi::Wifi(std::string iface)
    {
        this->_iface = iface;
    }

    Wifi::~Wifi()
    {
    }

    err::Err Wifi::start_scan()
    {
        // execute wpa_cli scan -i iface command and parse the result find OK string
        // fork a new process to execute the command
        int pipefd[2]; // ch 0 is read, ch 1 is for write
        (void)!pipe(pipefd);
        pid_t pid = fork();
        if (pid == 0)
        {                       /* child process */
            close(pipefd[0]);   // close reading end in the child
            dup2(pipefd[1], 1); // send stdout to the pipe
            dup2(pipefd[1], 2); // send stderr to the pipe
            close(pipefd[1]);   // this descriptor is no longer needed

            const char *program = "/usr/sbin/wpa_cli";
            char *argv[] = {(char *)program, (char *)"scan", (char *)"-i", (char *)_iface.c_str(), NULL};
            execv(program, argv);
            log::error("execv failed: %s", strerror(errno));
            exit(127); // only if execv fails
        }
        else
        { /* pid!=0; parent process */
            // read the result from the pipe
            close(pipefd[1]); // close the write end of the pipe in the parent
            std::string result;
            char buf[1024];
            while (1)
            {
                int ret = read(pipefd[0], buf, sizeof(buf));
                if (ret > 0)
                {
                    result.append(buf, ret);
                }
                else if (ret == 0)
                {
                    break;
                }
                else
                {
                    log::error("read failed: %s", strerror(errno));
                    break;
                }
            }
            waitpid(pid, NULL, 0);

            // parse the result
            if (result.find("OK") != std::string::npos)
            {
                return err::Err::ERR_NONE;
            }
            else
            {
                return err::Err::ERR_RUNTIME;
            }
        }
    }

    std::vector<wifi::AP_Info> Wifi::get_scan_result()
    {
        // execute wpa_cli scan_results -i iface command and parse the result
        // fork a new process to execute the command
        int pipefd[2]; // ch 0 is read, ch 1 is for write
        (void)!pipe(pipefd);
        pid_t pid = fork();
        if (pid == 0)
        {                       /* child process */
            close(pipefd[0]);   // close reading end in the child
            dup2(pipefd[1], 1); // send stdout to the pipe
            dup2(pipefd[1], 2); // send stderr to the pipe
            close(pipefd[1]);   // this descriptor is no longer needed

            const char *program = "/usr/sbin/wpa_cli";
            char *argv[] = {(char *)program, (char *)"scan_results", (char *)"-i", (char *)_iface.c_str(), NULL};
            execv(program, argv);
            log::error("execv failed: %s", strerror(errno));
            exit(127); // only if execv fails
        }
        else
        { /* pid!=0; parent process */
            // read the result from the pipe
            close(pipefd[1]); // close the write end of the pipe in the parent
            std::string result = "";
            char buf[1024];
            while (1)
            {
                int ret = read(pipefd[0], buf, sizeof(buf) - 1);
                if (ret > 0)
                {
                    buf[ret] = 0;
                    result += buf;
                }
                else if (ret == 0)
                {
                    break;
                }
                else
                {
                    log::error("read failed: %s", strerror(errno));
                    break;
                }
            }
            result += '\n';
            waitpid(pid, NULL, 0);

            // parse the result
            /*
bssid / frequency / signal level / flags / ssid
58:41:20:05:07:97	5745	-58	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_Guest
3c:cd:57:f1:44:c9	5240	-47	[WPA2-PSK-CCMP][WPS][ESS]	1\xe4\xb8\xad\xe7\xba\xa2\xe9\x9b\x86\xe5\x9b\xa2
94:83:c4:3c:b5:da	5180	-62	[WPA2-PSK+SAE+PSK-SHA256-CCMP][SAE-H2E][ESS][UTF-8]	GL-MT3000
58:41:20:04:e4:d2	5785	-60	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_Guest
5a:41:20:94:e4:d1	5785	-61	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_2.4G
5a:41:20:94:e1:59	5180	-70	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_2.4G
58:41:20:04:e1:5a	5180	-65	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_Guest
5a:41:20:95:07:96	5745	-58	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_2.4G
58:41:20:04:df:f5	5220	-70	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_Guest
5a:41:20:94:df:f4	5220	-69	[WPA-PSK-CCMP][WPA2-PSK+FT/PSK-CCMP][ESS]	Sipeed_2.4G
            */
            std::vector<wifi::AP_Info> ap_list;
            std::string line;
            for (size_t i = 0; i < result.size(); i++)
            {
                if (result[i] == '\n')
                {
                    if (line.size() > 0)
                    {
                        if (line.find("bssid") != std::string::npos)
                        {
                            line.clear();
                            continue;
                        }
                        std::vector<std::string> parts;
                        std::string part;
                        for (auto &c : line)
                        {
                            if (c == '\t')
                            {
                                parts.push_back(part);
                                part.clear();
                            }
                            else
                            {
                                part.push_back(c);
                            }
                        }
                        if (part.size() > 0)
                        {
                            parts.push_back(part);
                        }
                        wifi::AP_Info ap;
                        bool valid = false;
                        if (parts.size() >= 4)
                        {
                            valid = true;
                            ap.bssid = parts[0];
                            ap.frequency = std::stoi(parts[1]);
                            ap.rssi = std::stoi(parts[2]);
                            ap.security = parts[3];
                            ap.channel = wifi_freq_to_channel(ap.frequency);
                        }
                        if (parts.size() >= 5)
                        {
                            ap.ssid = bytes_str_to_bytes(parts[4]);
                        }
                        if (valid)
                        {
                            ap_list.push_back(ap);
                        }
                    }
                    line.clear();
                }
                else
                {
                    line.push_back(result[i]);
                }
            }
            return ap_list;
        }
    }

    void Wifi::stop_scan()
    {
        // wpa_cli not support stop scan, so do nothing
    }

    std::string Wifi::get_ip()
    {
        if (!is_connected())
            return "";
        int fd;
        struct ifreq ifr;

        fd = socket(AF_INET, SOCK_DGRAM, 0);

        // Type of address to retrieve - IPv4 IP address
        ifr.ifr_addr.sa_family = AF_INET;

        // Copy the interface name in the ifreq structure
        strncpy(ifr.ifr_name, _iface.c_str(), IFNAMSIZ - 1);

        ioctl(fd, SIOCGIFADDR, &ifr);

        close(fd);

        return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    }

    std::string Wifi::get_mac()
    {
        int fd;
        struct ifreq ifr;

        fd = socket(AF_INET, SOCK_DGRAM, 0);

        // Type of address to retrieve - MAC address
        ifr.ifr_addr.sa_family = ARPHRD_ETHER;

        // Copy the interface name in the ifreq structure
        strncpy(ifr.ifr_name, _iface.c_str(), IFNAMSIZ - 1);

        ioctl(fd, SIOCGIFHWADDR, &ifr);

        close(fd);

        char mac[18];
        sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        return mac;
    }

    /**
     * Get current WiFi SSID
     * @return SSID, string type.
     * @maixpy maix.network.wifi.Wifi.get_ssid
     */
    std::string Wifi::get_ssid(bool from_cache)
    {
        if ((!from_cache) || !_ssid_cached)
        {
            fs::File *f = fs::open("/boot/wifi.ssid", "r");
            if (f)
            {
                std::string *ssid = f->readline();
                _ssid = *ssid;
                _ssid_cached = true;
                delete ssid;
                delete f;
            }
            else
            {
                _ssid = "";
                _ssid_cached = true;
            }
        }
        return _ssid;
    }

    std::string Wifi::get_gateway()
    {
        // get gateway ip
        if (is_ap_mode())
        {
            std::string cmd = "ip route | grep " + _iface + " | grep src | awk '{print $NF}'";
            FILE *fp = popen(cmd.c_str(), "r");
            if (fp == NULL)
            {
                return "";
            }
            char buf[1024];
            if (fgets(buf, sizeof(buf), fp) == NULL)
            {
                pclose(fp);
                return "";
            }
            pclose(fp);
            return buf;
        }
        std::string cmd = "ip route | grep default | grep " + _iface + " | awk '{print $3}'";
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp == NULL)
        {
            return "";
        }
        char buf[1024];
        if (fgets(buf, sizeof(buf), fp) == NULL)
        {
            pclose(fp);
            return "";
        }
        pclose(fp);
        return buf;
    }
    err::Err Wifi::connect(const std::string &ssid, const std::string &password, bool wait, int timeout)
    {
        uint64_t t = time::ticks_s();
        _ssid = ssid;
        _ssid_cached = true;
#if PLATFORM_MAIXCAM
        // write ssid to /boot/wifi.ssid and password to /boot/wifi.pass
        // write wifi.sta remove wifi.ap
        // then opoen /etc/init.d/S30wifi restart

        // ensure wifi.sta
        if (fs::exists("/boot/wifi.ap"))
        {
            fs::remove("/boot/wifi.ap");
        }
        fs::File *f = fs::open("/boot/wifi.sta", "w");
        f->close();
        delete f;

        // write ssid to /boot/wifi.ssid
        FILE *fp = fopen("/boot/wifi.ssid", "w");
        if (fp == NULL)
        {
            log::error("open /boot/wifi.ssid failed");
            return err::Err::ERR_IO;
        }
        fwrite(ssid.c_str(), 1, ssid.size(), fp);
        fclose(fp);

        // write password to /boot/wifi.pass
        fp = fopen("/boot/wifi.pass", "w");
        if (fp == NULL)
        {
            log::error("open /boot/wifi.pass failed");
            return err::Err::ERR_IO;
        }
        fwrite(password.c_str(), 1, password.size(), fp);
        fclose(fp);

        sync();

        // restart wifi
        if (access("/etc/init.d/S30wifi", F_OK) == -1)
        {
            log::error("/etc/init.d/S30wifi not found");
            return err::Err::ERR_NOT_FOUND;
        }
        int ret = system("/etc/init.d/S30wifi restart");
        if (ret != 0)
        {
            log::error("restart wifi failed: %d", ret);
            return err::Err::ERR_RUNTIME;
        }
#else
        throw err::Exception(err::ERR_NOT_IMPL, "connect wifi not implemented in this platform");
#endif
        uint64_t last_t = time::ticks_s();
        while (wait && !is_connected() && time::ticks_s() - t < timeout && !app::need_exit())
        {
            if (time::ticks_s() - last_t > 8)
            {
                log::info("wait connect %.2f/%d s", time::ticks_s() - t, timeout);
                last_t = time::ticks_s();
            }
            time::sleep_ms(50);
        }
        last_t = time::ticks_s();
        while (wait && get_ip().empty() && time::ticks_s() - t < timeout && !app::need_exit())
        {
            if (time::ticks_s() - last_t > 8)
            {
                log::info("wait get ip %.2f/%d s", time::ticks_s() - t, timeout);
                last_t = time::ticks_s();
            }
            time::sleep_ms(50);
        }
        if (wait && !is_connected())
        {
            log::error("Connect failed, wait get ip timeout");
            return err::Err::ERR_TIMEOUT;
        }
        return err::Err::ERR_NONE;
    }
    err::Err Wifi::disconnect()
    {
#if PLATFORM_MAIXCAM
        // opoen /etc/init.d/S30wifi stop
        if (access("/etc/init.d/S30wifi", F_OK) == -1)
        {
            log::error("/etc/init.d/S30wifi not found");
            return err::Err::ERR_NOT_FOUND;
        }
        int ret = system("/etc/init.d/S30wifi stop");
        if (ret != 0)
        {
            log::error("stop wifi failed: %d", ret);
            return err::Err::ERR_RUNTIME;
        }
        return err::Err::ERR_NONE;
#else
        throw err::Exception(err::ERR_NOT_IMPL, "disconnect wifi not implemented in this platform");
#endif
    }

    bool Wifi::is_connected()
    {
        // check if have ip address
        std::string ip = get_gateway();
        return ip != "";
    }
    // std::string get_rssi();

    // AP mode
    err::Err Wifi::start_ap(const std::string &ssid, const std::string &password,
                            std::string mode, int channel,
                            const std::string &ip, const std::string &netmask,
                            bool hidden)
    {
        _ssid = ssid;
        _ssid_cached = true;
#if PLATFORM_MAIXCAM
        // write ssid to /boot/wifi.ssid and password to /boot/wifi.pass
        // write wifi.ap remove wifi.sta
        // write /boot/hostapd.conf
        // then opoen /etc/init.d/S30wifi restart

        if (channel <= 0)
            channel = 1;

        // ensure wifi.ap
        if (fs::exists("/boot/wifi.sta"))
        {
            fs::remove("/boot/wifi.sta");
        }
        fs::File *f = fs::open("/boot/wifi.ap", "w");
        f->close();
        delete f;

        // write ssid to /boot/hostapd.conf
        FILE *fp = fopen("/boot/hostapd.conf", "w");
        if (fp == NULL)
        {
            log::error("open /boot/hostapd.conf failed");
            return err::Err::ERR_IO;
        }
        // see https://w1.fi/cgit/hostap/plain/hostapd/hostapd.conf
        std::string conf = "ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
beacon_int=100\n\
dtim_period=2\n\
max_num_sta=255\n\
rts_threshold=-1\n\
fragm_threshold=-1\n\
macaddr_acl=0\n\
auth_algs=3\n\
wpa=2\n\
ieee80211n=1\n";
        conf += "ssid=" + ssid + "\n";
        conf += "hw_mode=" + mode + "\n";
        conf += "wpa_passphrase=" + password + "\n";
        conf += "channel=" + std::to_string(channel) + "\n";
        fwrite(conf.c_str(), 1, conf.size(), fp);
        fclose(fp);

        // write dhcp config
        fp = fopen("/etc/udhcpd.wlan0.conf", "w");
        if (fp == NULL)
        {
            log::error("open /etc/udhcpd.wlan0.conf failed");
            return err::Err::ERR_IO;
        }
        // see https://w1.fi/cgit/hostap/plain/hostapd/hostapd.conf
        std::string ip_prefix = split_ip3(ip);
        conf = "start " + ip_prefix + ".100\n" +
               "end " + ip_prefix + ".200\n" +
               "interface " + _iface + "\n\
pidfile /var/run/udhcpd." +
               _iface + ".pid\n\
lease_file /var/lib/misc/udhcpd." +
               _iface + ".leases\n\
option subnet " +
               netmask + "\n\
option lease 864000\n";
        fwrite(conf.c_str(), 1, conf.size(), fp);
        fclose(fp);

        // write ssid to /boot/wifi.ssid
        fp = fopen("/boot/wifi.ssid", "w");
        if (fp == NULL)
        {
            log::error("open /boot/wifi.ssid failed");
            return err::Err::ERR_IO;
        }
        fwrite(ssid.c_str(), 1, ssid.size(), fp);
        fclose(fp);

        // write ssid to /boot/wifi.ssid
        fp = fopen("/boot/wifi.ipv4_prefix", "w");
        if (fp == NULL)
        {
            log::error("open /boot/wifi.ipv4_prefix failed");
            return err::Err::ERR_IO;
        }
        fwrite(ip_prefix.c_str(), 1, ip_prefix.size(), fp);
        fclose(fp);

        sync();

        // restart wifi
        if (access("/etc/init.d/S30wifi", F_OK) == -1)
        {
            log::error("/etc/init.d/S30wifi not found");
            return err::Err::ERR_NOT_FOUND;
        }
        int ret = system("/etc/init.d/S30wifi restart");
        if (ret != 0)
        {
            log::error("restart wifi failed: %d", ret);
            return err::Err::ERR_RUNTIME;
        }
#else
        return err::Err::ERR_NOT_IMPL;
#endif
        return err::ERR_NONE;
    }
    err::Err Wifi::stop_ap()
    {
#if PLATFORM_MAIXCAM
        // opoen /etc/init.d/S30wifi stop
        if (access("/etc/init.d/S30wifi", F_OK) == -1)
        {
            log::error("/etc/init.d/S30wifi not found");
            return err::Err::ERR_NOT_FOUND;
        }
        int ret = system("/etc/init.d/S30wifi stop");
        if (ret != 0)
        {
            log::error("stop wifi failed: %d", ret);
            return err::Err::ERR_RUNTIME;
        }
        return err::Err::ERR_NONE;
#else
        throw err::Exception(err::ERR_NOT_IMPL, "stop_ap wifi not implemented in this platform");
#endif
    }
    bool Wifi::is_ap_mode()
    {
        return fs::exists("/boot/wifi.ap");
    }

} // namespace maix::wifi

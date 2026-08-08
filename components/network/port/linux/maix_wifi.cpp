#include "maix_wifi.hpp"
#include "maix_basic.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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

#if PLATFORM_MAIXCAM2
    struct ProcessResult
    {
        std::string output;
        int wait_status;
    };

    static err::Err run_process_capture(const char *program, char *const argv[], ProcessResult &result)
    {
        int pipefd[2];
        if (pipe(pipefd) != 0)
        {
            log::error("create process output pipe failed: %s", strerror(errno));
            return err::ERR_RUNTIME;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            log::error("fork %s failed: %s", program, strerror(errno));
            close(pipefd[0]);
            close(pipefd[1]);
            return err::ERR_RUNTIME;
        }
        if (pid == 0)
        {
            close(pipefd[0]);
            if (dup2(pipefd[1], STDOUT_FILENO) < 0 || dup2(pipefd[1], STDERR_FILENO) < 0)
            {
                _exit(127);
            }
            close(pipefd[1]);
            execv(program, argv);
            _exit(127);
        }

        close(pipefd[1]);
        result.output.clear();
        char buffer[1024];
        bool read_ok = true;
        while (true)
        {
            const ssize_t read_size = read(pipefd[0], buffer, sizeof(buffer));
            if (read_size > 0)
            {
                result.output.append(buffer, static_cast<size_t>(read_size));
            }
            else if (read_size == 0)
            {
                break;
            }
            else if (errno != EINTR)
            {
                log::error("read %s output failed: %s", program, strerror(errno));
                read_ok = false;
                break;
            }
        }
        close(pipefd[0]);

        pid_t wait_result;
        do
        {
            wait_result = waitpid(pid, &result.wait_status, 0);
        } while (wait_result < 0 && errno == EINTR);
        if (wait_result < 0)
        {
            log::error("wait for %s failed: %s", program, strerror(errno));
            return err::ERR_RUNTIME;
        }
        return read_ok ? err::ERR_NONE : err::ERR_RUNTIME;
    }

    static bool command_output_has_line(const std::string &output, const char *expected)
    {
        size_t line_start = 0;
        while (line_start <= output.size())
        {
            size_t line_end = output.find('\n', line_start);
            if (line_end == std::string::npos)
            {
                line_end = output.size();
            }
            size_t content_end = line_end;
            if (content_end > line_start && output[content_end - 1] == '\r')
            {
                --content_end;
            }
            if (output.compare(line_start, content_end - line_start, expected) == 0)
            {
                return true;
            }
            if (line_end == output.size())
            {
                break;
            }
            line_start = line_end + 1;
        }
        return false;
    }

    static err::Err run_wpa_cli_command_capture(const std::string &iface, const char *command,
                                                 std::string &output)
    {
        const char *program = "/usr/sbin/wpa_cli";
        char *argv[] = {
            const_cast<char *>(program),
            const_cast<char *>("-i"),
            const_cast<char *>(iface.c_str()),
            const_cast<char *>(command),
            NULL,
        };
        ProcessResult result;
        err::Err error = run_process_capture(program, argv, result);
        if (error != err::ERR_NONE)
        {
            return error;
        }
        if (!WIFEXITED(result.wait_status) || WEXITSTATUS(result.wait_status) != 0)
        {
            // if (WIFEXITED(result.wait_status))
            // {
            //     log::error("wpa_cli %s failed with exit status %d", command,
            //                WEXITSTATUS(result.wait_status));
            // }
            // else
            // {
            //     log::error("wpa_cli %s terminated abnormally", command);
            // }
            return err::ERR_RUNTIME;
        }
        output = result.output;
        return err::ERR_NONE;
    }

    static err::Err run_wpa_cli_command(const std::string &iface, const char *command)
    {
        std::string output;
        err::Err error = run_wpa_cli_command_capture(iface, command, output);
        if (error != err::ERR_NONE)
        {
            return error;
        }
        if (!command_output_has_line(output, "OK"))
        {
            log::error("wpa_cli %s did not return an OK response", command);
            return err::ERR_RUNTIME;
        }
        return err::ERR_NONE;
    }

    static bool maixcam2_wpa_is_connected(const std::string &iface)
    {
        std::string output;
        if (run_wpa_cli_command_capture(iface, "status", output) != err::ERR_NONE)
        {
            return false;
        }
        return command_output_has_line(output, "wpa_state=COMPLETED");
    }

    struct ConfigSnapshot
    {
        std::string path;
        std::string content;
        mode_t mode;
        bool existed;
    };

    static bool contains_config_separator(const std::string &value)
    {
        return value.find('\n') != std::string::npos || value.find('\r') != std::string::npos ||
               value.find('\0') != std::string::npos;
    }

    static err::Err write_config_file(const char *path, const std::string &content, mode_t mode = 0644)
    {
        std::string tmp_path = std::string(path) + ".tmp.XXXXXX";
        std::vector<char> tmp_path_buf(tmp_path.begin(), tmp_path.end());
        tmp_path_buf.push_back('\0');
        int fd = mkstemp(tmp_path_buf.data());
        if (fd < 0)
        {
            log::error("create temporary file for %s failed: %s", path, strerror(errno));
            return err::ERR_IO;
        }
        tmp_path = tmp_path_buf.data();
        if (fchmod(fd, mode) != 0)
        {
            log::error("set permissions for %s failed: %s", path, strerror(errno));
            close(fd);
            unlink(tmp_path.c_str());
            return err::ERR_IO;
        }

        FILE *fp = fdopen(fd, "w");
        if (fp == NULL)
        {
            log::error("open temporary file for %s failed: %s", path, strerror(errno));
            close(fd);
            unlink(tmp_path.c_str());
            return err::ERR_IO;
        }

        bool write_ok = content.empty() || fwrite(content.data(), 1, content.size(), fp) == content.size();
        if (write_ok && fflush(fp) != 0)
        {
            write_ok = false;
        }
        if (fclose(fp) != 0)
        {
            write_ok = false;
        }
        if (!write_ok)
        {
            log::error("write %s failed", path);
            unlink(tmp_path.c_str());
            return err::ERR_IO;
        }
        if (rename(tmp_path.c_str(), path) != 0)
        {
            log::error("replace %s failed: %s", path, strerror(errno));
            unlink(tmp_path.c_str());
            return err::ERR_IO;
        }
        return err::ERR_NONE;
    }

    static err::Err snapshot_config_file(const char *path, ConfigSnapshot &snapshot)
    {
        snapshot.path = path;
        snapshot.content.clear();
        snapshot.mode = 0644;
        snapshot.existed = false;

        struct stat file_stat;
        if (stat(path, &file_stat) != 0)
        {
            if (errno == ENOENT)
            {
                return err::ERR_NONE;
            }
            log::error("stat %s failed: %s", path, strerror(errno));
            return err::ERR_IO;
        }

        FILE *fp = fopen(path, "rb");
        if (fp == NULL)
        {
            log::error("open %s for rollback failed: %s", path, strerror(errno));
            return err::ERR_IO;
        }
        char buffer[4096];
        size_t read_size;
        while ((read_size = fread(buffer, 1, sizeof(buffer), fp)) > 0)
        {
            snapshot.content.append(buffer, read_size);
        }
        bool read_ok = ferror(fp) == 0;
        if (fclose(fp) != 0)
        {
            read_ok = false;
        }
        if (!read_ok)
        {
            log::error("read %s for rollback failed", path);
            return err::ERR_IO;
        }
        snapshot.mode = file_stat.st_mode & 0777;
        snapshot.existed = true;
        return err::ERR_NONE;
    }

    static bool restore_config_files(const std::vector<ConfigSnapshot> &snapshots)
    {
        bool restore_ok = true;
        for (auto it = snapshots.rbegin(); it != snapshots.rend(); ++it)
        {
            if (it->existed)
            {
                if (write_config_file(it->path.c_str(), it->content, it->mode) != err::ERR_NONE)
                {
                    restore_ok = false;
                }
            }
            else if (unlink(it->path.c_str()) != 0 && errno != ENOENT)
            {
                log::error("remove %s during rollback failed: %s", it->path.c_str(), strerror(errno));
                restore_ok = false;
            }
        }
        sync();
        return restore_ok;
    }

    static err::Err validate_maixcam2_ap_args(const std::string &iface,
                                               const std::string &ssid,
                                               const std::string &password,
                                               const std::string &mode,
                                               int channel,
                                               const std::string &ip,
                                               const std::string &netmask,
                                               std::string &ip_prefix,
                                               int &normalized_channel)
    {
        if (iface != "wlan0")
        {
            log::error("MaixCAM2 AP mode only supports wlan0, got %s", iface.c_str());
            return err::ERR_ARGS;
        }
        if (ssid.empty() || ssid.size() > 32 || contains_config_separator(ssid))
        {
            log::error("AP SSID must contain 1 to 32 bytes and no line breaks or NUL bytes");
            return err::ERR_ARGS;
        }
        if ((!password.empty() && (password.size() < 8 || password.size() > 63)) ||
            contains_config_separator(password))
        {
            log::error("AP password must be empty or contain 8 to 63 bytes without line breaks or NUL bytes");
            return err::ERR_ARGS;
        }
        if (mode != "a" && mode != "b" && mode != "g")
        {
            log::error("AP mode must be one of: a, b, g");
            return err::ERR_ARGS;
        }
        normalized_channel = channel == 0 ? (mode == "a" ? 36 : 1) : channel;
        if (channel < 0 ||
            ((mode == "a") && (normalized_channel < 34 || normalized_channel > 196)) ||
            ((mode == "b" || mode == "g") && normalized_channel > 14))
        {
            log::error("AP channel %d is invalid for mode %s", channel, mode.c_str());
            return err::ERR_ARGS;
        }
        if (netmask != "255.255.255.0")
        {
            log::error("MaixCAM2 AP mode only supports netmask 255.255.255.0");
            return err::ERR_ARGS;
        }

        struct in_addr address;
        if (contains_config_separator(ip) || inet_pton(AF_INET, ip.c_str(), &address) != 1)
        {
            log::error("AP IP address is not a valid IPv4 address");
            return err::ERR_ARGS;
        }
        uint32_t host_address = ntohl(address.s_addr);
        uint8_t octets[] = {
            static_cast<uint8_t>((host_address >> 24) & 0xff),
            static_cast<uint8_t>((host_address >> 16) & 0xff),
            static_cast<uint8_t>((host_address >> 8) & 0xff),
            static_cast<uint8_t>(host_address & 0xff),
        };
        if (octets[0] == 0 || octets[0] == 127 || octets[0] >= 224 || octets[3] != 1)
        {
            log::error("MaixCAM2 AP IP must be a unicast /24 address ending in .1");
            return err::ERR_ARGS;
        }
        ip_prefix = std::to_string(octets[0]) + "." + std::to_string(octets[1]) + "." +
                    std::to_string(octets[2]);
        return err::ERR_NONE;
    }
#endif

#if PLATFORM_MAIXCAM
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
#endif

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

#if PLATFORM_MAIXCAM2
    static bool wifi_iface_is_discoverable(const std::string &iface)
    {
        const std::vector<std::string> devices = list_devices();
        for (const std::string &device : devices)
        {
            if (device == iface)
            {
                return true;
            }
        }
        return false;
    }

    static err::Err ensure_maixcam2_wifi_iface(const std::string &iface)
    {
        if (wifi_iface_is_discoverable(iface))
        {
            return err::ERR_NONE;
        }

        constexpr int timeout_ms = 5000;
        constexpr int retry_interval_ms = 100;
        log::warn("WiFi interface %s is unavailable; starting wifi.service and waiting up to %d ms",
                  iface.c_str(), timeout_ms);
        const int ret = system("systemctl start wifi");
        if (ret != 0)
        {
            log::error("start wifi.service for interface %s recovery failed: %d", iface.c_str(), ret);
            return err::ERR_RUNTIME;
        }

        const uint64_t start_ms = time::ticks_ms();
        while (!app::need_exit() &&
               time::ticks_ms() - start_ms < static_cast<uint64_t>(timeout_ms))
        {
            if (wifi_iface_is_discoverable(iface))
            {
                return err::ERR_NONE;
            }
            time::sleep_ms(retry_interval_ms);
        }
        if (wifi_iface_is_discoverable(iface))
        {
            return err::ERR_NONE;
        }

        if (app::need_exit())
        {
            log::error("WiFi interface %s did not appear after starting wifi.service; wait stopped by app exit request",
                       iface.c_str());
        }
        else
        {
            log::error("WiFi interface %s did not appear within %d ms after starting wifi.service",
                       iface.c_str(), timeout_ms);
        }
        return err::ERR_NOT_FOUND;
    }
#endif

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
            close(pipefd[0]);   // close reading end in the child

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
        struct ifreq ifr = {};

        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0)
        {
            return "";
        }

        // Type of address to retrieve - IPv4 IP address
        ifr.ifr_addr.sa_family = AF_INET;

        // Copy the interface name in the ifreq structure
        strncpy(ifr.ifr_name, _iface.c_str(), IFNAMSIZ - 1);

        if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
        {
            close(fd);
            return "";
        }

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
#if !PLATFORM_MAIXCAM2
        _ssid = ssid;
        _ssid_cached = true;
#endif
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
#elif PLATFORM_MAIXCAM2
        err::Err error = ensure_maixcam2_wifi_iface(_iface);
        if (error != err::ERR_NONE)
        {
            return error;
        }

        const char *config_paths[] = {
            "/boot/wifi.ssid",
            "/boot/wifi.pass",
            "/boot/wifi.ap",
            "/boot/wifi.sta",
        };
        std::vector<ConfigSnapshot> snapshots(sizeof(config_paths) / sizeof(config_paths[0]));
        for (size_t i = 0; i < snapshots.size(); ++i)
        {
            err::Err snapshot_error = snapshot_config_file(config_paths[i], snapshots[i]);
            if (snapshot_error != err::ERR_NONE)
            {
                return snapshot_error;
            }
        }

        auto write_or_rollback = [&snapshots](const char *path, const std::string &content, mode_t file_mode) {
            err::Err write_error = write_config_file(path, content, file_mode);
            if (write_error != err::ERR_NONE && !restore_config_files(snapshots))
            {
                log::error("failed to fully roll back STA configuration");
            }
            return write_error;
        };

        error = write_or_rollback("/boot/wifi.ssid", ssid, 0644);
        if (error == err::ERR_NONE)
            error = write_or_rollback("/boot/wifi.pass", password, 0600);
        if (error != err::ERR_NONE)
        {
            return error;
        }

        if (unlink("/boot/wifi.ap") != 0 && errno != ENOENT)
        {
            log::error("remove /boot/wifi.ap failed: %s", strerror(errno));
            restore_config_files(snapshots);
            return err::ERR_IO;
        }
        error = write_config_file("/boot/wifi.sta", "");
        if (error != err::ERR_NONE)
        {
            restore_config_files(snapshots);
            return error;
        }

        sync();

        int ret = system("systemctl restart wifi");
        if (ret != 0)
        {
            log::error("restart wifi failed: %d", ret);
            bool restore_ok = restore_config_files(snapshots);
            if (!restore_ok)
            {
                log::error("failed to fully roll back STA configuration after restart failure");
            }
            else
            {
                int restore_ret = system("systemctl restart wifi");
                if (restore_ret != 0)
                {
                    log::error("restart wifi with restored configuration failed: %d", restore_ret);
                }
            }
            return err::Err::ERR_RUNTIME;
        }
        _ssid = ssid;
        _ssid_cached = true;
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
        if (wait && (!is_connected() || get_ip().empty()))
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
#elif PLATFORM_MAIXCAM2
        err::Err error = run_wpa_cli_command(_iface, "disconnect");
        if (error != err::ERR_NONE)
        {
            return err::Err::ERR_RUNTIME;
        }
        return err::Err::ERR_NONE;
#else
        throw err::Exception(err::ERR_NOT_IMPL, "disconnect wifi not implemented in this platform");
#endif
    }

    bool Wifi::is_connected()
    {
#if PLATFORM_MAIXCAM2
        if (is_ap_mode())
        {
            return !get_gateway().empty();
        }
        return maixcam2_wpa_is_connected(_iface);
#else
        // check if have ip address
        std::string ip = get_gateway();
        return ip != "";
#endif
    }
    // std::string get_rssi();

    // AP mode
    err::Err Wifi::start_ap(const std::string &ssid, const std::string &password,
                            std::string mode, int channel,
                            const std::string &ip, const std::string &netmask,
                            bool hidden)
    {
#if PLATFORM_MAIXCAM2
        std::string maixcam2_ip_prefix;
        int maixcam2_channel;
        err::Err error = validate_maixcam2_ap_args(_iface, ssid, password, mode, channel,
                                                    ip, netmask, maixcam2_ip_prefix, maixcam2_channel);
        if (error != err::ERR_NONE)
        {
            return error;
        }
        error = ensure_maixcam2_wifi_iface(_iface);
        if (error != err::ERR_NONE)
        {
            return error;
        }
#endif
#if !PLATFORM_MAIXCAM2
        _ssid = ssid;
        _ssid_cached = true;
#endif
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
#elif PLATFORM_MAIXCAM2
        std::string hostapd_conf = "ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
beacon_int=100\n\
dtim_period=2\n\
max_num_sta=255\n\
rts_threshold=-1\n\
fragm_threshold=-1\n\
macaddr_acl=0\n\
auth_algs=3\n";
        hostapd_conf += "ssid=" + ssid + "\n";
        hostapd_conf += "hw_mode=" + mode + "\n";
        hostapd_conf += "channel=" + std::to_string(maixcam2_channel) + "\n";
        hostapd_conf += "ieee80211n=" + std::to_string(mode == "b" ? 0 : 1) + "\n";
        hostapd_conf += "ignore_broadcast_ssid=" + std::to_string(hidden ? 1 : 0) + "\n";
        if (!password.empty())
        {
            hostapd_conf += "wpa=2\n";
            hostapd_conf += "wpa_passphrase=" + password + "\n";
        }

        std::string dhcp_conf = "start " + maixcam2_ip_prefix + ".100\n" +
                                "end " + maixcam2_ip_prefix + ".200\n" +
                                "interface wlan0\n" +
                                "pidfile /var/run/udhcpd.wlan0.pid\n" +
                                "lease_file /var/lib/misc/udhcpd.wlan0.leases\n" +
                                "option subnet " + netmask + "\n" +
                                "option router " + ip + "\n" +
                                "option lease 864000\n";

        const char *config_paths[] = {
            "/boot/hostapd.conf",
            "/etc/udhcpd.wlan0.conf",
            "/boot/wifi.ssid",
            "/boot/wifi.pass",
            "/boot/wifi.ipv4_prefix",
            "/boot/wifi.sta",
            "/boot/wifi.ap",
        };
        std::vector<ConfigSnapshot> snapshots(sizeof(config_paths) / sizeof(config_paths[0]));
        for (size_t i = 0; i < snapshots.size(); ++i)
        {
            error = snapshot_config_file(config_paths[i], snapshots[i]);
            if (error != err::ERR_NONE)
            {
                return error;
            }
        }

        auto write_or_rollback = [&snapshots](const char *path, const std::string &content, mode_t file_mode) {
            err::Err write_error = write_config_file(path, content, file_mode);
            if (write_error != err::ERR_NONE && !restore_config_files(snapshots))
            {
                log::error("failed to fully roll back AP configuration");
            }
            return write_error;
        };

        error = write_or_rollback("/boot/hostapd.conf", hostapd_conf, 0600);
        if (error == err::ERR_NONE)
            error = write_or_rollback("/etc/udhcpd.wlan0.conf", dhcp_conf, 0644);
        if (error == err::ERR_NONE)
            error = write_or_rollback("/boot/wifi.ssid", ssid, 0644);
        if (error == err::ERR_NONE)
            error = write_or_rollback("/boot/wifi.pass", password, 0600);
        if (error == err::ERR_NONE)
            error = write_or_rollback("/boot/wifi.ipv4_prefix", maixcam2_ip_prefix, 0644);
        if (error != err::ERR_NONE)
        {
            return error;
        }

        if (unlink("/boot/wifi.sta") != 0 && errno != ENOENT)
        {
            log::error("remove /boot/wifi.sta failed: %s", strerror(errno));
            restore_config_files(snapshots);
            return err::ERR_IO;
        }
        error = write_config_file("/boot/wifi.ap", "");
        if (error != err::ERR_NONE)
        {
            restore_config_files(snapshots);
            return error;
        }

        sync();
        int ret = system("systemctl restart wifi");
        if (ret != 0)
        {
            log::error("restart wifi failed: %d", ret);
            bool restore_ok = restore_config_files(snapshots);
            if (!restore_ok)
            {
                log::error("failed to fully roll back AP configuration after restart failure");
            }
            else
            {
                int restore_ret = system("systemctl restart wifi");
                if (restore_ret != 0)
                {
                    log::error("restart wifi with restored configuration failed: %d", restore_ret);
                }
            }
            return err::ERR_RUNTIME;
        }
        _ssid = ssid;
        _ssid_cached = true;
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
#elif PLATFORM_MAIXCAM2
        const char *marker_paths[] = {
            "/boot/wifi.ap",
            "/boot/wifi.sta",
        };
        std::vector<ConfigSnapshot> marker_snapshots(sizeof(marker_paths) / sizeof(marker_paths[0]));
        for (size_t i = 0; i < marker_snapshots.size(); ++i)
        {
            err::Err error = snapshot_config_file(marker_paths[i], marker_snapshots[i]);
            if (error != err::ERR_NONE)
            {
                return error;
            }
        }

        int ret = system("systemctl is-active --quiet wifi");
        if (ret == -1 || !WIFEXITED(ret))
        {
            log::error("query wifi service state failed: %d", ret);
            return err::ERR_RUNTIME;
        }
        const int service_status = WEXITSTATUS(ret);
        if (service_status != 0 && service_status != 3)
        {
            log::error("query wifi service state failed with exit status: %d", service_status);
            return err::ERR_RUNTIME;
        }
        const bool service_was_active = service_status == 0;

        ret = system("systemctl stop wifi");
        if (ret != 0)
        {
            log::error("stop wifi failed: %d", ret);
            if (service_was_active)
            {
                int restore_ret = system("systemctl restart wifi");
                if (restore_ret != 0)
                {
                    log::error("restore prior wifi service state after stop failure failed: %d", restore_ret);
                }
            }
            return err::ERR_RUNTIME;
        }

        auto restore_markers_and_service = [&marker_snapshots, service_was_active](bool stop_current_service) {
            if (stop_current_service)
            {
                int stop_ret = system("systemctl stop wifi");
                if (stop_ret != 0)
                {
                    log::error("stop failed STA service during rollback failed: %d", stop_ret);
                }
            }
            if (!restore_config_files(marker_snapshots))
            {
                log::error("failed to fully restore WiFi mode markers");
            }
            if (service_was_active)
            {
                int restore_ret = system("systemctl restart wifi");
                if (restore_ret != 0)
                {
                    log::error("restore prior wifi service state failed: %d", restore_ret);
                }
            }
        };

        if (unlink("/boot/wifi.ap") != 0 && errno != ENOENT)
        {
            log::error("remove /boot/wifi.ap failed: %s", strerror(errno));
            restore_markers_and_service(false);
            return err::ERR_IO;
        }

        err::Err error = write_config_file("/boot/wifi.sta", "");
        if (error != err::ERR_NONE)
        {
            restore_markers_and_service(false);
            return err::ERR_IO;
        }

        sync();

        ret = system("systemctl start wifi");
        if (ret != 0)
        {
            log::error("start wifi in STA mode failed: %d", ret);
            restore_markers_and_service(true);
            return err::ERR_RUNTIME;
        }
        return err::ERR_NONE;
#else
        throw err::Exception(err::ERR_NOT_IMPL, "stop_ap wifi not implemented in this platform");
#endif
    }
    bool Wifi::is_ap_mode()
    {
        return fs::exists("/boot/wifi.ap");
    }

} // namespace maix::wifi

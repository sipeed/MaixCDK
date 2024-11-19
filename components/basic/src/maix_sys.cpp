/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include "maix_basic.hpp"

#define LIB_VERSION_FILE_PATH "/maixapp/maixcam_lib.version"

namespace maix::sys
{
    std::string os_version()
    {
        FILE *file = fopen("/boot/ver", "r");
        if (!file)
        {
            log::error("Cannot open /boot/ver");
            return "Unkonwn";
        }
        char line[128];
        if (fgets(line, sizeof(line), file))
        {
            fclose(file);
            // strip start and end space
            std::string ver = line;
            ver.erase(0, ver.find_first_not_of(" \t\n"));
            ver.erase(ver.find_last_not_of(" \t\n") + 1);
            return ver;
        }
        return "Unkonwn";
    }

    std::string maixpy_version()
    {
#if PLATFORM_MAIXCAM
        // fast way, read /usr/lib/python3.11/site-packages/maix/version.py
        // find key value:
        //      version_major = 4
        //      version_minor = 4
        //      version_patch = 20
        // return $version_major.$version_minor.$version_patch
        std::ifstream version_file("/usr/lib/python3.11/site-packages/maix/version.py");
        if (!version_file.is_open())
        {
            log::warn("Failed to open version file.");
            return "";
        }

        std::string line;
        int version_major = -1, version_minor = -1, version_patch = -1;

        while (std::getline(version_file, line))
        {
            if (line.find("version_major") != std::string::npos)
            {
                std::string num = line.substr(line.find("=") + 1);
                version_major = std::stoi(num);
            }
            else if (line.find("version_minor") != std::string::npos)
            {
                std::string num = line.substr(line.find("=") + 1);
                version_minor = std::stoi(num);
            }
            else if (line.find("version_patch") != std::string::npos)
            {
                std::string num = line.substr(line.find("=") + 1);
                version_patch = std::stoi(num);
            }
            if (version_major >= 0 && version_minor >= 0 && version_patch >= 0)
                break;
        }

        version_file.close();

        if (version_major == -1 || version_minor == -1 || version_patch == -1)
        {
            log::warn("Version information incomplete or not found.");
            return "";
        }

        std::ostringstream version_stream;
        version_stream << version_major << "." << version_minor << "." << version_patch;
        return version_stream.str();
#else
        log::warn("maixpy_version() not implemented for this platform");
        return "";
#endif
    }

    std::string runtime_version()
    {
        fs::File *file = fs::open(LIB_VERSION_FILE_PATH, "r");
        if (!file)
        {
            return "";
        }
        std::string *version = file->readline();
        std::string curr_version = *version;

        // 去掉头尾的空字符
        const std::string whitespace = " \t\n\r";
        size_t start = curr_version.find_first_not_of(whitespace);
        size_t end = curr_version.find_last_not_of(whitespace);

        if (start != std::string::npos && end != std::string::npos)
        {
            curr_version = curr_version.substr(start, end - start + 1);
        }
        else
        {
            curr_version = ""; // 如果全是空白字符
        }

        delete version;
        file->close();
        delete file;
        return curr_version;
    }

    static std::map<std::string, std::string> _device_configs;
    std::map<std::string, std::string> device_configs(bool cache)
    {
        if (cache && !_device_configs.empty())
            return _device_configs;

        if (!fs::exists("/boot/board"))
        {
            return _device_configs;
        }
        fs::File *f = fs::open("/boot/board", "r");
        if (!f)
        {
            throw err::Exception(err::ERR_ARGS, "open /boot/board failed");
        }

        _device_configs.clear();
        while (1)
        {
            std::string line;
            int num = f->readline(line);
            if (num <= 0)
                break;

            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            size_t pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                _device_configs[key] = value;
            }
        }
        delete f;

        // 检查是否有 id
        if (_device_configs.find("id") == _device_configs.end())
        {
            throw err::Exception(err::ERR_ARGS, "/boot/board missing 'id' item");
        }

        // 读取 /boot/boards/board.{id}
        std::string id = _device_configs["id"];
        std::string board_file_path = "/boot/boards/board." + id;
        if (fs::exists(board_file_path))
        {
            fs::File *board_file = fs::open(board_file_path, "r");
            if (!board_file)
            {
                log::error(("open " + board_file_path + " failed").c_str());
                return _device_configs;
            }

            while (1)
            {
                std::string line;
                int num = board_file->readline(line);
                if (num <= 0)
                    break;

                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                size_t pos = line.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    key.erase(0, key.find_first_not_of(" \t\r\n"));
                    key.erase(key.find_last_not_of(" \t\r\n") + 1);
                    value.erase(0, value.find_first_not_of(" \t\r\n"));
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);

                    // 如果 /boot/board 没有该键值，则从 board.{id} 补充
                    if (_device_configs.find(key) == _device_configs.end())
                    {
                        _device_configs[key] = value;
                    }
                }
            }
            delete board_file;
        }
        else
        {
            log::warn(("Board config not found: " + board_file_path).c_str());
        }

        return _device_configs;
    }

    static std::string _device_id;
    std::string device_id(bool cache)
    {
        if(cache && !_device_id.empty())
        {
            return _device_id;
        }
        FILE *file = NULL;
        char line[128];
        std::string model = "";
        device_configs();
        file = fopen("/proc/device-tree/model", "r");
        if (file)
        {
            if (fgets(line, sizeof(line), file))
            {
                model = line;
                model.erase(0, model.find_first_not_of(" \t\n"));
                model.erase(model.find_last_not_of(" \t\n") + 1);
#if PLATFORM_MAIXCAM
                // if(model.find("MaixCAM") != std::string::npos || model.find("LicheeRv Nano") != std::string::npos)
                // 包含 maixcam 或者 licheerv nano (不区分大小写，先把 model 转换为小写)
                std::string model_lower = model;
                std::transform(model.begin(), model.end(), model_lower.begin(), ::tolower);
                if (model_lower.find("maixcam") != std::string::npos || model_lower.find("licheerv nano") != std::string::npos)
                {
                    fclose(file);
                    auto it = _device_configs.find("id");
                    if (it != _device_configs.end()) {
                        _device_id = it->second;
                    }
                    else
                        _device_id = "maixcam";
                    return _device_id;
                }
#else
                fclose(file);
                std::string model_lower = model;
                std::transform(model.begin(), model.end(), model_lower.begin(), ::tolower);
                _device_id = model_lower;
                return _device_id;
#endif
            }
            fclose(file);
        }
        std::string model_lower = model;
        std::transform(model.begin(), model.end(), model_lower.begin(), ::tolower);
        _device_id = model_lower;
        return _device_id;
    }

    static std::string _device_name;
    std::string device_name(bool cache)
    {
        if(cache && !_device_name.empty())
        {
            return _device_name;
        }
        FILE *file = NULL;
        char line[128];
        std::string model = "";
        file = fopen("/proc/device-tree/model", "r");
        if (file)
        {
            if (fgets(line, sizeof(line), file))
            {
                model = line;
                model.erase(0, model.find_first_not_of(" \t\n"));
                model.erase(model.find_last_not_of(" \t\n") + 1);
#if PLATFORM_MAIXCAM
                // if(model.find("MaixCAM") != std::string::npos || model.find("LicheeRv Nano") != std::string::npos)
                // 包含 maixcam 或者 licheerv nano (不区分大小写，先把 model 转换为小写)
                std::string model_lower = model;
                std::transform(model.begin(), model.end(), model_lower.begin(), ::tolower);
                if (model_lower.find("maixcam") != std::string::npos || model_lower.find("licheerv nano") != std::string::npos)
                {
                    fclose(file);
                    auto it = _device_configs.find("name");
                    if (it != _device_configs.end()) {
                        _device_name = it->second;
                    }
                    else
                        _device_name = "MaixCAM";
                    return _device_name;
                }
#else
                fclose(file);
                return model;
#endif
            }
            fclose(file);
        }
        _device_name = model;
        return model;
    }

    std::string host_name()
    {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0)
        {
            return std::string(hostname);
        }
        return "";
    }

    std::string host_domain()
    {
        std::string host = host_name();
        if (host.empty())
            return "";
        return host + ".local";
    }

    std::map<std::string, std::string> ip_address()
    {
        // get all ip addresses of all network interfaces
        std::map<std::string, std::string> ips;
        struct ifaddrs *ifaddr, *ifa;
        int s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1)
        {
            log::error("getifaddrs failed");
            return ips;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if ((strcmp(ifa->ifa_name, "lo") != 0) && (ifa->ifa_addr->sa_family == AF_INET))
            {
                if (s != 0)
                {
                    log::error("getnameinfo() failed: %s", gai_strerror(s));
                    freeifaddrs(ifaddr);
                    return ips;
                }
                ips[ifa->ifa_name] = host;
            }
        }
        freeifaddrs(ifaddr);
        return ips;
    }

    std::map<std::string, std::string> mac_address()
    {
        struct ifaddrs *ifaddr, *ifa;
        std::map<std::string, std::string> result;

        if (getifaddrs(&ifaddr) == -1)
        {
            log::error("getifaddrs failed");
            exit(EXIT_FAILURE);
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            if ((strcmp(ifa->ifa_name, "lo") != 0) && (ifa->ifa_addr->sa_family == AF_INET))
            {
                int fd = socket(AF_INET, SOCK_DGRAM, 0);
                struct ifreq ifr;
                ifr.ifr_addr.sa_family = AF_INET;
                strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);

                if (ioctl(fd, SIOCGIFHWADDR, &ifr) != -1)
                {
                    char mac[18];
                    unsigned char *hwaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
                    snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
                    result[ifa->ifa_name] = mac;
                }

                close(fd);
            }
        }
        freeifaddrs(ifaddr);
        return result;
    }

    std::string device_key()
    {
        FILE *file = fopen("/device_key", "r");
        if (!file)
        {
            log::error("Cannot open /device_key");
            return "";
        }
        char line[128];
        if (fgets(line, sizeof(line), file))
        {
            fclose(file);
            // strip start and end space
            std::string key = line;
            key.erase(0, key.find_first_not_of(" \t\n"));
            key.erase(key.find_last_not_of(" \t\n") + 1);
            return key;
        }
        return "";
    }

    std::map<std::string, int> memory_info()
    {
        std::map<std::string, int> res;
        FILE *file = fopen("/proc/meminfo", "r");
        if (!file)
        {
            log::error("Cannot open /proc/meminfo");
            return res;
        }

        unsigned long total_memory = 0;
        unsigned long free_memory = 0;
        char line[256];

        while (fgets(line, sizeof(line), file))
        {
            if (sscanf(line, "MemTotal: %lu kB", &total_memory) == 1)
            {
                // printf("Total memory: %lu kB\n", total_memory);
            }
            if (sscanf(line, "MemAvailable: %lu kB", &free_memory) == 1)
            {
                // printf("MemAvailable memory: %lu kB\n", free_memory);
                break;
            }
        }

        fclose(file);

        res["used"] = (total_memory - free_memory) * 1024;
        res["total"] = total_memory * 1024;
#if PLATFORM_MAIXCAM
        res["hw_total"] = 256 * 1024 * 1024;
#else
        res["hw_total"] = res["total"];
#endif
        return res;
    }

    std::string bytes_to_human(unsigned long long bytes, int precision, int base, const std::string &unit, const std::string &sep)
    {
        const char *units[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
        size_t unit_index = 0;
        double bytes_double = static_cast<double>(bytes);

        while (bytes_double >= base && unit_index < sizeof(units) / sizeof(*units))
        {
            bytes_double /= base;
            ++unit_index;
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << bytes_double << sep << units[unit_index] << unit;
        return out.str();
    }

    std::map<std::string, unsigned long> cpu_freq()
    {
        std::map<std::string, unsigned long> res;
#if PLATFORM_MAIXCAM
        /* format:
                                 enable  prepare  protect                                duty
        clock                          count    count    count        rate   accuracy phase  cycle
        ---------------------------------------------------------------------------------------------
        audio_clock                          0        0        0    24576000          0     0  50000
        i2s_mclk                             2        2        0    24576000          0     0  50000
        eth_ptpclk                           1        1        0    50000000          0     0  50000
        */
        FILE *file = fopen("/sys/kernel/debug/clk/clk_summary", "r");
        if (!file)
        {
            perror("Cannot open /sys/kernel/debug/clk/clk_summary");
            return res;
        }
        // read lines, find clk_c906_0 and clk_tpu
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            char *idx = strstr(line, "clk_c906_0");
            if (idx)
            {
                unsigned long freq;
                if (sscanf(idx, "clk_c906_0 %*d %*d %*d %lu", &freq) == 1)
                {
                    res["cpu0"] = freq;
                    return res;
                }
            }
        }
#else
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        int count = 0;
        while (std::getline(cpuinfo, line))
        {
            if (line.substr(0, 9) == "processor")
            {
                ++count;
            }
            else if (line.substr(0, 7) == "cpu MHz")
            {
                size_t pos = line.find(':');
                if (pos != std::string::npos)
                {
                    unsigned long freq = std::stoul(line.substr(pos + 1)) * 1000000; // MHz to Hz
                    res["cpu" + std::to_string(count)] = freq;
                }
            }
        }
#endif
        return res;
    }

    std::map<std::string, float> cpu_temp()
    {
        std::map<std::string, float> res;
        FILE *file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        if (!file)
        {
            perror("Cannot open /sys/class/thermal/thermal_zone0/temp");
            return res;
        }
        int temp = 0;
        if (fscanf(file, "%d", &temp) == 1)
        {
            // printf("CPU temp: %d\n", temp);
        }
        fclose(file);
        res["cpu"] = temp / 1000.0;
        return res;
    }

    std::map<std::string, float> cpu_usage()
    {
        std::map<std::string, float> usage;

        std::ifstream proc_stat("/proc/stat");
        std::string line;
        while (std::getline(proc_stat, line))
        {
            std::istringstream iss(line);
            std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

            if (words[0].substr(0, 3) == "cpu")
            {
                long total_time = 0;
                for (size_t i = 1; i < words.size(); i++)
                    total_time += std::stol(words[i]);

                long idle_time = std::stol(words[4]); // idle time is 4th field
                float cpu_usage = 100 * (1 - (float)idle_time / total_time);

                usage[words[0]] = cpu_usage;
            }
        }

        return usage;
    }

    std::map<std::string, unsigned long> npu_freq()
    {
        std::map<std::string, unsigned long> res;
#if PLATFORM_MAIXCAM
        /* format:
                                 enable  prepare  protect                                duty
        clock                          count    count    count        rate   accuracy phase  cycle
        ---------------------------------------------------------------------------------------------
        audio_clock                          0        0        0    24576000          0     0  50000
        i2s_mclk                             2        2        0    24576000          0     0  50000
        eth_ptpclk                           1        1        0    50000000          0     0  50000
        */
        FILE *file = fopen("/sys/kernel/debug/clk/clk_summary", "r");
        if (!file)
        {
            perror("Cannot open /sys/kernel/debug/clk/clk_summary");
            return res;
        }
        // read lines, find clk_c906_0 and clk_tpu
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            char *idx = strstr(line, "clk_tpu");
            if (idx)
            {
                unsigned long freq;
                if (sscanf(idx, "clk_tpu %*d %*d %*d %lu", &freq) == 1)
                {
                    res["npu0"] = freq;
                    return res;
                }
            }
        }
#endif
        return res;
    }

    std::map<std::string, unsigned long long> disk_usage(const std::string &path)
    {
        std::map<std::string, unsigned long long> usage;
        struct statvfs stat;

        if (statvfs(path.c_str(), &stat) != 0)
        {
            // error happens, just return an empty map
            return usage;
        }
        usage["total"] = stat.f_blocks * stat.f_bsize;
        unsigned long long free = stat.f_bfree * stat.f_bsize;
        usage["used"] = usage["total"] - free;
        return usage;
    }

    std::vector<std::map<std::string, std::string>> disk_partitions(bool only_disk)
    {
        std::vector<std::map<std::string, std::string>> partitions;

        std::ifstream mounts("/proc/mounts");
        std::string line;
        while (std::getline(mounts, line))
        {
            std::istringstream iss(line);
            std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

            // If only_disk is true, only add entries where device starts with "/dev/"
            if (only_disk && words[0].substr(0, 5) != "/dev/")
            {
                continue;
            }

            std::map<std::string, std::string> partition;
            partition["device"] = words[0];
            partition["mountpoint"] = words[1];
            partition["fstype"] = words[2];

            partitions.push_back(partition);
        }

        return partitions;
    }

    void poweroff()
    {
        int ret = system("poweroff");
        if (ret != 0)
        {
            log::error("power off failed, ret: %d", ret);
            throw err::Exception(err::Err::ERR_RUNTIME, "power off failed");
        }
    }

    void reboot()
    {
        int ret = system("reboot");
        if (ret != 0)
        {
            log::error("reboot failed, ret: %d", ret);
            throw err::Exception(err::Err::ERR_RUNTIME, "reboot failed");
        }
    }

} // namespace maix::sys

/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_time.hpp"
#include "maix_fs.hpp"
#include "maix_log.hpp"

#include <time.h>
#include <algorithm>
#include <sstream>

// NTP
#include <utility>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <numeric>
#include <ntp_client.h>
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstring>

namespace maix::time
{

    static FPS fps_obj;

    double time()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec + ts.tv_nsec / 1000000000.0;
    }

    uint64_t time_ms()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    uint64_t time_us()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    }

    uint64_t time_s()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec;
    }

    double time_diff(double last, double now)
    {
        if (now < 0)
            now = time();
        return now - last;
    }

    double ticks_s()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec + ts.tv_nsec / 1000000000.0;
    }

    uint64_t ticks_ms()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    uint64_t ticks_us()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    }

    double ticks_diff(double last, double now)
    {
        if (now < 0)
            now = ticks_s();
        return now - last;
    }

    void sleep(double s)
    {
        struct timespec ts;
        ts.tv_sec = (time_t)s;
        ts.tv_nsec = (s - ts.tv_sec) * 1000000000;
        nanosleep(&ts, NULL);
    }

    void sleep_ms(uint64_t ms)
    {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }

    void sleep_us(uint64_t us)
    {
        struct timespec ts;
        ts.tv_sec = us / 1000000;
        ts.tv_nsec = (us % 1000000) * 1000;
        nanosleep(&ts, NULL);
    }

    void fps_start()
    {
        fps_obj.start();
    }

    float fps()
    {
        return fps_obj.fps();
    }

    void fps_set_buff_len(int len)
    {
        fps_obj.set_buff_len(len);
    }

    DateTime *now()
    {
        time_t t = ::time(NULL);
        return gmtime(t);
    }

    DateTime *localtime()
    {
        time_t t = ::time(NULL);
        struct tm lt = {0};
        struct tm *tm = ::localtime_r(&t, &lt);
        DateTime *dt = new DateTime;
        dt->year = tm->tm_year + 1900;
        dt->month = tm->tm_mon + 1;
        dt->day = tm->tm_mday;
        dt->hour = tm->tm_hour;
        dt->minute = tm->tm_min;
        dt->second = tm->tm_sec;
        dt->yearday = tm->tm_yday;
        dt->weekday = tm->tm_wday;
        dt->zone = tm->tm_gmtoff / 3600.0;
        dt->zone_name = std::string(tm->tm_zone);
        return dt;
    }

    DateTime *strptime(const std::string &str, const std::string &format)
    {
        DateTime *dt = new DateTime;
        struct tm tm;
        ::strptime(str.c_str(), format.c_str(), &tm);
        dt->year = tm.tm_year + 1900;
        dt->month = tm.tm_mon + 1;
        dt->day = tm.tm_mday;
        dt->hour = tm.tm_hour;
        dt->minute = tm.tm_min;
        dt->second = tm.tm_sec;
        dt->yearday = tm.tm_yday;
        dt->weekday = tm.tm_wday;
        return dt;
    }

    time::DateTime *gmtime(double timestamp)
    {
        time_t t = (time_t)timestamp;
        struct tm *tm = ::gmtime(&t);
        DateTime *dt = new DateTime;
        dt->year = tm->tm_year + 1900;
        dt->month = tm->tm_mon + 1;
        dt->day = tm->tm_mday;
        dt->hour = tm->tm_hour;
        dt->minute = tm->tm_min;
        dt->second = tm->tm_sec;
        dt->yearday = tm->tm_yday;
        dt->weekday = tm->tm_wday;
        dt->zone = 0;
        dt->zone_name = std::string("UTC");
        return dt;
    }

    std::string DateTime::strftime(const std::string &format)
    {
        char buf[128];
        struct tm tm;
        tm.tm_year = this->year - 1900;
        tm.tm_mon = this->month - 1;
        tm.tm_mday = this->day;
        tm.tm_hour = this->hour;
        tm.tm_min = this->minute;
        tm.tm_sec = this->second;
        tm.tm_yday = this->yearday;
        tm.tm_wday = this->weekday;
        tm.tm_isdst = 0;
        tm.tm_gmtoff = this->zone * 3600;
        tm.tm_zone = (char *)this->zone_name.c_str();
        ::strftime(buf, sizeof(buf), format.c_str(), &tm);
        return std::string(buf);
    }

    double DateTime::timestamp()
    {
        struct tm tm;
        tm.tm_year = this->year - 1900;
        tm.tm_mon = this->month - 1;
        tm.tm_mday = this->day;
        tm.tm_hour = this->hour;
        tm.tm_min = this->minute;
        tm.tm_sec = this->second;
        tm.tm_isdst = 0;
        tm.tm_gmtoff = this->zone * 3600;
        tm.tm_zone = (char *)this->zone_name.c_str();
        return ::mktime(&tm);
    }

    std::string timezone(const std::string &timezone)
    {
        std::string res;
        if (!timezone.empty())
        {
            fs::File *f = fs::open("/etc/timezone", "w");
            if (!f)
            {
                log::error("write /etc/timezone failed");
                return "";
            }
            f->write(timezone.c_str(), (int)timezone.size());
            f->close();
            delete f;
            // rm -f /etc/localtime && ln -s /usr/share/zoneinfo/{timezone} /etc/localtime
            fs::symlink("/usr/share/zoneinfo/" + timezone, "/etc/localtime", true);
            fs::sync();
        }
        fs::File *f = fs::open("/etc/timezone", "r");
        if (!f)
        {
            log::error("read /etc/timezone failed");
            return "";
        }
        std::string *line = f->readline();
        res = *line;
        delete line;
        f->close();
        delete f;
        return res;
    }

    std::vector<std::string> timezone2(const std::string &region, const std::string &city)
    {
        std::vector<std::string> final;
        std::string locale_str;
        if (!region.empty() && !city.empty())
            locale_str = region + "/" + city;
        std::string res = time::timezone(locale_str);
        if (res.empty())
            return final;
        // Split region/city string to region and city
        std::stringstream ss(res);
        std::string item;
        while (std::getline(ss, item, '/'))
        {
            // Remove trailing carriage return ('\r') and newline ('\n') if present
            if(!item.empty())
            {
                if ((item.back() == '\r' || item.back() == '\n'))
                {
                    item.erase(item.find_last_not_of("\r\n") + 1);
                }
                final.push_back(item);
            }
        }
        return final;
    }

    /**
     * List all timezone info
     * @return A dict with key are regions, and value are region's cities.
     * @maixpy maix.time.list_timezones
     */
    std::map<std::string, std::vector<std::string>> list_timezones()
    {
        // get all regions from /usr/share/zoneinfo
        std::map<std::string, std::vector<std::string>> res;
        std::vector<std::string> *dirs = fs::listdir("/usr/share/zoneinfo");
        if (!dirs)
        {
            return res;
        }
        std::sort(dirs->begin(), dirs->end());
        for (std::string dir : *dirs)
        {
            if (fs::isdir("/usr/share/zoneinfo/" + dir))
            {
                std::vector<std::string> *cities = fs::listdir("/usr/share/zoneinfo/" + dir);
                if (cities)
                {
                    std::sort(cities->begin(), cities->end());
                    res[dir] = *cities;
                    delete cities;
                }
            }
        }
        delete dirs;
        return res;
    }

    namespace ntp::priv {

    static const char* TAG = "MAIX TIME NTP";

    class FileHandler {
    public:
        FileHandler() = delete;
        FileHandler(const FileHandler&) = delete;
        FileHandler& operator=(const FileHandler&) = delete;
        ~FileHandler() = delete;

        static bool file_exists(const std::string& filename) {
            std::ifstream file(filename);
            return file.good();
        }

        static bool is_path_valid(const std::string& path) {
            if (path.empty()) {
                maix::log::error("Path is empty.");
                return false;
            }
            // 检查路径中是否包含非法字符
            if (path.find_first_of("\\:*?\"<>|") != std::string::npos) {
                maix::log::error("Path contains invalid characters.");
                return false;
            }
            return true;
        }
    };

    struct Config {
        int retry;
        int total_timeout_ms;
    };

    struct NtpServer {
        std::string host;
        int port;
    };

    template<typename First, typename Second>
    static constexpr std::pair<First, Second> empty()
    {
        return {First{}, Second{}};
    }

    template<typename T>
    static constexpr void remake_value(const char* name, T& value, T v_min, T v_max)
    {
        if (value < v_min) {
            value = v_min;
            return;
        }
        if (value > v_max) {
            value = v_max;
        }
    }

    static std::pair<Config, std::vector<NtpServer>> load_conf(const std::string& path)
    {
        if (!FileHandler::file_exists(path)) {
            maix::log::error("[%s] Cannot find config file with path: %s", priv::TAG, path.c_str());
            return empty<Config, std::vector<NtpServer>>();
        }
        try {
            YAML::Node config = YAML::LoadFile(path);

            // 解析 g_config
            Config g_config;
            if (config["Config"]) {
                g_config.retry = config["Config"][0]["retry"].as<int>();
                g_config.total_timeout_ms = config["Config"][1]["total_timeout_ms"].as<int>();
                remake_value("Config::retry", g_config.retry, 1, std::numeric_limits<int>::max());
                remake_value("Config::total_timeout_ms", g_config.total_timeout_ms, 0, std::numeric_limits<int>::max());
            } else {
                maix::log::error("[%s] Config file has not <Config>!!!", priv::TAG);
                return empty<Config, std::vector<NtpServer>>();
            }

            maix::log::info("[%s] Get Config {retry:%d;total timeout(ms):%d;}", priv::TAG, g_config.retry, g_config.total_timeout_ms);

            // 解析 NtpServers
            std::vector<NtpServer> ntpServers;
            if (config["NtpServers"]) {
                for (const auto& serverNode : config["NtpServers"]) {
                    NtpServer server;
                    server.host = serverNode["host"].as<std::string>();
                    server.port = (!serverNode["port"]) ? 123 : serverNode["port"].as<int>();
                    remake_value("NtpServer::port", server.port, 0, 65535);
                    ntpServers.push_back(server);
                }
            }

            // std::cout << "NtpServers:" << std::endl;
            // for (const auto& server : ntpServers) {
            //     std::cout << "  Host: " << server.host << ", Port: " << server.port << std::endl;
            // }

            return std::pair<Config, std::vector<NtpServer>>{g_config, ntpServers};

        } catch (const YAML::Exception& e) {
            // std::cerr << "YAML parsing error: " << e.what() << std::endl;
            maix::log::error("[%s] YAML parsing error: %s", priv::TAG, e.what());
        } catch (const std::exception& e) {
            // std::cerr << "Error: " << e.what() << std::endl;
            maix::log::error("[%s] Error: %s", priv::TAG, e.what());
        }
        return empty<Config, std::vector<NtpServer>>();
    }

    }

    template<typename T>
    static constexpr std::vector<T> empty_timetuple()
    {
        return {};
    }

    static std::vector<int> req_time(xntp_cliptr_t obj, x_uint32_t timeout, bool full)
    {
        xtime_vnsec_t xtm_vnsec = ntpcli_req_time(obj, timeout);
        if (!XTMVNSEC_IS_VALID(xtm_vnsec)) {
            maix::log::error("[%s] ntpcli_req_time failed. errno : %d\n", ntp::priv::TAG, errno);
            return empty_timetuple<int>();
        }
        // auto xtm_ltime = time_vnsec();
        auto xtm_descr = time_vtod(xtm_vnsec);
        // auto xtm_local = time_vtod(xtm_ltime);
        if (!full)
            return {static_cast<int>(xtm_descr.ctx_year),
                    static_cast<int>(xtm_descr.ctx_month),
                    static_cast<int>(xtm_descr.ctx_day),
                    static_cast<int>(xtm_descr.ctx_hour),
                    static_cast<int>(xtm_descr.ctx_minute),
                    static_cast<int>(xtm_descr.ctx_second)};
        return {static_cast<int>(xtm_descr.ctx_year),
                    static_cast<int>(xtm_descr.ctx_month),
                    static_cast<int>(xtm_descr.ctx_day),
                    static_cast<int>(xtm_descr.ctx_week),
                    static_cast<int>(xtm_descr.ctx_hour),
                    static_cast<int>(xtm_descr.ctx_minute),
                    static_cast<int>(xtm_descr.ctx_second),
                    static_cast<int>(xtm_descr.ctx_msec)};
    }

    std::vector<int> ntp_timetuple(std::string host, int port, uint8_t retry, int timeout_ms)
    {
        // std::endl(std::cout);
        // maix::log::info("[%s] Host: %s\t\tPort: %d", priv::TAG, host.c_str(), port);
        // std::endl(std::cout);

        ntp::priv::remake_value("timeout_ms", timeout_ms, 0, std::numeric_limits<int>::max());
        timeout_ms = timeout_ms<0 ? 0 : timeout_ms;
        port = port==-1 ? (maix::log::info("[%s] used default port: 123", ntp::priv::TAG), 123) : port;
        xntp_cliptr_t xntp_this = ntpcli_open();
        if (X_NULL == xntp_this) {
            maix::log::error("[%s] ntpcli_open() failed, errno : %d\n", ntp::priv::TAG, errno);
            return empty_timetuple<int>();
        }

        if (ntpcli_config(xntp_this, host.c_str(), static_cast<x_uint16_t>(port))) {
            maix::log::error("[%s] ntpcli_config() failed!", ntp::priv::TAG);
            return empty_timetuple<int>();
        }

        for (uint8_t i = 0; i < retry; ++i) {
            auto r = req_time(xntp_this, static_cast<x_uint32_t>(timeout_ms), false);
            if (!r.empty()) {
                if (X_NULL != xntp_this) {
                    ntpcli_close(xntp_this);
                    xntp_this = X_NULL;
                }
                return r;
            }
        }

        if (X_NULL != xntp_this) {
            ntpcli_close(xntp_this);
            xntp_this = X_NULL;
        }

        return empty_timetuple<int>();
    }

    std::vector<int> ntp_timetuple_with_config(std::string path)
    {
        auto result = ntp::priv::load_conf(path);
        if (result.second.empty()) return empty_timetuple<int>();

        int retry = result.first.retry;
        ntp::priv::remake_value("retry", retry, 1, std::numeric_limits<int>::max());
        int total_timeout = result.first.total_timeout_ms;
        ntp::priv::remake_value("total_timeout", total_timeout, 0, std::numeric_limits<int>::max());
        std::size_t server_number = result.second.size();
        int avg_timeout = total_timeout / static_cast<int>(server_number) / retry;

        for (auto& item : result.second) {
            auto r = ntp_timetuple(item.host, item.port, retry, avg_timeout);
            if (!r.empty()) return r;
        }
        return empty_timetuple<int>();
    }


    std::vector<int> ntp_sync_sys_time(std::string host, int port, uint8_t retry, int timeout_ms)
    {
        port = port==-1 ? (maix::log::info("[%s] used default port: 123", ntp::priv::TAG), 123) : port;
        ntp::priv::remake_value("timeout_ms", timeout_ms, 0, std::numeric_limits<int>::max());
        xtime_vnsec_t vnsec = ntpcli_get_time(host.c_str(), static_cast<x_uint16_t>(port),
                        static_cast<x_uint32_t>(timeout_ms));
        if (!XTMVNSEC_IS_VALID(vnsec)) {
            // maix::log::error("[%s] ntpcli_get_time failed. errno : %d\n", ntp::prisv::TAG, errno);
            maix::log::warn("[%s] Try to get time from %s:%d failed! errno: %d", ntp::priv::TAG, host.c_str(), port, errno);
            return empty_timetuple<int>();
        }

        auto t = time_vtod(vnsec);

        struct tm tm_time;
        tm_time.tm_year = t.ctx_year - 1900;
        tm_time.tm_mon = t.ctx_month - 1;
        tm_time.tm_mday = t.ctx_day;
        tm_time.tm_hour = t.ctx_hour;
        tm_time.tm_min = t.ctx_minute;
        tm_time.tm_sec = t.ctx_second;
        tm_time.tm_isdst = -1;

        time_t sys_time = mktime(&tm_time);

        struct timeval tv;
        tv.tv_sec = sys_time;
        tv.tv_usec = t.ctx_msec * 1000; // 毫秒转换为微秒

        if (settimeofday(&tv, nullptr) == -1) {
            maix::log::error("Failed to set system time. errno<%d>: %s", errno, std::strerror(errno));
            return empty_timetuple<int>();
        }
        maix::log::info("System time set successfully");

        return {
            static_cast<int>(t.ctx_year),
            static_cast<int>(t.ctx_month),
            static_cast<int>(t.ctx_day),
            // static_cast<int>(t.ctx_week),
            static_cast<int>(t.ctx_hour),
            static_cast<int>(t.ctx_minute),
            static_cast<int>(t.ctx_second)
            // static_cast<int>(t.ctx_msec)
        };
    }

    std::vector<int> ntp_sync_sys_time_with_config(std::string path)
    {
        auto result = ntp::priv::load_conf(path);
        if (result.second.empty()) return empty_timetuple<int>();

        int retry = result.first.retry;
        ntp::priv::remake_value("retry", retry, 1, std::numeric_limits<int>::max());
        int total_timeout = result.first.total_timeout_ms;
        ntp::priv::remake_value("total_timeout", total_timeout, 0, std::numeric_limits<int>::max());
        std::size_t server_number = result.second.size();
        int avg_timeout = total_timeout / static_cast<int>(server_number) / retry;

        for (auto& item : result.second) {
            auto r = ntp_sync_sys_time(item.host, item.port, retry, avg_timeout);
            if (!r.empty()) {
                maix::log::info("[%s] Get time from %s:%d succ.", ntp::priv::TAG, item.host.c_str(), item.port);
                return r;
            }
        }

        return empty_timetuple<int>();
    }
} // namespace maix::time

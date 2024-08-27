#include "maix_ntp.hpp"
#include <utility>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <numeric>
#include "ntp_client.h"
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstring>

namespace maix::ext_dev::ntp::priv {

static const char* TAG = "MAIX NTP";

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
        maix::log::info0("[%s] value{%s} err. Reset it to ", TAG, name);
        std::cout << v_min << std::endl;
        return;
    }
    if (value > v_max) {
        value = v_max;
        maix::log::info0("[%s] value{%s} err. Reset it to ", TAG, name);
        std::cout << v_max << std::endl;
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




namespace maix::ext_dev::ntp {

template<typename T>
static constexpr std::vector<T> empty_timetuple()
{
    return {};
}

static std::vector<int> req_time(xntp_cliptr_t obj, x_uint32_t timeout, bool full)
{
    xtime_vnsec_t xtm_vnsec = ntpcli_req_time(obj, timeout);
    if (!XTMVNSEC_IS_VALID(xtm_vnsec)) {
        maix::log::error("[%s] ntpcli_req_time failed. errno : %d\n", priv::TAG, errno);
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

std::vector<int> time(std::string host, int port, uint8_t retry, int timeout_ms)
{
    // std::endl(std::cout);
    // maix::log::info("[%s] Host: %s\t\tPort: %d", priv::TAG, host.c_str(), port);
    // std::endl(std::cout);

    priv::remake_value("timeout_ms", timeout_ms, 0, std::numeric_limits<int>::max());
    timeout_ms = timeout_ms<0 ? 0 : timeout_ms;
    port = port==-1 ? (maix::log::info("[%s] used default port: 123", priv::TAG), 123) : port;
    xntp_cliptr_t xntp_this = ntpcli_open();
    if (X_NULL == xntp_this) {
        maix::log::error("[%s] ntpcli_open() failed, errno : %d\n", priv::TAG, errno);
        return empty_timetuple<int>();
    }

    if (ntpcli_config(xntp_this, host.c_str(), static_cast<x_uint16_t>(port))) {
        maix::log::error("[%s] ntpcli_config() failed!", priv::TAG);
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

std::vector<int> time_with_config(std::string path)
{
    auto result = priv::load_conf(path);
    if (result.second.empty()) return empty_timetuple<int>();

    int retry = result.first.retry;
    priv::remake_value("retry", retry, 1, std::numeric_limits<int>::max());
    int total_timeout = result.first.total_timeout_ms;
    priv::remake_value("total_timeout", total_timeout, 0, std::numeric_limits<int>::max());
    std::size_t server_number = result.second.size();
    int avg_timeout = total_timeout / static_cast<int>(server_number) / retry;

    for (auto& item : result.second) {
        auto r = time(item.host, item.port, retry, avg_timeout);
        if (!r.empty()) return r;
    }
    return empty_timetuple<int>();
}


std::vector<int> sync_sys_time(std::string host, int port, uint8_t retry, int timeout_ms)
{
    port = port==-1 ? (maix::log::info("[%s] used default port: 123", priv::TAG), 123) : port;
    priv::remake_value("timeout_ms", timeout_ms, 0, std::numeric_limits<int>::max());
    xtime_vnsec_t vnsec = ntpcli_get_time(host.c_str(), static_cast<x_uint16_t>(port),
                    static_cast<x_uint32_t>(timeout_ms));
    if (!XTMVNSEC_IS_VALID(vnsec)) {
        maix::log::error("[%s] ntpcli_get_time failed. errno : %d\n", priv::TAG, errno);
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

std::vector<int> sync_sys_time_with_config(std::string path)
{
    auto result = priv::load_conf(path);
    if (result.second.empty()) return empty_timetuple<int>();

    int retry = result.first.retry;
    priv::remake_value("retry", retry, 1, std::numeric_limits<int>::max());
    int total_timeout = result.first.total_timeout_ms;
    priv::remake_value("total_timeout", total_timeout, 0, std::numeric_limits<int>::max());
    std::size_t server_number = result.second.size();
    int avg_timeout = total_timeout / static_cast<int>(server_number) / retry;

    for (auto& item : result.second) {
        auto r = sync_sys_time(item.host, item.port, retry, avg_timeout);
        if (!r.empty()) return r;
    }
    return empty_timetuple<int>();
}


}
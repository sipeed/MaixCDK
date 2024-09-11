
#include "maix_basic.hpp"
#include "main.h"
#include "maix_bm8563.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <libgen.h>
#include <sys/time.h>
#include "cxxopts.hpp"

#define _NTP_USE_CONFIG_FILE_ false

static constexpr uint8_t BM8563_I2CBUS_NUM = 4;
static constexpr uint64_t LOG_FILE_MAX_BYTES = 4 * 1024 * 1024; //4MB

// #if _NTP_USE_CONFIG_FILE_
static const char* LOG_FILE_PATH = "./assets/ntp_config.yaml";
// #else
static const std::map<std::string, int> ntp_hosts {
    {"ntp.tencent.com",     123},
    {"ntp1.tencent.com",    123},
    {"ntp2.tencent.com",    123},
    {"ntp3.tencent.com",    123},
    {"ntp4.tencent.com",    123},
    {"ntp5.tencent.com",    123},
    {"ntp1.aliyun.com",     123},
    {"ntp2.aliyun.com",     123},
    {"ntp3.aliyun.com",     123},
    {"ntp4.aliyun.com",     123},
    {"ntp5.aliyun.com",     123},
    {"ntp6.aliyun.com",     123},
    {"ntp7.aliyun.com",     123},
    {"time.edu.cn",         123},
    {"s2c.time.edu.cn",     123},
    {"s2f.time.edu.cn",     123},
    {"s2k.time.edu.cn",     123},
    {"pool.ntp.org",        123},
    {"time.nist.gov",       123},
    {"time.windows.com",    123}
};
// #endif

static const std::map<std::string, std::string> addrs = {
    {"www.sipeed.com",  "80"},
    {"wiki.sipeed.com", "80"},
    {"www.baidu.com",   "80"},
    {"www.google.com",  "80"}
};

using namespace maix;

class FileHandler {
public:
    FileHandler() = delete;
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    ~FileHandler() = delete;

    static bool remove(const std::string& filename) {
        bool result = std::remove(filename.c_str()) == 0;
        return result;
    }

    static bool copy(const std::string& src_filename, const std::string& dest_filename) {
        std::ifstream src(src_filename, std::ios::binary);
        std::ofstream dest(dest_filename, std::ios::binary);

        if (!src || !dest) {
            return false;
        }

        dest << src.rdbuf();
        return src.good() && dest.good();
    }

    static std::streamsize size(const std::string& filename) {
        std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
        if (!file.is_open())
            return -1;
        std::streamsize size = file.tellg();
        file.close();
        return size;
    }
};

static bool is_internet_connected(const std::string& network_addr, const std::string& port) {
    struct addrinfo hints, *res;
    int status;

    ::memset(&hints, 0x00, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = ::getaddrinfo(network_addr.c_str(), port.c_str(), &hints, &res)) != 0) {
        return false;
    }

#if 0
    char ipstr[INET6_ADDRSTRLEN];
    struct addrinfo* p = nullptr;
    for (p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        ::inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << "Resolved IP: " << ipstr << std::endl; // 打印解析出的IP地址
    }
#endif

    ::freeaddrinfo(res);
    return true;
}

static bool change_working_directory_to_executable_path()
{
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        maix::log::info("Failed to read /proc/self/exe");
        return false;
    }
    path[len] = '\0';
    char* dir = dirname(path);
    if (chdir(dir) == 0) {
        maix::log::info("Working directory changed to: %s", dir);
        return true;
    }
    maix::log::error("Failed to change working directory to: %s", dir);
    return false;
}


static bool is_internet_connected(const std::map<std::string, std::string>& addr_infos)
{
    for (const auto& item : addr_infos) {
        if (is_internet_connected(item.first, item.second)) return true;
        // is_internet_connected(item.first, item.second);
    }
    return false;
}

/**
 * @brief Get the time string from system object.
 *
 * @return std::string type. "2024-08-26 14:12:16.319"
 */
std::string get_time_string_from_system(void)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << now_ms.count();

    return oss.str();
}

// #if _NTP_USE_CONFIG_FILE_ == false
static bool ntp_sync_from_map(const std::map<std::string, int>& m)
{
    for (const auto& item : m) {
        maix::log::info("[%s] NTP Try: host<%s> port<%d>", get_time_string_from_system().c_str(), item.first.c_str(), item.second);
        auto ret = maix::time::ntp_sync_sys_time(item.first, item.second, 3, 100);
        if (!ret.empty()) return true;
    }
    return false;
}
// #endif

static bool file_is_exists(std::string path)
{
    // std::fstream file(path, std::ios::in);
    // return file.good();
    return std::fstream(path, std::ios::in).good();
}

static bool sync_sys(bool use_config_file, int& retry)
{
    // #if _NTP_USE_CONFIG_FILE_
    if (use_config_file) {
        auto ret = time::ntp_sync_sys_time_with_config(LOG_FILE_PATH);
        if (!ret.empty()) return true;
        maix::log::error("[%s][%d] Sync NTP -> system FAILED!!!", get_time_string_from_system().c_str(), retry);
        ++retry;
        return false;
    }
    // #else
    if (ntp_sync_from_map(ntp_hosts))
        return true;
    maix::log::error("[%s][%d] Sync NTP -> system FAILED!!!", get_time_string_from_system().c_str(), retry);
    ++retry;
    return false;
    // #endif
}

static void setup(int argc, char** argv)
{
    if (setsid() < 0) {
        maix::log::error("setsid failed!");
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    std::string filename(argv[0]);
    filename = "./" + filename + ".log";

    std::streamsize filesize = FileHandler::size(filename);
    if (filesize < 0 || LOG_FILE_MAX_BYTES <= static_cast<uint64_t>(filesize))
        FileHandler::remove(filename);

    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        maix::log::error("Failed to open log file!");
        exit(1);
    }
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
}


static void deamon()
{
    namespace drv = maix::ext_dev;

    auto ltime = maix::time::ticks_ms();

    printf("\n\n\n");
    maix::log::info("----------------------------------------");

    if (!change_working_directory_to_executable_path()) return;

    bool use_config_file = file_is_exists(LOG_FILE_PATH);
    (void)((use_config_file) ?
        (maix::log::info("[%s] Use config file: %s", get_time_string_from_system().c_str(), LOG_FILE_PATH), use_config_file) :
        (maix::log::info("[%s] Use static config."), use_config_file));

    maix::log::info("[%s] Init BM5863 START...", get_time_string_from_system().c_str());
    drv::bm8563::BM8563 rtc(BM8563_I2CBUS_NUM);
    maix::log::info("[%s] Init BM8653 FINISH...", get_time_string_from_system().c_str());

    static constexpr int MAX_RETRY_NUM = 10;
    int retry = 0;
    for (; retry < MAX_RETRY_NUM;) {
        maix::log::info("[%s][%d] Sync RTC -> system", get_time_string_from_system().c_str(), retry);
        if (err::Err::ERR_NONE != rtc.hctosys()) {
            maix::log::error("[%s][%d] Sync RTC -> system FAILED!!!", get_time_string_from_system().c_str(), retry);
            ++retry;
            continue;
        }
        uint8_t print_flag = 0;
        const uint8_t print_flag_max = 3;
        while (!is_internet_connected(addrs)) {
            if (++print_flag >= print_flag_max) {
                print_flag = 0;
                maix::log::info("[%s] Waiting for network connect...", get_time_string_from_system().c_str());
            }
            maix::time::sleep(1);
        }
        maix::log::info("[%s] Network connected...", get_time_string_from_system().c_str());

        if (!sync_sys(use_config_file, retry)) continue;

        maix::log::info("[%s] Sync NTP -> system Succ...", get_time_string_from_system().c_str());
        if (err::Err::ERR_NONE != rtc.systohc()) {
            maix::log::error("[%s][%d] Sync system -> RTC FAILED!!!", get_time_string_from_system().c_str(), retry);
            ++retry;
            continue;
        }
        maix::log::info("[%s] Sync system -> RTC Succ...", get_time_string_from_system().c_str());
        maix::log::info("[%s] Sync FINISH...", get_time_string_from_system().c_str());
        break;
    }
    if (retry >= MAX_RETRY_NUM)
        maix::log::error("[%s] ALL retry FAILED!!!", get_time_string_from_system().c_str());
    auto rtime = maix::time::ticks_ms() - ltime;
    maix::log::info("[%s] EXIT, total time: %llu ms", get_time_string_from_system().c_str(), rtime);
    maix::log::info("----------------------------------------\n\n\n");
}

void print_sys_time(void)
{
    struct timeval tv;
    struct tm *tm;

    // 获取当前时间
    gettimeofday(&tv, NULL);

    // 将时间转换为本地时间
    tm = localtime(&tv.tv_sec);

    // 打印时间，精确到毫秒
    maix::log::info("当前时间: %04d-%02d-%02d %02d:%02d:%02d.%03ld",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec,
           tv.tv_usec / 1000); // 微秒转换为毫秒

}

int check(void)
{
    namespace drv = maix::ext_dev;
    printf("\n\n\n");
    maix::log::info("----------------------------------------");

    if (!change_working_directory_to_executable_path()) return -1;
    maix::log::info("[%s] Init BM5863 START...", get_time_string_from_system().c_str());
    drv::bm8563::BM8563 rtc(BM8563_I2CBUS_NUM);
    maix::log::info("[%s] Init BM8653 FINISH...", get_time_string_from_system().c_str());

    while (!is_internet_connected(addrs)) {
        maix::log::info("[%s] Waiting for network connect...", get_time_string_from_system().c_str());
        maix::time::sleep(3);
    }
    maix::log::info("[%s] Network connected...", get_time_string_from_system().c_str());

    int retry = 0;
    while (!sync_sys(false, retry));

    maix::log::info("[%s] Sync NTP -> system Succ...", get_time_string_from_system().c_str());

    // while (!app::need_exit()) {
    //     // ::system("date");
    //     maix::log::info("");
    //     maix::log::info("-----------");
    //     auto rt = rtc.datetime();
    //     if (rt.empty()) continue;
    //     maix::log::info("%d-%d-%d %d:%d:%d", rt[0], rt[1], rt[2], rt[3], rt[4], rt[5]);
    //     print_sys_time();
    //     maix::log::info("-----------");
    //     maix::time::sleep_ms(1000);
    // }

    // app::set_exit_flag(false);

    int prev_s = 0;
    {
        auto rt = rtc.datetime();
        prev_s = rt[5];
    }
    while (!app::need_exit()) {
        auto rt = rtc.datetime();
        if (rt.empty()) continue;
        if (rt[5] != prev_s) {
            prev_s = rt[5];
            maix::log::info("%d-%d-%d %d:%d:%d", rt[0], rt[1], rt[2], rt[3], rt[4], rt[5]);
            print_sys_time();
        }
    }

    return 0;
}

int _main(int argc, char* argv[])
{
    if (argc >= 2) {
        cxxopts::Options options(argv[0], "Sync NTP->SystemTime->RTC[bm8658]");
        options.add_options()
            ("d,debug", "Enable debugging")
            ("h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (result["debug"].as<bool>()) {
            return check();
        }
        return 0;
    }

    pid_t pid = fork();

    if (pid < 0) {
        maix::log::error("Start Failed...");
        return 1;
    } else if (pid > 0) {
        // 父进程
        // std::cout << "Parent process exiting, child process PID: " << pid << std::endl;
        // maix::time::sleep(1);
        maix::time::sleep_ms(50);
        exit(0); // 父进程退出
    } else {
        setup(argc, argv);
        deamon();
        return 0;
    }
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    // sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}



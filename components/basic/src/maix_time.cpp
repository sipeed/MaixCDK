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

namespace maix::time
{

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

} // namespace maix::time

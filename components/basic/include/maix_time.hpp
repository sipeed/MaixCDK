/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

namespace maix::time
{
    /**
     * Get current time in s
     * @return current time in s, double type
     * @attention If board have no RTC battery, when bootup and connect to network,
     * system will automatically sync time by NTP, will cause time() have big change,
     * e.g. before NTP: 10(s), after: 1718590639.5149617(s).
     * If you want to calculate time interval, please use ticks_s().
     * @maixpy maix.time.time
    */
    double time();

    /**
     * Get current time in ms
     * @return current time in ms, uint64_t type
     * @attention If board have no RTC battery, when bootup and connect to network,
     * system will automatically sync time by NTP, will cause time() have big change,
     * e.g. before NTP: 10000(ms), after: 1718590639000(ms)
     * If you want to calculate time interval, please use ticks_ms().
     * @maixpy maix.time.time_ms
    */
    uint64_t time_ms();

    /**
     * Get current time in s
     * @return current time in s, uint64_t type
     * @attention If board have no RTC battery, when bootup and connect to network,
     * system will automatically sync time by NTP, will cause time() have big change,
     * e.g. before NTP: 10(s), after: 1718590639(s)
     * @maixpy maix.time.time_s
    */
    uint64_t time_s();

    /**
     * Get current time in us
     * @return current time in us, uint64_t type
     * @attention If board have no RTC battery, when bootup and connect to network,
     * system will automatically sync time by NTP, will cause time() have big change,
     * e.g. before NTP: 10000000(us), after: 1718590639000000(s)
     * If you want to calculate time interval, please use ticks_us().
     * @maixpy maix.time.time_us
    */
    uint64_t time_us();

    /**
     * Calculate time difference in s.
     * @param last last time
     * @param now current time, can be -1 if use current time
     * @return time difference
     * @attention If board have no RTC battery, when bootup and connect to network,
     * system will automatically sync time by NTP, will cause time() have big change, and lead to big value.
     * e.g. before NTP: 1(s), after: 1718590500(s)
     * If you want to calculate time interval, please use ticks_diff().
     * @maixpy maix.time.time_diff
    */
    double time_diff(double last, double now = -1);


    /**
     * Get current time in s since bootup
     * @return current time in s, double type
     * @maixpy maix.time.ticks_s
    */
    double ticks_s();

    /**
     * Get current time in ms since bootup
     * @return current time in ms, uint64_t type
     * @maixpy maix.time.ticks_ms
    */
    uint64_t ticks_ms();

    /**
     * Get current time in us since bootup
     * @return current time in us, uint64_t type
     * @maixpy maix.time.ticks_us
    */
    uint64_t ticks_us();

    /**
     * Calculate time difference in s.
     * @param last last time
     * @param now current time, can be -1 if use current time
     * @return time difference
     * @maixpy maix.time.ticks_diff
    */
    double ticks_diff(double last, double now = -1);

    /**
     * Sleep seconds
     * @param s seconds, double type
     * @maixpy maix.time.sleep
    */
    void sleep(double s);

    /**
     * Sleep milliseconds
     * @param ms milliseconds, uint64_t type
     * @maixpy maix.time.sleep_ms
    */
    void sleep_ms(uint64_t ms);

    /**
     * Sleep microseconds
     * @param us microseconds, uint64_t type
     * @maixpy maix.time.sleep_us
    */
    void sleep_us(uint64_t us);

    /**
     * Calculate FPS since last call this method.
     * Attention, this method is not multi thread safe, only call this method in one threads.
     * If you want to use in multi threads, please use time.FPS class.
     * FPS is average value of recent n(buff_len) times, and you can call fps_set_buff_len(10) to change buffer length, default is 20.
     * Multiple invoke this function will calculate fps between two invoke, and you can also call fps_start() fisrt to manually assign fps calulate start point.
     * @return float type, current fps since last call this method
     * @maixpy maix.time.fps
    */
    float fps();

    /**
     * Manually set fps calculation start point, then you can call fps() function to calculate fps between fps_start() and fps().
     * @maixpy maix.time.fps_start
    */
    void fps_start();

    /**
     * Set fps method buffer length, by default the buffer length is 10.
     * @param len Buffer length to store recent fps value.
     * @maixpy maix.time.fps_set_buff_len
    */
    void fps_set_buff_len(int len);


    /**
     * FPS class to use average filter to calculate FPS.
     * @maixpy maix.time.FPS
    */
    class FPS
    {
    public:
        /**
         * FPS class constructor
         * @param buff_len Average buffer length, default 20, that is, fps() function will return the average fps in recent buff_len times fps.
         * @maixpy maix.time.FPS.__init__
         * @maixcdk maix.time.FPS.FPS
        */
        FPS(int buff_len = 20)
        {
            _fps_buff.resize(buff_len);
            _last = 0;
            _fps_init = false;
            _t_last = 0;
            _idx = 0;
        }

        ~FPS()
        {
        }

        /**
         * Manually set fps calculation start point, then you can call fps() function to calculate fps between start() and fps().
         * @maixpy maix.time.FPS.start
        */
        void start()
        {
            _t_last = ticks_us();
        }


        /**
         * The same as end function.
         * @return float type, current fps since last call this method
         * @maixpy maix.time.FPS.fps
        */
        float fps()
        {
            if(!_fps_init)
            {
                if(_t_last == 0)
                {
                    _t_last = ticks_us();
                    return 1;
                }
                if(_idx == _fps_buff.size() - 1)
                    _fps_init = true;
                float t = ticks_us() - _t_last;
                _t_last = ticks_us();
                _fps_buff[_idx] = t;
                size_t total = _idx + 1;
                float sum = _fps_buff[0] + 0.000001;
                for(size_t i=1; i<total; ++i)
                {
                    sum += _fps_buff[i];
                }
                _idx = (_idx + 1) % _fps_buff.size();
                return 1000000 / sum * total;
            }
            float t = ticks_us() - _t_last;
            _t_last = ticks_us();
            _fps_buff[_idx] = t;
            _idx = (_idx + 1) % _fps_buff.size();
            float sum = _fps_buff[0] + 0.000001;
            for(size_t i=1; i<_fps_buff.size(); ++i)
            {
                sum += _fps_buff[i];
            }
            return 1000000 / sum * _fps_buff.size();
        }

        /**
         * Calculate FPS since last call this method.
         * FPS is average value of recent n(buff_len) times, and you can call fps_set_buff_len(10) to change buffer length, default is 20.
         * Multiple invoke this function will calculate fps between two invoke, and you can also call fps_start() fisrt to manually assign fps calulate start point.
         * @return float type, current fps since last call this method
         * @maixpy maix.time.FPS.end
        */
        float end()
        {
            return fps();
        }

        /**
         * Set fps method buffer length, by default the buffer length is 10.
         * @param len Buffer length to store recent fps value.
         * @maixpy maix.time.FPS.set_buff_len
        */
        void set_buff_len(int len)
        {
            _fps_buff.resize(len);
            _fps_init = false;
            _idx = 0;
        }

    private:
        uint64_t _last;
        bool     _fps_init;
        float    _t_last;
        size_t   _idx;
        std::vector<uint64_t> _fps_buff;
    };

    /**
     * Date and time class
     * @maixpy maix.time.DateTime
    */
    class DateTime
    {
    public:
        /**
         * Year
         * @maixpy maix.time.DateTime.year
        */
        int year;
        /**
         * Month, 1~12
         * @maixpy maix.time.DateTime.month
        */
        int month;
        /**
         * Day
         * @maixpy maix.time.DateTime.day
        */
        int day;
        /**
         * Hour
         * @maixpy maix.time.DateTime.hour
        */
        int hour;
        /**
         * Minute
         * @maixpy maix.time.DateTime.minute
        */
        int minute;
        /**
         * Second
         * @maixpy maix.time.DateTime.second
        */
        int second;
        /**
         * Microsecond
         * @maixpy maix.time.DateTime.microsecond
        */
        int microsecond;
        /**
         * Year day
         * @maixpy maix.time.DateTime.yearday
        */
        int yearday;
        /**
         * Weekday, 0 is Monday, 6 is Sunday
         * @maixpy maix.time.DateTime.weekday
        */
        int weekday;
        /**
         * Time zone
         * @maixpy maix.time.DateTime.zone
        */
        float zone;
        /**
         * Time zone name
         * @maixpy maix.time.DateTime.zone_name
        */
        std::string zone_name;

        /**
         * Constructor
         * @param year year
         * @param month month
         * @param day day
         * @param hour hour
         * @param minute minute
         * @param second second
         * @param microsecond microsecond
         * @param yearday year day
         * @param weekday weekday
         * @param zone time zone
         * @maixcdk maix.time.DateTime.DateTime
         * @maixpy maix.time.DateTime.__init__
        */
        DateTime(int year = 0, int month = 0, int day = 0, int hour = 0, int minute = 0, int second = 0, int microsecond = 0, int yearday = 0, int weekday = 0, int zone = 0)
        {
            this->year = year;
            this->month = month;
            this->day = day;
            this->hour = hour;
            this->minute = minute;
            this->second = second;
            this->yearday = yearday;
            this->weekday = weekday;
            this->microsecond = microsecond;
            this->zone = zone;
        }
        ~DateTime(){};

        /**
         * Convert to string
         * @return date time string
         * @maixpy maix.time.DateTime.strftime
        */
        std::string strftime(const std::string &format);

        /**
         * Convert to float timestamp
         * @return float timestamp
         * @maixpy maix.time.DateTime.timestamp
        */
        double timestamp();
    };

    /**
     * Get current UTC date and time
     * @return current date and time, DateTime type
     * @maixpy maix.time.now
    */
    time::DateTime *now();

    /**
     * Get local time
     * @return local time, DateTime type
     * @maixpy maix.time.localtime
    */
    time::DateTime *localtime();

    /**
     * DateTime from string
     * @param str date time string
     * @param format date time format
     * @return DateTime
     * @maixpy maix.time.strptime
    */
    time::DateTime *strptime(const std::string &str, const std::string &format);

    /**
     * timestamp to DateTime(time zone is UTC (value 0))
     * @param timestamp double timestamp
     * @return DateTime
     * @maixpy maix.time.gmtime
    */
    time::DateTime *gmtime(double timestamp);

    /**
     * Set or get timezone
     * @param timezone string type, can be empty and default to empty, if empty, only return crrent timezone, a "region/city" string, e.g. Asia/Shanghai, Etc/UTC, you can get all by list_timezones function.
     * @return string type, return current timezone setting.
     * @attention when set new timezone, time setting not take effect in this process for some API, so you need to restart program.
     * @maixpy maix.time.timezone
    */
    std::string timezone(const std::string &timezone = "");

    /**
     * Set or get timezone
     * @param region string type, which region to set, can be empty means only get current, default empty.
     * @param city string type, which city to set, can be empty means only get current, default empty.
     * @return list type, return current timezone setting, first is region, second is city.
     * @attention when set new timezone, time setting not take effect in this process for some API, so you need to restart program.
     * @maixpy maix.time.timezone2
    */
    std::vector<std::string> timezone2(const std::string &region = "", const std::string &city = "");

    /**
     * List all timezone info
     * @return A dict with key are regions, and value are region's cities.
     * @maixpy maix.time.list_timezones
    */
    std::map<std::string, std::vector<std::string>> list_timezones();


    /**
     * @brief Retrieves time from an NTP server
     *
     * This function fetches the current time from the specified NTP server and port,
     * returning a tuple containing the time details.
     *
     * @param host The hostname or IP address of the NTP server.
     * @param port The port number of the NTP server. Use -1 for the default port 123.
     * @param retry The number of retry attempts. Must be at least 1.
     * @param timeout_ms The timeout duration in milliseconds. Must be non-negative.
     * @return A list of 6 elements: [year, month, day, hour, minute, second]
     *
     * @maixpy maix.time.ntp_timetuple
     */
    std::vector<int> ntp_timetuple(std::string host, int port=-1, uint8_t retry=3, int timeout_ms=0);

    /**
     * @brief Retrieves time from an NTP server using a configuration file
     *
     * This function reads the configuration from a YAML file to fetch the current time
     * from a list of specified NTP servers, returning a tuple containing the time details.
     *
     * @param path The path to the YAML configuration file, which should include:
     *  - Config:
     *      - retry: Number of retry attempts (must be at least 1)
     *      - total_timeout_ms: Total timeout duration in milliseconds (must be non-negative)
     *  - NtpServers:
     *      - host: Hostname or IP address of the NTP server
     *      - port: Port number of the NTP server (use 123 for default)
     *
     * Example YAML configuration:
     *  Config:
     *   - retry: 3
     *   - total_timeout_ms: 10000
     *
     *  NtpServers:
     *   - host: "pool.ntp.org"
     *     port: 123
     *   - host: "time.nist.gov"
     *     port: 123
     *   - host: "time.windows.com"
     *     port: 123
     *
     * @return A list of 6 elements: [year, month, day, hour, minute, second]
     *
     * @maixpy maix.time.ntp_timetuple_with_config
     */
    std::vector<int> ntp_timetuple_with_config(std::string path);

    /**
     * @brief Retrieves time from an NTP server and synchronizes the system time
     *
     * This function fetches the current time from the specified NTP server and port,
     * then synchronizes the system time with the retrieved time.
     *
     * @param host The hostname or IP address of the NTP server.
     * @param port The port number of the NTP server. Use 123 for the default port.
     * @param retry The number of retry attempts. Must be at least 1.
     * @param timeout_ms The timeout duration in milliseconds. Must be non-negative.
     * @return A list of 6 elements: [year, month, day, hour, minute, second]
     *
     * @maixpy maix.time.ntp_sync_sys_time
     */
    std::vector<int> ntp_sync_sys_time(std::string host, int port=-1, uint8_t retry=3, int timeout_ms=0);

    /**
     * @brief Retrieves time from an NTP server using a configuration file and synchronizes the system time
     *
     * This function reads the configuration from a YAML file to fetch the current time
     * from a list of specified NTP servers, then synchronizes the system time with the retrieved time.
     *
     * @param path The path to the YAML configuration file, which should include:
     *  - Config:
     *      - retry: Number of retry attempts (must be at least 1)
     *      - total_timeout_ms: Total timeout duration in milliseconds (must be non-negative)
     *  - NtpServers:
     *      - host: Hostname or IP address of the NTP server
     *      - port: Port number of the NTP server (use 123 for default)
     *
     * Example YAML configuration:
     *  Config:
     *   - retry: 3
     *   - total_timeout_ms: 10000
     *
     *  NtpServers:
     *   - host: "pool.ntp.org"
     *     port: 123
     *   - host: "time.nist.gov"
     *     port: 123
     *   - host: "time.windows.com"
     *     port: 123
     *
     * @return A vector of integers containing the time details: [year, month, day, hour, minute, second]
     *
     * @maixpy maix.time.ntp_sync_sys_time_with_config
     */
    std::vector<int> ntp_sync_sys_time_with_config(std::string path);
}


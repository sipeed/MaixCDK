/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <stdint.h>
#include <string>

namespace maix::time
{
    /**
     * Get current time in s
     * @return current time in s, double type
     * @maixpy maix.time.time
    */
    double time();

    /**
     * Get current time in ms
     * @return current time in ms, uint64_t type
     * @maixpy maix.time.time_ms
    */
    uint64_t time_ms();

    /**
     * Get current time in s
     * @return current time in s, uint64_t type
     * @maixpy maix.time.time_s
    */
    uint64_t time_s();

    /**
     * Get current time in us
     * @return current time in us, uint64_t type
     * @maixpy maix.time.time_us
    */
    uint64_t time_us();

    /**
     * Calculate time difference
     * @param last last time
     * @param now current time
     * @return time difference
     * @maixpy maix.time.time_diff
    */
    double time_diff(double last, double now = -1);

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

}


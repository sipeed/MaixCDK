/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.2: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include <functional>

namespace maix::peripheral::rtc
{
    /**
     * Peripheral rtc class
     * @maixpy maix.peripheral.rtc.RTC
     * @maixcdk maix.peripheral.rtc.RTC
     */
    class RTC
    {
    public:
        /**
         * @brief RTC constructor
         *
         * @param[in] id id of rtc, int type
         * @maixpy maix.peripheral.rtc.RTC.__init__
         */
        RTC(int id=0);
        ~RTC();

        /**
         * @brief Get the RTC id
         *
         * @return int type, id
         *
         * @maixpy maix.peripheral.rtc.RTC.id
         */
        int id() const;

        /**
         * @brief Get or set the date and time of the RTC.
         *
         * @param timetuple time tuple, like (year, month, day[, hour[, minute[, second]]])
         * @return time tuple, like (year, month, day[, hour[, minute[, second]]])
         *
         * @maixpy maix.peripheral.rtc.RTC.datetime
         */
        std::vector<int> datetime(std::vector<int> timetuple);

        /**
         * @brief Initialise the RTC.
         *
         * @param timetuple time tuple, like (year, month, day[, hour[, minute[, second]]])
         * @return err::Err type, if init success, return err::ERR_NONE
         *
         * @maixpy maix.peripheral.rtc.RTC.init
         * @maixcdk maix.peripheral.rtc.RTC.init
         */
        err::Err init(std::vector<int> timetuple);

        /**
         * @brief Get get the current datetime struct rtc::Time.
         *
         * @return time tuple, like (year, month, day[, hour[, minute[, second]]])
         *
         * @maixpy maix.peripheral.rtc.RTC.now
         * @maixcdk maix.peripheral.rtc.RTC.now
         */
        std::vector<int> now();

        /**
         * @brief Resets the RTC to the time of January 1, 2015 and starts running it again.
         *
         * @return err::Err err::Err type, if deinit success, return err::ERR_NONE
         *
         * @maixpy maix.peripheral.rtc.RTC.deinit
         * @maixcdk maix.peripheral.rtc.RTC.deinit
         */
        err::Err deinit();

        /**
         * @brief Set the RTC alarm.
         *        Time might be either a millisecond value to program the alarm to
         *        current time + time_in_ms in the future, or a datetimetuple.
         *        If the time passed is in milliseconds, repeat can be set to True to make the alarm periodic.
         *
         * @param timetuple time tuple, like (year, month, day[, hour[, minute[, second]]])
         * @return err::Err type, if set alarm success, return err::ERR_NONE
         *
         * @maixpy maix.peripheral.rtc.RTC.alarm
         * @maixcdk maix.peripheral.rtc.RTC.alarm
         */
        err::Err alarm(std::vector<int> timetuple);

        /**
         * @brief Get the number of milliseconds left before the alarm expires.
         *
         * @return number of milliseconds left before the alarm expires.
         *
         * @maixpy maix.peripheral.rtc.RTC.alarm_left
         * @maixcdk maix.peripheral.rtc.RTC.alarm_left
         */
        int alarm_left();

        /**
         * @brief Cancel a running alarm.
         *
         * @return err::Err type
         *
         * @maixpy maix.peripheral.rtc.RTC.cancel
         * @maixcdk maix.peripheral.rtc.RTC.cancel
         */
        err::Err cancel();

        /**
         * @brief Create an irq callback by a real time clock alarm.
         *
         * @param callback cb function
         * @param args cb function args
         * @return err::Err type
         *
         * @maixpy maix.peripheral.rtc.RTC.irq
         * @maixcdk maix.peripheral.rtc.RTC.irq
         */
        err::Err irq(std::function<void(void*)> callback, void* args);

        /**
         * @brief Set the system time from the RTC
         *
         * @return err::Err type
         *
         * @maixpy maix.peripheral.rtc.RTC.hctosys
         * @maixcdk maix.peripheral.rtc.RTC.hctosys
         */
        err::Err hctosys();

        /**
         * @brief Set the RTC from the system time
         *
         * @return err::Err type
         *
         * @maixpy maix.peripheral.rtc.RTC.systohc
         * @maixcdk maix.peripheral.rtc.RTC.systohc
         */
        err::Err systohc();

    private:
        int _id;
    };
}; // namespace maix::peripheral::wdt

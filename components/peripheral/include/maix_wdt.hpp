/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

namespace maix::peripheral::wdt
{
    /**
     * Peripheral wdt class
     * @maixpy maix.peripheral.wdt.WDT
     */
    class WDT
    {
    public:
        /**
         * @brief WDT constructor, after construct, the wdt will auto start.
         *
         * @param[in] id id of wdt, int type
         * @param[in] feed_ms feed interval, int type, unit is ms, you must feed wdt in this interval, or system will restart.
         * @maixpy maix.peripheral.wdt.WDT.__init__
         */
        WDT(int id, int feed_ms);
        ~WDT();

        /**
         * @brief feed wdt
         *
         * @return error code, if feed success, return err::ERR_NONE
         * @maixpy maix.peripheral.wdt.WDT.feed
         */
        int feed();

        /**
         * @brief stop wdt
         * @maixpy maix.peripheral.wdt.WDT.stop
         */
        int stop();

        /**
         * @brief restart wdt, stop and start watchdog timer.
         * @maixpy maix.peripheral.wdt.WDT.restart
         */
        int restart();
    };
}; // namespace maix::peripheral::wdt

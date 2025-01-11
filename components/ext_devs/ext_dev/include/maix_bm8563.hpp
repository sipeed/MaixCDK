/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.6: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"

namespace maix::ext_dev::bm8563 {

    /**
     * Peripheral BM8563 class
     * @maixpy maix.ext_dev.bm8563.BM8563
     */
    class BM8563 {
    public:
        /**
         * @brief BM8563 constructor
         *
         * @param i2c_bus i2c bus number.
         * @maixpy maix.ext_dev.bm8563.BM8563.__init__
         */
        BM8563(int i2c_bus=-1);
        ~BM8563();

        BM8563(const BM8563&)               = delete;
        BM8563& operator=(const BM8563&)    = delete;
        BM8563(BM8563&&)                    = delete;
        BM8563& operator=(BM8563&&)         = delete;

        /**
         * @brief Get or set the date and time of the BM8563.
         *
         * @param timetuple time tuple, like (year, month, day[, hour[, minute[, second]]])
         * @return time tuple, like (year, month, day[, hour[, minute[, second]]])
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.datetime
         */
        std::vector<int> datetime(std::vector<int> timetuple=std::vector<int>());

        /**
         * @brief Initialise the BM8563.
         *
         * @param timetuple time tuple, like (year, month, day[, hour[, minute[, second]]])
         * @return err::Err type, if init success, return err::ERR_NONE
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.init
         */
        err::Err init(std::vector<int> timetuple);

        /**
         * @brief Get get the current datetime.
         *
         * @return time tuple, like (year, month, day[, hour[, minute[, second]]])
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.now
         */
        std::vector<int> now();

        /**
         * @brief Deinit the BM8563.
         *
         * @return err::Err err::Err type, if deinit success, return err::ERR_NONE
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.deinit
         */
        err::Err deinit();

        /**
         * @brief Set the system time from the BM8563
         *
         * @return err::Err type
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.hctosys
         */
        err::Err hctosys();

        /**
         * @brief Set the BM8563 from the system time
         *
         * @return err::Err type
         *
         * @maixpy maix.ext_dev.bm8563.BM8563.systohc
         */
        err::Err systohc();
    };
}
/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <functional>
#include "maix_type.hpp"

namespace maix
{
    namespace thread
    {
        /**
         * @brief thread class
         * @maixpy maix.thread.Thread
         */
        class Thread
        {
        public:
            /**
             * @brief create thread
             * @param[in] func thread function, one `args` parameter, void* type, no return value
             * @param[in] args thread function parameter
             * @maixpy maix.thread.Thread.__init__
             * @maixcdk maix.thread.Thread.Thread
             */
            Thread(std::function<void(void *)> func, void *args = nullptr);
            ~Thread();

            /**
             * @brief wait thread exit
             * @maixpy maix.thread.Thread.join
             */
            void join();

            /**
             * @brief detach thread, detach will auto start thread and you can't use join anymore.
             * @maixpy maix.thread.Thread.detach
             */
            void detach();

            /**
             * Check if thread is joinable
             * @return true if thread is joinable
             * @maixpy maix.thread.Thread.joinable
             */
            bool joinable();

        protected:
            void *_thread;
            std::function<void(void *)> _func;
            void *_args;
        };

        void sleep_ms(uint32_t ms);


    }; // namespace thread
};     // namespace maix

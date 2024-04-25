/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <vector>
#include "maix_err.hpp"

namespace maix::touchscreen
{
    class TouchScreen_Base
    {
    public:
        // TouchScreen_Base(const std::string &device = "") {}
        virtual err::Err open() = 0;
        virtual err::Err close() = 0;
        virtual err::Err read(int &x, int &y, bool &pressed) = 0;
        virtual std::vector<int> read() = 0;
        virtual err::Err read0(int &x, int &y, bool &pressed) = 0;
        virtual std::vector<int> read0() = 0;
        virtual bool available(int timeout = 0) = 0;
        /**
         * @brief check display device is opened or not
         * @return opened or not, bool type
        */
        virtual bool is_opened() = 0;
    };
} // namespace maix::touchscreen




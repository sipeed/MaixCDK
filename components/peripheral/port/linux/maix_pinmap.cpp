/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.5.13: create this file.
 */

#include "maix_pinmap.hpp"

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace maix::peripheral::pinmap
{

    std::vector<std::string> get_pins()
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> get_pin_functions(const std::string &pin)
    {
        return std::vector<std::string>();
    }

    err::Err set_pin_function(const std::string &pin, const std::string &func)
    {
        return err::ERR_NOT_IMPL;
    }

}

/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.5.13: create this file.
 */

#pragma once

#include "maix_basic.hpp"

namespace maix::peripheral::pinmap
{

    /**
     * Get all pins of devices
     * @return pin name list, string type.
     * @maixpy maix.peripheral.pinmap.get_pins
    */
    std::vector<std::string> get_pins();

    /**
     * Get all function of a pin
     * @param pin pin name, string type.
     * @return function list, function name is string type.
     * @throw If pin name error will throwout err.Err.ERR_ARGS error.
     * @maixpy maix.peripheral.pinmap.get_pin_functions
    */
    std::vector<std::string> get_pin_functions(const std::string &pin);

    /**
     * Set function of a pin
     * @param pin pin name, string type.
     * @param func which function should this pin use.
     * @return if set ok, will return err.Err.ERR_NONE, else error occurs.
     * @maixpy maix.peripheral.pinmap.set_pin_function
    */
    err::Err set_pin_function(const std::string &pin, const std::string &func);

} // namespace maix::peripheral::pinmap




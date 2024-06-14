/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_timer.hpp"
#include "maix_basic.hpp"

namespace maix::peripheral::timer
{
    TIMER::TIMER()
    {
        throw err::Exception(err::Err::ERR_NOT_IMPL, "Not implemented");
    }

    TIMER::~TIMER()
    {
    }
}; // namespace maix

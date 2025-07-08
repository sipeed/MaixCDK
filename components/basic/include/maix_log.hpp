/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#ifndef __MAIX_LOG_H
#define __MAIX_LOG_H

#include <stdio.h>
#include "global_config.h"
#include <stdarg.h>

namespace maix::log
{
    /**
     * Error log level enums
     * @maixpy maix.log.LogLevel
     */
    enum class LogLevel
    {
        LEVEL_NONE  = 0,
        LEVEL_ERROR,
        LEVEL_WARN,
        LEVEL_INFO,
        LEVEL_DEBUG,
        LEVEL_MAX
    };

    /**
     * Set log level globally, by default log level is LEVEL_INFO.
     * @param level log level, @see maix.log.LogLevel
     * @param color true to enable color, false to disable color
     * @maixpy maix.log.set_log_level
     */
    void set_log_level(log::LogLevel level, bool color);

    /**
     * Get current log level
     * @return current log level
     * @maixpy maix.log.get_log_level
     */
    log::LogLevel get_log_level();

    /**
     * Get whether log use color
     * @return true if log use color, else false
     * @maixpy maix.log.get_log_use_color
     */
    bool get_log_use_color();

    /**
     * print error log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.error
    */
    void error(const char *fmt, ...);

    /**
     * print warning log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.warn
    */
    void warn(const char *fmt, ...);

    /**
     * print info log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.info
    */
    void info(const char *fmt, ...);

    /**
     * print debug log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.debug
    */
    void debug(const char *fmt, ...);


    /**
     * print error log, but not add '\n' at end
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.error0
    */
    void error0(const char *fmt, ...);

    /**
     * print warning log, but not add '\n' at end
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.warn0
    */
    void warn0(const char *fmt, ...);

    /**
     * print info log, but not add '\n' at end
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.info0
    */
    void info0(const char *fmt, ...);

    /**
     * print debug log, but not add '\n' at end
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.debug0
    */
    void debug0(const char *fmt, ...);

    /**
     * DEPRACATED, DO NOT USE THIS method.
     * same as printf, but recommend use this function instead of printf
     * this function will add prefix like "-- [E] " to log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.print
    */
    void print(const char *fmt, ...);

    /**
     * print log info with specified log level, no '\n' at end and no prefix like "-- [E] ".
     * this function will add prefix like "-- [E] " to log
     * @param fmt format string
     * @param ... args
     * @maixcdk maix.log.print
    */
    void print(log::LogLevel level, const char *fmt, ...);

    /**
      * flush log
      * @param level log level, @see maix.log.LogLevel
      * @maixcdk maix.log.flush
     */
    void flush(log::LogLevel level = log::LogLevel::LEVEL_NONE);

} // namespace maix::log



#endif

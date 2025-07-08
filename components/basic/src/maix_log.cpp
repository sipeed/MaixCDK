/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_log.hpp"
#include "maix_err.hpp"
#include <string>

namespace maix::log
{

    static char log_buf[512];
    static volatile LogLevel log_level = LogLevel::LEVEL_INFO;
    static volatile bool log_color = false;

    void set_log_level(LogLevel level, bool color)
    {
        log_level = level;
        log_color = color;
    }

    LogLevel get_log_level()
    {
        return log_level;
    }

    bool get_log_use_color()
    {
        return log_color;
    }

    void error(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_ERROR)
            return;
        static const char *err_start = "-- [E] ";
        // print error and call err::set_error
        va_list args;
        va_start(args, fmt);
        printf("%s", err_start);
        vsnprintf(log_buf, sizeof(log_buf), fmt, args);
        printf("%s\n", log_buf);
        va_end(args);
        err::set_error(err_start + std::string(log_buf));
    }

    void error0(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_ERROR)
            return;
        static const char *err_start = "-- [E] ";
        // print error and call err::set_error
        va_list args;
        va_start(args, fmt);
        printf("%s", err_start);
        vsnprintf(log_buf, sizeof(log_buf), fmt, args);
        printf("%s", log_buf);
        va_end(args);
        err::set_error(err_start + std::string(log_buf));
    }

    void warn(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_WARN)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [W] ");
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    void warn0(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_WARN)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [W] ");
        vprintf(fmt, args);
        va_end(args);
    }

    void info(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_INFO)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [I] ");
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    void info0(const char *fmt, ...)
    {
        if(log_level < LogLevel::LEVEL_INFO)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [I] ");
        vprintf(fmt, args);
        va_end(args);
    }

    void debug(const char *fmt, ...)
    {
#if DEBUG
        if(log_level < LogLevel::LEVEL_DEBUG)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [D] ");
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
#else
        (void)fmt;
#endif
    }

    void debug0(const char *fmt, ...)
    {
#if DEBUG
        if(log_level < LogLevel::LEVEL_DEBUG)
            return;
        va_list args;
        va_start(args, fmt);
        printf("-- [D] ");
        vprintf(fmt, args);
        va_end(args);
#else
        (void)fmt;
#endif
    }

    void print(const char *fmt, ...)
    {
        printf("[WARN] log::print(fmt, ...) function is deprecated, please use log::print(log::LogLevel, fmt, ...) instead\n");
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    void print(LogLevel level, const char *fmt, ...)
    {
        if(level > log_level)
            return;
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    void flush(log::LogLevel level)
    {
        fflush(stdout);
    }

} // namespace maix::log

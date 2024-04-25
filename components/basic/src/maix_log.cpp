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

    void error(const char *fmt, ...)
    {
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
        va_list args;
        va_start(args, fmt);
        printf("-- [W] ");
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    void warn0(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        printf("-- [W] ");
        vprintf(fmt, args);
        va_end(args);
    }

    void info(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        printf("-- [I] ");
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    void info0(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        printf("-- [I] ");
        vprintf(fmt, args);
        va_end(args);
    }

    void debug(const char *fmt, ...)
    {
#if DEBUG
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
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

} // namespace maix::log

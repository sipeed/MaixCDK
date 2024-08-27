/**
 * @file xtime.c
 * Copyright (c) 2015 Gaaagaa. All rights reserved.
 * 
 * @author  : Gaaagaa
 * @date    : 2015-12-07
 * @version : 1.0.0.0
 * @brief   : 实现 时间的相关辅助 函数接口。
 */

/**
 * The MIT License (MIT)
 * Copyright (c) Gaaagaa. All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "xtime.h"
#include <time.h>

#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#elif (defined(__linux__) || defined(__unix__))
#include <sys/time.h>
#else // UNKNOW
#error "unknow platform!"
#endif // PLATFORM

////////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif // __GNUC__

////////////////////////////////////////////////////////////////////////////////

//====================================================================

// 
// 内部相关的数据类型与常量
// 

/** 1601 ~ 1970 年之间的时间 百纳秒数 */
#define XTIME_VNSEC_1601_1970 116444736000000000LL

//====================================================================

// 
// 内部相关的操作接口
// 


//====================================================================

// 
// 外部相关操作接口
// 

/**********************************************************/
/**
 * @brief 获取当前系统的 时间计量值。
 */
xtime_vnsec_t time_vnsec(void)
{
    xtime_vnsec_t xtm_vnsec = XTIME_INVALID_VNSEC;

#if (defined(_WIN32) || defined(_WIN64))

    FILETIME       xtm_sfile;
    ULARGE_INTEGER xtm_value;

    GetSystemTimeAsFileTime(&xtm_sfile);
    xtm_value.LowPart  = xtm_sfile.dwLowDateTime;
    xtm_value.HighPart = xtm_sfile.dwHighDateTime;

    xtm_vnsec = (xtime_vnsec_t)(xtm_value.QuadPart - XTIME_VNSEC_1601_1970);

#elif (defined(__linux__) || defined(__unix__))

    struct timeval xtm_value;
    gettimeofday(&xtm_value, X_NULL);

    xtm_vnsec = (xtime_vnsec_t)(xtm_value.tv_sec * 10000000ULL + xtm_value.tv_usec * 10ULL);

#else // UNKNOW
#error "unknow platform!"
#endif // PLATFORM

    return xtm_vnsec;
}

/**********************************************************/
/**
 * @brief 获取当前系统的 时间描述信息。
 */
xtime_descr_t time_descr(void)
{
    xtime_descr_t xtm_descr = { 0 };

#if (defined(_WIN32) || defined(_WIN64))

    SYSTEMTIME xtm_local;

    GetLocalTime(&xtm_local);

    xtm_descr.ctx_year   = xtm_local.wYear        ;
    xtm_descr.ctx_month  = xtm_local.wMonth       ;
    xtm_descr.ctx_day    = xtm_local.wDay         ;
    xtm_descr.ctx_week   = xtm_local.wDayOfWeek   ;
    xtm_descr.ctx_hour   = xtm_local.wHour        ;
    xtm_descr.ctx_minute = xtm_local.wMinute      ;
    xtm_descr.ctx_second = xtm_local.wSecond      ;
    xtm_descr.ctx_msec   = xtm_local.wMilliseconds;

#elif (defined(__linux__) || defined(__unix__))

    time_t         xtm_time;
    struct tm      xtm_local;
    struct timeval xtm_value;

    gettimeofday(&xtm_value, X_NULL);

    xtm_time = (time_t)(xtm_value.tv_sec);
    localtime_r(&xtm_time, &xtm_local);

    xtm_descr.ctx_year   = xtm_local.tm_year + 1900;
    xtm_descr.ctx_month  = xtm_local.tm_mon  + 1   ;
    xtm_descr.ctx_day    = xtm_local.tm_mday       ;
    xtm_descr.ctx_week   = xtm_local.tm_wday       ;
    xtm_descr.ctx_hour   = xtm_local.tm_hour       ;
    xtm_descr.ctx_minute = xtm_local.tm_min        ;
    xtm_descr.ctx_second = xtm_local.tm_sec        ;
    xtm_descr.ctx_msec   = (x_uint32_t)(xtm_value.tv_usec / 1000);

#else // UNKNOW
#error "unknow platform!"
#endif // PLATFORM

    return xtm_descr;
}

/**********************************************************/
/**
 * @brief 将 时间描述信息 转换为 时间计量值。
 * 
 * @param [in ] xtm_descr : 待转换的 时间描述信息。
 * 
 * @return xtime_vnsec_t : 
 * 返回 时间计量值，可用 XTMVNSEC_IS_VALID() 判断其是否为有效。
 */
xtime_vnsec_t time_dtov(xtime_descr_t xtm_descr)
{
    xtime_vnsec_t xtm_vnsec = XTIME_INVALID_VNSEC;

#if 0
    if ((xtm_descr.ctx_year   < 1970) ||
        (xtm_descr.ctx_month  <    1) || (xtm_descr.ctx_month > 12) ||
        (xtm_descr.ctx_day    <    1) || (xtm_descr.ctx_day   > 31) ||
        (xtm_descr.ctx_hour   >   23) ||
        (xtm_descr.ctx_minute >   59) ||
        (xtm_descr.ctx_second >   60) ||
        (xtm_descr.ctx_msec   >  999))
    {
        return XTIME_INVALID_VNSEC;
    }
#endif

#if (defined(_WIN32) || defined(_WIN64))

    ULARGE_INTEGER xtm_value;
    FILETIME       xtm_sfile;
    FILETIME       xtm_lfile;
    SYSTEMTIME     xtm_local;

    xtm_local.wYear         = xtm_descr.ctx_year  ;
    xtm_local.wMonth        = xtm_descr.ctx_month ;
    xtm_local.wDay          = xtm_descr.ctx_day   ;
    xtm_local.wDayOfWeek    = xtm_descr.ctx_week  ;
    xtm_local.wHour         = xtm_descr.ctx_hour  ;
    xtm_local.wMinute       = xtm_descr.ctx_minute;
    xtm_local.wSecond       = xtm_descr.ctx_second;
    xtm_local.wMilliseconds = xtm_descr.ctx_msec  ;

    if (SystemTimeToFileTime(&xtm_local, &xtm_lfile))
    {
        if (LocalFileTimeToFileTime(&xtm_lfile, &xtm_sfile))
        {
            xtm_value.LowPart  = xtm_sfile.dwLowDateTime ;
            xtm_value.HighPart = xtm_sfile.dwHighDateTime;

            xtm_vnsec = (xtime_vnsec_t)(xtm_value.QuadPart - XTIME_VNSEC_1601_1970);
        }
    }

#elif (defined(__linux__) || defined(__unix__))

    struct tm      xtm_local;
    struct timeval xtm_value;

    xtm_local.tm_sec   = xtm_descr.ctx_second;
    xtm_local.tm_min   = xtm_descr.ctx_minute;
    xtm_local.tm_hour  = xtm_descr.ctx_hour  ;
    xtm_local.tm_mday  = xtm_descr.ctx_day   ;
    xtm_local.tm_mon   = xtm_descr.ctx_month - 1   ;
    xtm_local.tm_year  = xtm_descr.ctx_year  - 1900;
    xtm_local.tm_wday  = 0;
    xtm_local.tm_yday  = 0;
    xtm_local.tm_isdst = 0;

    xtm_value.tv_sec  = mktime(&xtm_local);
    xtm_value.tv_usec = xtm_descr.ctx_msec * 1000;
    if (-1 != xtm_value.tv_sec)
    {
        xtm_vnsec = (xtime_vnsec_t)(xtm_value.tv_sec * 10000000ULL + xtm_value.tv_usec * 10ULL);
    }

#else // UNKNOW
#error "unknow platform!"
#endif // PLATFORM

    return xtm_vnsec;
}

/**********************************************************/
/**
 * @brief 将 时间计量值 转换为 时间描述信息。
 * 
 * @param [in ] xtm_vnsec : 待转换的 时间计量值。
 * 
 * @return xtime_descr_t : 
 * 返回 时间描述信息，可用 XTMDESCR_IS_VALID() 判断其是否为有效。
 */
xtime_descr_t time_vtod(xtime_vnsec_t xtm_vnsec)
{
    xtime_descr_t xtm_descr = { 0 };

#if (defined(_WIN32) || defined(_WIN64))

    ULARGE_INTEGER xtm_value;
    FILETIME       xtm_sfile;
    FILETIME       xtm_lfile;
    SYSTEMTIME     xtm_local;

    xtm_value.QuadPart       = xtm_vnsec + XTIME_VNSEC_1601_1970;
    xtm_sfile.dwLowDateTime  = xtm_value.LowPart;
    xtm_sfile.dwHighDateTime = xtm_value.HighPart;
    if (FileTimeToLocalFileTime(&xtm_sfile, &xtm_lfile))
    {
        if (FileTimeToSystemTime(&xtm_lfile, &xtm_local))
        {
            xtm_descr.ctx_year   = xtm_local.wYear        ;
            xtm_descr.ctx_month  = xtm_local.wMonth       ;
            xtm_descr.ctx_day    = xtm_local.wDay         ;
            xtm_descr.ctx_week   = xtm_local.wDayOfWeek   ;
            xtm_descr.ctx_hour   = xtm_local.wHour        ;
            xtm_descr.ctx_minute = xtm_local.wMinute      ;
            xtm_descr.ctx_second = xtm_local.wSecond      ;
            xtm_descr.ctx_msec   = xtm_local.wMilliseconds;
        }
    }

#elif (defined(__linux__) || defined(__unix__))

    struct tm xtm_local;
    time_t xtm_time = (time_t)(xtm_vnsec / 10000000ULL);
    localtime_r(&xtm_time, &xtm_local);

    xtm_descr.ctx_year   = xtm_local.tm_year + 1900;
    xtm_descr.ctx_month  = xtm_local.tm_mon  + 1   ;
    xtm_descr.ctx_day    = xtm_local.tm_mday       ;
    xtm_descr.ctx_week   = xtm_local.tm_wday       ;
    xtm_descr.ctx_hour   = xtm_local.tm_hour       ;
    xtm_descr.ctx_minute = xtm_local.tm_min        ;
    xtm_descr.ctx_second = xtm_local.tm_sec        ;
    xtm_descr.ctx_msec   = (x_uint32_t)((xtm_vnsec % 10000000ULL) / 10000L);

#else // UNKNOW
#error "unknow platform!"
#endif // PLATFORM

    return xtm_descr;
}

/**********************************************************/
/**
 * @brief 判断 时间描述信息 是否有效。
 */
x_bool_t time_descr_valid(xtime_descr_t xtm_descr)
{
    x_bool_t xbt_valid = X_TRUE;

    do
    {
        //======================================

        if ((xtm_descr.ctx_year   < 1970) ||
            (xtm_descr.ctx_week   >    6) ||
            (xtm_descr.ctx_hour   >   23) ||
            (xtm_descr.ctx_minute >   59) ||
            (xtm_descr.ctx_second >   60) ||
            (xtm_descr.ctx_msec   >  999))
        {
            xbt_valid = X_FALSE;
            break;
        }

        //======================================

        switch (xtm_descr.ctx_month)
        {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            xbt_valid = (xtm_descr.ctx_day <= 31);
            break;

        case 4: case 6: case 9: case 11:
            xbt_valid = (xtm_descr.ctx_day <= 30);
            break;

        case 2:
#define IS_LEAP_YEAR(Y) ((0 == (Y) % 400) || ((0 == (Y) % 4) && (0 != (Y) % 100)))
            xbt_valid = (xtm_descr.ctx_day <= (IS_LEAP_YEAR(xtm_descr.ctx_year) ? 29U : 28U));
#undef IS_LEAP_YEAR
            break;

        default:
            xbt_valid = X_FALSE;
            break;
        }

        if (!xbt_valid) break;

        //======================================

        xbt_valid =
            (xtm_descr.ctx_week == 
                time_week(xtm_descr.ctx_year, xtm_descr.ctx_month,xtm_descr.ctx_day)
            );

        //======================================
    } while (0);

    return xbt_valid;
}

/**********************************************************/
/**
 * @brief 依据 Zeller 公式，求取 具体日期（年、月、日） 对应的 星期几 。
 * @note
 * Zeller 公式：w = y + [y / 4] + [c / 4] - (2 * c) + [26 * (m + 1) / 10] + d - 1;
 * 该公式只适合于 1582年10月15日 之后的情形，公式中的符号含义如下：
 *  w：星期 w 对7取模得：0-星期日，1-星期一，2-星期二，3-星期三，4-星期四，5-星期五，6-星期六；
 *  c：世纪数 - 1（四位数年份的前两位数）；
 *  y：年；
 *  m：月，(m >= 3 && m <= 14)，即在 Zeller 公式中，
 *     某年的 1、2 月要看作上一年的 13、14 月来计算，
 *     比如 2003年1月1日 要看作 2002年的13月1日 来计算；
 *  d：日。
 * 
 * @param [in ] xut_year  : 年。
 * @param [in ] xut_month : 月。
 * @param [in ] xut_day   : 日。
 * 
 * @return x_int32_t :
 * 星期编号 0 ~ 6 （星期日 对应 0， 星期一 对应 1， ...）。
 */
x_uint32_t time_week(x_uint32_t xut_year, x_uint32_t xut_month, x_uint32_t xut_day)
{
    x_int32_t xit_c = 0;
    x_int32_t xit_y = 0;
    x_int32_t xit_w = 0;

    if (xut_month < 3)
    {
        xut_year  -=  1;
        xut_month += 12;
    }

    xit_c = (x_int32_t)(xut_year / 100);
    xit_y = (x_int32_t)(xut_year % 100);

    xit_w = (xit_y       +
             (xit_y / 4) +
             (xit_c / 4) -
             (xit_c * 2) +
             ((26 * ((x_int32_t)(xut_month) + 1)) / 10) +
             ((x_int32_t)(xut_day) - 1)
            ) % 7;

    return (x_uint32_t)((xit_w < 0) ? (xit_w + 7) : xit_w);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

////////////////////////////////////////////////////////////////////////////////


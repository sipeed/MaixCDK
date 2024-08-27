/**
 * @file xtime.h
 * Copyright (c) 2015 Gaaagaa. All rights reserved.
 * 
 * @author  : Gaaagaa
 * @date    : 2015-12-07
 * @version : 1.0.0.0
 * @brief   : 声明 时间的相关辅助 数据类型 与 函数接口。
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

#ifndef __XTIME_H__
#define __XTIME_H__

#include "xtypes.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

//====================================================================

// 
// 相关的数据类型
// 

// /** 以 100纳秒 为单位，1970-01-01 00:00:00 至今的 时间计量值 类型 */
typedef x_uint64_t xtime_vnsec_t;

// /**
//  * @struct xtime_descr_t
//  * @brief  时间描述信息的联合体类型（共计 64 位）。
//  * @note   该结构体用于描述 1970-01-01 00:00:00 往后的时间。
//  */
typedef union xtime_descr_t
{
    /** 对齐的 64位 整数值 */
    x_uint64_t ctx_value;

    /** 描述信息的上下文描述结构体 */
    struct
    {
    x_uint32_t ctx_year   : 16;  ///< 年（1970 ~ ）
    x_uint32_t ctx_month  :  6;  ///< 月（1 ~ 12）
    x_uint32_t ctx_day    :  6;  ///< 日（1 ~ 31）
    x_uint32_t ctx_week   :  4;  ///< 周几（0 ~ 6）
    x_uint32_t ctx_hour   :  6;  ///< 时（0 ~ 23）
    x_uint32_t ctx_minute :  6;  ///< 分（0 ~ 59）
    x_uint32_t ctx_second :  6;  ///< 秒（0 ~ 60），上限定为 60 而不是 59，主要考虑 闰秒 的存在
    x_uint32_t ctx_msec   : 14;  ///< 毫秒（0 ~ 999）
    };
} xtime_descr_t;

/** 定义无效的 时间计量值 */
#define XTIME_INVALID_VNSEC         ((xtime_vnsec_t)~0ULL)

/** 判断 时间计量值 是否为 有效 */
#define XTMVNSEC_IS_VALID(xvnsec)   (XTIME_INVALID_VNSEC != (xvnsec))

/** 判断 时间描述信息 是否为 有效 */
#define XTMDESCR_IS_VALID(xdescr)   time_descr_valid(xdescr)

//====================================================================

// 
// 相关的操作接口
// 

/**********************************************************/
// /**
//  * @brief 获取当前系统的 时间计量值。
//  */
xtime_vnsec_t time_vnsec(void);

/**********************************************************/
// /**
//  * @brief 获取当前系统的 时间描述信息。
//  */
xtime_descr_t time_descr(void);

/**********************************************************/
// /**
//  * @brief 将 时间描述信息 转换为 时间计量值。
//  * 
//  * @param [in ] xtm_descr : 待转换的 时间描述信息。
//  * 
//  * @return xtime_vnsec_t : 
//  * 返回 时间计量值，可用 XTMVNSEC_IS_VALID() 判断其是否为有效。
//  */
xtime_vnsec_t time_dtov(xtime_descr_t xtm_descr);

/**********************************************************/
// /**
//  * @brief 将 时间计量值 转换为 时间描述信息。
//  * 
//  * @param [in ] xtm_vnsec : 待转换的 时间计量值。
//  * 
//  * @return xtime_descr_t : 
//  * 返回 时间描述信息，可用 XTMDESCR_IS_VALID() 判断其是否为有效。
//  */
xtime_descr_t time_vtod(xtime_vnsec_t xtm_vnsec);

/**********************************************************/
/**
 * @brief 判断 时间描述信息 是否有效。
 */
x_bool_t time_descr_valid(xtime_descr_t xtm_descr);

/**********************************************************/
// /**
//  * @brief 依据 Zeller 公式，求取 具体日期（年、月、日） 对应的 星期几 。
//  * @note
//  * Zeller 公式：w = y + [y / 4] + [c / 4] - (2 * c) + [26 * (m + 1) / 10] + d - 1;
//  * 该公式只适合于 1582年10月15日 之后的情形，公式中的符号含义如下：
//  *  w：星期 w 对7取模得：0-星期日，1-星期一，2-星期二，3-星期三，4-星期四，5-星期五，6-星期六；
//  *  c：世纪数 - 1（四位数年份的前两位数）；
//  *  y：年；
//  *  m：月，(m >= 3 && m <= 14)，即在 Zeller 公式中，
//  *     某年的 1、2 月要看作上一年的 13、14 月来计算，
//  *     比如 2003年1月1日 要看作 2002年的13月1日 来计算；
//  *  d：日。
//  * 
//  * @param [in ] xut_year  : 年。
//  * @param [in ] xut_month : 月。
//  * @param [in ] xut_day   : 日。
//  * 
//  * @return x_int32_t :
//  * 星期编号 0 ~ 6 （星期日 对应 0， 星期一 对应 1， ...）。
//  */
x_uint32_t time_week(x_uint32_t xut_year, x_uint32_t xut_month, x_uint32_t xut_day);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __XTIME_H__

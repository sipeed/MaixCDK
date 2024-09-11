/**
 * @file ntp_client.h
 * Copyright (c) 2018 Gaaagaa. All rights reserved.
 * 
 * @author  : Gaaagaa
 * @date    : 2018-10-19
 * @version : 1.0.0.0
 * @brief   : 使用 NTP 协议时的一些辅助函数接口以及相关的数据定义。
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

#ifndef __NTP_CLIENT_H__
#define __NTP_CLIENT_H__

#include "xtime.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

/**
 * 可通过定义 XNTP_DBG_OUTPUT 宏，
 * 使代码执行过程中，输出内部流程的调试信息
 */
// #define XNTP_DBG_OUTPUT

/** NTP 专用端口号 */
#define NTP_PORT   123

/** 定义 NTP 客户端工作对象的 指针类型 */
typedef struct xntp_client_t * xntp_cliptr_t;

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 打开 NTP 客户端工作对象。
 * 
 * @return xntp_cliptr_t : 
 * 成功，返回 工作对象；失败，返回 X_NULL，可通过 errno 查看错误码。
 */
xntp_cliptr_t ntpcli_open(void);

/**********************************************************/
/**
 * @brief 关闭 NTP 客户端工作对象。
 */
x_void_t ntpcli_close(xntp_cliptr_t xntp_this);

/**********************************************************/
/**
 * @brief 设置 NTP 服务端的 地址 与 端口号。
 * @note  必须在 ntpcli_req_time() 调用前，设置好这些工作参数。
 * 
 * @param [in ] xntp_this : NTP 客户端工作对象。
 * @param [in ] xszt_host : NTP 服务器的 IP（四段式 IP 地址） 或 域名（如 3.cn.pool.ntp.org）。
 * @param [in ] xut_port  : NTP 服务器的 端口号（可取默认的端口号 NTP_PORT : 123）。
 * 
 * @return x_int32_t : 
 * 返回 0 表示操作成功；其他值则表示操作失败的错误码。
 */
x_int32_t ntpcli_config(
                xntp_cliptr_t xntp_this,
                x_cstring_t xszt_host,
                x_uint16_t xut_port);

/**********************************************************/
/**
 * @brief 发送 NTP 请求，获取服务器时间戳。
 * 
 * @param [in ] xntp_this : NTP 客户端工作对象。
 * @param [in ] xut_tmout : 网络请求的超时时间（单位为毫秒）。
 * 
 * @return xtime_vnsec_t : 
 * 返回 时间计量值，可用 XTMVNSEC_IS_VALID() 判断是否为有效值；
 * 若值无效，则可通过 errno 获知错误码。
 */
xtime_vnsec_t ntpcli_req_time(
                xntp_cliptr_t xntp_this,
                x_uint32_t xut_tmout);

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 
 * 向 NTP 服务器发送 NTP 请求，获取服务器时间戳。
 * @note 
 * 该接口内部自动 创建/销毁 NTP 客户端对象，执行完整的 NTP 请求流程。
 * 这适用于只执行单次请求操作。
 *
 * @param [in ] xszt_host : NTP 服务器的 IP（四段式 IP 地址） 或 域名（如 3.cn.pool.ntp.org）。
 * @param [in ] xut_port  : NTP 服务器的 端口号（可取默认的端口号 NTP_PORT : 123）。
 * @param [in ] xut_tmout : 网络请求的超时时间（单位为毫秒）。
 *
 * @return xtime_vnsec_t : 
 * 返回 时间计量值，可用 XTMVNSEC_IS_VALID() 判断是否为有效值；
 * 若值无效，则可通过 errno 获知错误码。
 */
xtime_vnsec_t ntpcli_get_time(
                    x_cstring_t xszt_host,
                    x_uint16_t xut_port,
                    x_uint32_t xut_tmout);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __NTP_CLIENT_H__

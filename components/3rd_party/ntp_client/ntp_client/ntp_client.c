/**
 * @file ntp_client.c
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

#include "ntp_client.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if (defined(_WIN32) || defined(_WIN64))
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#elif (defined(__linux__) || defined(__unix__))
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
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
// 内部相关数据类型与常量
// 

/**
 * @struct xtime_stamp_t
 * @brief  NTP所使用的时间戳。
 */
typedef struct xtime_stamp_t
{
    x_uint32_t xut_seconds;  ///< 从 1900年至今所经过的秒数
    x_uint32_t xut_fraction; ///< 小数部份，其单位是 百纳秒数 的 (2^32 / 10^7) 倍
} xtime_stamp_t;

/**
 * 1900-01-01 00:00:00 ~ 1970-01-01 00:00:00 之间的时间 秒数。
 * Time of day conversion constant.  Ntp's time scale starts in 1900,
 * Unix in 1970.  The value is 1970 - 1900 in seconds, 0x83aa7e80 or
 * 2208988800.  This is larger than 32-bit INT_MAX, so unsigned
 * type is forced.
 */
#define XTIME_SEC_1900_1970 0x83AA7E80

/** 百纳秒 的进位基数 10^7 */
#define XTIME_100NS_BASE    10000000ULL

/** 将 时间计量值 转为 NTP 时间戳 */
#define XTIME_UTOS(xvnsec, xstamp)                                                                  \
do                                                                                                  \
{                                                                                                   \
    (xstamp).xut_seconds  = (x_uint32_t)((((xvnsec) / XTIME_100NS_BASE) + XTIME_SEC_1900_1970));    \
    (xstamp).xut_fraction = (x_uint32_t)((((xvnsec) % XTIME_100NS_BASE) << 32) / XTIME_100NS_BASE); \
} while (0)

/** 将 NTP 时间戳 转为 时间计量值 */
#define XTIME_STOU(xstamp, xvnsec)                                                     \
do                                                                                     \
{                                                                                      \
    if (((xstamp).xut_seconds) > XTIME_SEC_1900_1970)                                  \
        (xvnsec) = (((xstamp).xut_seconds - XTIME_SEC_1900_1970) * XTIME_100NS_BASE) + \
                    (((xstamp).xut_fraction * XTIME_100NS_BASE) >> 32);                \
    else                                                                               \
        (xvnsec) = XTIME_INVALID_VNSEC;                                                \
} while (0)

/**
 * @enum  xntp_mode_t
 * @brief NTP工作模式的相关枚举值。
 */
typedef enum xntp_mode_t
{
    ntp_mode_unknow     = 0,  ///< 未定义
    ntp_mode_initiative = 1,  ///< 主动对等体模式
    ntp_mode_passive    = 2,  ///< 被动对等体模式
    ntp_mode_client     = 3,  ///< 客户端模式
    ntp_mode_server     = 4,  ///< 服务器模式
    ntp_mode_broadcast  = 5,  ///< 广播模式或组播模式
    ntp_mode_control    = 6,  ///< 报文为 NTP 控制报文
    ntp_mode_reserved   = 7,  ///< 预留给内部使用
} xntp_mode_t;

/**
 * @struct xntp_pack_t
 * @brief  NTP 报文格式。
 */
typedef struct xntp_pack_t
{
#if 1

    x_uchar_t     xct_lvmflag  ;  ///< 2 bits，飞跃指示器；3 bits，版本号；3 bits，NTP工作模式（参看 xntp_mode_t 相关枚举值）
    x_uchar_t     xct_stratum  ;  ///< 系统时钟的层数，取值范围为1~16，它定义了时钟的准确度。层数为1的时钟准确度最高，准确度从1到16依次递减，层数为16的时钟处于未同步状态，不能作为参考时钟
    x_uchar_t     xct_ppoll    ;  ///< 轮询时间，即两个连续NTP报文之间的时间间隔
    x_char_t      xct_percision;  ///< 系统时钟的精度

    x_uint32_t    xut_rootdelay;  ///< 本地到主参考时钟源的往返时间
    x_uint32_t    xut_rootdisp ;  ///< 系统时钟相对于主参考时钟的最大误差
    x_uint32_t    xut_refid    ;  ///< 参考时钟源的标识

    /**
     * T1，客户端发送请求时的 本地系统时间戳；
     * T2，服务端接收到客户端请求时的 本地系统时间戳；
     * T3，服务端发送应答数据包时的 本地系统时间戳；
     * T4，客户端接收到服务端应答数据包时的 本地系统时间戳。
     */
    xtime_stamp_t xtms_reference; ///< 系统时钟最后一次被设定或更新的时间
    xtime_stamp_t xtms_originate; ///< 服务端应答时，将客户端请求时的 T1 返送回去
    xtime_stamp_t xtms_receive  ; ///< 服务端接收到客户端请求时的 本地系统时间戳 T2
    xtime_stamp_t xtms_transmit ; ///< 客户端请求时 发送 T1，服务端应答时 回复 T3

#else

    x_uchar_t     xct_lvmflag;    ///< peer leap indicator: leap, version, mode
    x_uchar_t     xct_stratum;    ///< peer stratum
    x_uchar_t     xct_ppoll;      ///< peer poll interval
    x_char_t      xct_precision;  ///< peer clock precision
    x_uint32_t    xut_rootdelay;  ///< roundtrip delay to primary source
    x_uint32_t    xut_rootdisp;   ///< dispersion to primary sourc
    x_uint32_t    xut_refid;      ///< reference id
    xtime_stamp_t xtms_reference; ///< last update time
    xtime_stamp_t xtms_originate; ///< originate time stamp
    xtime_stamp_t xtms_receive  ; ///< receive time stamp
    xtime_stamp_t xtms_transmit ; ///< transmit time stamp

/** max extension field size */
#define NTP_MAXEXTEN   2048

#define	MIN_V4_PKT_LEN (12 * sizeof(u_int32))    ///< min header length
#define	LEN_PKT_NOMAC  (12 * sizeof(u_int32))    ///< min header length
#define	MIN_MAC_LEN    (1 * sizeof(u_int32))     ///< crypto_NAK
#define	MAX_MD5_LEN    (5 * sizeof(u_int32))     ///< MD5
#define	MAX_MAC_LEN    (6 * sizeof(u_int32))     ///< SHA
#define	KEY_MAC_LEN    sizeof(u_int32)           ///< key ID in MAC
#define	MAX_MDG_LEN    (MAX_MAC_LEN-KEY_MAC_LEN) ///< max. digest len

    /**
     * The length of the packet less MAC must be a multiple of 64
     * with an RSA modulus and Diffie-Hellman prime of 256 octets
     * and maximum host name of 128 octets, the maximum autokey
     * command is 152 octets and maximum autokey response is 460
     * octets. A packet can contain no more than one command and one
     * response, so the maximum total extension field length is 864
     * octets. But, to handle humungus certificates, the bank must
     * be broke.
     * 
     * The different definitions of the 'exten' field are here for
     * the benefit of applications that want to send a packet from
     * an auto variable in the stack - not using the AUTOKEY version
     * saves 2KB of stack space. The receive buffer should ALWAYS be
     * big enough to hold a full extended packet if the extension
     * fields have to be parsed or skipped.
     */
#ifdef AUTOKEY
	x_uint32_t	xut_exten[(NTP_MAXEXTEN + MAX_MAC_LEN) / sizeof(x_uint32_t)];
#else // !AUTOKEY follows
	x_uint32_t	xut_exten[(MAX_MAC_LEN) / sizeof(x_uint32_t)];
#endif // !AUTOKEY

#endif
} xntp_pack_t;

////////////////////////////////////////////////////////////////////////////////

// 
// 定义相关的调试信息输出接口
// 

#ifdef XNTP_DBG_OUTPUT

#include <stdio.h>

/**********************************************************/
/**
 * @brief 输出 100ns 为单位 的 具体时间信息。
 */
static x_void_t output_ns(xtime_vnsec_t xtm_vnsec)
{
    xtime_descr_t xtm_descr = time_vtod(xtm_vnsec);
    printf("[%04d-%02d-%02d %d %02d:%02d:%02d.%03d]",
           xtm_descr.ctx_year  ,
           xtm_descr.ctx_month ,
           xtm_descr.ctx_day   ,
           xtm_descr.ctx_week  ,
           xtm_descr.ctx_hour  ,
           xtm_descr.ctx_minute,
           xtm_descr.ctx_second,
           xtm_descr.ctx_msec  );
}

/**********************************************************/
/**
 * @brief 输出 NTP 时间戳 的 具体时间信息。
 */
static x_void_t output_ts(xtime_stamp_t * xtm_stamp)
{
    xtime_vnsec_t xtm_vnsec = XTIME_INVALID_VNSEC;
    XTIME_STOU(*xtm_stamp, xtm_vnsec);
    output_ns(xtm_vnsec);
}

/**********************************************************/
/**
 * @brief 打上信息标号，输出 100ns 为单位 的 具体时间信息。
 */
static x_void_t output_tu(x_cstring_t xszt_info, xtime_vnsec_t xtm_vnsec)
{
    printf("%s : ", xszt_info);
    output_ns(xtm_vnsec);
    printf("\n");
}

/**********************************************************/
/**
 * @brief 打上信息标号，输出 NTP 时间戳 的 具体时间信息。
 */
static x_void_t output_tm(x_cstring_t xszt_info, xtime_stamp_t * xtm_stamp)
{
    printf("%s : ", xszt_info);
    output_ts(xtm_stamp);
    printf("\n");
}

#endif // XNTP_DBG_OUTPUT

////////////////////////////////////////////////////////////////////////////////

// 
// 一些相关辅助操作的接口
// 

/**********************************************************/
/**
 * @brief 判断字符串是否为有效的 4 段式 IP 地址格式。
 *
 * @param [in ] xszt_name : 判断的字符串。
 * @param [out] xut_value : 若入参不为 X_NULL，则操作成功时，返回对应的 IP 地址值。
 *
 * @return x_bool_t : 成功，返回 X_TRUE；失败，返回 X_FALSE。
 */
static x_bool_t name_is_ipv4(x_cstring_t xszt_name, x_uint32_t * xut_value)
{
    x_uchar_t xct_ipv[4] = { 0, 0, 0, 0 };

    x_int32_t xit_itv = 0;
    x_int32_t xit_sum = 0;
    x_bool_t  xbt_okv = X_FALSE;

    const x_char_t * xct_ptr = xszt_name;

    if (X_NULL == xszt_name)
    {
        return X_FALSE;
    }

    //======================================

    do
    {
        if ((*xct_ptr != '\0') && (*xct_ptr >= '0') && (*xct_ptr <= '9'))
        {
            xit_sum = 10 * xit_sum + (*xct_ptr - '0');
            xbt_okv = X_TRUE;
        }
        else if (xbt_okv                                   && 
                 (('.' == *xct_ptr) || ('\0' == *xct_ptr)) && 
                 (xit_itv < (x_int32_t)sizeof(xct_ipv))    && 
                 (xit_sum <= 0xFF))
        {
            xct_ipv[xit_itv++] = (x_uchar_t)xit_sum;
            xit_sum = 0;
            xbt_okv = X_FALSE;
        }
        else
        {
            break;
        }
    } while (*xct_ptr++);

    xbt_okv = (xit_itv == sizeof(xct_ipv)) ? X_TRUE : X_FALSE;

    //======================================

#define MAKE_IPV4_VALUE(b1, b2, b3, b4)      \
    ((x_uint32_t)(((x_uint32_t)(b1) << 24) + \
                  ((x_uint32_t)(b2) << 16) + \
                  ((x_uint32_t)(b3) <<  8) + \
                  ((x_uint32_t)(b4) <<  0)))

    if (X_NULL != xut_value)
    {
        if (xbt_okv)
            *xut_value = MAKE_IPV4_VALUE(xct_ipv[0], xct_ipv[1], xct_ipv[2], xct_ipv[3]);
        else
            *xut_value = 0xFFFFFFFF;
    }

#undef MAKE_IPV4_VALUE

//======================================

    return xbt_okv;
}

/**********************************************************/
/**
 * @brief 返回套接字当前操作失败的错误码。
 */
static x_int32_t sockfd_errno()
{
#ifdef _MSC_VER
    return (x_int32_t)WSAGetLastError();
#else // !_MSC_VER
    return errno;
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 设置套接字为 非阻塞模式。
 * 
 * @param [in ] xfdt_sockfd : 套接字。
 * 
 * @return x_int32_t : 成功，返回 0；失败，返回 错误码。
 */
static x_int32_t sockfd_nbio(x_sockfd_t xfdt_sockfd)
{
#if (defined(_WIN32) || defined(_WIN64))

    x_ulong_t xult_mode = X_TRUE;
    return ioctlsocket(xfdt_sockfd, FIONBIO, &xult_mode);

#elif (defined(__linux__) || defined(__unix__))

    x_int32_t xit_flag = fcntl(xfdt_sockfd, F_GETFL, 0);
    if (fcntl(xfdt_sockfd, F_SETFL, xit_flag | O_NONBLOCK) < 0)
        return errno;
    return 0;

#else // UNKNOW
    return EPERM;
#endif // PLATFORM
}

/**********************************************************/
/**
 * @brief 设置套接字的 数据收发缓存的 超时时间。
 * 
 * @param [in ] xut_tmout : 超时时间（单位为 毫秒）。
 */
static x_void_t sockfd_rwbuf_tmout(x_sockfd_t xfdt_sockfd, x_uint32_t xut_tmout)
{
#if (defined(_WIN32) || defined(_WIN64))

    setsockopt(
        xfdt_sockfd,
        SOL_SOCKET,
        SO_SNDTIMEO,
        (x_char_t *)&xut_tmout,
        sizeof(x_uint32_t));

    setsockopt(
        xfdt_sockfd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (x_char_t *)&xut_tmout,
        sizeof(x_uint32_t));

#elif (defined(__linux__) || defined(__unix__))

    struct timeval xtm_value;

    xtm_value.tv_sec  = (x_long_t)((xut_tmout / 1000));
    xtm_value.tv_usec = (x_long_t)((xut_tmout % 1000) * 1000);

    setsockopt(
        xfdt_sockfd,
        SOL_SOCKET,
        SO_SNDTIMEO,
        (x_char_t *)&xtm_value,
        sizeof(struct timeval));

    setsockopt(
        xfdt_sockfd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (x_char_t *)&xtm_value,
        sizeof(struct timeval));

#else // UNKNOW
#endif // PLATFORM
}

/**********************************************************/
/**
 * @brief 关闭套接字。
 */
static x_int32_t sockfd_close(x_sockfd_t xfdt_sockfd)
{
#if (defined(_WIN32) || defined(_WIN64))
    return closesocket(xfdt_sockfd);
#else // !(defined(_WIN32) || defined(_WIN64))
    return close(xfdt_sockfd);
#endif // (defined(_WIN32) || defined(_WIN64))
}

////////////////////////////////////////////////////////////////////////////////

// 
// NTP 相关辅助操作的接口
// 

/**********************************************************/
/**
 * @brief 初始化 NTP 的请求数据包。
 */
static x_void_t ntp_init_req_packet(xntp_pack_t * xnpt_dptr)
{
/*
 * Stuff for putting things back into li_vn_mode in packets and vn_mode
 * in ntp_monitor.c's mon_entry.
 */
#define NTP_VN_MODE(v, m)		((((v) & 7) << 3) | ((m) & 0x7))
#define	NTP_LI_VN_MODE(l, v, m) ((((l) & 3) << 6) | NTP_VN_MODE((v), (m)))

    xnpt_dptr->xct_lvmflag   = NTP_LI_VN_MODE(0, 3, ntp_mode_client);
    xnpt_dptr->xct_stratum   = 0;
    xnpt_dptr->xct_ppoll     = 4;
    xnpt_dptr->xct_percision = (x_char_t)(-6);

    xnpt_dptr->xut_rootdelay = (1 << 16);
    xnpt_dptr->xut_rootdisp  = (1 << 16);
    xnpt_dptr->xut_refid     = 0;

    xnpt_dptr->xtms_reference.xut_seconds  = 0;
    xnpt_dptr->xtms_reference.xut_fraction = 0;
    xnpt_dptr->xtms_originate.xut_seconds  = 0;
    xnpt_dptr->xtms_originate.xut_fraction = 0;
    xnpt_dptr->xtms_receive  .xut_seconds  = 0;
    xnpt_dptr->xtms_receive  .xut_fraction = 0;
    xnpt_dptr->xtms_transmit .xut_seconds  = 0;
    xnpt_dptr->xtms_transmit .xut_fraction = 0;
}

/**********************************************************/
/**
 * @brief 将 xntp_pack_t 中的 网络字节序 字段转换为 主机字节序。
 */
static x_void_t ntp_ntoh_packet(xntp_pack_t * xnpt_nptr)
{
#if 0
    xnpt_nptr->xct_lvmflag   = xnpt_nptr->xct_lvmflag  ;
    xnpt_nptr->xct_stratum   = xnpt_nptr->xct_stratum  ;
    xnpt_nptr->xct_ppoll     = xnpt_nptr->xct_ppoll    ;
    xnpt_nptr->xct_percision = xnpt_nptr->xct_percision;
#endif
    xnpt_nptr->xut_rootdelay               = ntohl(xnpt_nptr->xut_rootdelay              );
    xnpt_nptr->xut_rootdisp                = ntohl(xnpt_nptr->xut_rootdisp               );
    xnpt_nptr->xut_refid                   = ntohl(xnpt_nptr->xut_refid                  );
    xnpt_nptr->xtms_reference.xut_seconds  = ntohl(xnpt_nptr->xtms_reference.xut_seconds );
    xnpt_nptr->xtms_reference.xut_fraction = ntohl(xnpt_nptr->xtms_reference.xut_fraction);
    xnpt_nptr->xtms_originate.xut_seconds  = ntohl(xnpt_nptr->xtms_originate.xut_seconds );
    xnpt_nptr->xtms_originate.xut_fraction = ntohl(xnpt_nptr->xtms_originate.xut_fraction);
    xnpt_nptr->xtms_receive  .xut_seconds  = ntohl(xnpt_nptr->xtms_receive  .xut_seconds );
    xnpt_nptr->xtms_receive  .xut_fraction = ntohl(xnpt_nptr->xtms_receive  .xut_fraction);
    xnpt_nptr->xtms_transmit .xut_seconds  = ntohl(xnpt_nptr->xtms_transmit .xut_seconds );
    xnpt_nptr->xtms_transmit .xut_fraction = ntohl(xnpt_nptr->xtms_transmit .xut_fraction);
}

/**********************************************************/
/**
 * @brief 将 xntp_pack_t 中的 主机字节序 字段转换为 网络字节序。
 */
static x_void_t ntp_hton_packet(xntp_pack_t * xnpt_nptr)
{
#if 0
    xnpt_nptr->xct_lvmflag   = xnpt_nptr->xct_lvmflag  ;
    xnpt_nptr->xct_stratum   = xnpt_nptr->xct_stratum  ;
    xnpt_nptr->xct_ppoll     = xnpt_nptr->xct_ppoll    ;
    xnpt_nptr->xct_percision = xnpt_nptr->xct_percision;
#endif
    xnpt_nptr->xut_rootdelay               = htonl(xnpt_nptr->xut_rootdelay              );
    xnpt_nptr->xut_rootdisp                = htonl(xnpt_nptr->xut_rootdisp               );
    xnpt_nptr->xut_refid                   = htonl(xnpt_nptr->xut_refid                  );
    xnpt_nptr->xtms_reference.xut_seconds  = htonl(xnpt_nptr->xtms_reference.xut_seconds );
    xnpt_nptr->xtms_reference.xut_fraction = htonl(xnpt_nptr->xtms_reference.xut_fraction);
    xnpt_nptr->xtms_originate.xut_seconds  = htonl(xnpt_nptr->xtms_originate.xut_seconds );
    xnpt_nptr->xtms_originate.xut_fraction = htonl(xnpt_nptr->xtms_originate.xut_fraction);
    xnpt_nptr->xtms_receive  .xut_seconds  = htonl(xnpt_nptr->xtms_receive  .xut_seconds );
    xnpt_nptr->xtms_receive  .xut_fraction = htonl(xnpt_nptr->xtms_receive  .xut_fraction);
    xnpt_nptr->xtms_transmit .xut_seconds  = htonl(xnpt_nptr->xtms_transmit .xut_seconds );
    xnpt_nptr->xtms_transmit .xut_fraction = htonl(xnpt_nptr->xtms_transmit .xut_fraction);
}

/**********************************************************/
/**
 * @brief 计算最后的结果，公式：T = T4 + ((T2 - T1) + (T3 - T4)) / 2;
 * @note
 * T1，客户端发送请求时的 本地系统时间戳；
 * T2，服务端接收到客户端请求时的 本地系统时间戳；
 * T3，服务端发送应答数据包时的 本地系统时间戳；
 * T4，客户端接收到服务端应答数据包时的 本地系统时间戳。
 */
static xtime_vnsec_t ntp_calc_4T(xtime_vnsec_t xtm_4time[4])
{
    x_int64_t xtm_T21 = ((x_int64_t)xtm_4time[1]) - ((x_int64_t)xtm_4time[0]);
    x_int64_t xtm_T34 = ((x_int64_t)xtm_4time[2]) - ((x_int64_t)xtm_4time[3]);
    x_int64_t xtm_TXX = ((x_int64_t)xtm_4time[3]) + ((xtm_T21 + xtm_T34) / 2);

    return (xtime_vnsec_t)xtm_TXX;
}

////////////////////////////////////////////////////////////////////////////////

// 
// NTP 内部相关操作接口与数据类型
// 

/**
 * @struct xntp_client_t
 * @brief  NTP 客户端工作对象的结构体描述信息。
 */
typedef struct xntp_client_t
{
    x_sockfd_t    xfdt_sockfd;              ///< 网络通信使用的 套接字
    x_char_t      xszt_host[TEXT_LEN_256];  ///< 存储提供 NTP 服务的 服务端 地址
    x_uint16_t    xut_port;                 ///< 存储提供 NTP 服务的 服务端 端口号
    xtime_vnsec_t xtm_4time[4];             ///< 完成 NTP 请求后，所得到的 4 个相关时间戳
} xntp_client_t;

/**********************************************************/
/**
 * @brief 向 NTP 服务器发送 NTP 请求，获取相关计算所需的时间戳（T1、T2、T3、T4如下所诉）。
 * <pre>
 *  1. 客户端 发送一个NTP报文给 服务端，该报文带有它离开 客户端 时的时间戳，该时间戳为 T1。
 *  2. 当此NTP报文到达 服务端 时，服务端 加上自己的时间戳，该时间戳为 T2。
 *  3. 当此NTP报文离开 服务端 时，服务端 再加上自己的时间戳，该时间戳为 T3。
 *  4. 当 客户端 接收到该应答报文时，客户端 的本地时间戳，该时间戳为 T4。
 * </pre>
 * 
 * @param [in ] xntp_this : NTP 客户端工作对象。
 * @param [in ] xszt_host : NTP 服务器的 IP（四段式 IP 地址）。
 * @param [in ] xut_tmout : 超时时间（单位 毫秒）。
 * 
 * @return x_int32_t : 成功，返回 0；失败，返回 错误码。
 */
static x_int32_t ntpcli_get_4T(
                    xntp_cliptr_t xntp_this,
                    x_cstring_t xszt_host,
                    x_uint32_t xut_tmout)
{
    x_int32_t xit_errno = EPERM;

    xntp_pack_t        xnpt_pack;
    x_int32_t          xit_alen;
    struct sockaddr_in xin_addr;
    fd_set             xfds_rset;
    struct timeval     xtm_value;

    do 
    {
        //======================================

        if ((X_NULL == xntp_this) || (X_NULL == xszt_host))
        {
            xit_errno = EINVAL;
            break;
        }

        xntp_this->xtm_4time[0] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[1] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[2] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[3] = XTIME_INVALID_VNSEC;

        //======================================

        // 服务端主机地址
        memset(&xin_addr, 0, sizeof(struct sockaddr_in));
        xin_addr.sin_family = AF_INET;
        xin_addr.sin_port   = htons(xntp_this->xut_port);
        inet_pton(AF_INET, xszt_host, &xin_addr.sin_addr.s_addr);

        //======================================

        // 初始化请求数据包
        ntp_init_req_packet(&xnpt_pack);

        // T1
        xntp_this->xtm_4time[0] = time_vnsec();

        // NTP请求报文离开发送端时发送端的本地时间
        XTIME_UTOS(xntp_this->xtm_4time[0], xnpt_pack.xtms_transmit);

        // 转成网络字节序
        ntp_hton_packet(&xnpt_pack);

        // 发送 NTP 请求
        xit_errno = sendto(
                        xntp_this->xfdt_sockfd,
                        (x_char_t *)&xnpt_pack,
                        sizeof(xntp_pack_t),
                        0,
                        (struct sockaddr *)&xin_addr,
                        sizeof(struct sockaddr_in));
        if (xit_errno < 0)
        {
            xit_errno = sockfd_errno();
#if (defined(_WIN32) || defined(_WIN64))
            if (WSAEWOULDBLOCK != xit_errno)
#elif (defined(__linux__) || defined(__unix__))
            if ((EAGAIN != xit_errno) && (EWOULDBLOCK != xit_errno))
#else // UNKNOW
#endif // PLATFORM
            {
                break;
            }
        }

        //======================================
        // 使用 select() 检测套接字可读

        FD_ZERO(&xfds_rset);
        FD_SET(xntp_this->xfdt_sockfd, &xfds_rset);

        // 超时时间
        if (xut_tmout > 0)
        {
            xtm_value.tv_sec  = (xut_tmout / 1000);
            xtm_value.tv_usec = (xut_tmout % 1000) * 1000;
        }

        xit_errno = select(
                        (x_int32_t)(xntp_this->xfdt_sockfd + 1),
                        &xfds_rset,
                        X_NULL,
                        X_NULL,
                        (xut_tmout > 0) ? &xtm_value : X_NULL);
        if (xit_errno <= 0)
        {
            xit_errno = (0 == xit_errno) ? ETIMEDOUT : sockfd_errno();
            break;
        }

        if (!FD_ISSET(xntp_this->xfdt_sockfd, &xfds_rset))
        {
            xit_errno = EBADF;
            break;
        }

        //======================================

        memset(&xnpt_pack, 0, sizeof(xntp_pack_t));

        // 接收应答
        xit_alen  = sizeof(struct sockaddr_in);
        xit_errno = recvfrom(
                        xntp_this->xfdt_sockfd,
                        (x_char_t *)&xnpt_pack,
                        sizeof(xntp_pack_t),
                        0,
                        (struct sockaddr *)&xin_addr,
                        (socklen_t *)&xit_alen);
        // T4
        xntp_this->xtm_4time[3] = time_vnsec();

        if (xit_errno < 0)
        {
            xit_errno = sockfd_errno();
            break;
        }

        // 判断数据包长度是否有效
        if (sizeof(xntp_pack_t) != xit_errno)
        {
            xit_errno = ENODATA;
            break;
        }

        // 转成主机字节序
        ntp_ntoh_packet(&xnpt_pack);

        XTIME_STOU(xnpt_pack.xtms_receive , xntp_this->xtm_4time[1]); // T2
        XTIME_STOU(xnpt_pack.xtms_transmit, xntp_this->xtm_4time[2]); // T3

        if (!XTMVNSEC_IS_VALID(xntp_this->xtm_4time[1]) ||
            !XTMVNSEC_IS_VALID(xntp_this->xtm_4time[2]))
        {
            xit_errno = ETIME;
            break;
        }

        //======================================
#ifdef XNTP_DBG_OUTPUT
        printf("========================================\n"
               "%s : %s\n", xntp_this->xszt_host, xszt_host);
        output_tm("\tNTP RT", &xnpt_pack.xtms_reference);
        output_tm("\tNTP T1", &xnpt_pack.xtms_originate);
        output_tm("\tNTP T2", &xnpt_pack.xtms_receive  );
        output_tm("\tNTP T3", &xnpt_pack.xtms_transmit );
        output_tu("\tSYS T1", xntp_this->xtm_4time[0]);
        output_tu("\tSYS T2", xntp_this->xtm_4time[1]);
        output_tu("\tSYS T3", xntp_this->xtm_4time[2]);
        output_tu("\tSYS T4", xntp_this->xtm_4time[3]);
        printf("\n");
#endif // XNTP_DBG_OUTPUT
        //======================================
        xit_errno = 0;
    } while (0);

    return xit_errno;
}

/**********************************************************/
/**
 * @brief 向 NTP 服务器发送 NTP 请求，获取相关计算所需的时间戳。
 *
 * @param [in ] xntp_this : NTP 客户端工作对象。
 * @param [in ] xut_tmout : 网络请求的超时时间（单位为毫秒）。
 *
 * @return x_int32_t : 成功，返回 0；失败，返回 错误码。
 */
static x_int32_t ntpcli_get_4T_by_name(
                        xntp_cliptr_t xntp_this,
                        x_uint32_t xut_tmout)
{
    x_int32_t xit_errno = EPERM;

    struct addrinfo   xai_hint;
    struct addrinfo * xai_rptr = X_NULL;
    struct addrinfo * xai_iptr = X_NULL;

    x_char_t xszt_host[TEXT_LEN_256] = { 0 };

    do
    {
        //======================================

        if (X_NULL == xntp_this)
        {
            xit_errno = EINVAL;
            break;
        }

        memset(&xai_hint, 0, sizeof(xai_hint));
        xai_hint.ai_family   = AF_INET;
        xai_hint.ai_socktype = SOCK_DGRAM;

        xit_errno = getaddrinfo(xntp_this->xszt_host, X_NULL, &xai_hint, &xai_rptr);
        if (0 != xit_errno)
        {
            break;
        }

        //======================================

        for (xai_iptr = xai_rptr; X_NULL != xai_iptr; xai_iptr = xai_iptr->ai_next)
        {
            if (AF_INET != xai_iptr->ai_family)
            {
                continue;
            }

            memset(xszt_host, 0, TEXT_LEN_256);
            if (X_NULL == inet_ntop(
                            AF_INET,
                            &(((struct sockaddr_in *)(xai_iptr->ai_addr))->sin_addr),
                            xszt_host,
                            TEXT_LEN_256))
            {
                continue;
            }

            xit_errno = ntpcli_get_4T(xntp_this, xszt_host, xut_tmout);
            if (0 == xit_errno)
            {
                break;
            }
        }

        //======================================
    } while (0);

    if (X_NULL != xai_rptr)
    {
        freeaddrinfo(xai_rptr);
        xai_rptr = X_NULL;
    }

    return xit_errno;
}

////////////////////////////////////////////////////////////////////////////////

// 
// NTP 外部相关操作接口
// 

/**********************************************************/
/**
 * @brief 打开 NTP 客户端工作对象。
 * 
 * @return xntp_cliptr_t : 
 * 成功，返回 工作对象；失败，返回 X_NULL，可通过 errno 查看错误码。
 */
xntp_cliptr_t ntpcli_open(void)
{
    x_int32_t     xit_errno = EPERM;
    xntp_cliptr_t xntp_this = X_NULL;

    do
    {
        //======================================

        xntp_this = (xntp_cliptr_t)malloc(sizeof(xntp_client_t));
        if (X_NULL == xntp_this)
        {
            xit_errno = errno;
            break;
        }

        //======================================
        // socket fd

        xntp_this->xfdt_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (X_INVALID_SOCKFD == xntp_this->xfdt_sockfd)
        {
            xit_errno = sockfd_errno();
            break;
        }

        if (0 != sockfd_nbio(xntp_this->xfdt_sockfd))
        {
            xit_errno = sockfd_errno();
            break;
        }

        //======================================

        memset(xntp_this->xszt_host, 0, TEXT_LEN_256);
        xntp_this->xut_port = NTP_PORT;

        xntp_this->xtm_4time[0] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[1] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[2] = XTIME_INVALID_VNSEC;
        xntp_this->xtm_4time[3] = XTIME_INVALID_VNSEC;

        //======================================
        xit_errno = 0;
    } while (0);

    if (0 != xit_errno)
    {
        ntpcli_close(xntp_this);
        xntp_this = X_NULL;
    }

    return xntp_this;
}

/**********************************************************/
/**
 * @brief 关闭 NTP 客户端工作对象。
 */
x_void_t ntpcli_close(xntp_cliptr_t xntp_this)
{
    if (X_NULL == xntp_this)
    {
        return;
    }

    if (X_INVALID_SOCKFD != xntp_this->xfdt_sockfd)
    {
        sockfd_close(xntp_this->xfdt_sockfd);
    }

    free(xntp_this);
}

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
                x_uint16_t xut_port)
{
    if ((X_NULL == xntp_this) ||
        (X_NULL == xszt_host) ||
        (strlen(xszt_host) >= TEXT_LEN_256))
    {
        return EINVAL;
    }

#if (defined(_WIN32) || defined(_WIN64))
    strncpy_s(xntp_this->xszt_host, TEXT_LEN_256, xszt_host, TEXT_LEN_256);
#else // !(defined(_WIN32) || defined(_WIN64))
    strncpy(xntp_this->xszt_host, xszt_host, TEXT_LEN_256);
#endif // PLATFORM

    xntp_this->xut_port = xut_port;

    return 0;
}

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
xtime_vnsec_t ntpcli_req_time(xntp_cliptr_t xntp_this, x_uint32_t xut_tmout)
{
    x_int32_t xit_errno = EPERM;

    //======================================
    // 参数验证

    if (X_NULL == xntp_this)
    {
        errno = EINVAL;
        return XTIME_INVALID_VNSEC;
    }

    //======================================

    if (name_is_ipv4(xntp_this->xszt_host, X_NULL))
        xit_errno = ntpcli_get_4T(xntp_this, xntp_this->xszt_host, xut_tmout);
    else
        xit_errno = ntpcli_get_4T_by_name(xntp_this, xut_tmout);

    if (0 != xit_errno)
    {
        errno = xit_errno;
        return XTIME_INVALID_VNSEC;
    }

    //======================================

    return ntp_calc_4T(xntp_this->xtm_4time);

    //======================================
}

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
                    x_uint32_t xut_tmout)
{
    x_int32_t     xit_errno = EPERM;
    xtime_vnsec_t xtm_vnsec = XTIME_INVALID_VNSEC;
    xntp_cliptr_t xntp_this = X_NULL;

    //======================================

    do
    {
        xntp_this = ntpcli_open();
        if (X_NULL == xntp_this)
        {
            break;
        }

        xit_errno = ntpcli_config(xntp_this, xszt_host, xut_port);
        if (0 != xit_errno)
        {
            errno = xit_errno;
            break;
        }

        xtm_vnsec = ntpcli_req_time(xntp_this, xut_tmout);
    } while (0);

    if (X_NULL != xntp_this)
    {
        ntpcli_close(xntp_this);
        xntp_this = X_NULL;
    }

    //======================================

    return xtm_vnsec;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

////////////////////////////////////////////////////////////////////////////////

/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_SYS_LOG_H_
#define _AX_SYS_LOG_H_
#include "ax_base_type.h"
#include "ax_global_type.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
    LOG_EMERG     system is unusable
    LOG_ALERT     action must be taken immediately
    LOG_CRIT      critical conditions
    LOG_ERR       error conditions
    LOG_WARNING   warning conditions
    LOG_NOTICE    normal, but significant, condition
    LOG_INFO      informational message
    LOG_DEBUG     debug-level message
*/

AX_VOID AX_SYS_LogPrint(AX_S32 level, AX_CHAR const *tag, int id, AX_CHAR const *pFormat, ...);
AX_VOID AX_SYS_LogOutput(AX_LOG_TARGET_E target, AX_LOG_LEVEL_E level, AX_CHAR const *tag, int id, AX_CHAR const *format, va_list vlist);

#define AX_SYS_LogPrint_Ex AX_SYS_LogPrint
#define AX_SYS_LogOutput_Ex AX_SYS_LogOutput

#ifdef __cplusplus
}
#endif

#define MACRO_BLACK         "\033[1;30;30m"
#define MACRO_RED           "\033[1;30;31m"
#define MACRO_GREEN         "\033[1;30;32m"
#define MACRO_YELLOW        "\033[1;30;33m"
#define MACRO_BLUE          "\033[1;30;34m"
#define MACRO_PURPLE        "\033[1;30;35m"
#define MACRO_WHITE         "\033[1;30;37m"
#define MACRO_END           "\033[0m"

#define AX_MSYS_LOG_TAG  "MSYS"
#define AX_ISP_LOG_TAG  "ISP"

#define AX_LOG_EMERG(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_EMERGENCY, tag, id, "[M][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_ALERT(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_ALERT, tag, id, "[A][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_CRIT(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_CRITICAL, tag, id, "[C][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_ERR(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_ERROR, tag, id, "[E][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_WARN(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_WARN, tag, id, "[W][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_INFO(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_INFO, tag, id, "[I][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_DBG(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_DEBUG, tag, id, "[D][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_NOTICE(tag,id,fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_NOTICE, tag, id, "[N][%32s][%4d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif //_AX_SYS_LOG_H_

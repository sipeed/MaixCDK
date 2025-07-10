/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _SENSOR_USER_DEBUG_H_
#define _SENSOR_USER_DEBUG_H_

#include <stdio.h>

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_sys_log.h"

static AX_LOG_LEVEL_E sensor_print_level = SYS_LOG_DEBUG;
#define SENSOR_LOG_TAG "SENSOR"

#ifndef BUILD_TARGET_X86
#define sns_printf(level, fmt, ...) \
{ \
    if (level <= sensor_print_level) { \
        switch (level) { \
        case SYS_LOG_DEBUG: \
            AX_SYS_LogPrint(SYS_LOG_DEBUG, SENSOR_LOG_TAG, AX_ID_SENSOR, fmt, ##__VA_ARGS__); \
            break; \
        case SYS_LOG_INFO: \
            AX_SYS_LogPrint(SYS_LOG_INFO, SENSOR_LOG_TAG, AX_ID_SENSOR, fmt, ##__VA_ARGS__); \
            break; \
        case SYS_LOG_NOTICE: \
            AX_SYS_LogPrint(SYS_LOG_NOTICE, SENSOR_LOG_TAG, AX_ID_SENSOR, fmt, ##__VA_ARGS__); \
            break; \
        case SYS_LOG_WARN: \
            AX_SYS_LogPrint(SYS_LOG_WARN, SENSOR_LOG_TAG, AX_ID_SENSOR, fmt, ##__VA_ARGS__); \
            break; \
        case SYS_LOG_ERROR: \
            AX_SYS_LogPrint(SYS_LOG_ERROR, SENSOR_LOG_TAG, AX_ID_SENSOR, fmt, ##__VA_ARGS__); \
            break; \
        default: \
            break; \
        } \
    } \
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(SENSOR_LOG_USE_PRINTF) || (BUILD_TARGET_X86)
#define SNS_ERR(fmt, ...)       printf("[E][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_WRN(fmt, ...)       printf("[W][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_INFO(fmt, ...)      printf("[I][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_DBG(fmt, ...)       printf("[D][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define SNS_ERR(fmt, ...)       sns_printf(SYS_LOG_ERROR, "[E][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_WRN(fmt, ...)       sns_printf(SYS_LOG_WARN,  "[W][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_INFO(fmt, ...)      sns_printf(SYS_LOG_INFO,  "[I][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SNS_DBG(fmt, ...)       sns_printf(SYS_LOG_DEBUG, "[D][%32s][%4d]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


#ifdef __cplusplus
}
#endif

#endif //_SENSOR_USER_DEBUG_H_

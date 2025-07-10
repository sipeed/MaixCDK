/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#include <stdio.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ax_sys_log.h"
#include "isp_sensor_types.h"
#include "sensor_user_debug.h"

#define SENSOR_LOG_TAG "SENSOR"

static AX_LOG_TARGET_E sensor_print_target = SYS_LOG_TARGET_SYSLOG;

AX_S32 ax_sensor_set_log_level(AX_LOG_LEVEL_E level)
{
    if (level >= SYS_LOG_MAX || level <= SYS_LOG_MIN) {
        SNS_ERR("sensor log level is invalid[%d]\n", level);
        return AX_SNS_ERR_ILLEGAL_PARAM;
    }
    sensor_print_level = level;
    return AX_SNS_SUCCESS;
}

AX_LOG_LEVEL_E ax_sensor_get_log_level(void)
{
    return sensor_print_level;
}

AX_S32 ax_sensor_set_log_target(AX_LOG_TARGET_E target)
{
    if (target >= SYS_LOG_TARGET_MAX || target <= SYS_LOG_TARGET_MIN) {
        SNS_ERR("log target is invalid[%d]\n", target);
        return AX_SNS_ERR_ILLEGAL_PARAM;
    }
    sensor_print_target = target;
    return AX_SNS_SUCCESS;
}

AX_S32 ax_sensor_set_user_debug_log(AX_VOID)
{
    AX_CHAR *log_level;
    AX_CHAR *log_target;
    AX_LOG_LEVEL_E level = SYS_LOG_WARN;
    AX_LOG_TARGET_E target = SYS_LOG_TARGET_SYSLOG;

    log_level = (AX_CHAR *)getenv("SENSOR_LOG_level");
    if (log_level) {
        level = (AX_LOG_LEVEL_E)atoi((const char *)log_level);
        ax_sensor_set_log_level(level);
    }

    log_target = (AX_CHAR *)getenv("SENSOR_LOG_target");
    if (log_target) {
        target = (AX_LOG_TARGET_E)atoi((const char *)log_target);
        ax_sensor_set_log_target(target);
    }

    return AX_SNS_SUCCESS;
}

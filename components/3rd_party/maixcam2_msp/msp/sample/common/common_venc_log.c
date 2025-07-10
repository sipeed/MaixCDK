/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#include "common_venc_log.h"

AX_BOOL gVencLogLevel = COMM_VENC_LOG_INFO;

AX_VOID COMMON_VENC_LOG(AX_U32 eLv, char *fmt, ...)
{
    va_list args;
    AX_CHAR buf[MAX_VENC_LOG_INFO_SIZE];
    AX_U32 len = 0;

    memset(buf, 0x0, MAX_VENC_LOG_INFO_SIZE);
    if (eLv <= gVencLogLevel) {
        va_start(args, fmt);
        len = vsnprintf(buf, sizeof(buf), fmt, args);
        if (len < sizeof(buf))
            buf[len] = '\0';
        else
            buf[sizeof(buf) - 1] = '\0';
        printf("%s", buf);
        va_end(args);
    }
}
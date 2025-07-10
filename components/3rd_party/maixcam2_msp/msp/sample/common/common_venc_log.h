/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMM_VENC_LOG_H__
#define __COMM_VENC_LOG_H__

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "ax_venc_comm.h"

typedef enum {
    COMM_VENC_LOG_MIN         = -1,
    COMM_VENC_LOG_ERROR       = 0,
    COMM_VENC_LOG_WARN        = 1,
    COMM_VENC_LOG_INFO        = 2,
    COMM_VENC_LOG_DEBUG       = 3,
    COMM_VENC_LOG_MAX
} AX_COMM_VENC_LOG_LEVEL_E;

#define SAMPLE_VENC_NAME "SAMPLE-VENC"
#define MAX_VENC_LOG_INFO_SIZE (1024)

#define SAMPLE_LOG_ERR(fmt, ...)   \
    do { \
        COMMON_VENC_LOG(COMM_VENC_LOG_ERROR, "\033[1;30;31m[ERROR][%s][%s][%d]: "fmt"\033[0m\n\n", SAMPLE_VENC_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }  while(0)

#define SAMPLE_LOG_WARN(fmt, ...) \
    do { \
        COMMON_VENC_LOG(COMM_VENC_LOG_WARN, "\033[1;30;33m[WARN][%s][%s][%d]: "fmt"\033[0m\n\n", SAMPLE_VENC_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }  while(0)

#define SAMPLE_LOG_INFO(fmt, ...) \
    do { \
        COMMON_VENC_LOG(COMM_VENC_LOG_INFO, "[INFO][%s][%s][%d]: "fmt"", SAMPLE_VENC_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }  while(0)

#define SAMPLE_LOG(fmt, ...) \
    do { \
        COMMON_VENC_LOG(COMM_VENC_LOG_INFO, "[INFO][%s][%s][%d]: "fmt"", SAMPLE_VENC_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }  while(0)

#define SAMPLE_LOG_DEBUG(fmt, ...) \
    do { \
        COMMON_VENC_LOG(COMM_VENC_LOG_DEBUG, "\033[1;30;34m[DEBUG][%s][%s][%d]: "fmt"\033[0m\n\n", SAMPLE_VENC_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }  while(0)

void COMMON_VENC_LOG(AX_U32 eLv, char *fmt, ...);
#endif
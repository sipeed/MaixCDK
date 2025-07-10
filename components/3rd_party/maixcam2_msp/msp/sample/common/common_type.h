/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_TYPE_H__
#define __COMMON_TYPE_H__

#include "ax_global_type.h"

#define MAX_CAMERAS 16

#define AX_ALIGN_UP_SAMPLE(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#define AX_ALIGN_DOWN_SAMPLE(x, align) ((x) & ~((align)-1))

#define _ISP_BRANCH_DEBUG_H_

#define COMM_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)

typedef enum {
    SAMPLE_PIPE_MODE_VIDEO,
    SAMPLE_PIPE_MODE_PICTURE,
    SAMPLE_PIPE_MODE_FLASH_SNAP,      /* Snap of flash lamp */
    SAMPLE_PIPE_MODE_MAX,
} SAMPLE_PIPE_MODE_E;

typedef enum {
    SAMPLE_PIPE_COMB_MODE_NONE = 0,         /* no combined  */
    SAMPLE_PIPE_COMB_MODE0,                 /* combined mode0 type  */
    SAMPLE_PIPE_COMB_MODE_MAX,
} SAMPLE_PIPE_COMB_MODE_E;

typedef struct {
    SAMPLE_PIPE_MODE_E  ePipeMode;
    AX_BOOL             bAiispEnable;
    AX_CHAR             szBinPath[128];
    SAMPLE_PIPE_COMB_MODE_E eCombMode;
} SAMPLE_PIPE_INFO_T;

#endif //__COMMON_TYPE_H__

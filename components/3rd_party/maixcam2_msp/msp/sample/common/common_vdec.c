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
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/prctl.h>

#include "common_vdec_api.h"

AX_S32 SampleVdecDestroyUserPool(AX_VDEC_GRP VdGrp, AX_POOL PoolId)
{
    AX_S32 sRet = AX_SUCCESS;

    if (VdGrp >= AX_VDEC_MAX_GRP_NUM) {
        sRet = AX_ERR_VDEC_INVALID_GRPID;
        SAMPLE_CRIT_LOG("VdGrp:%d >= AX_VDEC_MAX_GRP_NUM:%d", VdGrp, AX_VDEC_MAX_GRP_NUM);
        goto ERR_RET;
    }

    if (PoolId == AX_INVALID_POOLID) {
        sRet = AX_ERR_VDEC_ILLEGAL_PARAM;
        SAMPLE_CRIT_LOG("PoolId == AX_INVALID_POOLID");
        goto ERR_RET;
    }

    sRet = AX_VDEC_DetachPool(VdGrp);
    if (sRet) {
        SAMPLE_CRIT_LOG("VdGrp=%d, PoolId:%d, AX_VDEC_DetachPool FAILED! ret:0x%x %s",
                        VdGrp, PoolId, sRet, SampleVdecRetStr(sRet));
        goto ERR_RET;
    }

    sRet = AX_POOL_DestroyPool(PoolId);
    if (sRet) {
        SAMPLE_CRIT_LOG("VdGrp=%d, PoolId:%d, AX_POOL_DestroyPool FAILED! ret:0x%x %s",
                        VdGrp, PoolId, sRet, SampleVdecRetStr(sRet));
        goto ERR_RET;
    }


ERR_RET:
    return sRet;
}

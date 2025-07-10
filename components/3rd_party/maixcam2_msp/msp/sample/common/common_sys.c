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
#include <string.h>
#include "ax_isp_api.h"
#include "ax_sys_api.h"
#include "common_sys.h"
#include "common_cam.h"
#include "common_type.h"
#include "ax_buffer_tool.h"
#include "ax_global_type.h"

AX_U32 COMMON_SYS_AddToPlan(AX_POOL_FLOORPLAN_T *pPoolFloorPlan, AX_U32 nCfgCnt, AX_POOL_CONFIG_T *pPoolConfig)
{
    AX_U32 i, done = 0;
    AX_POOL_CONFIG_T *pPC;

    for (i = 0; i < nCfgCnt; i++) {
        pPC = &pPoolFloorPlan->CommPool[i];
        if (pPC->BlkSize == pPoolConfig->BlkSize) {
            pPC->BlkCnt += pPoolConfig->BlkCnt;
            done = 1;
        }
    }

    if (!done) {
        pPoolFloorPlan->CommPool[i] = *pPoolConfig;
        nCfgCnt += 1;
    }

    return nCfgCnt;
}

AX_S32 COMMON_SYS_CalcPool(COMMON_SYS_POOL_CFG_T *pPoolCfg, AX_U32 nCommPoolCnt, AX_POOL_FLOORPLAN_T *pPoolFloorPlan)
{
    AX_S32 i, nCfgCnt = 0;
    AX_FRAME_COMPRESS_INFO_T tCompressInfo = {0};
    AX_POOL_CONFIG_T tPoolConfig = {
        .MetaSize  = 4 * 1024,
        .CacheMode = AX_POOL_CACHE_MODE_NONCACHE,
        .PartitionName = "anonymous"
    };

    for (i = 0; i < nCommPoolCnt; i++) {
        tCompressInfo.enCompressMode = pPoolCfg->enCompressMode;
        tCompressInfo.u32CompressLevel = pPoolCfg->u32CompressLevel;
        tPoolConfig.BlkSize = AX_VIN_GetImgBufferSize(pPoolCfg->nHeight, pPoolCfg->nWidthStride, pPoolCfg->nFmt, &tCompressInfo, 0);
        tPoolConfig.BlkCnt  = pPoolCfg->nBlkCnt;
        nCfgCnt = COMMON_SYS_AddToPlan(pPoolFloorPlan, nCfgCnt, &tPoolConfig);
        pPoolCfg += 1;
    }

    return 0;
}

AX_S32 COMMON_SYS_Init(COMMON_SYS_ARGS_T *pCommonArgs)
{
    AX_S32 axRet = 0;
    AX_POOL_FLOORPLAN_T tPoolFloorPlan = {0};

    axRet = AX_SYS_Init();
    if (0 != axRet) {
        COMM_SYS_PRT("AX_SYS_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* Release last Pool */
    axRet = AX_POOL_Exit();
    if (0 != axRet) {
        COMM_SYS_PRT("AX_POOL_Exit fail!!Error Code:0x%X\n", axRet);
    }

    /* Calc Pool BlkSize/BlkCnt */
    axRet = COMMON_SYS_CalcPool(pCommonArgs->pPoolCfg, pCommonArgs->nPoolCfgCnt, &tPoolFloorPlan);
    if (0 != axRet) {
        COMM_SYS_PRT("COMMON_SYS_CalcPool failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_POOL_SetConfig(&tPoolFloorPlan);
    if (0 != axRet) {
        COMM_SYS_PRT("AX_POOL_SetConfig fail!Error Code:0x%X\n", axRet);
        return -1;
    } else {
        printf("AX_POOL_SetConfig success!\n");
    }

    axRet = AX_POOL_Init();
    if (0 != axRet) {
        COMM_SYS_PRT("AX_POOL_Init fail!!Error Code:0x%X\n", axRet);
        return -1;
    } else {
        COMM_SYS_PRT("AX_POOL_Init success!\n");
    }

    return 0;
}

AX_S32 COMMON_SYS_DeInit()
{
    AX_POOL_Exit();
    AX_SYS_Deinit();
    return 0;
}

AX_S8 COMMON_SYS_GetBoardId(AX_CHAR *board_id)
{
    FILE *pFile = NULL;
    AX_CHAR temp[BOARD_ID_LEN] = {0};

    pFile = fopen("/proc/ax_proc/board_id", "r");
    if (pFile) {
        fread(temp, BOARD_ID_LEN, 1, pFile);
        fclose(pFile);
    } else {
        COMM_SYS_PRT("fopen /proc/ax_proc/board_id failed!!!\n");
        return -1;
    }

    strncpy(board_id, temp, BOARD_ID_LEN * sizeof(AX_CHAR));
    return 0;
}
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
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "common_type.h"
#include "common_nt.h"
#include "ax_nt_stream_api.h"
#include "ax_nt_ctrl_api.h"

AX_S32 COMMON_NT_Init(AX_U32 nStreamPort, AX_U32 nCtrlPort)
{
    AX_S32 axRet = 0;

    /* Net Preview */
    COMM_NT_PRT("Start the service on the tuning device side.\n");

    axRet =  AX_NT_StreamInit(nStreamPort);
    if (0 != axRet) {
        COMM_NT_PRT("AX_NT_StreamInit failed, ret=0x%x.\n", axRet);
        return -1;
    }
    axRet =  AX_NT_CtrlInit(nCtrlPort);
    if (0 != axRet) {
        COMM_NT_PRT("AX_NT_CtrlInit failed, ret=0x%x.\n", axRet);
        goto EXIT_FAIL;
    }

    COMM_NT_PRT("NT Stream Listen Port %d.\n", nStreamPort);
    COMM_NT_PRT("NT Ctrl Listen Port %d.\n", nCtrlPort);

    return 0;

EXIT_FAIL:
    AX_NT_StreamDeInit();
    return axRet;
}

AX_S32 COMMON_NT_UpdateSource(AX_U32 nPipeId)
{
    return AX_NT_SetStreamSource(nPipeId);
}

AX_VOID COMMON_NT_DeInit(AX_VOID)
{
    AX_NT_CtrlDeInit();
    AX_NT_StreamDeInit();
}

/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_NT_H__
#define __COMMON_NT_H__

#include "ax_base_type.h"


#ifndef COMM_NT_PRT
#define COMM_NT_PRT(fmt...)   \
do {\
    printf("[ COMM_NT][%s][%5d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif

AX_S32 COMMON_NT_Init(AX_U32 nStreamPort, AX_U32 nCtrlPort);
AX_S32 COMMON_NT_UpdateSource(AX_U32 nPipeId);
AX_VOID COMMON_NT_DeInit(AX_VOID);

#endif

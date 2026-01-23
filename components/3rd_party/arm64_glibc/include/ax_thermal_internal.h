/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_THERMAL_INTERNAL_H_
#define __AX_THERMAL_INTERNAL_H_
#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ISP_SIF_SNS_NUM_MAX 8

typedef enum axTHERMAL_DEVICE_ID_E
{
    AX_INNER_DEVICE_ISP = 0,
} AX_THERMAL_DEVICE_ID_E;

typedef struct axTHERMAL_ISP_TH_PARAM_T
{
    AX_F32 fps;
    AX_BOOL bSnsEnable;
    AX_BOOL bAiEnable;
    AX_BOOL bSdrEnable;
    AX_U8 nSnsId;
} AX_THERMAL_ISP_TH_PARAM_T;

typedef struct axTHERMAL_DEVICE_PARAM_T
{
   AX_BOOL bInitStatus;
   AX_U8 nSnsNum;
   AX_THERMAL_ISP_TH_PARAM_T tISPThParam[ISP_SIF_SNS_NUM_MAX] ;
} AX_THERMAL_DEVICE_PARAM_T;

typedef AX_S32 (*AX_THERMAL_INNER_DEV_CALLBACK)(AX_THERMAL_DEVICE_PARAM_T param);


AX_S32 AX_THERMAL_InternalDeviceRegister(AX_THERMAL_INNER_DEV_CALLBACK setting, AX_THERMAL_DEVICE_ID_E id);
AX_S32 AX_THERMAL_InternalDeviceUnregister(AX_THERMAL_DEVICE_ID_E id);

#ifdef __cplusplus
}
#endif

#endif
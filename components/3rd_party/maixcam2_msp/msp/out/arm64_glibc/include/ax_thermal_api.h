/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_THERMAL_API_H_
#define __AX_THERMAL_API_H_
#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    AX_THERMAL_LEVEL_0 = 0,
    AX_THERMAL_LEVEL_1,
    AX_THERMAL_LEVEL_2,
    AX_THERMAL_LEVEL_3,
    AX_THERMAL_LEVEL_4,
    AX_THERMAL_LEVEL_5,
    AX_THERMAL_LEVEL_6,
    AX_THERMAL_LEVEL_7,
} AX_THERMAL_LEVEL_E;

typedef AX_S32 (*AX_THERMAL_USER_DEV_CALLBACK)(AX_THERMAL_LEVEL_E eLevel);

AX_S32 AX_THERMAL_Init(AX_CHAR *jsonPath);
AX_S32 AX_THERMAL_Start(AX_BOOL default_ctl);
AX_S32 AX_THERMAL_Stop(AX_BOOL default_ctl);
AX_VOID AX_THERMAL_Deinit(AX_VOID);
AX_S32 AX_THERMAL_UserDeviceRegister(AX_THERMAL_USER_DEV_CALLBACK callback, AX_U32 nId);
AX_S32 AX_THERMAL_UserDeviceUnregister(AX_S32 nId);
AX_S32 AX_THERMAL_UserControl(AX_THERMAL_LEVEL_E eLevel);

#ifdef __cplusplus
}
#endif

#endif
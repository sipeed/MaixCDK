/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_LEN_IRIS_STRUCT_H_
#define _AX_LEN_IRIS_STRUCT_H_

#include "ax_base_type.h"
#include "ax_lens_af_struct.h"

#define ACTUATOR_MAX_NUM 8
#define AX_IRIS_MAX_STEP_FNO_NUM (1024)

typedef enum _AX_IRIS_TYPE_E_ {
    AX_IRIS_FIXED_TYPE   = 0,
    AX_IRIS_P_TYPE       = 1,
    AX_IRIS_DC_TYPE      = 2,
} AX_IRIS_TYPE_E;

typedef enum _AX_IRIS_F_NO_E_ {
    AX_IRIS_F_NO_32_0 = 0,
    AX_IRIS_F_NO_22_0,
    AX_IRIS_F_NO_16_0,
    AX_IRIS_F_NO_11_0,
    AX_IRIS_F_NO_8_0,
    AX_IRIS_F_NO_5_6,
    AX_IRIS_F_NO_4_0,
    AX_IRIS_F_NO_2_8,
    AX_IRIS_F_NO_2_0,
    AX_IRIS_F_NO_1_4,
    AX_IRIS_F_NO_1_0,
} AX_IRIS_F_NO_E;


typedef struct _AX_MANUAL_ATTR_T_T_ {
    AX_U32         nHoldValueForDcIris;   /* 自动光圈校正值， 用于 DC-Iris 的调试 */
    AX_IRIS_F_NO_E eIrisFNOForPIris;      /* 手动光圈大小， 根据光圈 F 值进行区分， 仅支持 P-Iris， 不支持 DC-Iris */
} AX_MANUAL_ATTR_T;


typedef struct _AX_IRIS_PARAMS_T_ {
    AX_U8 nEnable;                 /* 自动光圈使能 1: 自动光圈 0：手动光圈 */
    AX_IRIS_TYPE_E eIrisType;      /* 光圈类型， fixed, DC-Iris 或 P-Iris */
    AX_MANUAL_ATTR_T tManualAttr;  /* 手动光圈属性设置结构体 */
} AX_IRIS_PARAMS_T;

typedef struct _AX_DCIRIS_PARAMS_T_ {
    AX_S32 kp;            /* 比例增益 */
    AX_S32 ki;            /* 积分增益*/
    AX_S32 kd;            /* 微分增益 */
    AX_U32 nMinPwmDuty;   /* 最小 PWM 占空比 */
    AX_U32 nMaxPwmDuty;   /* 最大 PWM 占空比 */
    AX_U32 nOpenPwmDuty;  /* 光圈打开时的 PWM 占空比 */
} AX_DCIRIS_PARAMS_T;

typedef struct _AX_PIRIS_PARAMS_T_ {
    AX_U8 NStepFNOTableChange;                        /* P-Iris 步进电机位置与光圈 F 值映射表是否更新标志 */
    AX_U8 NZeroIsMax;                                 /* P-Iris 步进电机 Step 0 是否对应最大光圈位置标志 */
    AX_U32 nTotalStep;                                /* P-Iris 步进电机的总步数 */
    AX_U32 nStepCount;                                /* P-Iris 步进电机的可用步数 */
    AX_U32 nStepFNOTable[AX_IRIS_MAX_STEP_FNO_NUM];   /* P-Iris 步进电机位置与 F 值映射表 */
    AX_IRIS_F_NO_E eMaxIrisFNOTarget;                 /* 最大光圈目标值 */
    AX_IRIS_F_NO_E eMinIrisFNOTarget;                 /* 最小光圈目标值*/
    AX_U8 nFNOExValid;                                /* 对接 P-Iris 时， AE 分配路线是否采用更高精度的光圈 F 值等效增益标志*/
    AX_U32 nMaxIrisFNOGainTarget;                     /* 最大光圈 F 值等效增益目标值*/
    AX_U32 nMinIrisFNOGainTarget;                     /* 最小光圈 F 值等效增益目标值 */
} AX_PIRIS_PARAMS_T;


typedef struct _AX_LENS_ACTUATOR_IRIS_FUNC_T_ {
    AX_S32 (*pfn_iris_init)(AX_U8 nPipeId, AxAfMotorsType_s motorstype);
    AX_S32 (*pfn_iris_set_param)(AX_U8 nPipeId, AX_IRIS_PARAMS_T *pIrisParam);
    AX_S32 (*pfn_iris_get_param)(AX_U8 nPipeId, AX_IRIS_PARAMS_T *pIrisParam);
    AX_S32 (*pfn_dciris_set_param)(AX_U8 nPipeId, AX_DCIRIS_PARAMS_T *pDcIrisParam);
    AX_S32 (*pfn_dciris_get_param)(AX_U8 nPipeId, AX_DCIRIS_PARAMS_T *pDcIrisParam);
    AX_S32 (*pfn_piris_set_aperture_pos)(AX_U8 nPipeId, AX_S32 nPos);
    AX_S32 (*pfn_piris_get_aperture_pos)(AX_U8 nPipeId, AX_S32 *pPos);
    AX_S32 (*pfn_piris_set_param)(AX_U8 nPipeId, AX_PIRIS_PARAMS_T *pPIrisParam);
    AX_S32 (*pfn_piris_get_param)(AX_U8 nPipeId, AX_PIRIS_PARAMS_T *pPIrisParam);
    AX_S32 (*pfn_iris_exit)(AX_U8 nPipeId);
    AX_S32 (*pfn_dciris_set_duty)(AX_U8 nPipeId, AX_F32 nDuty);
    AX_S32 (*pfn_dciris_get_duty)(AX_U8 nPipeId, AX_F32 *pDuty);
} AX_LENS_ACTUATOR_IRIS_FUNC_T;

#endif


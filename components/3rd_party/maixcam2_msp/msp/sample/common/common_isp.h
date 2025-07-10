/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_ISP_H__
#define __COMMON_ISP_H__
#include <stdio.h>
#include <stdarg.h>

#include "ax_vin_api.h"
#include "ax_base_type.h"
#include "ax_isp_api.h"
#include "common_type.h"
#include "common_vin.h"

#ifndef COMM_ISP_PRT
#define COMM_ISP_PRT(fmt...)   \
do {\
    printf("[COMM_ISP][%s][%5d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif


/* macro definition: Function returns an error */
#define COMM_VIN_CHECK_PTR_VALID(h)                                                      \
do {                                                                            \
    if (AX_NULL == (h)) {                                                          \
        COMM_ISP_PRT("[CHECK_PTR_VALID] error: pointer is null!\n");              \
        return COMM_ISP_ERR_CODE_PTR_NULL;                                             \
    }                                                                           \
} while(0)

#define COMM_VIN_CHECK_VALUE_RANGE_STRING(string, val, min, max)                                                      \
do {                                                                                                \
    if ((val) < (min) || (val) > (max)) {                                                           \
        COMM_ISP_PRT("[CHECK_VALUE_RANGE_VALID]:error: [%s]:%d beyond the range:(%d, %u)\n",       \
                    (string), (val), (min), (max));                                                 \
        return COMM_ISP_ERR_CODE_ILLEGAL_PARAMS;                                                            \
    }                                                                                               \
} while(0)

#define COMM_VIN_CHECK_VALUE_RANGE_VALID(val, min, max)                                                      \
do {                                                                                                \
    if ((val) < (min) || (val) > (max)) {                                                           \
        COMM_ISP_PRT("[CHECK_VALUE_RANGE_VALID] error: value:%d beyond the range:(%d, %d)\n",         \
                    (val), (min), (max));                                                 \
        return COMM_ISP_ERR_CODE_ILLEGAL_PARAMS;                                                            \
    }                                                                                               \
} while(0)



#ifdef __cplusplus
extern "C"
{
#endif


AX_S32 COMMON_ISP_RegisterSns(AX_U8 pipe, AX_U8 nDevId, AX_SNS_CONNECT_TYPE_E eBusType,
                              AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_U8 nI2cAddr, AX_U8 nI2cNode);
AX_S32 COMMON_ISP_RegisterSns_mipi_switch(AX_U8 nPipeId, AX_U8 nDevId, AX_SNS_CONNECT_TYPE_E eBusType,
                            AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_U8 nI2cAddr_0, AX_U8 nI2cNode, AX_U8 nI2cAddr_1, AX_U8 nI2cNode_1);
AX_S32 COMMON_ISP_UnRegisterSns(AX_U8 pipe);
AX_SNS_CONNECT_TYPE_E COMMON_ISP_GetSnsBusType(SAMPLE_SNS_TYPE_E eSnsType);
AX_SENSOR_REGISTER_FUNC_T *COMMON_ISP_GetSnsObj(SAMPLE_SNS_TYPE_E eSnsType);
AX_S32 COMMON_ISP_SetSnsAttr(AX_U8 nPipeId, AX_SNS_ATTR_T *ptSnsAttr, AX_SNS_CLK_ATTR_T *pstSnsClkAttr);
AX_S32 COMMON_ISP_ResetSnsObj(AX_U8 nPipeId, AX_U8 nDevId, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl);

AX_S32 COMMON_ISP_RegisterAeAlgLib(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bUser3a,
                                   AX_ISP_AE_REGFUNCS_T *pAeFuncs);
AX_S32 COMMON_ISP_UnRegisterAeAlgLib(AX_U8 pipe);
AX_S32 COMMON_ISP_RegisterAwbAlgLib(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bUser3a,
                                    AX_ISP_AWB_REGFUNCS_T *pAwbFuncs);
AX_S32 COMMON_ISP_UnRegisterAwbAlgLib(AX_U8 pipe);
AX_S32 COMMON_ISP_RegisterLscAlgLib(AX_U8 pipe, AX_BOOL bUser3a, AX_ISP_LSC_REGFUNCS_T *pLscFuncs);
AX_S32 COMMON_ISP_UnRegisterLscAlgLib(AX_U8 pipe);
AX_S32 COMMON_ISP_Init(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bRegisterSns,
                           AX_BOOL bUser3a, AX_ISP_AE_REGFUNCS_T *tAeFuncs, AX_ISP_AWB_REGFUNCS_T *tAwbFuncs,
                           AX_ISP_AF_REGFUNCS_T *tAfFuncs, AX_ISP_LSC_REGFUNCS_T *tLscFuncs, AX_CHAR *pIspParamsFile);
AX_S32 COMMON_ISP_DeInit(AX_U8 pipe, AX_BOOL bRegisterSns);

AX_S32 COMMON_ISP_SetAeToManual(AX_U8 pipe);

#ifdef __cplusplus
}
#endif

#endif

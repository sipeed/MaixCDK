/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_VIN_H__
#define __COMMON_VIN_H__

#include "ax_vin_api.h"
#include "ax_base_type.h"
#include "ax_mipi_rx_api.h"
#include "ax_isp_3a_plus.h"


typedef struct _AX_SNS_CLK_ATTR_T_ {
    AX_U8               nSnsClkIdx;
    AX_SNS_CLK_RATE_E   eSnsClkRate;
} AX_SNS_CLK_ATTR_T;

typedef enum {
    SAMPLE_SNS_TYPE_NONE = -1,
    /* ov sensor */
    OMNIVISION_OS04A10 = 1,
    OMNIVISION_OS04A10_DCG = 2,
    OMNIVISION_OS04A10_DCG_VS = 3,
    // ### SIPEED EDIT ###
    OMNIVISION_OS04D10 = 4,
    // ### SIPEED EDIT END ###
    /* smartsens sensor */
    SMARTSENS_SC450AI = 10,
    SMARTSENS_SC200AI = 11,
// ### SIPEED EDIT ###
    SMARTSENS_SC850SL = 12,
// ### SIPEED EDIT END ###
    /*samsung sensor*/
    SAMSUNG_S5KJN1SQ03 = 20,
    /* dvp sensor */
    SAMPLE_SNS_DVP = 50,
    SAMPLE_SNS_DVP_IR = 51,
    /* bt sensor */
    SAMPLE_SNS_BT601 = 55,
    SAMPLE_SNS_BT656 = 56,
    SAMPLE_SNS_BT1120 = 57,
    /* lvds sensor */
    SAMPLE_SNS_LVDS = 60,

    /* dummy sensor */
    SAMPLE_SNS_DUMMY = 100,
    SAMPLE_SNS_TYPE_BUTT,
} SAMPLE_SNS_TYPE_E;

typedef enum {
    COMMON_VIN_NONE = -1,
    COMMON_VIN_LOADRAW = 0,
    COMMON_VIN_SENSOR = 1,
    COMMON_VIN_TPG = 2,
    COMMON_VIN_BUTT
} COMMON_VIN_MODE_E;

typedef enum {
    LOAD_RAW_NONE = -1,
    LOAD_RAW_IFE = 0,
    LOAD_RAW_ITP = 1,
    LOAD_RAW_RGB = 2,
    LOAD_RAW_YUV = 3,
    LOAD_RAW_BUTT
} SAMPLE_LOAD_RAW_NODE_E;

#ifndef COMM_VIN_PRT
#define COMM_VIN_PRT(fmt...)   \
do {\
    printf("[COMM_VIN][%s][%5d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 COMMON_VIN_StartMipi(AX_U8 nRxDev, AX_INPUT_MODE_E eInputMode, AX_MIPI_RX_ATTR_T *ptMipiAttr, AX_LANE_COMBO_MODE_E eLaneComboMode);
AX_S32 COMMON_VIN_StopMipi(AX_U8 devId);
AX_S32 COMMON_VIN_CreateDev(AX_U8 devId, AX_U32 nRxDev, AX_VIN_DEV_ATTR_T *pDevAttr, AX_VIN_DEV_BIND_PIPE_T *ptDevBindPipe);
AX_S32 COMMON_VIN_DestroyDev(AX_U8 devId);
AX_S32 COMMON_VIN_StartDev(AX_U8 devId, AX_BOOL bEnableDev, AX_VIN_DEV_ATTR_T *pDevAttr);
AX_S32 COMMON_VIN_StopDev(AX_U8 devId, AX_BOOL bEnableDev);

AX_S32 COMMON_VIN_SetPipeAttr(COMMON_VIN_MODE_E eSysMode, SAMPLE_LOAD_RAW_NODE_E eLoadRawNode, AX_U8 nPipeId, AX_VIN_PIPE_ATTR_T *pPipeAttr);

AX_S32 COMMON_VIN_StartChn(AX_U8 pipe, AX_VIN_CHN_ATTR_T *ptChnAttr, AX_BOOL *pChnEn);
AX_S32 COMMON_VIN_StopChn(AX_U8 pipe);


AX_S32 COMMON_VIN_GetSnsConfig(SAMPLE_SNS_TYPE_E eSnsType,
                               AX_MIPI_RX_ATTR_T *ptMipiAttr, AX_SNS_ATTR_T *ptSnsAttr,
                               AX_SNS_CLK_ATTR_T *ptSnsClkAttr, AX_VIN_DEV_ATTR_T *pDevAttr,
                               AX_VIN_PIPE_ATTR_T *pPipeAttr, AX_VIN_CHN_ATTR_T *pChnAttr);

#ifdef __cplusplus
}
#endif

#endif

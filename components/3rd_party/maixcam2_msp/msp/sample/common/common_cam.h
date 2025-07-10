/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_CAM_H__
#define __COMMON_CAM_H__

#include "ax_base_type.h"
#include "common_vin.h"
#include "common_isp.h"
#include "common_sys.h"
#include "ax_engine_api.h"
#include "ax_sensor_struct.h"
#include <pthread.h>

#define MAX_FILE_NAME_CHAR_SIZE       (128)

#ifndef COMM_CAM_PRT
#define COMM_CAM_PRT(fmt...)   \
do {\
    printf("[COMM_CAM][%s][%5d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif

typedef struct {
    AX_BOOL                 mipi_switch_en;
    AX_U8                   nI2cAddr;
    AX_U8                   nI2cNode;
} AX_MIPI_SWITCH_T;

typedef struct {
    /* common parameters */
    AX_U8                   nNumber;
    AX_BOOL                 bOpen;
    AX_SNS_HDR_MODE_E       eHdrMode;
    SAMPLE_SNS_TYPE_E       eSnsType;
    COMMON_VIN_MODE_E       eSysMode;
    AX_SNS_CONNECT_TYPE_E   eBusType;
    SAMPLE_LOAD_RAW_NODE_E  eLoadRawNode;
    AX_INPUT_MODE_E         eInputMode;
    AX_LANE_COMBO_MODE_E    eLaneComboMode;

    AX_U32                  nRxDev;
    AX_U8                   nDevId;
    AX_U8                   nPipeId;
    AX_U8                   nI2cAddr;
    AX_U8                   nI2cNode;

    /* Resource Control Parameters */
    AX_BOOL                 bRegisterSns;
    AX_BOOL                 bEnableDev;     /* loadraw mode, it is not necessary to enable dev */

    /* Isp processing thread */
    pthread_t               tIspProcThread[AX_VIN_MAX_PIPE_NUM];
    pthread_t               tIspAFProcThread;

    /* Module Attribute Parameters */
    AX_MIPI_RX_ATTR_T       tMipiAttr;
    AX_SNS_ATTR_T           tSnsAttr;
    AX_SNS_CLK_ATTR_T       tSnsClkAttr;
    AX_VIN_DEV_ATTR_T       tDevAttr;
    AX_VIN_DEV_BIND_PIPE_T  tDevBindPipe;
    AX_VIN_PIPE_ATTR_T      tPipeAttr[AX_VIN_MAX_PIPE_NUM];
    SAMPLE_PIPE_INFO_T      tPipeInfo[AX_VIN_MAX_PIPE_NUM];
    AX_VIN_CHN_ATTR_T       tChnAttr[AX_VIN_CHN_ID_MAX];
    AX_BOOL                 bChnEn[AX_VIN_CHN_ID_MAX];

    /* 3A Parameters */
    AX_BOOL                 bUser3a;
    AX_ISP_AE_REGFUNCS_T    tAeFuncs;
    AX_ISP_AWB_REGFUNCS_T   tAwbFuncs;
    AX_ISP_AF_REGFUNCS_T    tAfFuncs;
    AX_ISP_LSC_REGFUNCS_T   tLscFuncs;
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl[AX_VIN_MAX_PIPE_NUM];
    AX_MIPI_SWITCH_T        tSwitchInfo;
} AX_CAMERA_T;
AX_S32 COMMON_NPU_Init();
AX_S32 COMMON_CAM_Init(AX_VOID);
AX_S32 COMMON_CAM_Deinit(AX_VOID);
AX_S32 COMMON_CAM_PrivPoolInit(COMMON_SYS_ARGS_T *pPrivPoolArgs);

AX_S32 COMMON_CAM_Open(AX_CAMERA_T *pCamList, AX_U8 Num);
AX_S32 COMMON_CAM_Close(AX_CAMERA_T *pCamList, AX_U8 Num);

AX_S32 COMMON_CAM_CaptureFrameProc(AX_U32 nCapturePipeId, const AX_IMG_INFO_T *pImgInfo[]);
#endif //__COMMON_CAM_H__

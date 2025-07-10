/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_AVSCALI_API_H__
#define __AX_AVSCALI_API_H__

#include <string.h>
#include "ax_base_type.h"
#include "ax_avs_api.h"
#include "ax_isp_3a_api.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define AX_AVSCALI_MAX_SENSOR_NUM  (8)
#define AX_AVSCALI_MAX_PATH_LEN    (512)
#define AX_AVSCALI_IP_ADDR_LEN     (15)

/* error code */
#define AX_AVSCALI_SUCC                        (0)
#define AX_ERR_AVSCALI_NULL_PTR                AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_NULL_PTR)
#define AX_ERR_AVSCALI_ILLEGAL_PARAM           AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_AVSCALI_NOT_INIT                AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_NOT_INIT)
#define AX_ERR_AVSCALI_FILE_UNEXIST            AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_UNEXIST)
#define AX_ERR_AVSCALI_TIMEOUT                 AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_TIMED_OUT)
#define AX_ERR_AVSCALI_SYS_NOTREADY            AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_SYS_NOTREADY)
#define AX_ERR_AVSCALI_UNKNOWN                 AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_UNKNOWN)
#define AX_ERR_AVSCALI_NOT_SUPPORT             AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_NOT_SUPPORT)
#define AX_ERR_AVSCALI_INITED                  AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_EXIST)
#define AX_ERR_AVSCALI_NOMEM                   AX_DEF_ERR(AX_ID_AVSCALI, 1, AX_ERR_NOMEM)
#define AX_ERR_AVSCALI_CALI_FAIL               AX_DEF_ERR(AX_ID_AVSCALI, 1, 0x80)
#define AX_ERR_AVSCALI_GEO_CALI_FAIL           AX_DEF_ERR(AX_ID_AVSCALI, 1, 0x81)
#define AX_ERR_AVSCALI_DATA_IMCOMPATIBLE       AX_DEF_ERR(AX_ID_AVSCALI, 1, 0x82)

typedef struct axAVSCALI_MASK_AREA_INFO_T
{
    AX_U32 nOffsetX;
    AX_U32 nOffsetY;
    AX_U32 nWidth;
    AX_U32 nHeight;
} AX_AVSCALI_MASK_AREA_INFO_T, *AX_AVSCALI_MASK_AREA_INFO_PTR;

typedef struct axAVSCALI_MESH_FILE_INFO_T {
    AX_CHAR strMeshFile[AX_AVSCALI_MAX_SENSOR_NUM][AX_AVSCALI_MAX_PATH_LEN];
    AX_CHAR strMaskFile[AX_AVSCALI_MAX_SENSOR_NUM - 1][AX_AVSCALI_MAX_PATH_LEN];
} AX_AVSCALI_MESH_FILES_INFO_T, *AX_AVSCALI_MESH_FILES_INFO_PTR;

typedef struct axAVSCALI_AVS_PARAMS_T
{
    AX_U32                       nOutWidth;
    AX_U32                       nOutHeight;
    AX_AVSCALI_MASK_AREA_INFO_T  tMaskAreaInfo[AX_AVSCALI_MAX_SENSOR_NUM - 1];
    AX_AVSCALI_MESH_FILES_INFO_T tMeshFileInfo;
    AX_AVS_GRP_CAMERA_PARAM_T    stGrpCameraParam;
} AX_AVSCALI_AVS_PARAMS_T, *AX_AVSCALI_AVS_PARAMS_PTR;

typedef struct axAVSCALI_SNS_INFO_T {
    AX_U8   nSnsNum;                              // Sensor num
    AX_U8   arrPipeId[AX_AVSCALI_MAX_SENSOR_NUM]; // Pipe id: form left to right
    AX_U8   nMasterPipeId;                        // Master pipe id
    AX_U8   nChn;                                 // ISP chn
    AX_U32  nImgWidth;                            // Sensor out imgage width
    AX_U32  nImgHeight;                           // Sensor out imgage height
} AX_AVSCALI_SNS_T, *AX_AVSCALI_SNS_INFO_PTR;

typedef struct axAVSCALI_3A_SYNC_RATIO_T
{
    AX_ISP_IQ_AE_SYNC_RATIO_T  tAESyncRatio;
    AX_ISP_IQ_AWB_SYNC_RATIO_T tAWBSyncRatio;
} AX_AVSCALI_3A_SYNC_RATIO_T, *AX_AVSCALI_3A_SYNC_RATIO_PTR;

typedef AX_VOID (*AX_AVSCALI_CaliDone)(const AX_S32 nResult, AX_AVSCALI_AVS_PARAMS_T* pAVSParams, AX_AVSCALI_3A_SYNC_RATIO_T* p3ASyncRatio, AX_VOID* pPrivData);

typedef struct axAVSCALI_CALLBACK_T
{
    AX_AVSCALI_CaliDone CaliDoneCb;
} AX_AVSCALI_CALLBACK_T, *AX_AVSCALI_CALLBACK_PTR;

typedef struct axAVSCALI_INIT_PARAM_T {
    AX_AVSCALI_SNS_T        tSnsInfo;
    AX_AVSCALI_CALLBACK_T   tCallbacks;
    AX_CHAR                 strCaliDataPath[AX_AVSCALI_MAX_PATH_LEN]; // Calibration data save path
    AX_VOID*                pPrivData;
} AX_AVSCALI_INIT_PARAM_T, *AX_AVSCALI_INIT_PARAM_PTR;

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize
///
/// @param pParams  [I]: initialize parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_AVSCALI_Init(AX_AVSCALI_INIT_PARAM_T* pParams);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief uninitialize
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_AVSCALI_DeInit();

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Start calibration
///
/// @param pServerIP  [I]: server IP address
/// @param nPort      [I]: server port
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_AVSCALI_Start(const AX_CHAR* pServerIP, const AX_U16 nPort);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Stop calibration
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_AVSCALI_Stop();

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Load AVS parameter
///
/// @param pParamPath   [I ]: path of avs parameter data
/// @param pAVSGrpAttr  [IO]: avs group attribution
/// @param p3ASyncRatio [ O]: 3A sync ratio
/// @param pCalibrated  [ O]: whether cali data is calibrated
///
/// @return 0 if success, otherwise failure
/// @Notice: 1. You can get pAVSGrpAttr or pAVS3ARatio independently by pass non-null pointer
///          2. To get pAVSGrpAttr with AVS_CALIBRATION_PARAM_TRANSFORM mode, need call twice:
///             [first]:  set pAVSGrpAttr->stGrpTransformParam.stPipeMesh.s32MeshSize[0] zero,
///                       call and get mesh files and mask bin size, for caller to malloc memory for m_tMeshAddr[i].nVirAddr and m_tMaskAddr[i].nVirAddr
///             [second]: pass m_tMeshAddr[i].nVirAddr and m_tMaskAddr[i].nVirAddr
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_AVSCALI_LoadParam(const AX_CHAR* pParamPath, AX_AVS_GRP_ATTR_T* pAVSGrpAttr, AX_AVSCALI_3A_SYNC_RATIO_T* p3ASyncRatio, AX_BOOL* pCalibrated);

#ifdef __cplusplus
}
#endif

#endif
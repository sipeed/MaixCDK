/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_ISP_3A_PLUS_H_
#define _AX_ISP_3A_PLUS_H_

#include "ax_isp_api.h"
#include "ax_isp_iq_api.h"

/////////////////////////////////////////////////////////
/* discard this API*/
////////////////////////////////////////////////////////

/* LSC Grid & Hist */
#define AX_LSC_GRID_ROW            (64)
#define AX_LSC_GRID_COL            (48)
#define AX_LSC_GRID_CHN            (4)
#define AX_LSC_HIST_LINEAR_BIN     (256)
#define AX_LSC_HIST_LOG_BIN        (16)

typedef struct {
    AX_U32 uGridSum[AX_LSC_GRID_CHN];
    AX_U16 uGridNum[AX_LSC_GRID_CHN];
} AX_LSC_GRID_STATS;

typedef struct {
    AX_U8  uValid;
    AX_U32 uZoneRowSize;
    AX_U32 uZoneColumnSize;
    AX_U8  uChnNum;
    AX_LSC_GRID_STATS  sGridStats[AX_LSC_GRID_ROW * AX_LSC_GRID_COL];
} AX_LSC_GRID_STAT_T;

/**********************************************************************************
 *                                  CC
 * input (AX_ISP_CC_INPUT_INFO_T) --> cc_alg --> output (AX_ISP_CC_RESULT_T)
 **********************************************************************************/

typedef struct {
    AX_U32 SnsId;
    AX_SNS_HDR_MODE_E  eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 uFrameRate;
} AX_ISP_CC_INITATTR_T;

typedef struct {
    AX_U32 uTotalGain[AX_HDR_CHN_NUM];      /* Total Gain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]*/
    AX_U32 uLux;                            /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF]
                                            * fLux = (MeanLuma*LuxK) / (AGain*Dgain*IspGain)
                                            * where LuxK is a calibrated factor */
    AX_U32 uColorTemp;                      /* = CCT * 1000.0f, CCT:[1000, 20000]  Color Temperature */
    AX_U32 uSaturation;                     /* Saturation Used for CC or LSC Interpolation
                                             * Percetage * 1000.0f, e.g. 100000 for 100%
                                             * If Not Used, Just return 100000 */
} AX_ISP_CC_INPUT_INFO_T;

typedef struct {
    AX_S16                 nCcm[AX_ISP_CC_CCM_SIZE];                             /* Accuracy: S3.8 Range: [-2047, 2047] */
    AX_S16                 nXcm[AX_ISP_CC_ANGLE_SIZE][AX_ISP_CC_CCM_SIZE];      /* Accuracy: S3.8 Range: [-2047, 2047] */
} AX_ISP_CC_RESULT_T;

typedef struct {
    AX_S32(*pfnCc_Init)(AX_U8 pipe, AX_ISP_CC_INITATTR_T *pCcInitParam);
    AX_S32(*pfnCc_Run)(AX_U8 pipe, AX_ISP_CC_INPUT_INFO_T *pCcInputInfo, AX_ISP_CC_RESULT_T *pCcResult);
    AX_S32(*pfnCc_Exit)(AX_U8 pipe);
} AX_ISP_CC_REGFUNCS_T;


/**********************************************************************************
 *                                  LSC
 * input (AX_ISP_LSC_INPUT_INFO_T) --> lsc_alg --> output (AX_ISP_LSC_RESULT_T)
 **********************************************************************************/

typedef struct {
    AX_U32 SnsId;
    AX_SNS_HDR_MODE_E  eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 uFrameRate;
} AX_ISP_LSC_INITATTR_T;

typedef struct {
    AX_U32 uSeqNum;             /* frame seq num */
    AX_U64 uTimestamp;          /* frame timestamp */
    AX_LSC_GRID_STAT_T sLscGridStat;
} AX_ISP_LSC_STAT_INFO_T;

typedef struct {
    AX_U32 uTotalGain[AX_HDR_CHN_NUM];      /* Total Gain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]*/
    AX_U32 uLux;                            /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF]
                                            * fLux = (MeanLuma*LuxK) / (AGain*Dgain*IspGain)
                                            * where LuxK is a calibrated factor */
    AX_U32 uColorTemp;                      /* = CCT * 1000.0f, CCT:[1000, 20000]  Color Temperature */
    AX_U32 uSaturation;                     /* Saturation Used for CC or LSC Interpolation
                                             * Percetage * 1000.0f, e.g. 100000 for 100%
                                             * If Not Used, Just return 100000 */
    AX_ISP_LSC_STAT_INFO_T sLscStat;
} AX_ISP_LSC_INPUT_INFO_T;

typedef struct {
    AX_U32                          nRRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];
    AX_U32                          nGRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];
    AX_U32                          nGBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];
    AX_U32                          nBBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];
} AX_ISP_LSC_TABLE_T;

typedef struct {
    AX_S32(*pfnLsc_Init)(AX_U8 pipe, AX_ISP_LSC_TABLE_T  *tLscResult);
    AX_S32(*pfnLsc_Run)(AX_U8 pipe, AX_ISP_LSC_INPUT_INFO_T tLscInputInfo, AX_BOOL *bUpdateMeshRegs, AX_ISP_LSC_TABLE_T *pLscResult);
    AX_S32(*pfnLsc_Exit)(AX_U8 pipe);
} AX_ISP_LSC_REGFUNCS_T;

AX_S32 AX_ISP_IQ_GetLscStatus(AX_U8 pipe, AX_ISP_LSC_TABLE_T *pIspLscStatus);
#endif //_AX_ISP_3A_PLUS_H_

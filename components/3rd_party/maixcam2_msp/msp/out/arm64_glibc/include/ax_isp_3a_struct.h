/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_ISP_3A_STRUCT_H_
#define _AX_ISP_3A_STRUCT_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

////////////////////////////////////////////////////////////////////////////////////
//  AWB ALG Param
////////////////////////////////////////////////////////////////////////////////////
#define AX_ISP_AWB_GRID_NUM_MAX             (4096)
#define AX_ISP_AWB_ILLUM_NAME_LEN_MAX       (32)
#define AX_ISP_AWB_DESCRIPTION_LEN_MAX      (32)
#define AX_ISP_AWB_POLY_PNT_NUM_MAX         (15)
#define AX_ISP_AWB_ILLUM_NUM_MAX            (16)
#define AX_ISP_AWB_EXT_ILLUM_NUM_MAX        (32)
#define AX_ISP_AWB_ALL_ILLUM_NUM_MAX        (AX_ISP_AWB_ILLUM_NUM_MAX + AX_ISP_AWB_EXT_ILLUM_NUM_MAX)
#define AX_ISP_AWB_CTRL_PNT_PART_NUM_MAX    (32)
#define AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX     (AX_ISP_AWB_CTRL_PNT_PART_NUM_MAX * AX_ISP_AWB_ILLUM_NUM_MAX)
#define AX_ISP_AWB_DOMINANT_ZONE_NUM        (4)
#define AX_ISP_AWB_PLANCKIAN_ZONE_NUM       (24)
#define AX_ISP_AWB_SPATIAL_SEG_MAX_NUM      (8)
#define AX_ISP_AWB_PREFER_CCT_MAX_NUM       (32)
#define AX_ISP_AWB_LUX_TYPE_NUM             (8)
#define AX_ISP_AWB_ZONE_MAX_NUM             (40)
#define AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM      (32)
#define AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM     (32)
#define AX_ISP_AWB_GRID_WEIGHT_ROW_MAX      (54)
#define AX_ISP_AWB_GRID_WEIGHT_COLUMN_MAX   (72)
#define AX_ISP_AF_FV_SCAN_MAX_NUM           (2048)
#define AX_ISP_AWB_MANUAL_ILLUM_NUM_MAX     (15)
#define AX_ISP_AWB_MLC_ZONE_MAX_NUM         (15)
#define AX_ISP_AWB_DETECTION_ZONE_GROUP_MAX_NUM   (10)
#define AX_ISP_AWB_DETECTION_ZONE_MAX_NUM   (5)
#define AX_ISP_AWB_GC_LUX_MAX_NUM           (8)
#define AX_ISP_AWB_GC_CCT_MAX_NUM           (8)

#define AX_SUB_ID_AE         0x01
#define AX_SUB_ID_AWB        0x02
#define AX_SUB_ID_AF         0x03

typedef struct {
    AX_U32 nRg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nBg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
} AX_ISP_IQ_AWB_PNT_T;

typedef struct {
    AX_S32 nK;      /* Accuracy:S21.10 Range:[-2147483647, 2147483647 (2*1024*1024*1024)] */
    AX_S32 nB;      /* Accuracy:S21.10 Range:[-2147483647, 2147483647 (2*1024*1024*1024)] */
} AX_ISP_IQ_AWB_LINE_KB_T;

typedef struct {
    /* Illum Calib Info */
    AX_CHAR  szName[AX_ISP_AWB_ILLUM_NAME_LEN_MAX];
    AX_U16   nCct;      /* Accuracy:U14 Range:[0, 16000] */
    AX_U32   nRadius;   /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_ISP_IQ_AWB_PNT_T tCoord;
} AX_ISP_IQ_AWB_ILLUM_T;


typedef struct {
    AX_U8 nPntCnt; /*  Accuracy:U8 Range:[0, AX_ISP_AWB_POLY_PNT_NUM_MAX-1] */
    AX_ISP_IQ_AWB_PNT_T tPntArray[AX_ISP_AWB_POLY_PNT_NUM_MAX];
} AX_ISP_IQ_AWB_POLY_T;

typedef struct {
    AX_CHAR               szDescription[AX_ISP_AWB_DESCRIPTION_LEN_MAX];
    AX_U8                 nZoneType;                    /* Accuracy:U8 Range:[0, 32]  Rectangle=0, Triangle=1 */
    AX_ISP_IQ_AWB_POLY_T  tPoly;
    AX_U32                nLux[2];                      /* Accuracy:U22.10  Range:[0, 4294967295 (4*1024*1024*1024)] */
} AX_ISP_IQ_AWB_DETECTION_ZONE_T;

typedef struct {
    AX_U8                            nDetectionZoneNum;    /* Accuracy:U8 Range:[0, 5] */
    AX_ISP_IQ_AWB_DETECTION_ZONE_T   tDetectionZoneList[AX_ISP_AWB_DETECTION_ZONE_MAX_NUM];
} AX_ISP_IQ_AWB_GROUP_DETECTION_ZONE_T;

typedef struct {
    AX_U8   nEnable;                       /* Accuracy:U8 Range:[0, 1]*/
    AX_CHAR szDescription[AX_ISP_AWB_DESCRIPTION_LEN_MAX];
    AX_U8   nZoneType;                     /* Accuracy:U8 Range:[0, 32]  Rectangle=0, Triangle=1 */
    AX_ISP_IQ_AWB_POLY_T                  tPoly;
    AX_U8   nDetectionZoneGroupNum;        /* Accuracy:U8 Range:[0, 10]*/
    AX_ISP_IQ_AWB_GROUP_DETECTION_ZONE_T  tGroupDetectionZoneList[AX_ISP_AWB_DETECTION_ZONE_GROUP_MAX_NUM];
    AX_U8   nTrigerType1st;                   /* Accuracy:U8 Range:[0, 14] */
    AX_U8   nTrigerType2nd;                   /* Accuracy:U8 Range:[0, 14] */
    AX_U8   nTrigerType3rd;                   /* Accuracy:U8 Range:[0] */
    AX_U32  nTrigerValue1st[4];               /* Lux                      nTrigerType1st = 0  Accuracy:U22.10  Range:[0, 4294967295 (4*1024*1024*1024)]
                                                 CCT                      nTrigerType1st = 1  Accuracy:U32 Range:[0, 20000]
                                                 Valid Stats Cnt Ratio    nTrigerType1st = 2  Accuracy:U1.10 Range:[0, 1024]
                                                 Agw Rg                   nTrigerType1st = 3  Accuracy:U4.8 Range:[0, 4096]
                                                 Agw Bg                   nTrigerType1st = 4  Accuracy:U4.8 Range:[0, 4096]
                                                 Zone Group 0-9 Cnt Ratio nTrigerType1st = [5, 14] Accuracy:U1.8 Range:[0, 255]
                                              */
    AX_U32  nTrigerValue2nd[4];               /* Lux                      nTrigerType2nd = 0  Accuracy:U22.10  Range:[0, 4294967295 (4*1024*1024*1024)]
                                                 CCT                      nTrigerType2nd = 1  Accuracy:U32 Range:[0, 20000]
                                                 Valid Stats Cnt Ratio    nTrigerType2nd = 2  Accuracy:U1.10 Range:[0, 1024]
                                                 Agw Rg                   nTrigerType2nd = 3  Accuracy:U4.8 Range:[0, 4096]
                                                 Agw Bg                   nTrigerType2nd = 4  Accuracy:U4.8 Range:[0, 4096]
                                                 Zone Group 0-9 Cnt Ratio nTrigerType2nd = [5, 14] Accuracy:U1.8 Range:[0, 255]
                                              */
    AX_U8   nTrigerValue3rd;                   /* Flash Sensitivity       nTrigerType3rd = 0  Accuracy:U8 Range:[0, 1] */
} AX_ISP_IQ_AWB_MLC_ZONE_T;

typedef struct {
    AX_U8  nDominantEnable;         /* Accuracy:U1 Range:[0, 1] */
    AX_U16 nDomMinCctThresh;        /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nDomMaxCctThresh;        /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nDom2AllRatioThresh;     /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2MinorRatioThresh;   /* Accuracy:U10.10 Range:[0, 1,048,575(1024*1024-1)] */
    AX_U16 nMinorWeight;            /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nSmoothPercent;          /* Accuracy:U7.10 Range:[0, 131071 (128*1024)] */
} AX_ISP_3A_AWB_DOMINANT_T;

typedef struct {
    AX_U16 nGainR;    /* Accuracy:U4.8 Range:[256, 4095] */
    AX_U16 nGainGr;   /* Accuracy:U4.8 Range:[256, 4095] */
    AX_U16 nGainGb;   /* Accuracy:U4.8 Range:[256, 4095] */
    AX_U16 nGainB;    /* Accuracy:U4.8 Range:[256, 4095] */
} AX_ISP_IQ_AWB_GAIN_T;


typedef struct {
    AX_ISP_IQ_AWB_GAIN_T tGains;
    AX_U32               nDampRatio;       /* Accuracy:U1.20 Range:[0, 1048576 (1024*1024)] */
    //AX_U8                nFrameSkipping;   /* Accuracy:U8 Range:[0, 255] */
}AX_ISP_IQ_AWB_AUTO_START_T;
typedef struct {
    /* Gray Zone Common Info */
    AX_ISP_IQ_AWB_PNT_T tCenterPnt;
    AX_U32 nCenterPntRadius;    /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    AX_U8 nLowCut;     /* Accuracy:U8 Range:[0, 64] */
    AX_U8 nHighCut;    /* Accuracy:U8 Range:[0, 64] */
    AX_U16 nCctMax;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctMin;     /* Accuracy:U14 Range:[0, 16000] */

    AX_U8 nPartCtrlPntNum;     /* Accuracy:U8 Range:[0, 15] */
    AX_U8 nCtrlPntNum;         /* Accuracy:U8 Range:[0, 128] */
    AX_U8 nCtrlSegKbNum;       /* Accuracy:U8 Range:[0, 127] Should Always be nCtrlPntNum-1 */

    AX_U16                  nCctList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];      /* Accuracy:U14 Range:[0, 16000] */
    AX_ISP_IQ_AWB_LINE_KB_T tChordKB;
    AX_ISP_IQ_AWB_PNT_T     tChordPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tArcPointList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_LINE_KB_T tRadiusLineList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    /* Gray Zone Borders */
    AX_ISP_IQ_AWB_PNT_T     tInLeftBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tInRightBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    AX_ISP_IQ_AWB_PNT_T     tOutLeftBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tOutRightBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    /* Illum Info */
    AX_U8 nIllumNum;       /* Accuracy:U8 Range:[0, 64] */
    AX_ISP_IQ_AWB_ILLUM_T tIllumList[AX_ISP_AWB_ILLUM_NUM_MAX];
    AX_U8 nExtIllumNum;    /* Accuracy:U8 Range:[0, 16] */
    AX_ISP_IQ_AWB_ILLUM_T tExtIllumList[AX_ISP_AWB_EXT_ILLUM_NUM_MAX];

    /* MLC Info */
    AX_U8                        nMLCNum;     /* Accuracy:U8 Range:[0, 15] */
    AX_ISP_IQ_AWB_MLC_ZONE_T     tMLCZoneList[AX_ISP_AWB_MLC_ZONE_MAX_NUM];


    /*   Tuning Params  */

    /*Auto Initialization Parameters */
    AX_ISP_IQ_AWB_AUTO_START_T tInitParam;

    /* Common Settings */
    AX_U8  nMode;            /* Accuracy:U6 Range:[0, 2] INVALID=0, MANUAL=1, AUTO=2 */
    AX_U8  nIndex;           /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nDampRatio;       /* Accuracy:U1.20 Range:[0, 1048576 (1024*1024)] */
    AX_U32 nToleranceRg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nToleranceBg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    /* Lux Type Threshold */
    AX_U32 nLuxVeryDarkStart;   /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxVeryDarkEnd;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxDarkStart;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxDarkEnd;         /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxIndoorStart;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxIndoorEnd;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransInStart;    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransInEnd;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransOutStart;   /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransOutEnd;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxOutdoorStart;    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxOutdoorEnd;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxBrightStart;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxBrightEnd;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxVeryBrightStart; /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */

    /* Gray Zone CCT Split Threshold */
    AX_U16 nCctMinInner;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctMaxInner;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctMinOuter;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctMaxOuter;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctSplitHtoA;    /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctSplitAtoF;    /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctSplitFtoD5;   /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctSplitD5toD6;  /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nCctSplitD6toS;   /* Accuracy:U14 Range:[0, 16000] */

    /* Grid Weight Params */
    AX_U8  nGridWeightEnable;         /* Accuracy: U8    Range: [0x0, 0x1] */
    AX_U8  nGridWeightRow;            /* Accuracy: U8    Range: [0x1, 0x1B] */
    AX_U8  nGridWeightColumn;         /* Accuracy: U8    Range: [0x1, 0x24] */
    AX_U16 nGridWeightTable[AX_ISP_AWB_GRID_WEIGHT_ROW_MAX][AX_ISP_AWB_GRID_WEIGHT_COLUMN_MAX]; /* Accuracy: U1.10 Range: [0x0, 0x400] */

    /* Lux Weights of Gray Zones and Extra Illuminations */
    AX_U16 nGrayZoneLuxWeight[AX_ISP_AWB_PLANCKIAN_ZONE_NUM][AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U10 Range:[0, 1000] */
    AX_U16 nExtIlllumLuxWeight[AX_ISP_AWB_EXT_ILLUM_NUM_MAX][AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U10 Range:[0, 1000] */

    /* Luma Weight*/
    AX_U8  nLumaWeightNum;       /* Accuracy:U6 Range:[0, 32]*/
    AX_U32 nLumaSplitList[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM];   /* Accuracy:U8.10 Range:[0, 262143 (256 *1024)] */
    AX_U16 nLumaWeightList[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM];   /* Accuracy:U1.10 Range:[0, 1024] */

    /* Mix Light*/
    AX_U8  bMixLightEn;         /* Accuracy:U1 Range:[0, 1] */ // 1: Enable Mix Light Weight & CCM Saturation Discount,  0: Disable
    AX_U16 nMixLightProba_0_CctStd[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U14 Range:[0, 9999] */   // Proba=0,   if CCT Std below this Thresh
    AX_U16 nMixLightProba_100_CctStd[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U14 Range:[0, 9999] */ // Proba=100, if CCT Std above this Thresh
    AX_U8  nMixLightProba_100_SatDiscnt[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U8 Range:[0, 100] */ // CCM Saturation Discount When Proba=100
    AX_U8  nMixLightKneeNum;                                    /* Accuracy:U6 Range:[0, 32]*/   // Weight LUT: Size
    AX_U16 nMixLightKneeCctList[AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM];  /* Accuracy:U14 Range:[0, 16000] */ // Weight LUT: Key
    AX_U16 nMixLightKneeWtList[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM]; /* Accuracy:U1.10 Range:[0, 1024] */ // Weight LUT: Value

    /* Dominant Params */
    AX_ISP_3A_AWB_DOMINANT_T tDomParamList[AX_ISP_AWB_DOMINANT_ZONE_NUM];

    AX_U16 nTmpoStabTriggerAvgBlkWt; /* Accuracy:U10 Range:[0, 1000] */

    /* Planckian Locus Project in High Lux Scene */
    AX_U8  nPlanckianLocusProjEn;           /* Accuracy:U6 Range:[0, 1] Enable or Disable*/
    AX_U32 nPlanckianLocusNotProjLux;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nPlanckianLocusFullProjLux;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */

    /* Spatial Predictor Params */
    AX_U8  nSpatialEn;                                          /* Accuracy:U8 Range:[0, 1] Enable or Disable*/
    AX_U8  nSpatialSegmetNum;                                   /* Accuracy:U8  Range:[0, 8] */
    AX_U32 nSpatialStartLux[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nSpatialEndLux[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nSpatialRg[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];          /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nSpatialBg[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];          /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    /* Fusion Params */
    AX_U16 nFusionGrayZoneConfid_0_AvgBlkWeight;        /* Accuracy:U10 Range:[0, 1000] */
    AX_U16 nFusionGrayZoneConfid_100_AvgBlkWeight;      /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nFusionSpatialConfid_0_Lux;                  /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nFusionSpatialConfid_100_Lux;                /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U16 nFusionWeightGrayZone;                       /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nFusionWeightSpatial;                        /* Accuracy:U1.10 Range:[0, 1024] */

    /* Preference Params */
    AX_U8  nPreferEn;            /* Accuracy:U8 Range:[0, 1] Enable or Disable*/
    AX_U8  nPreferCctNum;        /* Accuracy:U8 Range:[0, 64] */
    AX_U16 nPreferSrcCctList[AX_ISP_AWB_PREFER_CCT_MAX_NUM];                             /* Accuracy:U14 Range:[0, 16000] */
    AX_U16 nPreferDstCct[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_PREFER_CCT_MAX_NUM];        /* Accuracy:U14 Range:[0, 16000] */
    AX_S32 nPreferGrShift[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_PREFER_CCT_MAX_NUM];       /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */

    /* Green Cut Params */
    AX_U8  nGreenCutEn;                           /* Accuracy:U8 Range:[0, 1] Enable or Disable*/
    AX_U8  nGreenCutLuxListNum;                   /* Accuracy:U8 Range:[0, 8] */
    AX_U8  nGreenCutCctListNum;                   /* Accuracy:U8 Range:[0, 8] */
//    AX_U32 nGreenCutConfid_0_Lx;                  /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
//    AX_U32 nGreenCutConfid_100_Lx;                /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nGreenCutLuxList[AX_ISP_AWB_GC_LUX_MAX_NUM];     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U16 nGreenCutWeight[AX_ISP_AWB_GC_LUX_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nGreenCutBreakAngle[AX_ISP_AWB_GC_LUX_MAX_NUM];  /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_S32 nGreenCutOffsetRg[AX_ISP_AWB_GC_LUX_MAX_NUM];    /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */
    AX_S32 nGreenCutOffsetBg[AX_ISP_AWB_GC_LUX_MAX_NUM];    /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */
    AX_U32 nGreenCutCctList[AX_ISP_AWB_GC_CCT_MAX_NUM];     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U16 nGreenCutCctDiscount[AX_ISP_AWB_GC_CCT_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */

    /* Multi Camera Sync Params */
    AX_U8  nMultiCamSyncMode;   /* Accuracy:U2 Range:[0, 3] */
} AX_ISP_IQ_AWB_ALG_CONFIG_T;


typedef struct {
    AX_U8 nblkRowNum;      /* Accuracy:U7 Range:[0, 54] */
    AX_U8 nblkColNum;      /* Accuracy:U7 Range:[0, 72] */
    AX_ISP_IQ_AWB_PNT_T tStats[AX_ISP_AWB_GRID_NUM_MAX];

    AX_U16 nCct;            /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nLux;            /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U8  nLuxTypeInd;     /* Accuracy:U8 Range:[0, 64] */
    AX_U16 nCctStd;         /* Accuracy:U14 Range:[0, 9999] */
    AX_U16 nMixLightProba; /* Accuracy:U10 Range:[0, 1000] Current Mix Light Probability  */
    AX_U8  nSatDiscount;   /*  Accuracy:U8 Range:[0, 100] Current Saturation Discount  */

    /* Stat Info of each Planckian Zone and Extra Illumination.
     *   Element[0~23]: Stat Info of each Planckian Zone.
     *   Element[23~ ]: Stat Info of each Extra Illumination.
     */
    AX_U16 nGrayZoneCnt[AX_ISP_AWB_ZONE_MAX_NUM];               /* Accuracy:U12 Range:[0, 4096] */
    AX_U16 nGrayZoneLuxWeight[AX_ISP_AWB_ZONE_MAX_NUM];         /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nGrayZoneLuxWeightSum[AX_ISP_AWB_ZONE_MAX_NUM];      /* Accuracy:U32 Range:[0, 4294967295] */
    AX_U32 nGrayZoneFinalWeightSum[AX_ISP_AWB_ZONE_MAX_NUM];    /* Accuracy:U32 Range:[0, 4294967295] */
    AX_U32 nGrayZoneLumaSum[AX_ISP_AWB_ZONE_MAX_NUM];           /* Accuracy:U32 Range:[0, 4294967295] */

    /* Fusion Status */
    AX_U16 nGrayZoneBlkWeightAvg;   /* Accuracy:U10 Range:[0, 1000] */
    AX_U16 nGrayZoneConfid;         /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nSpatialConfid;          /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nGrayZoneFusionRatio;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nSpatialFusionRatio;     /* Accuracy:U1.10 Range:[0, 1024] */
    AX_ISP_IQ_AWB_PNT_T tGrayZonePoint;
    AX_ISP_IQ_AWB_PNT_T tSpatialPoint;
    AX_ISP_IQ_AWB_PNT_T tFusionPoint;

    /* Classic Gray World */
    AX_ISP_IQ_AWB_PNT_T tGrayWorldPoint;

    /* Green Cut Status*/
    AX_U32 nGcBreakAngle;                   /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_S32 nGcOffsetRg;                     /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */
    AX_S32 nGcOffsetBg;                     /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */
    AX_U16 nGreenCutCnt;                    /* Accuracy:U13 Range:[0, 4096] */
    AX_U16 nGreenCutFinalWeight;            /* Accuracy:U1.10 Range:[0, 1024] */

    /* MLC Status*/
    AX_U16 nMLC1stWeight;         /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nMLC2ndWeight;         /* Accuracy:U1.10 Range:[0, 1024] */
    // AX_U16 nMLCTotalWeight;       /* Accuracy:U1.10 Range:[0, 1024] */

    /* Dominant Status */
    AX_U16 nDomCntH;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U16 nDomCntA;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U16 nDomCntF;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U16 nDomCntD;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U16 nDom2MinorRatioH;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2MinorRatioA;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2MinorRatioF;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2MinorRatioD;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2AllRatioH;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2AllRatioA;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2AllRatioF;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U16 nDom2AllRatioD;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U8  nDomActiveH;         /* Accuracy:U1 Range:[0, 1] */
    AX_U8  nDomActiveA;         /* Accuracy:U1 Range:[0, 1] */
    AX_U8  nDomActiveF;         /* Accuracy:U1 Range:[0, 1] */
    AX_U8  nDomActiveD;         /* Accuracy:U1 Range:[0, 1] */

    /* Luma Counter */
    AX_U16 nLumaWeight[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM+1];  /* Accuracy:U1.10 Range:[0, 1024] Current Luma Weight*/
    AX_U16 nLumaCount[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM+1];   /* Accuracy:U12 Range:[0, 4000] Current Luma Count*/
} AX_ISP_IQ_AWB_ALG_STATUS_T;


typedef struct {
    AX_U8  nLogLevel;                /* AXAWB_LOG_EMERG:0,AXAWB_LOG_ALERT:1,AXAE_LOG_CRIT:2,AXAWB_LOG_ERROR:3, AXAWB_LOG_WARN:4, AXAWB_LOG_NOTICE:5, AXAWB_LOG_INFO:6,AXAWB_LOG_DBG:7*/
    AX_U8  nLogTarget;               /* AXAWB_LOG_TARGET_STDERR:1, AXAWB_LOG_TARGET_SYSLOG:2 */
    AX_U8  nAlgoPrintInterval;       /*Accuracy:U8 Range:[0, 100]   How many frames to print at intervals. If set to 0, do not print */
    AX_U8  nStatisticsPrintInterval; /*Accuracy:U8 Range:[0, 100]   How many frames to print at intervals. If set to 0, do not print */
} AX_ISP_IQ_AWB_LOG_T;


typedef struct {
    AX_CHAR szName[AX_ISP_AWB_ILLUM_NAME_LEN_MAX];
    AX_U32  nColorTemperature; /* Accuracy:U32 Range:[1000, 15000] */
    AX_S32  nGreenShift;       /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */
} AX_ISP_IQ_AWB_LIGHTSOURCE_T;

typedef struct {
    AX_U8                       nLightSourceIndex;  /*Accuracy:U8 Range:[0, 14] */
    AX_ISP_IQ_AWB_LIGHTSOURCE_T tLightSource[AX_ISP_AWB_MANUAL_ILLUM_NUM_MAX];
} AX_ISP_IQ_AWB_MANUAL_LIGHTSOURCE_T;

typedef struct {
    AX_U8  nManualMode;                  /* Accuracy:U8  Range:[0, 1] 0: manual gain mode; 1: light source mode*/
    AX_ISP_IQ_AWB_GAIN_T                 tGain;
    AX_ISP_IQ_AWB_MANUAL_LIGHTSOURCE_T   tManualLightSource;
} AX_ISP_IQ_AWB_MANUAL_T;

typedef struct {
    AX_U8 nEnable;
    AX_ISP_IQ_AWB_MANUAL_T     tManualParam;
    AX_ISP_IQ_AWB_ALG_CONFIG_T tAutoParam;
    AX_ISP_IQ_AWB_LOG_T        tLogParam;
} AX_ISP_IQ_AWB_PARAM_T;


typedef struct {
    AX_ISP_IQ_AWB_GAIN_T       tGainStatus;
    AX_ISP_IQ_AWB_ALG_STATUS_T tAlgoStatus;
} AX_ISP_IQ_AWB_STATUS_T;

typedef struct {
    AX_U32       nRGainRatio;             /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32       nBGainRatio;             /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
} AX_ISP_IQ_AWB_SYNC_RATIO_T;
////////////////////////////////////////////////////////////////////////////////////
//  AE ALG Param
////////////////////////////////////////////////////////////////////////////////////
#define AX_ISP_AE_LUX_MAX_PAIR_NUM          (10)
#define AX_TFLICKER_100HZ                   (1000000.0/100.0 )                             /* predefined flicker period value for ECM module, uint:us */
#define AX_TFLICKER_120HZ                   (1000000.0/120.0)                              /* predefined flicker period value for ECM module, uint:us */
#define AX_ISP_AE_GRID_WEIGHT_ROW_MAX       (54)
#define AX_ISP_AE_GRID_WEIGHT_COLUMN_MAX    (72)

typedef enum AX_ISP_3A_INIT_ANTI_FLICKER_MODE_S
{
    AX_ISP_INIT_FLICKER_DISABLE = 0,
    AX_ISP_INIT_FLICKER_100HZ   = 1,
    AX_ISP_INIT_FLICKER_120HZ   = 2,
    AX_ISP_INIT_FLICKER_MODE_MAX
} AX_ISP_3A_INIT_ANTI_FLICKER_MODE_T;


#define AE_ISP_ANTI_FLICKER_MAX_NUM 12
typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xC] */
    AX_U32     nRefList[AE_ISP_ANTI_FLICKER_MAX_NUM];     /* <lux value> Accuracy: U22.10; lux range: [0x0, 0x3D090000] */
    AX_U32     nAntiFlickerToleranceList[AE_ISP_ANTI_FLICKER_MAX_NUM]; /* Uints: us; Accuracy:U32 Range:[0x0, 0x208d] */
} AX_ISP_IQ_AE_ANTI_FLICKER_TOLERANCE_CURVE_T;

typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xC] */
    AX_U32     nRefList[AE_ISP_ANTI_FLICKER_MAX_NUM];     /* <lux value> Accuracy: U22.10; lux range: [0x0, 0x3D090000] */
    AX_U16     nFlickerValidThList[AE_ISP_ANTI_FLICKER_MAX_NUM]; /* Accuracy: U6.10, Range: [1, 65535] */
    AX_U16     nFlickerTrigTimeList[AE_ISP_ANTI_FLICKER_MAX_NUM]; /* Accuracy: U16, Range: [1, 65535] */
} AX_ISP_IQ_AE_FLICKER_CURVE_T;

typedef struct{
    AX_U8  nAntiFlickerMode;       /* Accuracy: U2, 0: Disable; 1: Enable Manual Anti-flicker; 2：Enable Auto Flicker Det; 3: Enable Anto Anti-flicker */
    AX_U8  nFlickerPeriod;         /* 0: 100HZ, 1: 120HZ */
    AX_ISP_IQ_AE_ANTI_FLICKER_TOLERANCE_CURVE_T tAntiFlickerToleranceCurve;  /* Uints: us. Accuracy:U32 Range:[0x0, 0x208d] */
    AX_U8  nOverExpMode;           /* 0: ANTI PRIOR,1: LUMA PRIOR */
    AX_U8  nUnderExpMode;          /* 0: ANTI PRIOR,1: LUMA PRIOR */
} AX_ISP_IQ_AE_ANTIFLICKER_PARAMS_T;

typedef struct{
    AX_U8   nFlickerValidNum;      /* Accuracy: U8, Range: [0, 255]*/
    AX_ISP_IQ_AE_FLICKER_CURVE_T  tFlickerParamCurve;
    AX_U8   nSkipTh;               /* Accuracy: U8,  Range: [0, 255]*/
    AX_S16  nUpSlopeTh;            /* Accuracy: S8.10 Range: [-262144, 262144] */
    AX_S16  nDownSlopeTh;          /* Accuracy: S8.10 Range: [-262144, 262144] */
} AX_ISP_IQ_AE_AUTO_FLICKER_DETECT_PARAMS_T;

typedef struct{
    AX_U32 nIntergrationTime;    /* Uints: us. Accuracy:U32
                                  * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                                  * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
    AX_U32 nGain;                /* Accuracy: U22.10  Range: [nTotalGainMin, nTotalGainMax] */
    AX_U32 nAperture;            /* Accuracy: U10  Range: [0x0, 0x3FF] */
    AX_U8 nIncrementPriority;    /* 0: Exp Time 1: Gain */
} AX_ISP_IQ_AE_ROUTE_CURVE_NODE_T;

#define AE_ISP_ROUTE_MAX_NODES 16
#define AX_AE_TABLE_NAME_LENGTH_MAX 32
typedef struct{
    AX_CHAR sTableName[AX_AE_TABLE_NAME_LENGTH_MAX];
    AX_U8 nRouteCurveNum;    /* Accuracy: U8 Range: [0x1, 0x10] */
    AX_ISP_IQ_AE_ROUTE_CURVE_NODE_T  tRouteCurveList[AE_ISP_ROUTE_MAX_NODES];
} AX_ISP_IQ_AE_ROUTE_TABLE_T;

#define AX_AE_TABLE_NUM_MAX 8
typedef struct{
    AX_U8 nTableNum;     /* Accuracy: U8 Range: [0x1, 0x8] */
    AX_U8 nUsedTableId;  /* Accuracy: U8 Range: [0x0, 0x7] */
    AX_ISP_IQ_AE_ROUTE_TABLE_T  tRouteTable[AX_AE_TABLE_NUM_MAX];
} AX_ISP_IQ_AE_ROUTE_PARAM_T;

typedef struct{
    AX_U32 nIntergrationTime;    /* Uints: us. Accuracy:U32
                                  * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                                  * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
    AX_U32 nTotalAGain;                /* Accuracy: U22.10  Range: [nSnsTotalAGainMin, nSnsTotalAGainMax] */
    AX_U32 nDGain;                /* Accuracy: U22.10  Range: [nDGainMin, nDGainMax] */
    AX_U32 nIspGain;               /* Accuracy: U22.10  Range: [nIspGainMin, nIspGainMax] */
} AX_ISP_IQ_AE_ROUTE_ADVANCE_CURVE_NODE_T;

typedef struct{
    AX_CHAR sAdvanceTableName[AX_AE_TABLE_NAME_LENGTH_MAX];     /* AeRouteAdvance */
    AX_U8 nRouteAdvanceCurveNum;    /* Accuracy: U8 Range: [0x1, 0x10] */
    AX_ISP_IQ_AE_ROUTE_ADVANCE_CURVE_NODE_T  tRouteAdvanceCurveList[AE_ISP_ROUTE_MAX_NODES];
} AX_ISP_IQ_AE_ROUTE_ADVANCE_TABLE_T;

#define AE_ISP_SETPOINT_MAX_NUM 10
typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U32     nRefList[AE_ISP_SETPOINT_MAX_NUM];     /* <gain value/lux value> if nSetPointMode = 1, use gain range; if nSetPointMode = 2,use lux range;
                                                       * Accuracy: U22.10
                                                       * gain range: nTotalGainMin, nTotalGainMax]
                                                       * lux range: [0x0, 0x3D090000]*/
    AX_U32     nSetPointList[AE_ISP_SETPOINT_MAX_NUM]; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
} AX_ISP_IQ_AE_SETPOINT_CURVE_T;

#define AX_AE_OBJECT_TARGET_MAX_NUM 12
typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xC] */
    AX_U32     nRefList[AX_AE_OBJECT_TARGET_MAX_NUM];     /* <lux value> if nFaceTargetMode = 1, use lux range;
                                                       * Accuracy: U22.10
                                                       * lux range: [0x0, 0x3D090000]*/
    AX_U32     nFaceTargetList[AX_AE_OBJECT_TARGET_MAX_NUM]; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
} AX_ISP_IQ_AE_FACE_TARGET_CURVE_T;

typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xC] */
    AX_U32     nRefList[AX_AE_OBJECT_TARGET_MAX_NUM];     /* <lux value> if nVehicleTargetMode = 1, use lux range;
                                                       * Accuracy: U22.10
                                                       * lux range: [0x0, 0x3D090000]*/
    AX_U32     nVehicleTargetList[AX_AE_OBJECT_TARGET_MAX_NUM]; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
} AX_ISP_IQ_AE_VEHICLE_TARGET_CURVE_T;

#define AX_AE_HDR_RATIO_LIST_MAX_NUM (10)
typedef struct
{
    AX_U8 nListSize;                                    /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U32 nRefList[AX_AE_HDR_RATIO_LIST_MAX_NUM];      /* Accuracy: U22.10  Range: [0x0, 0x3D090000] */
    AX_U32 nSatLumaList[AX_AE_HDR_RATIO_LIST_MAX_NUM];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nMinRatioList[AX_AE_HDR_RATIO_LIST_MAX_NUM]; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nMaxRatioList[AX_AE_HDR_RATIO_LIST_MAX_NUM]; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
} AX_ISP_IQ_AE_HDR_RATIO_PARAM_CURVE_T;

typedef struct {
    AX_U32 nShortNonSatAreaPercent; /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nTolerance;              /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U8  nConvergeCntFrameNum;    /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U16 nDampRatio;              /* Accuracy: U0.10 Range: [0x0, 0x400] */
    AX_ISP_IQ_AE_HDR_RATIO_PARAM_CURVE_T tHdrRatioParamCurve;
} AX_ISP_IQ_AE_HDR_RATIO_STRATEGY_PARAM_T;

typedef struct
{
    AX_U8 nListSize;                                    /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U32 nRefList[AX_AE_HDR_RATIO_LIST_MAX_NUM];      /* Accuracy: U22.10  Range: [0x0, 0x3D090000] */
    AX_U32 nSatLumaList[AX_AE_HDR_RATIO_LIST_MAX_NUM];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
} AX_ISP_IQ_AE_HDR_RATIO_EXTEND_PARAM_CURVE_T;

typedef struct {
    AX_U32 nCommonHdrRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nExtendHdrRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nHdrRatioTh; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nShortNonSatAreaPercent; /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nTolerance;              /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U8  nConvergeCntFrameNum;    /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U16 nDampRatio;              /* Accuracy: U0.10 Range: [0x0, 0x400] */
    AX_ISP_IQ_AE_HDR_RATIO_EXTEND_PARAM_CURVE_T tHdrRatioExtendParamCurve;
} AX_ISP_IQ_AE_HDR_RATIO_EXTEND_PARAM_T;

typedef struct {
    AX_U8  nHdrMode;       /* 0: fixed mode; 1: Dynamic mode; 2: Dynamic Extend */
    AX_ISP_IQ_AE_HDR_RATIO_STRATEGY_PARAM_T tRatioStrategyParam;
    AX_ISP_IQ_AE_HDR_RATIO_EXTEND_PARAM_T tHdrRatioExtendParam;
    AX_U32 nFixedHdrRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
} AX_ISP_IQ_AE_HDR_RATIO_T;

typedef struct
{
    AX_U32 nBigStepFactor;              /* Accuracy: U4.20  Range: [0x0, 0xA00000], 6 decimal places  */
    AX_U32 nSmallStepFactor;            /* Accuracy: U4.20  Range: [0x0, 0xA00000], 6 decimal places  */
    AX_U32 nLumaDiffOverThresh;         /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nLumaDiffUnderThresh;        /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nLumaSpeedThresh;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nSpeedDownFactor;            /* Accuracy: U4.20  Range: [0x0, 0xA00000], 6 decimal places  */
    AX_U32 nMinUserPwmDuty;             /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nMaxUserPwmDuty;             /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nOpenPwmDuty;                /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nConvergeLumaDiffTolerance;  /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nConvergeFrameCntThresh;     /* Accuracy: U32 Range: [0x0, 0x64] */
} AX_ISP_IQ_AE_DCIRIS_PARAMS_T;

#define AX_ISP_AE_SPARSE_SLOW_SHUTTER_MAX_NUM 5
typedef struct
{
    AX_U32 nNodeNum;  /* Accuracy: U32  Range: [0x0, 0x5]*/
    AX_U32 nFpsList[AX_ISP_AE_SPARSE_SLOW_SHUTTER_MAX_NUM];  /* Accuracy: U8.10  Range: [nSnsSlowShutterModeFpsMin, nSnsSlowShutterModeFpsMax] */
} AX_ISP_IQ_AE_SPARSE_MODE_PARAM_T;

typedef struct
{
    AX_U8 nFrameRateMode;             /* 0: FIX FRAME RATE MODE; 1: SLOW SHUTTER MODE */
    AX_U8 nFpsIncreaseDelayFrame;     /* Accuracy: U8 Range: [0x0, 0xA] */
} AX_ISP_IQ_AE_SLOW_SHUTTER_PARAM_T;

typedef struct
{
    AX_U8  nIrisType;           /* Accuracy: U8 0: FIXED TYPE; 1: DC-IRIS; 2: P-IRIS  */
    AX_ISP_IQ_AE_DCIRIS_PARAMS_T  tDcIrisParam;
} AX_ISP_IQ_AE_IRIS_PARAMS_T;

typedef struct
{
    AX_U32 nToFastLumaThOver;   /* Accuracy: U4.10 Range: [0x0, 0x2800]*/
    AX_U16 nToFastLumaThUnder;  /* Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U32 nToSlowFrameTh;      /* Accuracy: U32 Range: [0x0, 0x12C]*/
    AX_U32 nToConvergedFrameTh; /* Accuracy: U32 Range: [0x0, 0xA]*/
}AX_ISP_IQ_AE_STATE_MACHINE_T;

#define AX_ISP_AE_SPEED_KNEE_MAX_NUM (16)
typedef struct
{
    AX_U32 nFastOverKneeCnt;                                           /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nFastOverLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];        /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U16 nFastOverStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFastOverSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastOverSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nFastUnderKneeCnt;                                           /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nFastUnderLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U16 nFastUnderStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFastUnderSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastUnderSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nSlowOverKneeCnt;                                             /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nSlowOverLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];              /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U16 nSlowOverStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];        /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nSlowOverSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];   /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowOverSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];              /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nSlowUnderKneeCnt;                                            /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nSlowUnderLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];             /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U16 nSlowUnderStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];       /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nSlowUnderSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];  /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowUnderSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];             /* Accuracy: U32 Range: [0x0, 0xA] */
}AX_ISP_IQ_AE_CONVERGE_SPEED_T;


typedef struct
{
    AX_ISP_IQ_AE_STATE_MACHINE_T tStateMachineParam;
    AX_ISP_IQ_AE_CONVERGE_SPEED_T tConvergeSpeedParam;
}AX_ISP_IQ_AE_TIME_SMOOTH_PARAM_T;

#define AX_ISP_AE_LUMA_WEIGHT_MAX_NUM (64)
typedef struct
{
    AX_U8 nEnable;          /* 0: disable luma weight 1: enable luma weight */
    AX_U32 nLumaWeightNum;  /* Accuracy: U32  Range: [0x0, 0x40]*/
    AX_U32 nLumaSplitList[AX_ISP_AE_LUMA_WEIGHT_MAX_NUM];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U16 nWeightList[AX_ISP_AE_LUMA_WEIGHT_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_LUMA_WEIGHT_PARAM_T;

#define AX_AE_OBJECT_SCORE_LEVEL (8)
#define AX_AE_OBJECT_DISTANCE_LEVEL (8)
#define AX_AE_OBJECT_LUMA_DIFF_LEVEL (8)
#define AX_AE_DETECT_OBJECT_MAX_NUM (10)

typedef enum AxAeDetectObjectCategory
{
    AXAE_FACE_ROI = 0,
    AXAE_BODY_ROI = 1,
    AXAE_VEHICLE_ROI = 2,
    AXAE_CYCLE_ROI = 3,
    AXAE_PLATE_ROI = 4
} AxAeDetectObjectCategory;

typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0x5] */
    AX_U32     nFaceLumaDiff[AX_AE_OBJECT_LUMA_DIFF_LEVEL];     /* the luma diff between face weighted meanluma and background weighted meanluma. Accuracy: U9.10 Range: [0x0, 0x4 0000]*/
    AX_U32     nFaceWeight[AX_AE_OBJECT_LUMA_DIFF_LEVEL]; /* the weight of face meanluma counted into frame meanluma; Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_FACE_WEIGHT_LIST_T;

typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0x5] */
    AX_U32     nVehicleLumaDiff[AX_AE_OBJECT_LUMA_DIFF_LEVEL];     /* the luma diff between vehicle weighted meanluma and background weighted meanluma. Accuracy: U9.10 Range: [0x0, 0x4 0000]*/
    AX_U32     nVehicleWeight[AX_AE_OBJECT_LUMA_DIFF_LEVEL]; /* the weight of vehicle meanluma counted into frame meanluma; Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_VEHICLE_WEIGHT_LIST_T;

typedef struct
{
    AX_U8  nEnable;                                      /* 0: disable face ae 1: enable face ae */
    AX_ISP_IQ_AE_FACE_TARGET_CURVE_T tFaceTargetCurve;
    AX_U16 nFaceScoreList[AX_AE_OBJECT_SCORE_LEVEL];               /* Face score level list:order from low to high. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFaceScoreWeightList[AX_AE_OBJECT_SCORE_LEVEL];         /* Weight table based on score level: order from low to high. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFaceDistanceList[AX_AE_OBJECT_DISTANCE_LEVEL];       /* Face distance between image center and face center;order from low to high. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U16 nFaceDistanceWeightList[AX_AE_OBJECT_DISTANCE_LEVEL];         /* Distance weight table: for calculating face score:order from high to low. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFaceTargetWeight;                                /* the weight of face target counted into frame target; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nFaceRoiFactor;                                   /* the factor of face roi counted for face luma; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nNoFaceFrameTh;                                   /*  the frame th of lost face. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nToNormalFrameTh;                                 /*  the frame th of blending FaceAE to normal AE. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nWithFaceFrameTh;                                   /*  the frame th of with face. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nToFaceAeFrameTh;                                 /*  the frame th of blending normal AE to FaceAE. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_ISP_IQ_AE_FACE_WEIGHT_LIST_T tFaceWeightList;
    AX_U16 nFaceWeightDampRatio;                            /* the damp ratio of face weight; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nToleranceAdjustRatio;                               /* the adjust ratio of ae algo tolerance in face AE enable.  Accuracy: U4.10 Range: [0x400, 0x2800] */
    AX_U16 nNoFaceDampRatio;                                    /* the damp ratio of no face; Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_FACE_UI_PARAM_T;


typedef struct
{
    AX_U8  nEnable;                                      /* 0: disable vehicle ae 1: enable vehicle ae */
    AX_U8 nPriorityMode;                                /* 0: default; 1: vehicle priority; 2: face priority. */
    AX_ISP_IQ_AE_VEHICLE_TARGET_CURVE_T tVehicleTargetCurve;
    AX_U16 nVehicleScoreList[AX_AE_OBJECT_SCORE_LEVEL];               /* vehicle score level list:order from low to high. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nVehicleScoreWeightList[AX_AE_OBJECT_SCORE_LEVEL];         /* Weight table based on score level: order from low to high. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nVehicleDistanceList[AX_AE_OBJECT_DISTANCE_LEVEL];       /* vehicle distance between image center and vehicle center;order from low to high. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U16 nVehicleDistanceWeightList[AX_AE_OBJECT_DISTANCE_LEVEL];         /* Distance weight table: for calculating vehicle score:order from high to low. Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nVehicleTargetWeight;                                /* the weight of vehicle target counted into frame target; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nVehicleRoiFactor;                                   /* the factor of vehicle roi counted for vehicle luma; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nNoVehicleFrameTh;                                   /*  the frame th of lost vehicle. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nToNormalFrameTh;                                 /*  the frame th of blending vehicleAE to normal AE. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nWithVehicleFrameTh;                                   /*  the frame th of with vehicle. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_U32 nToVehicleAeFrameTh;                                 /*  the frame th of blending normal AE to vehicleAE. Accuracy: U32 Range: [0x0, 0x12C] */
    AX_ISP_IQ_AE_VEHICLE_WEIGHT_LIST_T tVehicleWeightList;
    AX_U16 nVehicleWeightDampRatio;                            /* the damp ratio of vehicle weight; Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nToleranceAdjustRatio;                               /* the adjust ratio of ae algo tolerance in vehicle AE enable.  Accuracy: U4.10 Range: [0x400, 0x2800] */
    AX_U16 nNoVehicleDampRatio;                                    /* the damp ratio of no vehicle; Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_VEHICLE_UI_PARAM_T;


typedef struct {
    AX_U16 nObjectStartX;                   /* Object ROI start point. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U16 nObjectStartY;                   /* Object ROI start point. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U16 nObjectWidth;                    /* Object ROI width. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_U16 nObjectHeight;                   /* Object ROI height. Accuracy: U1.10 Range: [0x0, 0x400]*/
} AX_ISP_AE_OBJECT_RECT_INPUT_PARAM_T;

typedef struct {
    AX_U32 nObjectNum;                                        /* Numbers of detected Object. Accuracy: U32 Range: [0x0, 0xA] */
    AX_U32 nObjectID[AX_AE_DETECT_OBJECT_MAX_NUM];                                         /* ID of each Object. Accuracy: U32 */
    AX_U32 nObjectCategory[AX_AE_DETECT_OBJECT_MAX_NUM];                                    /* 0: face; 1: body; 2: vehicle; 3: cycle; 4: plate. */
    AX_U16 nObjectConfidence[AX_AE_DETECT_OBJECT_MAX_NUM];             /* Confidence of each Object. Accuracy: U1.10 Range: [0x0, 0x400]*/
    AX_ISP_AE_OBJECT_RECT_INPUT_PARAM_T tObjectInfos[AX_AE_DETECT_OBJECT_MAX_NUM];              /* Input Information of each Object */
} AX_ISP_AE_DETECT_OBJECT_PARAM_T;

typedef struct
{
    AX_U32 nAGain;         /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nDGain;         /* Accuracy: U22.10 Range: [tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nIspGain;       /* Accuracy: U22.10 Range: [tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U8  nHcgLcg;        /* HCG:0, LCG:1, NOT SUPPORT:2 */
    AX_U32 nShutter;       /* Uints: us. Accuracy: U32
                            * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                            * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
} AX_ISP_AE_SLEEP_SETTING_T;

#define AX_ISP_AE_OVEREXP_COMP_LUT_MAX_NUM (8)
typedef struct
{
    AX_U32 nLutNum;  /* Accuracy: U32  Range: [0x0, 0x8]*/
    AX_U32 nLumaSplitList[AX_ISP_AE_OVEREXP_COMP_LUT_MAX_NUM];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U16 nCompFactorList[AX_ISP_AE_OVEREXP_COMP_LUT_MAX_NUM]; /* Accuracy: U4.10 Range: [0x400, 0x2800] */
} AX_ISP_AE_OVEREXP_COMP_LUT_T;

#define AX_ISP_AE_HIST_POINT_CTRL_LUT_MAX_NUM (2)
#define AX_ISP_AE_HIST_POINT_CTRL_LUX_LIST_NUM_MAX (10)

typedef struct
{
    AX_U32 nLumaThList[AX_ISP_AE_HIST_POINT_CTRL_LUT_MAX_NUM];     /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nPercentThList[AX_ISP_AE_HIST_POINT_CTRL_LUT_MAX_NUM];   /* Accuracy: U7.10  Range: [0x0, 0x19000]*/
} AX_ISP_AE_HIST_POINT_CTRL_TH;

typedef struct
{
    AX_U32 nLuxStart;         /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_U32 nLuxEnd;         /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_ISP_AE_HIST_POINT_CTRL_TH tHistPointTh;
} AX_ISP_IQ_AE_HIST_POINT_CTRL_LUT_T;

typedef struct
{
    AX_U8 nEnable;           /* 0: disable hist point ctrl 1: enable hist point ctrl */
    AX_U32 nHistPointLutNum; /* Accuracy: U32  Range: [0x0, 0xA]*/
    AX_ISP_IQ_AE_HIST_POINT_CTRL_LUT_T tHistPointCtrlLut[AX_ISP_AE_HIST_POINT_CTRL_LUX_LIST_NUM_MAX];
} AX_ISP_IQ_AE_HIST_POINT_CTRL_PARAM_T;

typedef struct
{
    AX_U8  nEnableSleepSetting; /* 0: aov mode on; 1: aov mode off,fixed sleep-wakeup expo settings */
    AX_U8  nAovSmoothFrameNums; /* Accuracy: U8  Range: [0x1, 0x1E] */
    AX_U16 nNoiseLevel;    /* Accuracy: U1.10  Range: [0x0, 0x400] */
    AX_U32 nLinearLumaTh;  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U8  nAeStatsDelayFrame;  /* Accuracy: U8  Range: [0x0, 0xA] */
    AX_ISP_AE_SLEEP_SETTING_T tSleepSetting;
    AX_U32 nOverExpCompLumaTh; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_ISP_AE_OVEREXP_COMP_LUT_T tOverExpCompLut;
} AX_ISP_IQ_AE_SLEEP_WAKEUP_PARAM_T;

#define AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX (12)
typedef struct{
    AX_U32 nListSize; /* Accuracy: U32  Range: [0x0, 0xC]*/
    AX_U32 nRefList[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];     /* Accuracy: U22.10; lux range: [0x0, 0x3D090000]. */
    AX_U32 nDarkRegionStart[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX]; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nDarkRegionEnd[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];   /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nLightRegionStart[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];    /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nLightRegionEnd[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nThdDarkRegion[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];   /* Accuracy: U7.10  Range: [0x0, 0x19000]*/
    AX_U32 nThdUnSatLightRegion[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX]; /* Accuracy: U7.10  Range: [0x0, 0x19000]*/
    AX_U32 nThdSatLightRegion[AX_ISP_AE_DYNAMIC_RANGE_DET_PARAM_NUM_MAX];   /* Accuracy: U7.10  Range: [0x0, 0x19000]*/
} AX_ISP_IQ_AE_DYNAMIC_RANGE_DET_INFO_T;

typedef struct
{
    AX_U8 nEnable;           /* 0: disable dynamic range detect; 1: enable dynamic range detect */
    AX_ISP_IQ_AE_DYNAMIC_RANGE_DET_INFO_T tDynamicRangeDetInfo;
    AX_U32 nSwitchFrameTh;  /* SDR/HDR mode switch frame th. Accuracy: U32 Range: [0x0, 0x12C] */
} AX_ISP_IQ_AE_DYNAMIC_RANGE_DET_PARAM_T;

typedef struct {
    AX_U32 nSetPoint;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nFaceTarget;          /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nVehicleTarget;          /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nTolerance;           /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nAgainLcg2HcgTh;      /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nAgainHcg2LcgTh;      /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nAgainLcg2HcgRatio;   /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U32 nAgainHcg2LcgRatio;   /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U32 nLuxk;                /* Accuracy: U24    Range: [0x0, 0x989680] */

    AX_U8 nCompensationMode;     /* Accuracy: U8     Range: [0x0, 0x2] 0:Again Compensatition; 1:Dgain Compensatition; 2:Isp Dgain Compensatition*/

    AX_U32 nMaxIspGain;          /* Accuracy:U22.10 Range:[tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nMinIspGain;          /* Accuracy:U22.10 Range:[tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nMaxUserDgain;        /* Accuracy:U22.10 Range:[tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nMinUserDgain;        /* Accuracy:U22.10 Range:[tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nMaxUserTotalAgain;   /* Accuracy:U22.10 Range:[nSnsTotalAGainMin, nSnsTotalAGainMax] */
    AX_U32 nMinUserTotalAgain;   /* Accuracy:U22.10 Range:[nSnsTotalAGainMin, nSnsTotalAGainMax] */
    AX_U32 nMaxUserSysGain;      /* Accuracy:U22.10 Range:[nTotalGainMin, nTotalGainMax] */
    AX_U32 nMinUserSysGain;      /* Accuracy:U22.10 Range:[nTotalGainMin, nTotalGainMax] */

    AX_U32 nMaxShutter;          /* Uints: us. Accuracy:U32
                                  * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                                  * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
    AX_U32 nMinShutter;          /* Uints: us. Accuracy:U32 Range:[tSnsShutterLimit.nMin, tSnsShutterLimit.nMax] */

    AX_U8  nPositionWeightMode;  /* Accuracy: U8    Range: [0x0, 0x2] 0:Close  1:GridWeightMode  2:RoiWeightMode   */
    AX_U16 nRoiStartX;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nRoiStartY;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nRoiWidth;            /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nRoiHeight;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nWeightRoi;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U16 nWeightBackgnd;       /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U8  nGridWeightRow;       /* Accuracy: U8    Range: [0x1, 0x36]  */
    AX_U8  nGridWeightColumn;    /* Accuracy: U8    Range: [0x1, 0x48]  */
    AX_U16 nGridWeightTable[AX_ISP_AE_GRID_WEIGHT_ROW_MAX][AX_ISP_AE_GRID_WEIGHT_COLUMN_MAX];  /* Accuracy: U1.10 Range: [0x0, 0x400] */

    AX_ISP_IQ_AE_ANTIFLICKER_PARAMS_T tAntiFlickerParam;
    AX_ISP_IQ_AE_AUTO_FLICKER_DETECT_PARAMS_T tAutoFlickerDetectParam;
    AX_U32 nSetPointMode;     /* 0: fixed; 1: gain; 2: lux */
    AX_U32 nFaceTargetMode;     /* 0: fixed; 1: lux */
    AX_U32 nVehicleTargetMode;     /* 0: fixed; 1: lux */
    AX_U32 nStrategyMode;     /* 0: SHUTTER_PRIOR; 1:GAIN_PRIOR; 2:AE ROUTE */
    AX_U8 nAeRouteMode;       /* 0: default AeRoute; 1: Advanced AeRoute. */
    AX_ISP_IQ_AE_ROUTE_PARAM_T    tAeRouteParam;
    AX_ISP_IQ_AE_ROUTE_ADVANCE_TABLE_T  tAeRouteAdvanceTable;
    AX_ISP_IQ_AE_SETPOINT_CURVE_T tAeSetPointCurve;
    AX_ISP_IQ_AE_FACE_UI_PARAM_T tFaceUIParam;
    AX_ISP_IQ_AE_VEHICLE_UI_PARAM_T tVehicleUIParam;
    AX_ISP_IQ_AE_HDR_RATIO_T      tAeHdrRatio;
    AX_U8 nMultiCamSyncMode;  /* 0: INDEPEND MODE; 1: MASTER SLAVE MODE; 2: SPLIT HDR MODE; 3: SPLICE MODE  */
    AX_U32 nMultiCamSyncRatio; /* Accuracy: U7.20  Range: [0x0, 0x8000000] */
    AX_ISP_IQ_AE_SLOW_SHUTTER_PARAM_T tSlowShutterParam;
    AX_ISP_IQ_AE_IRIS_PARAMS_T  tIrisParam;
    AX_ISP_IQ_AE_LUMA_WEIGHT_PARAM_T tLumaWeightParam;
    AX_ISP_IQ_AE_TIME_SMOOTH_PARAM_T tTimeSmoothParam;
    AX_ISP_IQ_AE_SLEEP_WAKEUP_PARAM_T tSleepWakeUpParam;
    AX_ISP_IQ_AE_HIST_POINT_CTRL_PARAM_T tHistPointCtrlParam;
    AX_ISP_IQ_AE_DYNAMIC_RANGE_DET_PARAM_T tDynamicRangeDetParam;
} AX_ISP_IQ_AE_ALG_CONFIG_T;

typedef struct {
    AX_U32 nIspGain;       /* Accuracy: U22.10 Range: [tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nAGain;         /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nDgain;         /* Accuracy: U22.10 Range: [tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U8  nHcgLcg;        /* HCG:0, LCG:1 , Not Support: 2*/
    AX_U32 nSnsTotalAGain; /* Accuracy: U22.10 Range: [nSnsTotalAGainMin, nSnsTotalAGainMax] */
    AX_U32 nSysTotalGain;  /* Accuracy: U22.10 Range: [nTotalGainMin, nTotalGainMax] */
    AX_U32 nShutter;       /* Uints: us. Accuracy: U32
                            * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                            * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */

    AX_U32 nIrisPwmDuty;   /* Accuracy: U7.10  Range: [0x0, 0x19000] */
    AX_U32 nPos;           /* Accuracy: U10    Range: [0x0, 0x3FF] */
    AX_U32 nHdrRealRatioLtoS;   /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nHdrRealRatioStoVS;  /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nSetPoint;           /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */

    /* Below Unused on Tool, first defined, not nsed */
    AX_U32 nShortAgain;    /* Accuracy: U22.10 Range: [tShortAgainLimit.nMin, tShortAgainLimit.nMax] */
    AX_U32 nShortDgain;    /* Accuracy: U22.10 Range: [tShortDgainLimit.nMin, tShortDgainLimit.nMax] */
    AX_U32 nShortShutter;  /* Uints: us. Accuracy: U32 Range: [tShortShutterLimit.nMin, tShortShutterLimit.nMax] */
    AX_U32 nVsAgain;       /* Accuracy: U22.10 Range: [tVsAgainLimit.nMin, tVsAgainLimit.nMax] */
    AX_U32 nVsDgain;       /* Accuracy: U22.10 Range: [tVsDgainLimit.nMin, tVsDgainLimit.nMax] */
    AX_U32 nVsShutter;     /* Uints: us. Accuracy: U32 Range: [tVsShutterLimit.nMin, tVsShutterLimit.nMax] */
    AX_U32 nHdrRatio;      /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */

    AX_U32 nHdrMaxShutterHwLimit;  /* Accuracy: U32 Range [0x0,0xFFFF FFFF]*/
    AX_U32 nRealMaxShutter;        /* Accuracy: U32 Range [0x0,0xFFFF FFFF]*/
} AX_ISP_IQ_EXP_SETTING_T;


typedef struct {
    AX_U32 nMeanLuma;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
    AX_U32 nWeightedMeanLuma;    /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
    AX_U32 nLux;         /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_U64 nExpVal;      /* Accuracy: U36.10
                          * Range: min = tIspDgainLimit.nMin * tSnsAgainLimit.nMin * tSnsDgainLimit.nMin * tSnsShutterLimit.nMin
                          * Range: max = tIspDgainLimit.nMax * tSnsAgainLimit.nMax * tSnsDgainLimit.nMax * tSnsShutterLimit.nMax * nDcgRatio */
    AX_U32 nFps;         /* Accuracy: U8.10 Range: [0x400,0x19000]*/
    AX_U32 nFrameTarget;           /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nMinRatio;    /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nMaxRatio;    /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nShortSatLuma;   /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U8 nAeStable;        /* 0:AE Not Stabled; 1:AE Stabled */
    AX_U32 nAntiFlickerTolerance; /* Uints: us; Accuracy:U32 Range:[0x0, 0x208d] */
    AX_U16 nFlickerValidTh; /* Accuracy: U6.10, Range: [1, 65535] */
    AX_U16 nFlickerTrigTime;  /* Accuracy: U16, Range: [1, 65535] */
    AX_U8 nOutPutHdrMode;      /* Accuracy: U8, 1: LINEAR; 2:HDR. The result of dynamic range detection. */
} AX_ISP_IQ_AE_ALG_STATUS_T;

typedef struct {
    AX_U8 nEnable;
    AX_ISP_IQ_EXP_SETTING_T   tExpManual;
    AX_ISP_IQ_AE_ALG_CONFIG_T tAeAlgAuto;
    AX_U32 nLogLevel;     /* AXAE_LOG_EMERG:0,AXAE_LOG_ALERT:1,AXAE_LOG_CRIT:2,AXAE_LOG_ERROR:3, AXAE_LOG_WARN:4, AXAE_LOG_NOTICE:5, AXAE_LOG_INFO:6,AXAE_LOG_DBG:7*/
    AX_U32 nLogTarget;    /* AXAE_LOG_TARGET_STDERR:1, AXAE_LOG_TARGET_SYSLOG:2 */
} AX_ISP_IQ_AE_PARAM_T;

typedef struct {
    AX_ISP_IQ_AE_ALG_STATUS_T tAlgStatus;
    AX_ISP_IQ_EXP_SETTING_T   tExpStatus;
} AX_ISP_IQ_AE_STATUS_T;

typedef struct{
    AX_U8 scanStatus;          /*Accuracy:U6 Range:[0, 1] SCANRUNING = 0, SCANFINISH = 1*/
}AX_ISP_AE_SCAN_STATUS;

typedef struct {
    AX_U32 nMax;
    AX_U32 nMin;
    AX_U32 nStep;
} AX_ISP_IQ_AE_UNIT_T;

typedef struct {
    AX_U8  nGainMode;             /* 0: AGain_Only, 1: AGain_DGain_Separate, 2: AGain_DGain_Combined */
    AX_U8  nDcgEn;                /* 1: support LCG/HCG switch, 0: Not Support */
    AX_U32 nDcgRatio;             /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U64 nExpValLimitMin;       /* Accuracy: U36.10 Range: [0x0, 400000000000], Max/Min gain Depend on Sensor Spec */
    AX_U64 nExpValLimitMax;       /* Accuracy: U36.10 Range: [0x0, 400000000000], Max/Min gain Depend on Sensor Spec */
    AX_U32 nTotalGainMin;         /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_U32 nTotalGainMax;         /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_U32 nHdrRatioMin;          /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nHdrRatioMax;          /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nSnsTotalAGainMin;     /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_U32 nSnsTotalAGainMax;     /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */

    AX_ISP_IQ_AE_UNIT_T tIspDgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]  */
    AX_ISP_IQ_AE_UNIT_T tSnsShutterLimit;        /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */
    AX_ISP_IQ_AE_UNIT_T tSnsAgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tSnsDgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */

    AX_ISP_IQ_AE_UNIT_T tSnsSlowShutterModeShutterLimit;   /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */

    /*first defined, temp not used*/
    AX_ISP_IQ_AE_UNIT_T tShortAgainLimit;        /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tShortDgainLimit;        /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tShortShutterLimit;      /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */
    AX_ISP_IQ_AE_UNIT_T tVsAgainLimit;           /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tVsDgainLimit;           /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tVsShutterLimit;         /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */

} AX_ISP_IQ_EXP_HW_LIMIT_T;


typedef struct {
    AX_U64 nExpList[AX_ISP_AE_LUX_MAX_PAIR_NUM];     /* Accuracy: U36.10
                                                      * Range: min = tIspDgainLimit.nMin * tSnsAgainLimit.nMin * tSnsDgainLimit.nMin * tSnsShutterLimit.nMin
                                                      *  max = tIspDgainLimit.nMax * tSnsAgainLimit.nMax * tSnsDgainLimit.nMax * tSnsShutterLimit.nMax * nDcgRatio */
    AX_U32 nLuxList[AX_ISP_AE_LUX_MAX_PAIR_NUM];     /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_U32 nLumaList[AX_ISP_AE_LUX_MAX_PAIR_NUM];    /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
} AX_ISP_IQ_LUX_K_CALIB_INPUT_T;

#define AX_ISP_AE_NOISE_LEVEL_MAX_NUM (5)
typedef struct {
    AX_U32 nLumaList[AX_ISP_AE_NOISE_LEVEL_MAX_NUM];    /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
} AX_ISP_IQ_NOISE_LEVEL_CALIB_INPUT_T;

typedef struct {
    AX_U32 nAeSyncRatio;                             /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
} AX_ISP_IQ_AE_SYNC_RATIO_T;


typedef struct _AX_ISP_AE_AFD_STATUS_FUNC_T_
{
    AX_S32 (*pfnAeSetFlickerStatus)(AX_U8 pipe, AX_U8 initAfdStatus);
    AX_S32 (*pfnAeGetFlickerStatus)(AX_U8 pipe, AX_U8 *initAfdStatus);
} AX_ISP_AE_AFD_STATUS_FUNC_T;


typedef struct {
    AX_U16 nFlickerFreq;
} AX_AE_AFD_OUT_T;

typedef struct
{
    AX_U8             size;
    AX_F32            pRefList[AE_ISP_ANTI_FLICKER_MAX_NUM];
    AX_F32            pFlickerValidThList[AE_ISP_ANTI_FLICKER_MAX_NUM];
    AX_U16            pFlickerTrigTimeList[AE_ISP_ANTI_FLICKER_MAX_NUM];
} AX_ISP_ALG_AE_FLICKER_INFO_T;

typedef struct{
    AX_U8  nFlickerValidNum;
    AX_ISP_ALG_AE_FLICKER_INFO_T tFlickerParamCurve;
    AX_F32 fFlickerValidTh;
    AX_U8  nSkipTh;
    AX_F32 fUpSlopeTh;
    AX_F32 fDownSlopeTh;
}AX_AE_AFD_PARAMS_T;

typedef struct{
    AX_U8 nValidPipeCnt;

    AX_U8 nAeLogLevel;
    AX_U8 nAeLogTarget;
    AX_U8 nLogLevelExtern;
    AX_U8 nLogTargetExtern;
}AX_AE_AFD_LOG_T;
////////////////////////////////////////////////////////////////////////////////////
//  AF ALG Param
////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    AX_S32 nDefaultDistance;                             /*Accuracy:S15 Range:[-32768,32767]*/
    AX_U8  nStepFactor;                                  /*Accuracy:U8 Range:[1,256]*/
    AX_S32 nProbStepCoff;                                 /*Accuracy:S15 Range:[-32768,32767]*/
    AX_U8  nMaxProbeStep;                               /*Accuracy:U8 Range:[1,256]*/
    AX_U8  nMinProbeStep;                              /*Accuracy:U8 Range:[1,256]*/
    AX_S64 nFvDiffOverThresh;                           /*Accuracy:S32 Range:[-2147483648,2147483647]*/
    AX_S64 nFvDiffMiddleThresh;                         /*Accuracy:S32 Range:[-2147483648,2147483647]*/
    AX_S64 nFvDiffUnderThresh;                          /*Accuracy:S32 Range:[-2147483648,2147483647]*/
    AX_U32 nHeightDistanceCoff;                       /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32 nMiddleDistanceCoff;                       /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32 nUnderDistanceCoff;                        /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32 nProportionalCoff;                        /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32 nIntegralCoff;                             /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32 nDifferentialCoff;                         /* Accuracy:U10.5 Range: [0, 1048575] */
} AX_ISP_CAF_ZOOM_TRACKING_T;
typedef struct {
    AX_U32 nMuX;                                  /* Accuracy:U6.10 Range:[0, 20480] */
    AX_U32 nMuY;                                  /* Accuracy:U6.10 Range:[0, 20480] */
    AX_U32 nSigmaX;                               /* Accuracy:U6.10 Range:[10, 10240] */
    AX_U32 nSigmaY;                               /* Accuracy:U6.10 Range:[10, 10240] */
    AX_U32 nCoeffV1;                               /* Accuracy:U6.10 Range:[10, 1024] */
    AX_U32 nCoeffV2;                               /* Accuracy:U6.10 Range:[10, 1024] */
    AX_U32 nCoeffH1;                              /* Accuracy:U6.10 Range:[10, 1024] */
    AX_U32 nCoeffH2;                              /* Accuracy:U6.10 Range:[10, 1024] */
} AX_ISP_CAF_WEIGHT_T;

typedef struct {

    /* Settings of Scan */
    AX_U8   nScanType;                  /* Accuracy:U1 Range: [0, 1] */
    AX_U32  nGlobalScanStep;            /* Accuracy:U10 Range: [0, 1023] */

    /* Tuning Params ClimbHill Search */
    AX_U32  nSearchDownFrameTh;         /* Accuracy:U8 Range: [0, 255] */
    AX_U32  nSearchDownSerialSlopeTh;   /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32  nSearchDownSingleSlopeTh;   /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32  nSearchUpFrameTh;           /* Accuracy:U8 Range: [0, 255] */
    AX_U32  nSearchUpSerialSlopeTh;     /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32  nSearchUpSingleSlopeTh;     /* Accuracy:U10.5 Range: [0, 1048575] */

    AX_U32  nSearchBigStepUpSlopeTh;    /* Accuracy:U10.5 Range: [0, 1048575] */
    AX_U32  nSearchBigStep;             /* Accuracy:U10 Range: [0, 1023] */
    AX_U32  nSearchSmallStep;           /* Accuracy:U10 Range: [0, 1023] */
    AX_U32  nSearchProbeStep;           /* Accuracy:U10 Range: [0, 1023] */
    AX_U32  nSearchProbeFrameTh;        /* Accuracy:U8 Range: [0, 255] */

    /* Continuous AF Enable */
    AX_U8   bContinuousAfEn;            /* Accuracy:U1 Range: [0, 1] */
    AX_U32  nSceneChangeLumaTh;         /* Accuracy:U12.0 Range: [0, 4095] */
    AX_U32  nSceneSettledLumaTh;        /* Accuracy:U12.0 Range: [0, 4095] */
    AX_U32  nSceneSlowChangeLumaTh;     /* Accuracy:U12.0 Range: [0, 4095] */
    AX_U32  nSceneFvChangeLumaTh;       /* Accuracy:U12.0 Range: [0, 4095] */
    AX_U32  nSceneFvChangeRatioTh;      /* Accuracy:U1.10 Range: [0, 1024] */

    AX_U32  nSpotlightZoomRatioTh;      /*Accuracy:U7.10 Range:[0, 4194303]*/
    AX_ISP_CAF_WEIGHT_T  tWeight;
    AX_ISP_CAF_ZOOM_TRACKING_T tZoomTracking;
} AX_ISP_IQ_CAF_PARAM_T;

typedef struct
{
    AX_S32 zoomPos;           /*Accuracy:S21.10 Range:[-169,1331]*/
}AX_ISP_CAF_ZOOM_POS;
typedef struct
{
    AX_S32 focusPos;           /*Accuracy:S21.10 Range:[-280,1500]*/
}AX_ISP_CAF_FOCUS_POS;
typedef struct
{
    AX_U32 zoomRatio;           /*Accuracy:U5.10 Range:[1*1024,30*1024]*/
}AX_ISP_CAF_ZOOM_RATIO;

typedef struct{
    AX_U8 scanStatus;          /*Accuracy:U6 Range:[0, 2] SCANIDLE=0, SCANRUNING=1, SCANFINISH=2*/
}AX_ISP_CAF_SCAN_STATUS;

typedef struct
{
    AX_S32 focusPos;      /*Accuracy:S21.10 Range:[-280,1500]*/
    AX_U64 fv;        /*Accuracy: U36.10 Range:[0x0,0xFFFF FFFF FFFF FFFF]*/
}AX_FV_FOCUS_INFO;

typedef struct {
    AX_U32 dataNum;     /* Accuracy:U14 Range: [0, 2048] */
    AX_FV_FOCUS_INFO fvfposInfo[AX_ISP_AF_FV_SCAN_MAX_NUM];
    AX_FV_FOCUS_INFO fvFocusPosPeak;
} AX_ISP_CAF_SCAN_FV_INFO;

////////////////////////////////////////////////////////////////////////////////////
//  IR ALG Param
////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    AX_U32 nIrCalibR;               /* Accuracy: U8.10 Range: [0x0, 0x4000], R of all IR scene ,calibration it if sensor or ir light change */
    AX_U32 nIrCalibG;               /* Accuracy: U8.10 Range: [0x0, 0x4000], G of all IR scene ,calibration it if sensor or ir light change */
    AX_U32 nIrCalibB;               /* Accuracy: U8.10 Range: [0x0, 0x4000], B of all IR scene ,calibration it if sensor or ir light change */
    AX_U32 nNight2DayIrStrengthTh;  /* Accuracy: U8.10 Range: [0x0, 0x400], if the value big, IR cut is easy to switch */
    AX_U32 nNight2DayIrDetectTh;    /* Accuracy: U8.10 Range: [0x0, 0x2800], if the value big, IR cut is difficult to switch,do not need to change in most cases */
    AX_U32 nNight2DayLuxTh;         /* Accuracy: U22.10 Range: [0x0, 0xFFFFFFFF] ,if lux bigger zhan this,and satisfy nNight2DayIrStrengthTh and nNight2DayIrDetectTh. turn day mode*/
    AX_U32 nNight2DayBrightTh;      /* Accuracy: U8.10 Range: [0x0, 0x3FC00],if WP Y lagger than this,abandon it*/
    AX_U32 nNight2DayDarkTh;        /* Accuracy: U8.10 Range: [0x0, 0x3FC00], if WP Y litter than this,abandon it*/
    AX_U32 nNight2DayUsefullWpRatio;/* Accuracy: U8.10 Range: [0x0, 0x400],if lux litter zhan this,turn Day mode*/
    AX_U32 nDay2NightLuxTh;         /* Accuracy: U22.10 Range: [0x0, 0xFFFFFFFF], cur daynight mode */
    AX_U32 nInitDayNightMode;       /* 0: day 1:night */
    AX_U32 nCacheTime;              /* cache time */
    //float nDayNightSwitch;        /* 0: not need to switch ircut 1:need to switch ircut */
} AX_ISP_IQ_IR_PARAM_T;

////////////////////////////////////////////////////////////////////////////////////
//  sensor 3a default Param
////////////////////////////////////////////////////////////////////////////////////
typedef struct _AX_SENSOR_3A_DEFAULT_PARAM_T_ {
    AX_ISP_IQ_AE_PARAM_T        *ptAeDftParam;
    AX_ISP_IQ_AWB_PARAM_T       *ptAwbDftParam;
} AX_SENSOR_3A_DEFAULT_PARAM_T;

#ifdef __cplusplus
}
#endif

#endif //_AX_ISP_3A_STRUCT_H_

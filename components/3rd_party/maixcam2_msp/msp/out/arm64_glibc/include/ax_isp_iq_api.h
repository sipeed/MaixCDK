/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_ISP_IQ_API_H__
#define __AX_ISP_IQ_API_H__

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_isp_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AX_ISP_MAX_PATH_SIZE                        (256)
#define AX_ISP_AUTO_TABLE_MAX_NUM                   (16)
#define AX_ISP_GAIN_GRP_NUM                         (16)
#define AX_ISP_EXPOSE_TIME_GRP_NUM                  (10)
#define AX_ISP_BIAS_OUT_OFFSET_NUM                  (4)

#define AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM         (12)
#define AX_ISP_REF_AUTOTBL_DEHAZE_EXPRESS_NUM       (12)
#define AX_ISP_REF_AUTOTBL_CCMP_EXPRESS_NUM         (12)
#define AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM      (12)
#define AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM      (12)
#define AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM      (12)
#define AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM      (12)
#define AX_ISP_REF_AUTOTBL_LSC_EXPRESS_NUM          (12)
#define AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM           (8)
#define AX_ISP_HDR_RATIO_GRP_NUM                    (4)


/************************************************************************************
 *  BLC IQ Param: SBL
 ************************************************************************************/

typedef struct {
    AX_S32 nBiasOut[AX_ISP_BIAS_OUT_OFFSET_NUM]; /*Accuracy: S15.4 Range: [-16384, 16383],Default:[0,0,0,0]*/
} AX_ISP_BIAS_OUT_PARAM_T;

typedef struct {
    AX_U32 nSblRValue;    /* Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nSblGrValue;   /* Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nSblGbValue;   /* Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nSblBValue;    /* Accuracy: U12.8 Range: [0, 0xFFFFF] */
} AX_ISP_IQ_BLC_MANUAL_T;

typedef struct {
    AX_U8 nGainGrpNum;                    /* Gain dimension num. Accuracy: U8.0 Range: [0, AX_ISP_GAIN_GRP_NUM] */
    AX_U8 nExposeTimeGrpNum;              /* ExposeTime dimension num. Accuracy: U8.0 Range: [0, AX_ISP_EXPOSE_TIME_GRP_NUM] */
    AX_U32 nGain[AX_ISP_GAIN_GRP_NUM];    /* Again value for sbl tunning. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF] */
    AX_U32 nExposeTime[AX_ISP_EXPOSE_TIME_GRP_NUM];    /* ExposeTime value for sbl tunning. Accuracy: U32 Range: [0x0, 0xFFFFFFFF] */
    AX_U32 nAutoSblRValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];    /* offline sbl tunning value for R channel.  Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblGrValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];   /* offline sbl tunning value for Gr channel. Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblGbValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];   /* offline sbl tunning value for Gb channel. Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblBValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];    /* offline sbl tunning value for B channel.  Accuracy: U12.8 Range: [0, 0xFFFFF] */
    AX_ISP_BIAS_OUT_PARAM_T tBiasOut[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];
} AX_ISP_IQ_BLC_AUTO_TABLE_T;

typedef struct {
    AX_ISP_IQ_BLC_AUTO_TABLE_T      tHcgAutoTable;
    AX_ISP_IQ_BLC_AUTO_TABLE_T      tLcgAutoTable;
} AX_ISP_IQ_BLC_AUTO_T;

typedef struct {
    AX_U8                           nBlcEnable;     /* blc enable */
    AX_U8                           nSblEnable;     /* sbl correction enable */
    AX_U8                           nAutoMode;      /* BLC Automode enable 0: manual, 1: auto */
    AX_ISP_BIAS_OUT_PARAM_T         tBiasOut;
    AX_ISP_IQ_BLC_MANUAL_T          tManualParam[AX_HDR_CHN_NUM]; /* used [0] for SDR or (0, 1, 2, 3) for HDR:(L, S, VS, VVS) */
    AX_ISP_IQ_BLC_AUTO_T            tAutoParam;
} AX_ISP_IQ_BLC_PARAM_T;

/************************************************************************************
 *  NUC IQ Param
 ************************************************************************************/
typedef struct {
    AX_U8 nNuc1stFrame;        /* config 1st frame: default 1, Accuracy: U1.0 Range: [0, 1] */
    AX_U8 nStepEnhEn;           /* step enhance enable: default 0, Accuracy: U1.0 Range: [0, 1] */
    AX_U8 nEffectiveBits;         /* calib shift bits: default 7, Accuracy: U3.0 Range: [0, 7] */
    AX_U8 nNucAdjustTrend;   /* nuc flag: default 0, Accuracy: U1.0 Range: [0, 1] */
    AX_U8 nNucFineBitMask;      /* fine bit mask: default 15, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucCoarseBitMask;   /* coarse bit mask: default 240, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucInitFine;            /* nuc init fine: default 0, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucInitCoarse;       /* nuc init coarse: default 0, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucFineStep;         /* fine step: default 17, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucCoarseStep;     /* Coarse step: default 17, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucMaxFVal;         /* max fine val: default 17, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucMaxCVal;        /* max coarse val: default 17, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucMinFVal;         /* min fine val: default 0, Accuracy: U8.0 Range: [0, 255] */
    AX_U8 nNucMinCVal;         /* min coarse val: default 0, Accuracy: U8.0 Range: [0, 255] */
    AX_U16 nNucAdFineHigh;       /* adjust fine high: default 65535, Accuracy: U12.4 Range: [0, 65535] */
    AX_U16 nNucAdCoarseHigh;   /* adjust coarse high: default 65535, Accuracy: U12.4 Range: [0, 65535] */
    AX_U16 nNucAdFineLow;      /* adjust fine low: default 65535, Accuracy: U12.4 Range: [0, 65535] */
    AX_U16 nNucAdCoarseLow;   /* adjust coarse low: default 65535, Accuracy: U12.4 Range: [0, 65535] */
    AX_U16 nNucFineCnt;         /* nuc fine count: default 1, Accuracy: U16.0 Range: [0, 65535] */
    AX_U16 nNucCoarseCnt;    /* nuc coarse count: default 1, Accuracy: U16.0 Range: [0, 65535] */
} AX_ISP_IQ_SENSOR_NUC_PARAM_T;

typedef struct {
    AX_U8 nNucFpnEn;          /* nuc fpn enable: default 0, Accuracy: U1.0 Range: [0, 1] */
    AX_U16 nNucBaseGain;    /* nuc base gain: default 256, Accuracy: U4.8 Range: [0, 4095] */
    AX_U16 nNucFpnGain;      /* nuc fpn gain: default 256, Accuracy: U4.8 Range: [0, 4095] */
} AX_ISP_IQ_TWO_POINTS_PARAM_T;

typedef struct {
    AX_ISP_IQ_SENSOR_NUC_PARAM_T tSensorNucCalib;
    AX_ISP_IQ_TWO_POINTS_PARAM_T tTwoPointsCalib;
} AX_ISP_IQ_NUC_MANUAL_PARAM_T;

typedef struct {
    AX_U8 nNucEnable;          /* nuc enable: default 0,Accuracy: U1 Range: [0, 1] */
    AX_U8 nModuleMode;       /* nuc work mode: default 0, Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_NUC_MANUAL_PARAM_T tManualParam;
} AX_ISP_IQ_NUC_PARAM_T;

/************************************************************************************
 *  DPC IQ Param
 ************************************************************************************/

#define AX_ISP_DPC_NOISE_PARAM_NUM          (4)
#define AX_ISP_DPC_SDPC_BUFFER_MAX          (8192)
#define AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM  (2)
#define AX_ISP_DPC_MARGIN_LIMIT_NUM         (2)
#define AX_ISP_DPC_MARGIN_NUM               (2)

typedef struct{
    AX_U16 nUpperLimit;
    AX_U16 nLowerLimit;
}AX_ISP_IQ_DPC_MARGIN_T;

typedef struct {
    AX_U16 nNoiseRatio;           /* Accuracy: U4.10 Range: [0, 0x27FF], default: 1024*/
    AX_U8  nDpType;                                                /* Defective Pixel Type. Accuracy: U1.0 Range: [0, 1], default: 0, 0: single defective pixel mode, 1: dual defective pixel mode */
    AX_U8  nNonChwiseEn;                                           /* Non Chwise Enable. Accuracy: U1.0 Range: [0, 1], default: 0 */
    AX_U8  nChwiseStr;                                             /* Accuracy: U1.4 Range: [0, 0x1F], default: 20 */
    AX_U8  nDetCoarseStr;                                          /* Accuracy: U4.4 Range: [0, 0xFF], default: 236 */
    AX_U8  nDetFineStr;                                            /* Accuracy: U0.6 Range: [0, 0x3F], default: 48 */
    AX_U16 nDynamicDpcStr;                                         /* Accuracy: U4.8 Range: [0, 0xFFF], default: 128 */
    AX_U8  nEdgeStr;                                               /* Accuracy: U1.7 Range: [0, 0xFF], default: 102 */
    AX_U8  nHotColdTypeStr;                                        /* Accuracy: U1.7 Range: [0, 0x80], default: 32 */
    AX_U8  nSupWinkStr;                                            /* Accuracy: U4.4 Range: [0, 0xFF], default: 16 */
    AX_ISP_IQ_DPC_MARGIN_T nDynamicDpClrLimOffset;                 /* Accuracy: U12.4 Range: [0, 0xFFFF], default: 1024 */
    AX_ISP_IQ_DPC_MARGIN_T nStaticDpClrLimOffset;                  /* Accuracy: U12.4 Range: [0, 0xFFFF], default: 1024 */
    AX_ISP_IQ_DPC_MARGIN_T nNormalPixDpClrLimOffset;               /* Accuracy: U12.4 Range: [0, 0xFFFF], default: 1024 */
    AX_U8  nDynamicDpClrLimStr;                                    /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nStaticDpClrLimStr;                                     /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nNormalPixDpClrLimStr;                                  /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nPreDetLevelSlope;                               /* Predet Level Slope.Accuracy: U0.4 Range: [0, 0xF], default: 4 */
    AX_U16  nPreDetLevelOffset;                              /* Predet Level Offset.Accuracy: U8.4 Range: [0, 0xFFF], default: 0 */
    AX_U16  nPreDetLevelMax;                              /* Predet Level Max.Accuracy: U8.4 Range: [0, 0xFFF], default: 256 */
} AX_ISP_IQ_DPC_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                                        /* Accuracy: U8.0 Range:[0, AX_ISP_AUTO_TABLE_MAX_NUM], default: 1 */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];                  /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF], default: 1/0x400 */
    AX_U16 nNoiseRatio[AX_ISP_AUTO_TABLE_MAX_NUM];              /* Accuracy: U4.10 Range: [0, 0x27FF], default: 1024*/
    AX_U8  nDpType[AX_ISP_AUTO_TABLE_MAX_NUM];                  /* Defective Pixel Type. Accuracy: U1.0 Range: [0, 1], default: 0, 0: single defective pixel mode, 1: dual defective pixel mode  */
    AX_U8  nNonChwiseEn[AX_ISP_AUTO_TABLE_MAX_NUM];             /* Non Chwise Enable. Accuracy: U1.0 Range: [0, 1], default: 0 */
    AX_U8  nChwiseStr[AX_ISP_AUTO_TABLE_MAX_NUM];               /* Accuracy: U1.4 Range: [0, 0x1F], default: 20 */
    AX_U8  nDetCoarseStr[AX_ISP_AUTO_TABLE_MAX_NUM];            /* Accuracy: U4.4 Range: [0, 0xFF], default: 237 */
    AX_U8  nDetFineStr[AX_ISP_AUTO_TABLE_MAX_NUM];              /* Accuracy: U0.6 Range: [0, 0x3F], default: 16 */
    AX_U16 nDynamicDpcStr[AX_ISP_AUTO_TABLE_MAX_NUM];           /* Accuracy: U4.8 Range: [0, 0xFFF], default: 2048 */
    AX_U8  nEdgeStr[AX_ISP_AUTO_TABLE_MAX_NUM];                 /* Accuracy: U1.7 Range: [0, 0xFF], default: 102 */
    AX_U8  nHotColdTypeStr[AX_ISP_AUTO_TABLE_MAX_NUM];          /* Accuracy: U1.7 Range: [0, 0x80], default: 96 */
    AX_U8  nSupWinkStr[AX_ISP_AUTO_TABLE_MAX_NUM];              /* Accuracy: U4.4 Range: [0, 0xFF], default: 256 */
    AX_ISP_IQ_DPC_MARGIN_T nDynamicDpClrLimOffset[AX_ISP_AUTO_TABLE_MAX_NUM];        /* Accuracy: U12.4 Range: [0, 0xFFFF], default: [256, 256] */
    AX_ISP_IQ_DPC_MARGIN_T nStaticDpClrLimOffset[AX_ISP_AUTO_TABLE_MAX_NUM];         /* Accuracy: U12.4 Range: [0, 0xFFFF], default: [65535, 65535] */
    AX_ISP_IQ_DPC_MARGIN_T nNormalPixDpClrLimOffset[AX_ISP_AUTO_TABLE_MAX_NUM];      /* Accuracy: U12.4 Range: [0, 0xFFFF], default: [0, 32678] */
    AX_U8  nDynamicDpClrLimStr[AX_ISP_AUTO_TABLE_MAX_NUM];      /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nStaticDpClrLimStr[AX_ISP_AUTO_TABLE_MAX_NUM];       /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nNormalPixDpClrLimStr[AX_ISP_AUTO_TABLE_MAX_NUM];    /* Accuracy: U1.7 Range: [0, 0x80], default: 128 */
    AX_U8  nPreDetLevelSlope[AX_ISP_AUTO_TABLE_MAX_NUM];                               /* Predet Level Slope.Accuracy: U0.4 Range: [0, 0xF], default: 4 */
    AX_U16  nPreDetLevelOffset[AX_ISP_AUTO_TABLE_MAX_NUM];                              /* Predet Level Offset.Accuracy: U8.4 Range: [0, 0xFFF], default: 0 */
    AX_U16  nPreDetLevelMax[AX_ISP_AUTO_TABLE_MAX_NUM];                              /* Predet Level Max.Accuracy: U8.4 Range: [0, 0xFFF], default: 256 */
} AX_ISP_IQ_DPC_AUTO_T;

typedef struct {
    AX_S32 nShotNoiseCoeffsA[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nShotNoiseCoeffsB[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsA[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsB[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsC[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
} AX_ISP_DPC_NOISE_SBPC_T;

typedef struct {
    AX_ISP_DPC_NOISE_SBPC_T         tHcgTable;
    AX_ISP_DPC_NOISE_SBPC_T         tLcgTable;
} AX_ISP_DPC_TABLE_T;

typedef struct {
    AX_U8  nDpcEnable;                                      /* Accuracy: U1.0 Range: [0, 1], default: 1   */
    AX_U8  nStaticDpcEnable;                                /* Accuracy: U1.0 Range: [0, 1], default: 0 */
    AX_U8  nDynamicDpcEnable;                               /* Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_U8  nColorLimitEnable;                               /* Color Limit Enable. Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_U8  nAutoMode;                                       /* Accuracy: U1.0 Range: [0, 1], default: 0 */
    AX_U8  nRefMode;                                        /* Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_ISP_DPC_TABLE_T              tDpcParam;
    AX_ISP_IQ_DPC_MANUAL_T          tManualParam;
    AX_ISP_IQ_DPC_AUTO_T            tAutoParam;
} AX_ISP_IQ_DPC_PARAM_T;

/************************************************************************************
 *  HDR IQ Param
 ************************************************************************************/
#define AX_ISP_HDR_DGST_THRE_SIZE  (2)
#define AX_ISP_HDR_DGST_LIMIT_SIZE  (2)
#define AX_ISP_HDR_MOT_MASK_RATIO_SIZE    (2)
#define AX_ISP_HDR_MOT_IIR_RATIO_SIZE    (2)
#define AX_ISP_HDR_EXP_MASK_RATIO_SIZE    (2)
#define AX_ISP_HDR_EXP_IIR_RATIO_SIZE    (2)
#define AX_ISP_HDR_EXP_LUT_SIZE     (257)
#define AX_ISP_HDR_FUS_PROT_SIZE    (2)
#define AX_ISP_HDR_EXP_FRAME_SIZE   (2)
#define AX_ISP_HDR_EXP_MASK_SEN_SIZE   (2)

typedef struct {
    AX_U32      nFusionProtectThreshold[AX_ISP_HDR_EXP_FRAME_SIZE][AX_ISP_HDR_FUS_PROT_SIZE];      /* Accuracy: U12.6 Range: [0, 262143] Default: (16000, 16192) */
} AX_ISP_IQ_HDR_FUSION_PARAM_T;

typedef struct {
    AX_U16      nCoarseMotMaskRatio[AX_ISP_HDR_MOT_MASK_RATIO_SIZE]; /* Accuracy: U1.8 Range: [0, 256] Default: [0, 0] */
    AX_U16      nMotIirRatio[AX_ISP_HDR_MOT_IIR_RATIO_SIZE];        /* Accuracy: U1.8 Range: [0, 256] Default: [0, 0] */
} AX_ISP_IQ_HDR_MOT_DET_PARAM_T;

typedef struct {
    AX_U16      nCoarseExpMaskRatio[AX_ISP_HDR_EXP_FRAME_SIZE][AX_ISP_HDR_EXP_MASK_RATIO_SIZE]; /* Accuracy: U1.8 Range: [0, 256] Default: all 0 */
    AX_U16      nExpIirRatio[AX_ISP_HDR_EXP_FRAME_SIZE][AX_ISP_HDR_EXP_IIR_RATIO_SIZE];        /* Accuracy: U1.8 Range: [0, 256] Default: all 0 */
    AX_U16      nExpYRatio[AX_ISP_HDR_EXP_FRAME_SIZE];                                     /* Accuracy: U1.8 Range: [0, 256] Default: [0, 0] */
    AX_U16      nExpWeightLut[AX_ISP_HDR_EXP_FRAME_SIZE][AX_ISP_HDR_EXP_LUT_SIZE];        /* Accuracy: U1.15 Range: [0, 32768] Default: all 32768 */
    AX_U16      nExpWeightGain[AX_ISP_HDR_EXP_FRAME_SIZE];                                 /* Accuracy: U1.8 Range: [0, 256] Default: [256, 256] */
} AX_ISP_IQ_HDR_EXP_MASK_PARAM_T;

typedef struct {
    AX_U8   nDeghostEnable;                                 /* Accuracy: U1 Range: [0, 1] Default: 0 */
    AX_U16  nDgstStrenThre[AX_ISP_HDR_DGST_THRE_SIZE];     /* Accuracy: U1.10 Range: [0, 1024] Default: 1024 */
    AX_U16  nDgstStrenLimit[AX_ISP_HDR_DGST_LIMIT_SIZE];    /* Accuracy: U1.8 Range: [0, 256] Default: [0, 256] */
    AX_U8   nDgstBaseFid;                                   /* Accuracy: U1 Range: [0, 1] Default: 0 */
} AX_ISP_IQ_HDR_DGST_PARAM_T;

typedef struct {
    AX_U16  nNoiseLutScale;                                 /* Accuracy: U4.12 Range: [0, 65535] Default: 4096 */
    AX_U16  nCoarseMotMaskNoiseLvl;                          /* Accuracy: U1.11 Range: [0, 2048] Default: 0 */
    AX_U16  nCoarseMotMaskSen;                               /* Accuracy: U1.15 Range: [0, 32768] Default: 0 */
    AX_U16  nCoarseExpMaskSen[AX_ISP_HDR_EXP_MASK_SEN_SIZE]; /* Accuracy: U1.15 Range: [0, 32768] Default: [0, 0] */
} AX_ISP_IQ_HDR_MANUAL_PARAM_T;

typedef struct {
    AX_U8   nParamGrpNum;                                                           /* Accuracy: U8 Range: [0, 16] */
    AX_U32  nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];                                     /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16  nNoiseLutScale[AX_ISP_AUTO_TABLE_MAX_NUM];                              /* Accuracy: U4.12 Range: [0, 65535] Default: 4096 */
    AX_U16  nCoarseMotMaskNoiseLvl[AX_ISP_AUTO_TABLE_MAX_NUM];                      /* Accuracy: U1.11 Range: [0, 2048] Default: all 0 */
    AX_U16  nCoarseMotMaskSen[AX_ISP_AUTO_TABLE_MAX_NUM];                           /* Accuracy: U1.15 Range: [0, 32768] Default: all 0 */
    AX_U16  nCoarseExpMaskSen[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_HDR_EXP_MASK_SEN_SIZE]; /* Accuracy: U1.15 Range: [0, 32768] Default: all 0 */
} AX_ISP_IQ_HDR_AUTO_PARAM_T;

typedef struct {
    AX_U8                           nEnable;        /* Accuracy: U1 Range: [0, 1] Default: 0 */
    AX_U8                           nAutoMode;      /* Accuracy: U1 Range: [0, 1] Default: 0 */
    AX_U8                           nRefMode;       /* Accuracy: U1 Range: [0, 1] Default: 0 */
    AX_U8                           nDebugMode;     /* Accuracy: U3 Range: [0, 4] Default: 0 */
    AX_ISP_IQ_HDR_MOT_DET_PARAM_T   tMotDetParam;
    AX_ISP_IQ_HDR_EXP_MASK_PARAM_T  tExpMaskParam;
    AX_ISP_IQ_HDR_DGST_PARAM_T      tDgstParam;
    AX_ISP_IQ_HDR_FUSION_PARAM_T    tFusionParam;
    AX_ISP_IQ_HDR_MANUAL_PARAM_T    tHdrManualParam;
    AX_ISP_IQ_HDR_AUTO_PARAM_T      tHdrAutoParam;
} AX_ISP_IQ_HDR_PARAM_T;


/************************************************************************************
 *  AINR IQ Param
 ************************************************************************************/
#define AX_ISP_AINR_ISO_MODEL_MAX_NUM        (12)
#define AX_ISP_AINR_REF_VALUE_MAX_NUM        (4)
#define AX_ISP_AINR_MODEL_MAX_NUM            (AX_ISP_AINR_ISO_MODEL_MAX_NUM)

#define AX_ISP_AINR_LUT_COL_NUM     (2)
#define AX_ISP_AINR_LUT_ROW_NUM     (17)
#define AX_ISP_AINR_OFFSET_NUM  (4)

typedef struct {
    AX_CHAR   szModelPath[AX_ISP_MAX_PATH_SIZE];                                      /*model path, absolute path */
    AX_CHAR   szModelName[AX_ISP_MAX_PATH_SIZE];                                      /*model path, relative path */
    AX_CHAR   szTemporalBaseNrName[AX_ISP_MAX_PATH_SIZE];
    AX_CHAR   szSpatialBaseNrName[AX_ISP_MAX_PATH_SIZE];
    AX_U8     nHcgMode;                                                               /*model param, based on the real param of model. Accuracy: U2 Range: [0, 3] 0:LCG,1:HCG,2:LCG NOT SURPPORT*/
    AX_U32    nIsoThresholdMin;                                                       /*Accuracy: U32 Range: [1, 0xFFFFFFFF] <= */
    AX_U32    nIsoThresholdMax;                                                       /* Accuracy: U32 Range: [1, 0xFFFFFFFF] > */
    AX_U32    nHdrRatioMin;                                                           /* Accuracy: U7.10 Range: [0x400, 0x1FC00] */
    AX_U32    nHdrRatioMax;                                                           /* Accuracy: U7.10 Range: [0x400, 0x1FC00] */
    AX_U32    nBiasIn2D[AX_ISP_AINR_OFFSET_NUM];                                      /*Accuracy: U16.4 Range: [0,65535],Default:[0,0,0,0]*/
    AX_U32    nBiasIn3D[AX_ISP_AINR_OFFSET_NUM];                                      /*Accuracy: U16.4 Range: [0,65535],Default:[0,0,0,0]*/
} AX_ISP_AINR_META_PARAM_T;

typedef struct {
    AX_S32    nBiasIn[AX_ISP_AINR_OFFSET_NUM];                                        /*Accuracy: U16.4 Range: [-4096,4095],Default:[0,0,0,0]*/
}AX_ISP_AINR_BIAS_MANUAL_PARAM_T;

typedef struct {
    AX_U16    nTemporalFilterStrLut[AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM];           /*Temporal Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
    AX_U16    nVstTemporalFilterStrLut[AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM];        /*VstTemporal Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
}AX_ISP_AINR_NON_SENS_MANUAL_PARAM_T;

typedef struct {
    AX_U16    nSpatialNrStrLut[AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM];                 /*Spatial Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
}AX_ISP_AINR_SENS_MANUAL_PARAM_T;

typedef struct {
     AX_ISP_AINR_BIAS_MANUAL_PARAM_T        tBias;
    AX_ISP_AINR_NON_SENS_MANUAL_PARAM_T     tNonSens;
    AX_ISP_AINR_SENS_MANUAL_PARAM_T         tSens;
} AX_ISP_AINR_MANUAL_PARAM_T;

typedef struct {
    AX_U8     nRefGrpNum;                                                                   /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32    nRefValStart[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                  /* Gain start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32    nRefValEnd[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                    /* Gain end: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_S32    nBiasIn[AX_ISP_AINR_REF_VALUE_MAX_NUM][AX_ISP_AINR_OFFSET_NUM];               /* Accuracy: U16.4 Range: [-4096,4095],Default:[0,0,0,0]*/
}AX_ISP_AINR_BIAS_AUTO_PARAM_T;

typedef struct {
    AX_U8     nRefGrpNum;                                                                   /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32    nRefValStart[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                  /* Gain start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32    nRefValEnd[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                    /* Gain end: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16    nTemporalFilterStrLut[AX_ISP_AINR_REF_VALUE_MAX_NUM][AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM];    /*Temporal Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
    AX_U16    nVstTemporalFilterStrLut[AX_ISP_AINR_REF_VALUE_MAX_NUM][AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM]; /*VstTemporal Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
}AX_ISP_AINR_NON_SENS_AUTO_PARAM_T;

typedef struct {
    AX_U8     nRefGrpNum;                                                                   /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32    nRefValStart[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                  /* Gain start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32    nRefValEnd[AX_ISP_AINR_REF_VALUE_MAX_NUM];                                    /* Gain end: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16    nSpatialNrStrLut[AX_ISP_AINR_REF_VALUE_MAX_NUM][AX_ISP_AINR_LUT_ROW_NUM][AX_ISP_AINR_LUT_COL_NUM];          /*Spatial Strength Lut,Accuracy: U1.12 Range: [0,4096],Default:4096*/
}AX_ISP_AINR_SENS_AUTO_PARAM_T;

typedef struct {
    AX_ISP_AINR_BIAS_AUTO_PARAM_T           tBias;
    AX_ISP_AINR_NON_SENS_AUTO_PARAM_T       tNonSens;
    AX_ISP_AINR_SENS_AUTO_PARAM_T           tSens;
} AX_ISP_IQ_AINR_AUTO_PARAMS_T;

typedef struct {
    AX_ISP_AINR_META_PARAM_T            tMeta;
    AX_ISP_IQ_AINR_AUTO_PARAMS_T        tParams;
} AX_ISP_IQ_AINR_AUTO_TABLE_T;

typedef struct {
    AX_U8  nAutoModelNum;                                                             /*total number of models. Accuracy: U8.0 Range: [0, AX_ISP_NPU_MODEL_MAX_NUM] */
    AX_ISP_IQ_AINR_AUTO_TABLE_T     tAutoModelTable[AX_ISP_AINR_MODEL_MAX_NUM];
} AX_ISP_IQ_AINR_AUTO_T;

typedef struct  {
    AX_ISP_AINR_META_PARAM_T        tMeta;
    AX_ISP_AINR_MANUAL_PARAM_T      tParams;
} AX_ISP_IQ_AINR_MANUAL_T;

typedef struct {
    AX_U8 nEnable;                          /*Accuracy: U1.0 Range: [0, 1],default:1*/
    AX_U8 nAutoMode;                        /*for NR auto or manual adjust mode, 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8 bUpdateTable;                     /*for NR model table switch enable mode, 0: disable, 1:enable, default:0. Accuracy: U1 Range: [0, 1] */
    AX_U8 nRefMode;                         /*Accuracy: U1.0 Range: [0x0, 0x1], 0:use lux as ref, 1:use gain as ref*/
    AX_U8 nHdrMode;                         /*for NR model hdr mode get/set, 1: sdr, 2:hdr_2x, 3:hdr_3x, default:1. Accuracy: U2 Range: [1, 2, 3] */
    AX_ISP_IQ_AINR_MANUAL_T tManualParam;
    AX_ISP_IQ_AINR_AUTO_T   tAutoParam;
} AX_ISP_IQ_AINR_PARAM_T;

typedef struct {
    AX_U8 nMode;                  /*0: Relative NR level mode.Use the current parameter as the maximum level value. 1: Absolute NR level mode. Use the maximum NR parameter in the principle as the maximum level. Accuracy: U1 Range: [0, 1],default:0.*/
    AX_S8 nSpatialNrLevel;        /*for spatial NR level. default:0. Accuracy: S8 Range: [-127, 127] */
    AX_S8 nTemporalNrLevel;       /*for temporal NR level. default:0. Accuracy: S8 Range: [-127, 127] */
} AX_ISP_IQ_AINR_LEVEL_T;

/************************************************************************************
 *  AINR IQ Capability
************************************************************************************/
typedef struct {
    AX_U8   nTemporalBaseNrValidNum;
    AX_CHAR szTemporalBaseNrList[AX_ISP_AINR_MODEL_MAX_NUM][AX_ISP_MAX_PATH_SIZE];
    AX_U8   nSpatialBaseNrValidNum;
    AX_CHAR szSpatialBaseNrList[AX_ISP_AINR_MODEL_MAX_NUM][AX_ISP_MAX_PATH_SIZE];
} AX_ISP_IQ_AINR_BASE_NR_LIST;

typedef struct {
    AX_CHAR szModelName[AX_ISP_MAX_PATH_SIZE];  /* model name, only name */
    AX_BOOL bSuppBias;                          /* Accuracy: U1.0 Range: [0, 1] */
    AX_BOOL bSuppBiasIn3D;                      /* Accuracy: U1.0 Range: [0, 1] */
    AX_BOOL bSuppBiasIn2D;                      /* Accuracy: U1.0 Range: [0, 1] */
    AX_BOOL bSuppTemporalFilterStrLut;          /* Accuracy: U1.0 Range: [0, 1] */
    AX_BOOL bSuppVstTemporalFilterStrLut;       /* Accuracy: U1.0 Range: [0, 1] */
    AX_BOOL bSuppSpatialNrStrLut;                /* Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_AINR_BASE_NR_LIST  tNrList;
} AX_ISP_IQ_AINR_CAPABILITY_T;

 typedef struct {
    AX_U8                           nValidNum;
    AX_ISP_IQ_AINR_CAPABILITY_T     tModelCapList[AX_ISP_AINR_MODEL_MAX_NUM];
} AX_ISP_IQ_AINR_CAP_TABLE_T;

/************************************************************************************
 *  RAW2DNR IQ Param
 ************************************************************************************/
#define AX_ISP_RAW2DNR_CORING_LIMIT_NUM      (2)
#define AX_ISP_RAW2DNR_BAYER_CHN_NUM         (4)
#define AX_ISP_RAW2DNR_LUT_SIZE              (17)
#define AX_ISP_RAW2DNR_SETTING_NUM           (2)

typedef struct  {
    AX_U8  nMfEnable;                                                   /* mf_enable. Default: 0 Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8  nHfEnable;                                                   /* hf_enable. Default: 0 Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8  nLutType;                                                    /* lut_type. Default: 0 Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8  nMaskThreshold;                                              /* mask_coring_thre. Default: 0 Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_U8  nMaskLimit[AX_ISP_RAW2DNR_CORING_LIMIT_NUM];                 /* mask_coring_limit. Default: (0, 255) Accuracy: U0.8 Range: [0x0, 0xff] */
} AX_ISP_IQ_RAW2DNR_GLB_T;

typedef struct {
    AX_U16 nHighFreqNrFactor[AX_ISP_RAW2DNR_SETTING_NUM][AX_ISP_RAW2DNR_LUT_SIZE];  /* HighFreqNr_factor. Default: (512, 512, ..., 512) Accuracy: U4.8 Range: [0x0, 0xfff] */
    AX_U8  nLowFreqNrFactor[AX_ISP_RAW2DNR_SETTING_NUM][AX_ISP_RAW2DNR_LUT_SIZE];   /* LowFreqNr_factor. Default: (16, 16, ..., 16) Accuracy: U4.4 Range: [0x0, 0xff] */
    AX_U8  nHfNrStrength[AX_ISP_RAW2DNR_SETTING_NUM];                               /* org_blend[0]. Default: (128, 128) Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_U8  nMfNrStrength[AX_ISP_RAW2DNR_SETTING_NUM];                               /* org_blend[1]. Default: (128, 128) Accuracy: U0.8 Range: [0x0, 0xff] */
} AX_ISP_IQ_RAW2DNR_HDRRATIO_PARAM_T;

typedef struct {
    AX_U16 nHighFreqNrFactor[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM][AX_ISP_RAW2DNR_SETTING_NUM][AX_ISP_RAW2DNR_LUT_SIZE];  /* HighFreqNr_factor. Default: (512, 512, ..., 512) Accuracy: U4.8 Range: [0x0, 0xfff] */
    AX_U8  nLowFreqNrFactor[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM][AX_ISP_RAW2DNR_SETTING_NUM][AX_ISP_RAW2DNR_LUT_SIZE];   /* LowFreqNr_factor. Default: (16, 16, ..., 16) Accuracy: U4.4 Range: [0x0, 0xff] */
    AX_U8  nHfNrStrength[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM][AX_ISP_RAW2DNR_SETTING_NUM];                               /* org_blend[0]. Default: (128, 128) Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_U8  nMfNrStrength[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM][AX_ISP_RAW2DNR_SETTING_NUM];                               /* org_blend[1]. Default: (128, 128) Accuracy: U0.8 Range: [0x0, 0xff] */
} AX_ISP_IQ_RAW2DNR_HDRRATIO_AUTO_PARAM_T;

typedef struct  {
    AX_U8  nEdgePreserveRatio;                                                      /* freqsep_edge_preserve. Default: 128 Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_U8  nNoiseProfileFactor;                                                     /* bayer lut and y lut. Default: 128 Accuracy: U1.7 Range: [0x0, 0xff] */
    AX_U8  nInterChannelStrength;                                                   /* nlm_inter_g_strength. Default: 255 Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_ISP_IQ_RAW2DNR_HDRRATIO_PARAM_T tHrParam;
} AX_ISP_IQ_RAW2DNR_MANUAL_T;

typedef struct {
    AX_U8  nHrGrpNum;                                                       /* Accuracy: U8.0 Range: [1, AX_ISP_HDR_RATIO_GRP_NUM] */
    AX_U32 nHrRefVal[AX_ISP_HDR_RATIO_GRP_NUM];                             /* Hdr Ratio: Accuracy: U7.0 Range: [0x400, 0x1FC00] */
    AX_ISP_IQ_RAW2DNR_HDRRATIO_AUTO_PARAM_T tHrParam[AX_ISP_HDR_RATIO_GRP_NUM];
} AX_ISP_IQ_RAW2DNR_HR_AUTO_T;

typedef struct  {
    AX_U8  nParamGrpNum;                                                                                                    /* Accuracy: U8.0 Range: [0, AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM] */
    AX_U32 nRefVal[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM];                                                                 /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nEdgePreserveRatio[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM];                                                      /* freqsep_edge_preserve. Default: 128 Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_U8  nNoiseProfileFactor[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM];                                                     /* bayer lut and y lut. Default: 128 Accuracy: U1.7 Range: [0x0, 0xff] */
    AX_U8  nInterChannelStrength[AX_ISP_REF_AUTOTBL_RAW2DNR_EXPRESS_NUM];                                                   /* nlm_inter_g_strength. Default: 255 Accuracy: U0.8 Range: [0x0, 0xff] */
    AX_ISP_IQ_RAW2DNR_HR_AUTO_T tHrAuto;
} AX_ISP_IQ_RAW2DNR_AUTO_T;

typedef struct  {
    AX_U8 nRaw2dnrEn;                           /* RAW 2DNR enable */
    AX_U8 nAutoMode;                            /* for AE auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:0 */
    AX_U8 nRefMode;                             /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_IQ_RAW2DNR_GLB_T tGlb;               /* all non-auto params  */
    AX_ISP_IQ_RAW2DNR_MANUAL_T tManual;
    AX_ISP_IQ_RAW2DNR_AUTO_T tAuto;
} AX_ISP_IQ_RAW2DNR_PARAM_T;

/************************************************************************************
 *  LSC IQ Param
 ************************************************************************************/
#define AX_ISP_LSC_MESH_SIZE_V              (15)
#define AX_ISP_LSC_MESH_SIZE_H              (19)
#define AX_ISP_LSC_MESH_POINTS              (AX_ISP_LSC_MESH_SIZE_V * AX_ISP_LSC_MESH_SIZE_H)
#define AX_ISP_LSC_AUTO_COLOR_TEMP_MAX_NUM  (10)

typedef struct {
    AX_U8                        nLumaRatio;                                    /* Accuacy: U8 Range: [0, 100] */
    AX_U8                        nColorRatio;                                   /* Accuacy: U8 Range: [0, 100] */
    AX_U32                       nLumaMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];  /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nRRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];    /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nGRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];    /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nGBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];    /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nBBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];    /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
} AX_ISP_IQ_LSC_MANUAL_T;

typedef struct{
    AX_U8                        nParamGrpNum;                                         /* Luma Grp Num; Accuacy: U8 Range: [0, AX_ISP_REF_AUTOTBL_LSC_EXPRESS_NUM] */
    AX_U32                       nRefValStart[AX_ISP_REF_AUTOTBL_LSC_EXPRESS_NUM];     /* Ref Gain Start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux Start: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32                       nRefValEnd[AX_ISP_REF_AUTOTBL_LSC_EXPRESS_NUM];       /* Ref Gain End: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux End: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8                        nLumaRatio[AX_ISP_REF_AUTOTBL_LSC_EXPRESS_NUM];       /* Luma Ratio; Accuacy: U8 Range: [0, 100] */
    AX_U32                       nLumaMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H];    /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
} AX_ISP_IQ_LSC_LUMA_PARAM_T;

typedef struct{
    AX_U8                        nColTempNum;                                              /*Calib Color Temp Num; Accuracy: U8 Range: [0, AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM] */
    AX_U32                       nRefColorTempStart[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM];    /* Ref CCT Start; Accuracy: U32.0 Range: [0, 100000]*/
    AX_U32                       nRefColorTempEnd[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM];      /* Ref CCT End; Accuracy: U32.0 Range: [0, 100000]*/
    AX_U32                       nColorTemp[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM];            /* Calib CCT; Accuracy: U32.0 Range: [0, 100000] */
    AX_U8                        nColorRatio[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM];           /* Color Ratio; Accuacy: U8 Range: [0, 100] */
    AX_U32                       nRRMeshLut[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM][AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nGRMeshLut[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM][AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nGBMeshLut[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM][AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32                       nBBMeshLut[AX_ISP_CT_AUTOTBL_LSC_EXPRESS_NUM][AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
} AX_ISP_IQ_LSC_CT_PARAM_T;

typedef struct {
    AX_U8                        nDampRatio;        /* Damp Ratio; Accuacy: U8 Range: [0, 100] */
    AX_U8                        nToleranceRatio;   /* Tolerance Ratio; Accuacy: U8 Range: [0, 100] */
    AX_ISP_IQ_LSC_LUMA_PARAM_T   tLumaParam;        /* Luma Params */
    AX_ISP_IQ_LSC_CT_PARAM_T     tColTempParam;     /* Color Temp Params */
} AX_ISP_IQ_LSC_AUTO_T;

typedef struct {
    AX_U8                        nLscEn;            /* Acuracy: U8 Range: [0, 1] */
    AX_U8                        nRefMode;          /* choose ref mode, Accuracy: U8 Range: [0, 1], 0: use lux as ref, 1: use gain as ref */
    AX_U8                        nMeshMode;         /* mesh mode, Accuracy: U8 Range: [0, 1], 1: mirror mode, 0: normal mode */
    AX_U8                        nAutoMode;         /* for ref auto or manual adjust mode, Accuracy: U8 Range: [0, 1], 0: manual, 1:auto, default:1 */
    AX_U8                        nAlgMode;          /* Alg Performance mode, Range: [0, 2] 0: quality priority mode  1: balance mode 2: performance mode */
    AX_ISP_IQ_LSC_MANUAL_T       tManualParam;
    AX_ISP_IQ_LSC_AUTO_T         tAutoParam;
} AX_ISP_IQ_LSC_PARAM_T;

/************************************************************************************
 *  WB Gain Info
 ************************************************************************************/
typedef struct {
    AX_U8  nWbcEn;                                                      /* WBC Hardware Enable. Default data:1. Accuracy: U1 Range: [0, 1] */
    AX_U16 nRGain;                                                      /* WBC RGain. Default data:0x200. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nGrGain;                                                     /* WBC GrGain. Default data:0x100. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nGbGain;                                                     /* WBC GbGain. Default data:0x100. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nBGain;                                                      /* WBC BGain. Default data:0x200.Accuracy: U4.8 Range: [0, 0xFFF] */
} AX_ISP_IQ_WB_GAIN_PARAM_T;

/************************************************************************************
 *  RLTM IQ Param
 ************************************************************************************/
#define AX_ISP_RLTM_SCURVE_MAX_LEN              (1025)
#define AX_ISP_RLTM_HISTOGRAM_WEIGHT_MAX_LEN    (63)
#define AX_ISP_RLTM_HISTOGRAM_WEIGHT_NUM        (16)
#define AX_ISP_RLTM_HIST_REGION_NUM             (4)
#define AX_ISP_RLTM_LUMA_WEIGHT_NUM             (5)
#define AX_ISP_RLTM_SLOPE_STRENGTH_LUT_NUM      (33)

typedef struct {
    AX_U16 nAlpha;          /* Accuracy: U1.15 Range: [0, 32768] */
    AX_U8  nReset;          /* Accuracy: U8 Range: [0, 2], default 0 */
    AX_U8  nStopUpdating;   /* Accuracy: U1 Range: [0, 1] */
} AX_ISP_IQ_RLTM_TEMPO_FILTER_T;

typedef struct {
    AX_U8 nLumaWeight[AX_ISP_RLTM_LUMA_WEIGHT_NUM]; /* luma weight. (R, Gr, Gb, B, Max). Accuracy: U1.7 Range: [0, 128], default (0, 0, 0, 0, 128) */
} AX_ISP_IQ_RLTM_LUMAWT_T;

typedef struct {
    AX_U16 nHistogramWeight[AX_ISP_RLTM_HISTOGRAM_WEIGHT_MAX_LEN]; /* histogram bin weights. Accuracy: U16 Range: [0, 65535], default 1 */
} AX_ISP_IQ_RLTM_HISTWT_T;

typedef struct {
    AX_U16 nDetailSigmaDis;     /* Accuracy: U3.13 Range: [0, 65535], default 9000 */
    AX_U16 nDetailSigmaVal;     /* Accuracy: U3.13 Range: [0, 65535], default 1000 */
    AX_U8  nDetailExtraStrPos;  /* Accuracy: U3.5 Range: [0, 255], default 0 */
    AX_U8  nDetailExtraStrNeg;  /* Accuracy: U3.5 Range: [0, 255], default 0 */
    AX_U16 nDetailGainLimitPos; /* Accuracy: U3.13 Range: [0, 65535], default 57344 */
    AX_U16 nDetailGainLimitNeg; /* Accuracy: U3.13 Range: [0, 65535], default 57344 */
    AX_U16 nSlopeStrengthLut[AX_ISP_RLTM_SLOPE_STRENGTH_LUT_NUM]; /* Accuracy: U3.13 Range: [0, 65535], default [8192, 8192, ...] */
    AX_U16 nSlopeCoeff;         /* Accuracy: U8.8 Range: [0, 65535], default 256 */
} AX_ISP_IQ_RLTM_HF_ENH_T;

typedef struct {
    AX_U8  nRltmDetailLowEn;    /* rltm detail low en -- detail low control */
    AX_U16 nSigmaDisBlur;       /* Accuracy: U3.13 Range: [1, 65535], default 9000 */
    AX_U8  nDetailGainPosLow;   /* Accuracy: U3.5 Range: [0, 255], default 32 */
    AX_U8  nDetailGainNegLow;   /* Accuracy: U3.5 Range: [0, 255], default 32 */
    AX_U16 nDetailLimitPosLow;  /* Accuracy: U3.13 Range: [0, 65535], default 57344 */
    AX_U16 nDetailLimitNegLow;  /* Accuracy: U3.13 Range: [0, 65535], default 57344 */
    AX_U16 nSigmaDisPst;        /* Accuracy: U3.13 Range: [0, 65535], default 9000 */
    AX_U16 nSigmaValPst;        /* Accuracy: U3.13 Range: [0, 65535], default 1000 */
} AX_ISP_IQ_RLTM_LF_ENH_T;

typedef struct {
    AX_U16 nTop;        /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nBottom;     /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nLeft;       /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nRight;      /* Accuracy: U16 Range: [0, 8192] */
} AX_ISP_IQ_RLTM_ROI_T;

typedef struct {
    AX_U8 nMode;                /* rltm base&advance mode. Accuracy:U8 Range: [0, 1] */
    AX_U8 nRegionNum;           /* valide region number. Accuracy:U8 Range: [0, 4] */
    AX_U8 nHistWtNum;           /* hist weight number. Accuracy:U8 Range: [1, 16] */
    AX_ISP_IQ_RLTM_ROI_T    tRoi;
    AX_ISP_IQ_RLTM_HISTWT_T tHistWt;
    AX_U8 nFlagHistId[AX_ISP_RLTM_HISTOGRAM_WEIGHT_NUM][AX_ISP_RLTM_HISTOGRAM_WEIGHT_NUM]; /* Read Only. Accuracy: U8 Range: [0, 1] */
} AX_ISP_IQ_RLTM_MUL_HISTWT_T;

typedef struct {
    AX_U8  nDownSampleRatio;    /* Accuracy: U1.0 Range: [0, 1], default 1 */
    AX_U8  nCoeffWinRad;        /* Accuracy: U8.0 Range: [0, 5], default 2 */
    AX_U16 nCoeffEps;           /* Accuracy: U0.16 Range: [0, 65535], default 200 */
} AX_ISP_IQ_RLTM_LF_ENH_GLB_T;

typedef struct {
    AX_U8  nGtmSwEn;            /* gtm software switch. Accuracy: U1.0 Range: [0, 1], default 0 */
    AX_U16 nGtmSwDgain;         /* gtm dgain for software gtm curve. Accuracy: U8.8 Range: [256, 65535], default 256 */
    AX_U16 nWinSize;            /* for hist. Accuracy: U16.0 Range: [128, 256, 512, 1024], default 512 */
} AX_ISP_IQ_RLTM_COMMON_GLB_T;

typedef struct {
    AX_U8  nLocalFactor;        /* Accuracy: U1.7 Range: [0, 128], default 90 */
    AX_U8  nRltmStrength;       /* Accuracy: U1.7 Range: [0, 128], default 64 */
    AX_U8  nContrastStrength;   /* contrast strength. Accuracy: U1.7 Range: [0, 255], default 42 */
    AX_U8  nPostGamma;          /* for invgamma lut. Accuracy: U3.5 Range: [32, 255], default 64 */
    AX_U8  nHighlightSup;       /* highlight suppress. Accuracy: U5.3 Range: [0, 255], default 50 */
    AX_U16 nDetailCoringPos;    /* Accuracy: U3.13 Range: [0, 65535], default 40 */
    AX_U16 nDetailCoringNeg;    /* Accuracy: U3.13 Range: [0, 65535], default 40 */
    AX_U8  nDetailGainPos;      /* Accuracy: U3.5 Range: [0, 255], default 32 */
    AX_U8  nDetailGainNeg;      /* Accuracy: U3.5 Range: [0, 255], default 32 */
} AX_ISP_IQ_RLTM_HDRRATIO_PARAM_T;

typedef struct {
    AX_U8  nLocalFactor[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];        /* Accuracy: U1.7 Range: [0, 128], default 90 */
    AX_U8  nRltmStrength[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];       /* Accuracy: U1.7 Range: [0, 128], default 64 */
    AX_U8  nContrastStrength[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];   /* contrast strength. Accuracy: U1.7 Range: [0, 255], default 42 */
    AX_U8  nPostGamma[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];          /* for invgamma lut. Accuracy: U3.5 Range: [32, 255], default 64 */
    AX_U8  nHighlightSup[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];       /* highlight suppress. Accuracy: U5.3 Range: [0, 255], default 50 */
    AX_U16 nDetailCoringPos[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];    /* Accuracy: U3.13 Range: [0, 65535], default 40 */
    AX_U16 nDetailCoringNeg[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];    /* Accuracy: U3.13 Range: [0, 65535], default 40 */
    AX_U8  nDetailGainPos[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];      /* Accuracy: U3.5 Range: [0, 255], default 32 */
    AX_U8  nDetailGainNeg[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];      /* Accuracy: U3.5 Range: [0, 255], default 32 */
} AX_ISP_IQ_RLTM_HDRRATIO_AUTO_PARAM_T;

typedef struct {
    AX_U16 nKMax;               /* limit brightness. Accuracy: U12.4 Range: [0, 65535], default 65535 */
    AX_U8  nPreGamma;           /* for gamma lut. Accuracy: U3.5 Range: [32, 255], default 32 */
    AX_U16 nExtraDgain;         /* for invgamma lut. Accuracy: U4.4 Range: [16, 255], default 16 */
    AX_U8  nLog10Offset;        /* log10 offset. Accuracy: U3.5 Range: [0, 211], default 0 */
    AX_U16 nBaseGain;           /* base gain. Accuracy: U10.6 Range: [1, 65535], default 64 */
    AX_U8  nDitherMode;         /* 0: no-dither, 1: enable dither. Accuracy: U1 Range: [0, 1], default 0 */
    AX_U16 nDitherScale;        /* for dither strength. Accuracy: U10.6 Range: [0, 65535], default 64 */
    AX_U8  nHistWtBrightLow[AX_ISP_RLTM_HIST_REGION_NUM];   /* Hist weight Lower limit of brightness range. Actual nums deps nRegionNum. Accuracy: U8 Range: [0, 62] */
    AX_U8  nHistWtBrightHigh[AX_ISP_RLTM_HIST_REGION_NUM];  /* Hist weight Upper limit of brightness range. Actual nums deps nRegionNum. Accuracy: U8 Range: [0, 62] */
    AX_U8  nHistWtThreshold[AX_ISP_RLTM_HIST_REGION_NUM];   /* Hist weight threshod. Actual nums deps nRegionNum. Accuracy: U1.7 Range: [0, 129] */
    AX_U16 nSCurveList[AX_ISP_RLTM_SCURVE_MAX_LEN];         /* s curve lut. Accuracy: U1.15 Range: [0, 32768] */
    AX_ISP_IQ_RLTM_HISTWT_T tHistWt[AX_ISP_RLTM_HISTOGRAM_WEIGHT_NUM];
    AX_ISP_IQ_RLTM_HF_ENH_T tHighFreqEnh;   /* high frequnce enhance */
    AX_ISP_IQ_RLTM_LF_ENH_T tLowFreqEnh;    /* low frequnce enhance */
    AX_ISP_IQ_RLTM_HDRRATIO_PARAM_T tHrParam;
} AX_ISP_IQ_RLTM_MANUAL_T;

typedef struct {
    AX_U8  nHrGrpNum;                                                       /* Accuracy: U8.0 Range: [1, AX_ISP_HDR_RATIO_GRP_NUM] */
    AX_U32 nHrRefVal[AX_ISP_HDR_RATIO_GRP_NUM];                             /* Hdr Ratio: Accuracy: U7.0 Range: [0x400, 0x1FC00] */
    AX_ISP_IQ_RLTM_HDRRATIO_AUTO_PARAM_T tHrParam[AX_ISP_HDR_RATIO_GRP_NUM];
} AX_ISP_IQ_RLTM_HR_AUTO_T;

typedef struct {
    AX_U8  nParamGrpNum;                                            /* Accuracy: U8 Range: [0, AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM]  */
    AX_U32 nRefVal[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];            /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16 nKMax[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];              /* limit brightness. Accuracy: U12.4 Range: [0, 65535], default 65535 */
    AX_U8  nPreGamma[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];          /* for gamma lut. Accuracy: U3.5 Range: [32, 255], default 32 */
    AX_U16 nExtraDgain[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];        /* for invgamma lut. Accuracy: U4.4 Range: [16, 255], default 16 */
    AX_U8  nLog10Offset[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];       /* log10 offset. Accuracy: U3.5 Range: [0, 211], default 0 */
    AX_U16 nBaseGain[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];          /* base gain. Accuracy: U10.6 Range: [1, 65535], default 64 */
    AX_U8  nDitherMode[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];        /* 0: no-dither, 1: before pre-gamma, 2: after pre-gamma 0. Accuracy: U2.0 Range: [0, 2], default 0 */
    AX_U16 nDitherScale[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];       /* for dither strength. Accuracy: U10.6 Range: [0, 65535], default 64 */
    AX_U8  nHistWtBrightLow[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM][AX_ISP_RLTM_HIST_REGION_NUM];    /* Hist weight Lower limit of brightness range. Actual nums deps nRegionNum. Accuracy: U8 Range: [0, 62] */
    AX_U8  nHistWtBrightHigh[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM][AX_ISP_RLTM_HIST_REGION_NUM];   /* Hist weight Upper limit of brightness range. Actual nums deps nRegionNum. Accuracy: U8 Range: [0, 62] */
    AX_U8  nHistWtThreshold[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM][AX_ISP_RLTM_HIST_REGION_NUM];    /* Hist weight threshod. Actual nums deps nRegionNum. Accuracy: U1.7 Range: [0, 129] */
    AX_U16 nSCurveList[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM][AX_ISP_RLTM_SCURVE_MAX_LEN];          /* s curve lut. Accuracy: U1.15 Range: [0, 32768] */
    AX_ISP_IQ_RLTM_HISTWT_T tHistWt[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM][AX_ISP_RLTM_HISTOGRAM_WEIGHT_NUM];
    AX_ISP_IQ_RLTM_HF_ENH_T tHighFreqEnh[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];
    AX_ISP_IQ_RLTM_LF_ENH_T tLowFreqEnh[AX_ISP_REF_AUTOTBL_RLTM_EXPRESS_NUM];
    AX_ISP_IQ_RLTM_HR_AUTO_T tHrAuto;
} AX_ISP_IQ_RLTM_AUTO_T;

typedef struct {
    AX_U8  nRltmEn;             /* rltm en -- module control */
    AX_U8  nMultiCamSyncMode;   /* 0：INDEPEND MODE; 1： MASTER SLAVE MODE; 2: OVERLAP MODE */
    AX_U8  nAutoMode;           /* for ref auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;            /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_U8  nAlgMode;            /* RLTM ALG Run Mode Sel. Range: [0, 2] 0: effect priority mode  1: balance mode 2: performance mode */
    AX_ISP_IQ_RLTM_COMMON_GLB_T tCommonGlb;
    AX_ISP_IQ_RLTM_TEMPO_FILTER_T tTempoFilter;
    AX_ISP_IQ_RLTM_LUMAWT_T     tLumaWt;
    AX_ISP_IQ_RLTM_MUL_HISTWT_T tMultiHistWt;
    AX_ISP_IQ_RLTM_LF_ENH_GLB_T tLfEnhGlb;
    AX_ISP_IQ_RLTM_MANUAL_T     tManualParam;
    AX_ISP_IQ_RLTM_AUTO_T       tAutoParam;
} AX_ISP_IQ_RLTM_PARAM_T;

typedef struct {
    AX_U8  nShadowPoint;        /* Accuracy U0.8 Range: [1, 255] */
    AX_U16 nShadowStrength;     /* Accuracy U3.7 Range: [1, 512] */
    AX_U8  nHighlightPoint;     /* Accuracy U0.8 Range: [1, 255] */
    AX_U16 nHighlightStrength;  /* Accuracy U3.7 Range: [1, 512] */
} AX_ISP_IQ_SCURVE_PARAM_T;

/************************************************************************************
 *  Demosaic IQ Param
 ************************************************************************************/
#define AX_ISP_DEMOSAIC_AUTO_MAX_NUM        (16)
#define AX_ISP_DEMOSAIC_EDES_LUT_SIZE       (9)

typedef struct {
    AX_U8   nEdgeDirectEstStrength;         /* Accuracy:U2.6 Range: [0x0,0xFF] Default: 0x1 */
} AX_ISP_IQ_DEMOSAIC_MANUAL_T;

typedef struct {
    AX_U8                       nParamGrpNum;                                           /* Accuracy: U8.0 Range: [0x1, 0x10] Default: 0x10 */
    AX_U32                      nRefVal[AX_ISP_DEMOSAIC_AUTO_MAX_NUM];                  /* Accuracy: U22.10 GainRange: [0x400, 0xFFFFFFFF] LuxRange: [0x0, 0xFFFFFFFF] */
    AX_U8                       nEdgeDirectEstStrength[AX_ISP_DEMOSAIC_AUTO_MAX_NUM];   /* Accuracy: U2.6 Range: [0x0,0xFF] Default: all 0x1 */
} AX_ISP_IQ_DEMOSAIC_AUTO_T;

typedef struct {
    AX_U8                           nDemosaicEn;                                                /* Accuracy: U1.0 Range:[0x0, 0x1] Default: 0x1 */
    AX_U8                           nMode;                                                      /* Accuracy: U2.0 Range:[0x0, 0x3] Default: 0x0 */
    AX_U8                           nFreqSensitivity;                                           /* Accuracy: U1.4 Range:[0x0, 0x10] Default: 0xC */
    AX_U16                          nEdgeDirectEstStrengthLut[AX_ISP_DEMOSAIC_EDES_LUT_SIZE];   /* Accuracy: U4.8 Range:[0x0, 0xFFF] */
    AX_U8                           nAutoMode;                                                  /* Accuracy: U1.0 Range: [0x0, 0x1] Default: 0x0 */
    AX_U8                           nRefMode;                                                   /* Accuracy: U1.0 Range: [0x0, 0x1] Default: 0x0 */
    AX_ISP_IQ_DEMOSAIC_MANUAL_T     tManualParam;
    AX_ISP_IQ_DEMOSAIC_AUTO_T       tAutoParam;
} AX_ISP_IQ_DEMOSAIC_PARAM_T;

/************************************************************************************
 *  FCC IQ Param
 ************************************************************************************/
#define AX_ISP_FCC_SAT_LUT_SIZE         (9)
#define AX_ISP_FCC_AUTO_MAX_NUM         (16)

typedef struct {
    AX_U8       nFcCorStrength;                              /* Accuracy: U4.4 Range: [0x0, 0xFF] Default: 0x10 */
    AX_U16      nFcCorSatLut[AX_ISP_FCC_SAT_LUT_SIZE];       /* Accuracy: U8.4 Range: [0x, 0xFFF] */
    AX_U8       nFcCorSatLevel0;                             /* Accuracy: U2.2 Range: [0x0, 0xF] Default: 0xB */
    AX_U8       nFcCorSatLevel1;                             /* Accuracy: U4.4 Range: [0x0, 0xFF] Default: 0x10 */
} AX_ISP_IQ_FCC_MANUAL_T;

typedef struct {
    AX_U8                       nParamGrpNum;                                                   /* Accuracy: U8.0 Range: [0x1, 0x10] Default: 0x10 */
    AX_U32                      nRefVal[AX_ISP_FCC_AUTO_MAX_NUM];                               /* Accuracy: U22.10 GainRange: [0x400, 0xFFFFFFFF] LuxRange: [0x0, 0xFFFFFFFF]*/
    AX_U8                       nFcCorStrength[AX_ISP_FCC_AUTO_MAX_NUM];                        /* Accuracy: U4.4 Range: [0x0, 0xFF] Default: all 0x10 */
    AX_U16                      nFcCorSatLut[AX_ISP_FCC_AUTO_MAX_NUM][AX_ISP_FCC_SAT_LUT_SIZE]; /* Accuracy: U8.4 Range: [0x, 0xFFF] */
    AX_U8                       nFcCorSatLevel0[AX_ISP_FCC_AUTO_MAX_NUM];                       /* Accuracy: U2.2 Range: [0x0, 0xF] Default: 0xB */
    AX_U8                       nFcCorSatLevel1[AX_ISP_FCC_AUTO_MAX_NUM];                       /* Accuracy: U4.4 Range: [0x0, 0xFF] Default: 0x10 */
} AX_ISP_IQ_FCC_AUTO_T;

typedef struct {
    AX_U8                           nFcCorEnable;  /* Accuracy: U1.0 Range:[0x0, 0x1] Default: 0x1 */
    AX_U16                          nFcCorLimit;   /* Accuracy: U8.4 Range:[0x0, 0xFFF] Default: 0x200 */
    AX_U8                           nAutoMode;     /* Accuracy: U1.0 Range:[0x0, 0x1] Default: 0x0 */
    AX_U8                           nRefMode;      /* Accuracy: U1.0 Range:[0x0, 0x1] Default: 0x0 */
    AX_ISP_IQ_FCC_MANUAL_T          tManualParam;
    AX_ISP_IQ_FCC_AUTO_T            tAutoParam;
} AX_ISP_IQ_FCC_PARAM_T;

/************************************************************************************
 *  GIC IQ Param
 ************************************************************************************/
#define AX_ISP_GIC_STRENGTH_LUT_SIZE    (9)
#define AX_ISP_GIC_AUTO_MAX_NUM         (16)

typedef struct {
    AX_U8       nGicStrengthLut[AX_ISP_GIC_STRENGTH_LUT_SIZE];  /* Accuracy: U1.7 Range: [0x0, 0x80] Default: all 0x80 */
} AX_ISP_IQ_GIC_MANUAL_T;

typedef struct {
    AX_U8                    nParamGrpNum;                                                              /* Accuracy: U8.0 Range: [0x1, 0x10] Default: 0x10 */
    AX_U32                   nRefVal[AX_ISP_GIC_AUTO_MAX_NUM];                                          /* Accuracy: U22.10 GainRange: [0x400, 0xFFFFFFFF] LuxRange: [0x0, 0xFFFFFFFF] */
    AX_U8                    nGicStrengthLut[AX_ISP_GIC_AUTO_MAX_NUM][AX_ISP_GIC_STRENGTH_LUT_SIZE];    /* Accuracy: U1.7 Range: [0x0, 0x80] Default: all 0x80 */
} AX_ISP_IQ_GIC_AUTO_T;

typedef struct {
    AX_U8                           nGicEnable;     /* Accuracy: U1.0 Range: [0x0, 0x1] Default: 0x0 */
    AX_U8                           nAutoMode;      /* Accuracy: U1.0 Range: [0x0, 0x1] Default: 0x0 */
    AX_U8                           nRefMode;       /* Accuracy: U1.0 Range: [0x0, 0x1] Default: 0x0 */
    AX_ISP_IQ_GIC_MANUAL_T          tManualParam;
    AX_ISP_IQ_GIC_AUTO_T            tAutoParam;
} AX_ISP_IQ_GIC_PARAM_T;

/************************************************************************************
 *  DEPURPLE IQ Param
 ************************************************************************************/
#define AX_ISP_DEPURPLE_DET_EDGE_WEIGHT_SIZE (7)
#define AX_ISP_DEPURPLE_CMP_LUMA_LUT_SIZE (8)
#define AX_ISP_DEPURPLE_CMP_HUE_LUT_SIZE (16)
#define AX_ISP_DEPURPLE_CMP_SAT_LUT_SIZE (6)
#define AX_ISP_DEPURPLE_DET_MASK_WEIGHT_PREV_SIZE (11)

typedef struct {
    AX_U8 nDetMode;        /* Accuracy: U1 Range: [0x0, 0x1] */
    AX_U8 nDetEdgeSlopeY;  /* Accuracy: U1.6 Range: [0, 0x7F] */
    AX_S8 nDetEdgeOffsetY; /* Accuracy: S1.6 Range: [-128, 0] */
    AX_U8 nDetEdgeEnableC;  /* Accuracy: U1.0 Range: [0, 0x1] */
    AX_U8 nDetEdgeSlopeC;  /* Accuracy: U1.6 Range: [0, 0x7F] */
    AX_S8 nDetEdgeOffsetC; /* Accuracy: S1.6 Range: [-128, 0] */
    AX_U16 nDetSeledgeThrY; /* Accuracy: U8.6 Range: [0, 0x3FFF] */
    AX_U8 nDetSeledgeSlopeY; /* Accuracy: U1.6 Range: [0, 0x7F] */
    AX_U8 nDetSeledgeWeight[AX_ISP_DEPURPLE_DET_EDGE_WEIGHT_SIZE]; /* Accuracy: U1.6 Range: [0, 0x7F] */
    AX_U16 nDetMaskStrength;                                   /* Accuracy: U3.6 Range: [0x0, 0x1FF] */
    AX_U8 nDetMaskStrengthPre;                                   /* Accuracy: U1.6 Range: [0x0, 0x7F] */
    AX_U8 nDetMaskWeightPre[AX_ISP_DEPURPLE_DET_MASK_WEIGHT_PREV_SIZE]; /* Accuracy: U1.6 Range: [0x0, 0x7F] */
} AX_ISP_IQ_DEPURPLE_DET_PARAM_T;

typedef struct {
    AX_U8 nDebugMaskHighlightEnable;                                       /* Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8 nDebugMaskHighlightThr;                                       /* Accuracy: U1.4 Range: [0x0, 0x10] */
} AX_ISP_IQ_DEPURPLE_CMP_PARAM_T;

typedef struct {
    AX_U8  nParamGrpNum;                       /* Accuracy: U8.0 Range: [0x0, 0xF]  */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM]; /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nCompStrength[AX_ISP_AUTO_TABLE_MAX_NUM]; /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8  nCompTargetLuma[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DEPURPLE_CMP_LUMA_LUT_SIZE]; /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8  nCompTargetHue[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DEPURPLE_CMP_HUE_LUT_SIZE];   /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8  nCompTargetSat[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DEPURPLE_CMP_SAT_LUT_SIZE];   /* Accuracy: U1.7 Range: [0x0, 0x80] */
} AX_ISP_IQ_DEPURPLE_AUTO_PARAM_T;

typedef struct {
    AX_U8 nCompStrength; /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8 nCompTargetLuma[AX_ISP_DEPURPLE_CMP_LUMA_LUT_SIZE]; /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8 nCompTargetHue[AX_ISP_DEPURPLE_CMP_HUE_LUT_SIZE];   /* Accuracy: U1.7 Range: [0x0, 0x80] */
    AX_U8 nCompTargetSat[AX_ISP_DEPURPLE_CMP_SAT_LUT_SIZE];   /* Accuracy: U1.7 Range: [0x0, 0x80] */
} AX_ISP_IQ_DEPURPLE_MANUAL_PARAM_T;

typedef struct {
    AX_U8 nEnable;       /* Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8 nAutoMode;     /* Accuracy: U1.0 Range: [0x0, 0x1] */
    AX_U8 nRefMode;      /* Accuracy: U4.0 Range: [0x0, 0x1] */
    AX_U8 nDepurpleMode; /* Accuracy: U4.0 Range: [0x0, 0x2] */
    AX_ISP_IQ_DEPURPLE_DET_PARAM_T tDetParam;
    AX_ISP_IQ_DEPURPLE_CMP_PARAM_T tCmpParam;
    AX_ISP_IQ_DEPURPLE_MANUAL_PARAM_T tManualParam;
    AX_ISP_IQ_DEPURPLE_AUTO_PARAM_T tAutoParam;
} AX_ISP_IQ_DEPURPLE_PARAM_T;

/************************************************************************************
 *  CC IQ Param
 ************************************************************************************/
#define AX_ISP_CC_LUMA_RATIO_SIZE          (2)
#define AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM  (12)
#define AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM    (5)
#define AX_ISP_CC_SPC_MAX_NUM              (12)
#define AX_ISP_CC_ANGLE_SIZE               (16)
#define AX_ISP_CC_CCM_V_SIZE               (3)
#define AX_ISP_CC_CCM_H_SIZE               (2)
#define AX_ISP_CC_CCM_SIZE                 (AX_ISP_CC_CCM_V_SIZE * AX_ISP_CC_CCM_H_SIZE)

typedef struct {
    AX_U16                 nCcmCtrlLevel;                           /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nCcmSat;                                 /* Accuracy: S1.6 Range: [-64, 64] */
    AX_S16                 nCcmHue;                                 /* Accuracy: S5.6 Range: [-1920, 1920] */
    AX_S16                 nCcmMatrix[AX_ISP_CC_CCM_SIZE];         /* Accuracy: S3.8 Range: [-2047, 2047] */
    AX_U16                 nXcmCtrlLevel[AX_ISP_CC_ANGLE_SIZE];    /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nXcmSat[AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S1.6 Range: [-32, 32] */
    AX_S16                 nXcmHue[AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S5.6 Range: [-640, 640] */
} AX_ISP_IQ_CC_MANUAL_T;

typedef struct {
    AX_U8                  nLightSource;        /* index for which light source is selected, Accuracy: U4.0 Range: [0, AX_ISP_CC_SPC_MAX_NUM] */
    AX_U8                  nLightSourceNum;     /* Accuracy: U8.0 Range: [0, AX_ISP_CC_SPC_MAX_NUM] */
    AX_U16                 nCcmCtrlLevel[AX_ISP_CC_SPC_MAX_NUM];                          /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nCcmSat[AX_ISP_CC_SPC_MAX_NUM];                                /* Accuracy: S1.6 Range: [-64, 64] */
    AX_S16                 nCcmHue[AX_ISP_CC_SPC_MAX_NUM];                                /* Accuracy: S5.6 Range: [-1920, 1920] */
    AX_S16                 nCcmMatrix[AX_ISP_CC_SPC_MAX_NUM][AX_ISP_CC_CCM_SIZE];         /* Accuracy: S3.8 Range: [-2047, 2047] */
    AX_U16                 nXcmCtrlLevel[AX_ISP_CC_SPC_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];    /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nXcmSat[AX_ISP_CC_SPC_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S1.6 Range: [-32, 32] */
    AX_S16                 nXcmHue[AX_ISP_CC_SPC_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S5.6 Range: [-640, 640] */
} AX_ISP_IQ_CC_LIGHT_SOURCE_AUTO_T;

typedef struct {
    AX_U8                  nCtNum;                /* color temp ref num,  Accuracy: U8.0 Range: [0, AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM] */
    AX_U32                 nRefValCt[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM];                         /* color temp, Accuracy: U32.0 Range: [0, 100000] */
    AX_U8                  nLuxGainNum;           /* lux/gain ref num,  Accuracy: U8.0 Range: [0, AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM] */
    AX_U32                 nRefValLuxGain[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM];/* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16                 nCcmCtrlLevel[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM];                          /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nCcmSat[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM];                                /* Accuracy: S1.6 Range: [-64, 64] */
    AX_S16                 nCcmHue[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM];                                /* Accuracy: S5.6 Range: [-1920, 1920] */
    AX_S16                 nCcmMatrix[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM][AX_ISP_CC_CCM_SIZE];         /* Accuracy: S3.8 Range: [-2047, 2047] */
    AX_U16                 nXcmCtrlLevel[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];    /* Accuracy: U1.8 Range: [0, 256] */
    AX_S8                  nXcmSat[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S1.6 Range: [-32, 32] */
    AX_S16                 nXcmHue[AX_ISP_CC_AUTO_COLOR_TEMP_MAX_NUM][AX_ISP_CC_AUTO_LUX_GAIN_MAX_NUM][AX_ISP_CC_ANGLE_SIZE];          /* Accuracy: S5.6 Range: [-640, 640] */
} AX_ISP_IQ_CC_COLOR_TEMP_AUTO_T;

typedef struct {
    AX_U16                  nLightSourceRatio;   /* Accuracy: U1.8 Range:[0, 256], 0: use tAutoMode0, 256: use tAutoMode1,  1-256:blending with tAutoMode0 and tAutoMode1*/
    AX_ISP_IQ_CC_COLOR_TEMP_AUTO_T   tColorTempAuto;      /* interpolation by color temp and lux/gain */
    AX_ISP_IQ_CC_LIGHT_SOURCE_AUTO_T tLightSourceAuto;    /* special light source param */
} AX_ISP_IQ_CC_AUTO_T;

typedef struct {
    AX_U8                  nCcEn;               /* cc enable, Accuracy: U1.0 Range: [0, 1], 0: Disable, 1: Enable */
    AX_U8                  nCcMode;             /* color control mode, Accuracy: U1.0 Range: [0, 1], 0: basic mode 1: advanced mode */
    AX_U8                  nAutoMode;           /* for ref auto or manual adjust mode, Accuracy: U1.0 Range:[0, 1], 0: manual, 1:auto, default:1 */
    AX_U8                  nRefMode;            /* choose ref mode, Accuracy: U1.0 Range: [0, 1], 0:use color temp and lux as ref, 1:use color temp and gain as ref */
    AX_U8                  nLumaRatio[AX_ISP_CC_LUMA_RATIO_SIZE];    /* Accuracy: U1.7  Range: [0, 128] */
    AX_ISP_IQ_CC_MANUAL_T  tManualParam;
    AX_ISP_IQ_CC_AUTO_T    tAutoParam;
} AX_ISP_IQ_CC_PARAM_T;

/************************************************************************************
 *  Gamma IQ Param
 ************************************************************************************/
#define AX_ISP_GAMMA_LUT_SIZE               (129)
#define DEF_ISP_GAMMA_CURVE_MAX_NUM          (3)
typedef enum {
    AX_ISP_GAM_USER_GAMMA = 0,
    AX_ISP_GAM_PRESET_GAMMA,
} AX_ISP_GAM_MODE_E;

typedef enum {
    AX_ISP_GAM_LINEAR           = 0,
    AX_ISP_GAM_BT709            = 1,
    AX_ISP_GAM_SRGB             = 2,
    AX_ISP_GAM_AX_GAM0          = 10,
    AX_ISP_GAM_AX_GAM1          = 11,
    AX_ISP_GAM_AX_GAM2          = 12,
    AX_ISP_GAM_MODE_CUSTOMER    = 255,
} AX_ISP_GAM_PRESET_GAMMA_TYPE_E;

typedef enum {
    AX_ISP_GAM_LUT_LINEAR = 0,
    AX_ISP_GAM_EXPONENTIAL,
} AX_ISP_LUT_MODE_E;

typedef struct {
    AX_U16                          nLut[AX_ISP_GAMMA_LUT_SIZE];               /* Accuracy: U8.4 Range: [0, 0xFFF] */
} AX_ISP_IQ_GAMMA_LUT_T;

typedef struct {
    AX_ISP_LUT_MODE_E                   eLutMode;                                   /* Interpolation method of LUT table, 0: linear, 1: exponential, default:1. Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_ISP_IQ_GAMMA_LUT_T           tGammaLut;                                        /* Gamma lut */
} AX_ISP_IQ_GAMMA_LUT_USER_MANUAL_T;

typedef struct {
    AX_ISP_LUT_MODE_E                   eLutMode;                                   /* Interpolation method of LUT table, 0: linear, 1: exponential, default:1. Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_U32                              nRefValStart[DEF_ISP_GAMMA_CURVE_MAX_NUM];    /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32                              nRefValEnd[DEF_ISP_GAMMA_CURVE_MAX_NUM];      /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_ISP_IQ_GAMMA_LUT_T               tGammaLut[DEF_ISP_GAMMA_CURVE_MAX_NUM];       /* Gamma lut */
}AX_ISP_IQ_GAMMA_LUT_USER_AUTO_T;

typedef struct {
    AX_U8                               nGammaEn;                                   /* Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_U8                               nAutoMode;                                  /* Accuracy: U1.0 Range: [0, 1], default: 0 */
    AX_U8                               nRefMode;                                   /* Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_ISP_GAM_MODE_E                   eGammaMode;                                 /* Choice of custom gamma and preset gamma. Accuracy: U1.0 Range: [0, 1], default: 1 */
    AX_ISP_GAM_PRESET_GAMMA_TYPE_E      ePresetGammaType;                           /* Optional type of preset gamma. Accuracy: U8.0 Range: [0, 255], default: 10 */
    AX_ISP_IQ_GAMMA_LUT_USER_MANUAL_T    tManualParam;
    AX_ISP_IQ_GAMMA_LUT_USER_AUTO_T      tAutoParam;
} AX_ISP_IQ_GAMMA_PARAM_T;

/************************************************************************************
 *  Dehaze IQ Param
 ************************************************************************************/
typedef struct {
    AX_U8  nCalcMode;                             /* calc_mode, default 1. Accuracy: U1.0 Range: [0, 1] */
    AX_U16 nAirReflect;                           /* air_reflect, default 0xcc00. Accuracy: U8.8 Range: [0, 0xff00] */
    AX_U8  nSpatialSmoothness;                    /* spatial_smoothness, default 2. Accuracy: U4.0 Range: [0, 3] */
    AX_U16 nStrengthLimit;                        /* strength_limit, default 26. Accuracy: U1.8 Range: [1, 256] */
    AX_U8  nMeshSize;                             /* mesh_size, default 32. Accuracy: U8.0 Range: [16, 128] */
    AX_U16 nEps;                                  /* eps, default 8192. Accuracy: U4.12 Range: [1, 65535] */
    AX_U8  nBlurEnable;                           /* blur_enable, default 1. Accuracy: U1.0 Range: [0, 1] */
    AX_U16 nSigmaBlur;                            /* sigma_blur, default 4096. Accuracy: U3.13 Range: [1, 65535] */
} AX_ISP_IQ_DEHAZE_GLB_T;

typedef struct {
    AX_U8  nEffectStrength;                                     /* manual effect_strength, default 102. Accuracy: U1.7 Range: [0, 128] */
    AX_U8  nGrayDcRatio;                                        /* global dark channel strength, default 64. Accuracy: U1.7 Range: [0, 128] */
} AX_ISP_IQ_DEHAZE_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                                                /* Accuracy: U8.0 Range: [0, AX_ISP_REF_AUTOTBL_DEHAZE_EXPRESS_NUM] */
    AX_U32 nRefVal[AX_ISP_REF_AUTOTBL_DEHAZE_EXPRESS_NUM];              /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nEffectStrength[AX_ISP_REF_AUTOTBL_DEHAZE_EXPRESS_NUM];      /* manual effect_strength, default 102. Accuracy: U1.7 Range: [0, 128] */
    AX_U8  nGrayDcRatio[AX_ISP_REF_AUTOTBL_DEHAZE_EXPRESS_NUM];         /* global dark channel strength, default 64. Accuracy: U1.7 Range: [0, 128] */
} AX_ISP_IQ_DEHAZE_AUTO_T;

typedef struct {
    AX_U8  nDehazeEn;   /* dehaze enable */
    AX_U8  nAutoMode;   /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;    /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_IQ_DEHAZE_GLB_T          tGlbParam;
    AX_ISP_IQ_DEHAZE_MANUAL_T       tManualParam;
    AX_ISP_IQ_DEHAZE_AUTO_T         tAutoParam;
} AX_ISP_IQ_DEHAZE_PARAM_T;

/************************************************************************************
 *  CSC IQ Param
 ************************************************************************************/
#define AX_ISP_YUV_CSC_MATRIX_SIZE           (3)

typedef enum {
    AX_ISP_CSC_USER = 0,
    AX_ISP_CSC_BT601,
    AX_ISP_CSC_BT709,
    AX_ISP_CSC_BT2020,
    AX_ISP_CSC_BUTT,
} AX_ISP_CSC_COlOR_SPACE_MODE_E;

typedef enum {
    AX_ISP_NV12_SEL = 0,
    AX_ISP_NV21_SEL = 1,
} AX_ISP_UV_SEQ_SEL_E;

typedef struct  {
    AX_S16 nMatrix[AX_ISP_YUV_CSC_MATRIX_SIZE][AX_ISP_YUV_CSC_MATRIX_SIZE]; /* color matrix. Accuracy: S2.8 Range: [-1024, 1023] */
    AX_ISP_CSC_COlOR_SPACE_MODE_E  eColorSpaceMode; /* color space select. 0:rgb2yuv_matrix(customized), 1:BT601, 2:BT709, 3:BT2020(1~3 is nMatrix, can not be modified). Accuracy: U4.0 Range: [0, 3] */
    AX_ISP_UV_SEQ_SEL_E  eUvSeqSel;                 /* U/V sequence select. Accuracy: U1.0 Range: [0, 1] */
} AX_ISP_IQ_CSC_PARAM_T;

/************************************************************************************
 *  CA IQ Param
 ************************************************************************************/
#define AX_ISP_CA_COLOR_TEMP_AUTO_CT_MAX_NUM         (12)
#define AX_ISP_CA_COLOR_TEMP_AUTO_LG_MAX_NUM         (5)
#define AX_ISP_CA_CMTX_NUM                           (2)
#define AX_ISP_CA_CMTX_ARRAY_NUM       (AX_ISP_CA_CMTX_NUM * AX_ISP_CA_CMTX_NUM)
typedef struct {
    AX_U16                  nCtrlLevel;                                                             /* Accuracy: U1.8 Range: [0, 256] Default: 256 */
    AX_S16                  nSat;                                                                   /* Accuracy: S6.4 Range: [-800, 800] Default: 0 */
    AX_S16                  nHue;                                                                   /* Accuracy: S6.4 Range: [-480, 480] Default: 0 */
    AX_S16                  nCmtx[AX_ISP_CA_CMTX_NUM][AX_ISP_CA_CMTX_NUM];                          /* Accuracy: S2.8 Range: [-1024, 1023] */
} AX_ISP_IQ_CA_MANUAL_T;

typedef struct {
    AX_U8                   nParamGrpNumCt;                                                         /* Accuracy: U8.0 Range: [1, 12] */
    AX_U8                   nParamGrpNumLG;                                                         /* Accuracy: U8.0 Range: [1, 5] */
    AX_U32                  nRefValCt[AX_ISP_CA_COLOR_TEMP_AUTO_CT_MAX_NUM];                        /* Accuracy: U32.0 Range: [0x0, 0xFFFFFFFF] */
    AX_U32                  nRefValLG[AX_ISP_CA_COLOR_TEMP_AUTO_CT_MAX_NUM][AX_ISP_CA_COLOR_TEMP_AUTO_LG_MAX_NUM];                           /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF] */
    AX_S16                  nCmtx[AX_ISP_CA_COLOR_TEMP_AUTO_CT_MAX_NUM][AX_ISP_CA_COLOR_TEMP_AUTO_LG_MAX_NUM][AX_ISP_CA_CMTX_ARRAY_NUM];     /* Accuracy: S2.8 Range: [-1024, 1023] */
} AX_ISP_IQ_CA_AUTO_T;

typedef struct {
    AX_U8                   nCppEn;                                                                 /* Accuracy: U1.0 Range: [0, 1] */
    AX_U8                   nAutoMode;                                                              /* Accuracy: U1.0 Range: [0, 1] */
    AX_U8                   nRefMode;                                                               /* Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_CA_MANUAL_T   tManualParam;
    AX_ISP_IQ_CA_AUTO_T     tAutoParam;
} AX_ISP_IQ_CA_PARAM_T;

/************************************************************************************
 *  CLP IQ Param
 ************************************************************************************/
#define AX_ISP_CLP_LUT_SIZE     (256)
#define AX_ISP_CLP_LUT_NUM      (3)

typedef struct {
    AX_U8 nClpEn;                                                       /* enable clp. Accuracy: U1. Range: [0, 1] */
    AX_U8 nColorPaletteId;                                              /* pre-defined colormap id. Accuracy: U3. Range: [0, 7] */
    AX_U8 nColorPaletteYuvLut[AX_ISP_CLP_LUT_NUM][AX_ISP_CLP_LUT_SIZE]; /* customized colormap's yuv lut. Accuracy: U8. Range: [0, 255] */
} AX_ISP_IQ_CLP_PARAM_T;

/************************************************************************************
 *  YUV-3DNR IQ Param
 ************************************************************************************/
#define YUV3DNR_UPDATE_LUT_SIZE                (16)
#define YUV3DNR_MOTION_DET_LUT_SIZE            (17)
#define YUV3DNR_BLEND_REGS_NUM                 (2)
#define YUV3DNR_MOTION_LUT_SIZE                (4)
#define YUV3DNR_AUTO_SF_LUT_SIZE               (9)
#define YUV3DNR_AUTO_LUMA_BLEND_RATIO_SIZE     (3)

typedef struct {
    AX_U8 nExtMaskEnable;                               /* external mask enable,default:0,Accuracy: U1 Range: [0, 1] */
    AX_U8 nExtMaskMode;                                 /* external mask mode: 0: AI 3DNR Mask, 1:HDR Mask, default:0, Accuracy: U1 Range: [0, 1] */
} AX_ISP_IQ_EXTMASK_PARAM_T;

typedef struct {
    AX_U8 nMotionDetEnable;     /* motion detect enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nTfEnable;            /* tf enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nSf0YnrEnable;        /* sf0ynr enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nSf1YnrEnable;        /* sf1ynr enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nSf2YnrEnable;        /* sf2ynr enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nSf0CnrEnable;        /* sf0cnr enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nSf1CnrEnable;        /* sf1cnr enable,default:1,Accuracy: U1 Range: [0, 1] */
} AX_ISP_IQ_SUB_MODULE_EN_T;

typedef struct {
    AX_U16 nMotDetNoiseLut[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_MOTION_DET_LUT_SIZE];/* noise and inverse profile for motion detection,default:[128...128][128...128],Accuracy: U8.4 Range: [0, 4095] */
    AX_S16 nMotDetStrenLuma[YUV3DNR_BLEND_REGS_NUM];                            /* motion detection coring threshold and slope for Y channel,default:[0,0], Accuracy: S0.8 Range: [-255, 255] */
    AX_S16 nMotDetStrenChrom[YUV3DNR_BLEND_REGS_NUM];                           /* motion detection coring threshold and slope for uv channel,default:[0,0] Accuracy: S0.8 Range: [-255, 255] */
    AX_U8 nMotDetSmoothLuma[YUV3DNR_BLEND_REGS_NUM];                            /* motion detection coring slope for Y channel,default:[16,16],Accuracy: U8 Range: [0, 16] */
    AX_U8 nMotDetSmoothChroma[YUV3DNR_BLEND_REGS_NUM];                          /* motion detection coring threshold for UV channel,default:[16,16],Accuracy: U8 Range: [0, 16] */
    AX_U16 nMotDetLimitLuma[YUV3DNR_BLEND_REGS_NUM];                            /* motion detection limit for Y channel,default:[0,256],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nMotDetLimitChrom[YUV3DNR_BLEND_REGS_NUM];                           /* motion detection limit for uv channel,default:[0,256],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nMotDetBlendRatio[YUV3DNR_BLEND_REGS_NUM];   /* motion detection blend ratio(Y and UV channel result),default:[0,0] Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nStasUpdateLut[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_UPDATE_LUT_SIZE];      /* motion status update step lut,Pos default:[4,4,4,4,3,3,3,2,2,2,1,1,1,1,1,1],Neg default:[12...12],Accuracy: U4 Range: [0, 15] */
    AX_U8 nTfRatioLut[YUV3DNR_UPDATE_LUT_SIZE];         /* status refine lut,default:[128,86,64,51,43,37,32,28,26,23,21,20,18,17,16,15],Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nTfRatLimitLuma[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_BLEND_REGS_NUM];   /* Temporal Filter Limit for Y Channel,default:[0,255],Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nTfRatLimitChroma[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_BLEND_REGS_NUM]; /* Temporal Filter Limit for UV Channel,default:[0,255],Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nSf0EdgeStre;                                /* Edge Detection Coring Threshold and Coring Slope,default:128,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0DirStre;                                 /* Edge Direction Detection Coring Threshold,default:128, Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0DetailStre;                              /* Detail Detection Coring Threshold and Coring Slope,default:128, Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0LumaNoiseLut[YUV3DNR_MOTION_LUT_SIZE][YUV3DNR_AUTO_SF_LUT_SIZE]; /* SF0 NLM Threshold LUT,default:[0...0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf0LumaKernelSize[YUV3DNR_BLEND_REGS_NUM];                          /* SF0 NLM Spatial Coefficients,default:(3,3),Accuracy: U2 Range: [0, 1, 2, 3] */
    AX_U16 nSf0LumaGauStre[YUV3DNR_BLEND_REGS_NUM];     /* SF0 Gaussian FIlter Strength,default:128,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0lumaBlendRatio[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_LUMA_BLEND_RATIO_SIZE]; /* SF0 YNR Blend Ratio,default:[256,0,0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0ChromaAttenStre[YUV3DNR_BLEND_REGS_NUM]; /* SF0 CNR Saturation Threshold adn Slope,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf0ChromaAttenLimit[YUV3DNR_BLEND_REGS_NUM];/* SF0 CNR Saturation Limit,default:[0,0],Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nSf0ChromaProtLut[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE]; /* SF0 CNR Protect Threshold and Slope LUT,default:[0...0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf1LumaMedEn;                               /* median filter enable,defaule:1,Accuracy: U1 Range: [0, 1] */
    AX_U16 nSf1LumaGauStre[YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Gaussian Strength,defaule:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1LumaGauRatio[YUV3DNR_BLEND_REGS_NUM];    /* SF1 YNR Gaussian Blend Ratio,defaule:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U8  nSf1LumaKernelSize;                          /* SF1 YNR Bilateral Spatial Coefficients,defaule:2,Accuracy: U3 Range: [0, 1, 2] */
    AX_U16 nSf1LumaNoiseLut[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE];  /*SF1 YNR Bilateral Threshold and slope LUT,defaule:[1023,...1023],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaIdrThre[YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Isolated Dot Removal Detection Threshold,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaIdrGain[YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Isolated Dot Removal Detection Gain,default:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1LumaClipLevel[YUV3DNR_BLEND_REGS_NUM];   /* SF1 YNR Original Clip Level,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaCoring[YUV3DNR_BLEND_REGS_NUM];      /* SF1 YNR Coring Threshold and coring slope,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaBlendRatio[YUV3DNR_BLEND_REGS_NUM];  /* SF1 CNR Blend Ratio,default:0,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1ChromaCoring;                            /* SF1 CNR Coring Threshold and coring slope,default:0,Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1ChromaBlendRatio;                        /* SF1 CNR Blend Ratio,default:0,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf2LumaGauStre[YUV3DNR_BLEND_REGS_NUM];     /* SF2 YNR Gaussian FIlter Strength,default:[128,128]Accuracy: U1.8 Range: [0, 256] */
    AX_U8  nSf2LumaMedEn;                               /* SF2 YNR Median Filter Enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8  nSf2KernelSize;                              /* SF2 YNR Bilateral Spatial Coefficients,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U16 nSf2NoiseLut[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE];      /* SF2 YNR Bilateral Threshold LUT and slope Lut,default:[1023,1023,1023],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf2BlendRatio[YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_LUMA_BLEND_RATIO_SIZE];         /* SF2 YNR Blend Ratio,default:[256,0,0],Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nExtMaskStrenLuma;                             /* adjustment for Y channel motion mask or SF guide,default:128,Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nExtMaskStrenChrom;                            /* adjustment for UV channel motion mask or SF guide,default:128,Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nExtMaskStrenStatus;                           /* adjustment for motion STATUS update,default:128,Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nExtMaskRatioLuma[YUV3DNR_BLEND_REGS_NUM];    /* blend ratio for Y channel motion mask or SF guide,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nExtMaskRatioChrom[YUV3DNR_BLEND_REGS_NUM];   /* blend ratio for UV channel motion mask or SF guide,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nExtMaskRatioStatus[YUV3DNR_BLEND_REGS_NUM];  /* blend ratio for motion STATUS update,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nY3dnrPos;                                     /* the position for y3dnr and lce,default:0,Accuracy: U1 Range: [0, 1] */
    AX_U8 nStaRefineErode;                               /* status refine erode radius,default:0,0: erode disable,1: erode 3x3,2: erode 5x5:Accuracy: U2 Range: [0, 1, 2] */
    AX_U16 nSfGuideMapLuma[YUV3DNR_BLEND_REGS_NUM];      /* guide map luma,default:[128, 128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSfGuideMapChroma[YUV3DNR_BLEND_REGS_NUM];    /* guide map chroma,default:[128, 128],Accuracy: U1.8 Range: [0, 256] */
} AX_ISP_IQ_YUV_3DNR_MANUAL_PARAM_T;

typedef struct {
    AX_U8  nParamGrpNum;                                                          /* auto group number,Accuracy: U8.0 Range: [0x0, AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM] */
    AX_U32 nRefVal[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];
    AX_U16 nMotDetNoiseLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_MOTION_DET_LUT_SIZE]; /* noise and inverse profile for motion detection,default:[128...128][128...128],Accuracy: U8.4 Range: [0, 4095] */
    AX_S16 nMotDetStrenLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                             /* motion detection coring threshold and slope for Y channel,default:[0,0], Accuracy: S0.8 Range: [-255, 255] */
    AX_S16 nMotDetStrenChrom[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                            /* motion detection coring threshold and slope for uv channel,default:[0,0] Accuracy: S0.8 Range: [-255, 255] */
    AX_U8 nMotDetSmoothLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                             /* motion detection coring slope for Y channel,default:[16,16],Accuracy: U8 Range: [0, 16] */
    AX_U8 nMotDetSmoothChroma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                           /* motion detection coring threshold for UV channel,default:[16,16],Accuracy: U8 Range: [0, 16] */
    AX_U16 nMotDetLimitLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                             /* motion detection limit for Y channel,default:[0,256],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nMotDetLimitChrom[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                            /* motion detection limit for uv channel,default:[0,256],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nMotDetBlendRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];  /* motion detection blend ratio(Y and UV channel result),default:[0,0] Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nStasUpdateLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_UPDATE_LUT_SIZE];       /* motion status update step lut,Pos default:[4,4,4,4,3,3,3,2,2,2,1,1,1,1,1,1],Neg default:[12...12],Accuracy: U4 Range: [0, 15] */
    AX_U8 nTfRatioLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_UPDATE_LUT_SIZE];        /* status refine lut,default:[128,86,64,51,43,37,32,28,26,23,21,20,18,17,16,15],Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nTfRatLimitLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_BLEND_REGS_NUM];   /* Temporal Filter Limit for Y Channel,default:[0,255],Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nTfRatLimitChroma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_BLEND_REGS_NUM]; /* Temporal Filter Limit for UV Channel,default:[0,255],Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nSf0EdgeStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                                /* Edge Detection Coring Threshold and Coring Slope,default:128,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0DirStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                                 /* Edge Direction Detection Coring Threshold,default:128, Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0DetailStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                              /* Detail Detection Coring Threshold and Coring Slope,default:128, Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0LumaNoiseLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_MOTION_LUT_SIZE][YUV3DNR_AUTO_SF_LUT_SIZE]; /* SF0 NLM Threshold LUT,default:[0...0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf0LumaKernelSize[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];                          /* SF0 NLM Spatial Coefficients,default:(3,3),Accuracy: U2 Range: [0, 1, 2, 3] */
    AX_U16 nSf0LumaGauStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* SF0 Gaussian FIlter Strength,default:128,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0lumaBlendRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_LUMA_BLEND_RATIO_SIZE]; /* SF0 YNR Blend Ratio,default:[256,0,0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf0ChromaAttenStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM]; /* SF0 CNR Saturation Threshold adn Slope,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf0ChromaAttenLimit[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];/* SF0 CNR Saturation Limit,default:[0,0],Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nSf0ChromaProtLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE]; /* SF0 CNR Protect Threshold and Slope LUT,default:[0...0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nSf1LumaMedEn[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                               /* median filter enable,defaule:1,Accuracy: U1 Range: [0, 1] */
    AX_U16 nSf1LumaGauStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Gaussian Strength,defaule:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1LumaGauRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];    /* SF1 YNR Gaussian Blend Ratio,defaule:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U8  nSf1LumaKernelSize[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                          /* SF1 YNR Bilateral Spatial Coefficients,defaule:2,Accuracy: U3 Range: [0, 1, 2] */
    AX_U16 nSf1LumaNoiseLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE];  /*SF1 YNR Bilateral Threshold and slope LUT,defaule:[1023,...1023],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaIdrThre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Isolated Dot Removal Detection Threshold,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaIdrGain[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* SF1 YNR Isolated Dot Removal Detection Gain,default:[128,128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1LumaClipLevel[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];   /* SF1 YNR Original Clip Level,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaCoring[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];      /* SF1 YNR Coring Threshold and coring slope,default:[0,0],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1LumaBlendRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];  /* SF1 CNR Blend Ratio,default:0,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf1ChromaCoring[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                            /* SF1 CNR Coring Threshold and coring slope,default:0,Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf1ChromaBlendRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                        /* SF1 CNR Blend Ratio,default:0,Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSf2LumaGauStre[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* SF2 YNR Gaussian FIlter Strength,default:[128,128]Accuracy: U1.8 Range: [0, 256] */
    AX_U8  nSf2LumaMedEn[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                               /* SF2 YNR Median Filter Enable,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U8  nSf2KernelSize[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                              /* SF2 YNR Bilateral Spatial Coefficients,default:1,Accuracy: U1 Range: [0, 1] */
    AX_U16 nSf2NoiseLut[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_SF_LUT_SIZE];      /* SF2 YNR Bilateral Threshold LUT and slope Lut,default:[1023,1023,1023],Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nSf2BlendRatio[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM][YUV3DNR_AUTO_LUMA_BLEND_RATIO_SIZE];         /* SF2 YNR Blend Ratio,default:[256,0,0],Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nExtMaskStrenLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                            /* adjustment for Y channel motion mask or SF guide,default:128,Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nExtMaskStrenChrom[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                           /* adjustment for UV channel motion mask or SF guide,default:128,Accuracy: U0.8 Range: [0, 255] */
    AX_U8 nExtMaskStrenStatus[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                          /* adjustment for motion STATUS update,128,Accuracy: U0.8 Range: [0, 255] */
    AX_U16 nExtMaskRatioLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];   /* blend ratio for Y channel motion mask or SF guide,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nExtMaskRatioChrom[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];  /* blend ratio for UV channel motion mask or SF guide,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nExtMaskRatioStatus[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM]; /* blend ratio for motion STATUS update,default:[0, 0],Accuracy: U1.8 Range: [0, 256] */
    AX_U8 nY3dnrPos[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                                    /* the position for y3dnr and lce,default:0,Accuracy: U1 Range: [0, 1] */
    AX_U8 nStaRefineErode[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM];                              /* status refine erode radius,default:0,0: erode disable,1: erode 3x3,2: erode 5x5:Accuracy: U2 Range: [0, 1, 2] */
    AX_U16 nSfGuideMapLuma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];     /* guide map luma,default:[128, 128],Accuracy: U1.8 Range: [0, 256] */
    AX_U16 nSfGuideMapChroma[AX_ISP_REF_AUTOTBL_YUV3DNR_EXPRESS_NUM][YUV3DNR_BLEND_REGS_NUM];   /* guide map chroma,default:[128, 128],Accuracy: U1.8 Range: [0, 256] */
} AX_ISP_IQ_YUV_3DNR_AUTO_PARAM_T;

typedef struct {
    AX_U8 nYuv3dnrEnable;                         /* yuv 3dnr enable: default 1,Accuracy: U1 Range: [0, 1] */
    AX_U8 nAutoMode;                              /*the parameter is dynamically differed or manually configured: Accuracy: U1 Range: [0, 1] */
    AX_U8 nRefMode;                               /* use lux for auto interpolation or gain for interpolation,Accuracy: U1 Range: [0, 1] */
    AX_U8 nConvergeSpeed;                         /* status refine lut,default:[0, 1, 2, 3, ... 15], Accuracy: U0.8 Range: [0, 255] */
    AX_ISP_IQ_EXTMASK_PARAM_T tExtMaskParam;
    AX_ISP_IQ_SUB_MODULE_EN_T tSubModuleEnParam;
    AX_ISP_IQ_YUV_3DNR_MANUAL_PARAM_T tManualParam;
    AX_ISP_IQ_YUV_3DNR_AUTO_PARAM_T   tAutoParam;
} AX_ISP_IQ_YUV3DNR_PARAM_T;

/************************************************************************************
 *  Sharpen IQ Param
 ************************************************************************************/
#define AX_ISP_SHP_HPF_NUM                 (3)
#define AX_ISP_SHP_TEXTURE_LUT_SIZE        (9)
#define AX_ISP_SHP_LUMA_MASK_LUT_SIZE      (17)
#define AX_ISP_SHP_GAIN_SIZE               (2)
#define AX_ISP_SHP_LIMIT_SIZE              (2)
#define AX_ISP_SHP_COLOR_SIZE              (3)
#define AX_ISP_SHP_COLOR_MASK_NUM          (4)
#define AX_ISP_SHP_COLOR_TARGET_NUM        (8)
#define AX_ISP_SHP_EDGE_LEVEL_LUT_SIZE     (9)
#define AX_ISP_SHP_GENERAL_LUT_SIZE        (5)

typedef struct {
    AX_S16  nHighFreqLogGain[AX_ISP_SHP_GAIN_SIZE];             /* shp_HighFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nHighFreqGainLimit[AX_ISP_SHP_LIMIT_SIZE];          /* shp_HighFreq_gain_limit. default: 0,255. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nHighFreqCorBaseGain;                               /* shp_HighFreq_coring_base_gain. default: 0. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nHighFreqCorSlope;                                  /* shp_HighFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nHighFreqCorOffset;                                 /* shp_HighFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nHighFreqLimit[AX_ISP_SHP_LIMIT_SIZE];              /* shp_HighFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nMedFreqLogGain[AX_ISP_SHP_GAIN_SIZE];             /* shp_MedFre_log_gain.default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nMedFreqGainLimit[AX_ISP_SHP_LIMIT_SIZE];          /* shp_MedFre_gain_limit. default: 0,255. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nMedFreqCorBaseGain;                               /* shp_MedFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nMedFreqCorSlope;                                  /* shp_MedFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nMedFreqCorOffset;                                 /* shp_MedFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nMedFreqLimit[AX_ISP_SHP_LIMIT_SIZE];              /* shp_MedFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nDirFreqLogGain[AX_ISP_SHP_GAIN_SIZE];             /* shp_DirFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nDirFreqGainLimit[AX_ISP_SHP_LIMIT_SIZE];          /* shp_DirFreq_gain_limit. default: 0,512. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nDirFreqCorBaseGain;                               /* shp_DirFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nDirFreqCorSlope;                                  /* shp_DirFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nDirFreqCorOffset;                                 /* shp_DirFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nDirFreqLimit[AX_ISP_SHP_LIMIT_SIZE];              /* shp_DirFreq_limit. default: 512,512. Accuracy: U1 Range: [0, 1023] */
    AX_S16  nLowFreqLogGain[AX_ISP_SHP_GAIN_SIZE];             /* shp_LowFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nLowFreqGainLimit[AX_ISP_SHP_LIMIT_SIZE];          /* shp_LowFreq_gain_limit. default: 0,512. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nLowFreqCorBaseGain;                               /* shp_LowFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nLowFreqCorSlope;                                  /* shp_LowFreq_coring_slope. default: 0. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nLowFreqCorOffset;                                 /* shp_LowFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nLowFreqLimit[AX_ISP_SHP_LIMIT_SIZE];              /* shp_LowFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_U8   nShpHpfBsigma[AX_ISP_SHP_HPF_NUM];              /* shp_hpf_bsigma. default: 16,16,16. Accuracy: U3.5 Range: high:[0, 48]. med:[0, 80]. low:[0, 96] */
    AX_U8   nShpHpfDsigma[AX_ISP_SHP_HPF_NUM];              /* shp_hpf_dsigma. default: 32,32,32. Accuracy: U3.5 Range: [0, 255] */
    AX_U8   nShpHpfScale[AX_ISP_SHP_HPF_NUM];               /* shp_hpf_scale. default: 128,128,128. Accuracy: U1.7 Range: [0, 128] */
    AX_U8   nShpGain[AX_ISP_SHP_GAIN_SIZE];                 /* shp_gain. Accuracy: U4.4 Range: [16, 255] */
    AX_S16  nShpLimit[AX_ISP_SHP_LIMIT_SIZE];               /* shp_limit. default: 0,0. Accuracy: S8.4 nShpLimit[0] Range: [-4096, 0], nShpLimit[1] Range: [0, 4095] */
    AX_S16  nGrainLogGain;                                  /* shp_grain_log_gain. default: 0. Accuracy: U1 Range: [-256, 255] */
} AX_ISP_IQ_SHP_MANUAL_T;

typedef struct {
    AX_U8   nParamGrpNum;                                                                           /* default: 12. Accuracy: U8.0 Range: [1, AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM] */
    AX_U32  nRefVal[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                        /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_S16  nHighFreqLogGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_GAIN_SIZE];             /* shp_HighFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nHighFreqGainLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];          /* shp_HighFreq_gain_limit. default: 0,255. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nHighFreqCorBaseGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                               /* shp_HighFreq_coring_base_gain. default: 0. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nHighFreqCorSlope[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                  /* shp_HighFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nHighFreqCorOffset[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                 /* shp_HighFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nHighFreqLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];              /* shp_HighFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nMedFreqLogGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_GAIN_SIZE];             /* shp_MedFreq_log_gain.default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nMedFreqGainLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];          /* shp_MedFreq_gain_limit. default: 0,255. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nMedFreqCorBaseGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                               /* shp_MedFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nMedFreqCorSlope[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                  /* shp_MedFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nMedFreqCorOffset[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                 /* shp_MedFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nMedFreqLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];              /* shp_MedFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nDirFreqLogGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_GAIN_SIZE];             /* shp_DirFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nDirFreqGainLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];          /* shp_DirFreq_gain_limit. default: 0,512. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nDirFreqCorBaseGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                               /* shp_DirFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nDirFreqCorSlope[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                  /* shp_DirFreq_coring_slope. default: 4. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nDirFreqCorOffset[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                 /* shp_DirFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nDirFreqLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];              /* shp_DirFreq_limit. default: 512,512. Accuracy: U1 Range: [0, 1023] */
    AX_S16  nLowFreqLogGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_GAIN_SIZE];             /* shp_LowFreq_log_gain. default: 0,0. Accuracy: S3.5 Range: [-256, 255] */
    AX_U8   nLowFreqGainLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];          /* shp_LowFreq_gain_limit. default: 0,512. Accuracy: U4.4 Range: [0, 255] */
    AX_U8   nLowFreqCorBaseGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                               /* shp_LowFreq_coring_base_gain. default: 0. Accuracy: U0.4 Range: [0, 15] */
    AX_U8   nLowFreqCorSlope[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                  /* shp_LowFreq_coring_slope. default: 0. Accuracy: U2.2 Range: [0, 15] */
    AX_U8   nLowFreqCorOffset[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                 /* shp_LowFreq_coring_offset. default: 0. Accuracy: U4.2 Range: [0, 63] */
    AX_U16  nLowFreqLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];              /* shp_LowFreq_limit. default: 512,512. Accuracy: U8.2 Range: [0, 1023] */
    AX_U8   nShpHpfBsigma[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_HPF_NUM];              /* shp_hpf_bsigma. default: 16,16,16. Accuracy: U3.5 Range: high:[0, 48]. med:[0, 80]. low:[0, 96] */
    AX_U8   nShpHpfDsigma[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_HPF_NUM];              /* shp_hpf_dsigma. default: 32,32,32. Accuracy: U3.5 Range: [0, 255] */
    AX_U8   nShpHpfScale[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_HPF_NUM];               /* shp_hpf_scale. default: 128,128,128. Accuracy: U1.7 Range: [0, 128] */
    AX_U8   nShpGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_GAIN_SIZE];                 /* shp_gain. Accuracy: U4.4 Range: [16, 255] */
    AX_S16  nShpLimit[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM][AX_ISP_SHP_LIMIT_SIZE];               /* shp_limit. default: 0,0. Accuracy: S8.4 nShpLimit[0] Range: [-4096, 0], nShpLimit[1] Range: [0, 4095] */
    AX_S16  nGrainLogGain[AX_ISP_REF_AUTOTBL_SHARPEN_EXPRESS_NUM];                                  /* shp_grain_log_gain. default: 0. Accuracy: U1 Range: [-256, 255] */
} AX_ISP_IQ_SHP_AUTO_T;

/* mask control */
typedef struct {
    AX_U8                   nMotionMaskEn;                                                     /* motion mask on-off. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nShpClnpNpuMaskEn;                                                 /* shp_mask_clnp_npu on-off. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nShpLumaMaskLut[AX_ISP_SHP_LUMA_MASK_LUT_SIZE];                    /* shp_mask_luma_lut. default: 0.... Accuracy: U0.8 Range: [0, 255] */
} AX_ISP_IQ_SHP_MASK_CONTROL_T;

/* fine grain noise generation */
typedef struct {
    AX_U8                   nGrainNoiseEn;                                                     /* shp_grain_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nGnMotionMskEn;                                                    /* shp_grain_mask_motion_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_S8                   nGnMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                         /* shp_grain_mask_motion_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nGnLumaMskEn;                                                      /* shp_grain_mask_luma_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_S8                   nGnLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                              /* shp_grain_mask_luma_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nGnTextureMskEn;                                                   /* shp_grain_mask_texture_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_S8                   nGnTextureLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];                        /* shp_grain_mask_texture_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nGnClnpMskEn[AX_ISP_SHP_COLOR_MASK_NUM];                           /* shp_grain_mask_clnp_enable. default: 0,0.... Accuracy: U1 Range: [0, 1] */
    AX_S8                   nGnClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE]; /* shp_grain_mask_clnp_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
} AX_ISP_IQ_SHP_FINE_GRAIN_NOISE_T;

/* overshot/undershoot control for static/motion region */
typedef struct {
    AX_U8                   nOsStaticLimit[AX_ISP_SHP_LIMIT_SIZE];                             /* shp_os_static_limit. default: 127,127. Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nOsStaticGain[AX_ISP_SHP_GAIN_SIZE];                               /* shp_os_static_gain. default: 8,8. Accuracy: U1.3 Range: [0, 8] */
    AX_U8                   nOsMotionLimit[AX_ISP_SHP_LIMIT_SIZE];                             /* shp_os_motion_limit. default: 127,127 Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nOsMotionGain[AX_ISP_SHP_GAIN_SIZE];                               /* shp_os_motion_gain. default: 8,8. Accuracy: U1.3 Range: [0, 8] */
} AX_ISP_IQ_SHP_OS_STATIC_MOTION_T;

/* common motion/lumagain/texture gain/color and npu gain lut */
typedef struct {
    AX_U8                   nCommonLutEnable;                                                 /* shp_common_lut_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_S8                   nCommonMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                    /* shp_common_motion_gain_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nCommonLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                      /* shp_common_luma_gain_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nCommonTextureStaticLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];             /* shp_common_texture_gain_static_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nCommonTexturemotionLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];             /* shp_common_texture_gain_motion_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nCommonClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE]; /* shp_common_color_npu_gain_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
} AX_ISP_IQ_SHP_COMMON_LUT_CORE_T;

/* color target */
typedef struct {
    AX_U8                   nColorEnable[AX_ISP_SHP_COLOR_TARGET_NUM];                         /* shp_color_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_S16                  nColorCenter[AX_ISP_SHP_COLOR_TARGET_NUM][AX_ISP_SHP_COLOR_SIZE];  /* shp_mask_clnp_yuv_center_y/uv. default: 0,0.... Accuracy: S8.2 Range: Y:[0, 1023].UV:[-512, 511] */
    AX_U16                  nColorRange[AX_ISP_SHP_COLOR_TARGET_NUM][AX_ISP_SHP_COLOR_SIZE];   /* shp_mask_clnp_yuv_radius. default: 0,0.... Accuracy: U7.2 Range: [0, 511] */
    AX_U8                   nColorSmooth[AX_ISP_SHP_COLOR_TARGET_NUM][AX_ISP_SHP_COLOR_SIZE];  /* shp_mask_clnp_yuv_t_grad. default: 0,0.... Accuracy: U4 Range: [0, 7] */
    AX_U8                   nColorIoFlag[AX_ISP_SHP_COLOR_TARGET_NUM];                         /* shp_mask_clnp_yuv_io_flag. default: 1,1.... Accuracy: U1 Range: [0, 1] */
    AX_U8                   nColorMaskIndex[AX_ISP_SHP_COLOR_TARGET_NUM];                      /* shp_mask_clnp_yuv_ch. default: 0,0.... Accuracy: U2 Range: [0, 3] */
    AX_U8                   nColorMaskLimit[AX_ISP_SHP_COLOR_TARGET_NUM];                      /* shp_mask_clnp_yuv_limit. default: 0,0.... Accuracy: U1.7 Range: [0, 128] */
} AX_ISP_IQ_SHP_COLOR_TARGET_T;

/* sharpen control for each sharpen core */
typedef struct {
    AX_U8                   nHighFreqEnable;                                                       /* shp_HighFreq_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nHighFreqMotionMskEn;                                                  /* shp_HighFreq_mask_motion on-off. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nHighFreqMotionLutLevel;                                               /* shp_HighFreq_mask_motion_lut_level. default: 4,4.... Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nHighFreqLumaMskEn;                                                    /* shp_HighFreq_mask_luma on-off. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nHighFreqLumaLutLevel;                                                 /* shp_HighFreq_mask_luma_lut_level. default: 4,4.... Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nHighFreqTextureMskEn;                                                 /* shp_HighFreq_mask_texture_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nHighFreqTextureLutLevel;                                              /* shp_HighFreq_mask_texture_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nHighFreqTextureCoring;                                                /* shp_HighFreq_texture_coring. Accuracy: U4.4 Range: [0, 255] */
    AX_U8                   nHighFreqTextureLutMotionLevel;                                        /* shp_HighFreq_mask_texture_lut_motion_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nHighFreqClnpMskEn[AX_ISP_SHP_COLOR_MASK_NUM];                         /* shp_HighFreq_mask_clnp_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nHighFreqClnpLutLevel[AX_ISP_SHP_COLOR_MASK_NUM];                      /* shp_HighFreq_mask_clnp_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_S8                   nHighFreqMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                       /* shp_HighFreq_mask_motion_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nHighFreqLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                         /* shp_HighFreq_mask_luma_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nHighFreqTextureLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];                      /* shp_HighFreq_mask_texture_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nHighFreqTextureLutMotion[AX_ISP_SHP_TEXTURE_LUT_SIZE];                /* shp_HighFreq_mask_texture_lut_motion. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nHighFreqClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE];/* shp_HighFreq_mask_clnp_lut. default: 0,0.... Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nHighFreqOsLimit[AX_ISP_SHP_LIMIT_SIZE];                               /* shp_HighFreq_os_limit. Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nHighFreqOsGain[AX_ISP_SHP_GAIN_SIZE];                                 /* shp_HighFreq_os_gain. Accuracy: U1.3 Range: [0, 8] */
} AX_ISP_IQ_SHP_HIGH_FREQ_CONTROL_T;

typedef struct {
    AX_U8                   nMedFreqEnable;                                                       /* shp_MedFreq_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nMedFreqMotionMskEn;                                                  /* shp_MedFreq_mask_motion on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nMedFreqMotionLutLevel;                                               /* shp_MedFreq_mask_motion_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nMedFreqLumaMskEn;                                                    /* shp_MedFreq_mask_luma on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nMedFreqLumaLutLevel;                                                 /* shp_MedFreq_mask_luma_lu_levelt. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nMedFreqTextureMskEn;                                                 /* shp_MedFreq_mask_texture_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nMedFreqTextureLutLevel;                                              /* shp_MedFreq_mask_texture_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nMedFreqTextureCoring;                                                /* shp_MedFreq_texture_coring. Accuracy: U4.4 Range: [0, 255] */
    AX_U8                   nMedFreqTextureLutMotionLevel;                                        /* shp_MedFreq_mask_texture_lut_motion_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nMedFreqClnpMskEn[AX_ISP_SHP_COLOR_MASK_NUM];                         /* shp_MedFreq_mask_clnp_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nMedFreqClnpLutLevel[AX_ISP_SHP_COLOR_MASK_NUM];                      /* shp_MedFreq_mask_clnp_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_S8                   nMedFreqMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                       /* shp_MedFreq_mask_motion_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nMedFreqLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                         /* shp_MedFreq_mask_luma_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nMedFreqTextureLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];                      /* shp_MedFreq_mask_texture_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nMedFreqTextureLutMotion[AX_ISP_SHP_TEXTURE_LUT_SIZE];                /* shp_MedFreq_mask_texture_lut_motion. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nMedFreqClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE];/* shp_MedFreq_mask_clnp_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nMedFreqOsLimit[AX_ISP_SHP_LIMIT_SIZE];                               /* shp_MedFreq_os_limit. Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nMedFreqOsGain[AX_ISP_SHP_GAIN_SIZE];                                 /* shp_MedFreq_os_gain. Accuracy: U1.3 Range: [0, 8] */
} AX_ISP_IQ_SHP_MED_FREQ_CONTROL_T;

typedef struct {
    AX_U8                   nDirFreqEnable;                                                       /* shp_DirFreq_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nDirFreqMotionMskEn;                                                  /* shp_DirFreq_mask_motion on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nDirFreqMotionLutLevel;                                               /* shp_DirFreq_mask_motion_lut_level. U2.2 Range: [0, 16] */
    AX_U8                   nDirFreqLumaMskEn;                                                    /* shp_DirFreq_mask_luma on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nDirFreqLumaLutLevel;                                                 /* shp_DirFreq_mask_luma_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nDirFreqTextureMskEn;                                                 /* shp_DirFreq_mask_texture_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nDirFreqTextureLutLevel;                                              /* shp_DirFreq_mask_texture_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nDirFreqTextureLutMotionLevel;                                        /* shp_DirFreq_mask_texture_lut_motion_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nDirFreqClnpMskEn[AX_ISP_SHP_COLOR_MASK_NUM];                         /* shp_DirFreq_mask_clnp_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nDirFreqClnpLutLevel[AX_ISP_SHP_COLOR_MASK_NUM];                      /* shp_DirFreq_mask_clnp_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_S8                   nDirFreqMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                       /* shp_DirFreq_mask_motion_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nDirFreqLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                         /* shp_DirFreq_mask_luma_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nDirFreqTextureLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];                      /* shp_DirFreq_mask_texture_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nDirFreqTextureLutMotion[AX_ISP_SHP_TEXTURE_LUT_SIZE];                /* shp_DirFreq_mask_texture_lut_motion. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nDirFreqClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE];/* shp_DirFreq_mask_clnp_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nDirFreqOsLimit[AX_ISP_SHP_LIMIT_SIZE];                               /* shp_DirFreq_os_limit. Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nDirFreqOsGain[AX_ISP_SHP_GAIN_SIZE];                                 /* shp_DirFreq_os_gain. Accuracy: U1.3 Range: [0, 8] */
    /* directional and non-directional sharpen blending weights */
    AX_U8                   nDirFreqDirEdgeLevelLut[AX_ISP_SHP_EDGE_LEVEL_LUT_SIZE];                    /* shp_DirFreq_dir_edge_level_lut. Accuracy: U0.8 Range: [0, 255] */
} AX_ISP_IQ_SHP_DIR_FREQ_CONTROL_T;

typedef struct {
    AX_U8                   nLowFreqEnable;                                                       /* shp_LowFreq_enable. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nLowFreqMotionMskEn;                                                  /* shp_LowFreq_mask_motion on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nLowFreqMotionLutLevel;                                               /* shp_LowFreq_mask_motion_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nLowFreqLumaMskEn;                                                    /* shp_LowFreq_mask_luma on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nLowFreqLumaLutLevel;                                                 /* shp_LowFreq_mask_luma_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nLowFreqTextureMskEn;                                                 /* shp_LowFreq_mask_texture_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nLowFreqTextureLutLevel;                                              /* shp_LowFreq_mask_texture_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nLowFreqTextureCoring;                                                /* shp_LowFreq_texture_coring. Accuracy: U4.4 Range: [0, 255] */
    AX_U8                   nLowFreqTextureLutMotionLevel;                                        /* shp_LowFreq_mask_texture_lut_motion_level. Accuracy: U2.2 Range: [0, 16] */
    AX_U8                   nLowFreqClnpMskEn[AX_ISP_SHP_COLOR_MASK_NUM];                         /* shp_LowFreq_mask_clnp_enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nLowFreqClnpLutLevel[AX_ISP_SHP_COLOR_MASK_NUM];                      /* shp_LowFreq_mask_clnp_lut_level. Accuracy: U2.2 Range: [0, 16] */
    AX_S8                   nLowFreqMotionLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                       /* shp_LowFreq_mask_motion_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nLowFreqLumaLut[AX_ISP_SHP_GENERAL_LUT_SIZE];                         /* shp_LowFreq_mask_luma_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nLowFreqTextureLut[AX_ISP_SHP_TEXTURE_LUT_SIZE];                      /* shp_LowFreq_mask_texture_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nLowFreqTextureLutMotion[AX_ISP_SHP_TEXTURE_LUT_SIZE];                /* shp_LowFreq_mask_texture_lut_motion. Accuracy: S3.4 Range: [-128, 127] */
    AX_S8                   nLowFreqClnpLut[AX_ISP_SHP_COLOR_MASK_NUM][AX_ISP_SHP_GENERAL_LUT_SIZE];/* shp_LowFreq_mask_clnp_lut. Accuracy: S3.4 Range: [-128, 127] */
    AX_U8                   nLowFreqOsLimit[AX_ISP_SHP_LIMIT_SIZE];                               /* shp_LowFreq_os_limit. Accuracy: U5.2 Range: [0, 127] */
    AX_U8                   nLowFreqOsGain[AX_ISP_SHP_GAIN_SIZE];                                 /* shp_LowFreq_os_gain. Accuracy: U1.3 Range: [0, 8] */
} AX_ISP_IQ_SHP_LOW_FREQ_CONTROL_T;

typedef struct {
    AX_U8                   nShpEn;                                                            /* sharpen on-off. default: 1. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nAutoMode;                                                         /* for lux auto or manual adjust mode. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nRefMode;                                                          /* choose ref mode. default: 0. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_SHP_MASK_CONTROL_T     tMaskControl;
    AX_ISP_IQ_SHP_FINE_GRAIN_NOISE_T tFineGrainNoise;
    AX_ISP_IQ_SHP_OS_STATIC_MOTION_T tOsStaticMotion;
    AX_ISP_IQ_SHP_COMMON_LUT_CORE_T tCommonLutCore;
    AX_ISP_IQ_SHP_COLOR_TARGET_T    tColorTarget;
    AX_ISP_IQ_SHP_HIGH_FREQ_CONTROL_T   tHighFreqControl;
    AX_ISP_IQ_SHP_MED_FREQ_CONTROL_T    tMedFreqControl;
    AX_ISP_IQ_SHP_DIR_FREQ_CONTROL_T    tDirFreqControl;
    AX_ISP_IQ_SHP_LOW_FREQ_CONTROL_T    tLowFreqControl;
    AX_ISP_IQ_SHP_MANUAL_T          tManualParam;
    AX_ISP_IQ_SHP_AUTO_T            tAutoParam;
} AX_ISP_IQ_SHARPEN_PARAM_T;

/************************************************************************************
 *  YNR IQ Param: Luma Noise Reduction : YNR + DBPC
 ************************************************************************************/
#define AX_ISP_YNR_CMASK_CENTER_SIZE    (3)
#define AX_ISP_YNR_CMASK_RADIUS_SIZE    (3)
#define AX_ISP_YNR_CMASK_SMOOTH_SIZE    (3)
#define AX_ISP_YNR_LEVEL_SIZE           (2)
#define AX_ISP_YNR_INV_LUT_SIZE         (4)

typedef struct {
    AX_S16  nYnrCenter[AX_ISP_YNR_CMASK_CENTER_SIZE];   /* ynr cmask center [0]:Y, [1]:U, [2]:V. Accuracy: S8.2 Range: Y:[0, 1023] UV: [-512, 511] */
    AX_U16  nYnrRadius[AX_ISP_YNR_CMASK_RADIUS_SIZE];   /* ynr cmask radius [0]:Y, [1]:U, [2]:V. Accuracy: U7.2 Range: [0, 511] */
    AX_U8   nYnrSmooth[AX_ISP_YNR_CMASK_SMOOTH_SIZE];   /* ynr cmask smooth [0]:Y, [1]:U, [2]:V. Accuracy: U3 Range: [0, 7] */
    AX_U8   nYnrLevel[AX_ISP_YNR_LEVEL_SIZE];           /* ynr level [0]:(mask)ratio=1, [1]:(mask)ratio=0. Accuracy: U0.8 Range: [0, 255] */
    AX_U16  nYnrInvNrLut[AX_ISP_YNR_INV_LUT_SIZE];      /* ynr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_YNR_MANUAL_T;

typedef struct {
    AX_U8   nParamGrpNum;                                                           /* Accuracy: U8.0 Range: [0, AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM] */
    AX_U32  nRefVal[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM];                                     /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_S16  nYnrCenter[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_YNR_CMASK_CENTER_SIZE];    /* ynr cmask center [0]:Y, [1]:U, [2]:V. Accuracy: S8.2 Range: Y:[0, 1023] UV: [-512, 511] */
    AX_U16  nYnrRadius[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_YNR_CMASK_RADIUS_SIZE];    /* ynr cmask radius [0]:Y, [1]:U, [2]:V. Accuracy: U7.2 Range: [0, 511] */
    AX_U8   nYnrSmooth[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_YNR_CMASK_SMOOTH_SIZE];    /* ynr cmask smooth [0]:Y, [1]:U, [2]:V. Accuracy: U3 Range: [0, 7] */
    AX_U8   nYnrLevel[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_YNR_LEVEL_SIZE];            /* ynr level [0]:(mask)ratio=0, [1]:(mask)ratio=1. Accuracy: U0.8 Range: [0, 255] */
    AX_U16  nYnrInvNrLut[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_YNR_INV_LUT_SIZE];       /* ynr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_YNR_AUTO_T;

typedef struct {
    AX_U8                       nYnrEn;         /* ynr on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nColorTargetEn; /* color target on-ffset Accuracy: U1 Range: [0: 1] */
    AX_U8                       nAutoMode;      /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nRefMode;       /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nIoFlag;        /* ynr color mask in/out flag. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_YNR_MANUAL_T      tManualParam;
    AX_ISP_IQ_YNR_AUTO_T        tAutoParam;
} AX_ISP_IQ_YNR_PARAM_T;

/************************************************************************************
 *  CNR IQ Param: Chroma Noise Reduction : CNR
 ************************************************************************************/
#define AX_ISP_CNR_INV_LUT_SIZE         (4)

typedef struct {
    AX_U8   nCnrLevel;                              /* cnr level. Accuracy: U1.4 Range: [0, 16] */
    AX_U16  nCnrInvNrLut[AX_ISP_CNR_INV_LUT_SIZE];  /* cnr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_CNR_MANUAL_T;

typedef struct {
    AX_U8   nParamGrpNum;                                                       /* Accuracy: U8.0 Range: [0, AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM] */
    AX_U32  nRefVal[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM];                                 /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8   nCnrLevel[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM];                               /* cnr level. Accuracy: U1.4 Range: [0, 16] */
    AX_U16  nCnrInvNrLut[AX_ISP_REF_AUTOTBL_YUV2DNR_EXPRESS_NUM][AX_ISP_CNR_INV_LUT_SIZE];   /* cnr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_CNR_AUTO_T;

typedef struct {
    AX_U8                       nCnrEn;     /* cnr on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nAutoMode;  /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nRefMode;   /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_CNR_MANUAL_T      tManualParam;
    AX_ISP_IQ_CNR_AUTO_T        tAutoParam;
} AX_ISP_IQ_CNR_PARAM_T;

/************************************************************************************
 *  SCM IQ Param: Special Color Mapping
 ************************************************************************************/
#define AX_ISP_SCM_COLOR_SIZE          (2)
#define AX_ISP_SCM_MASK_CENTER_UV_SIZE (2)
#define AX_ISP_SCM_MASK_SIZE           (3)

typedef struct {
    AX_S16  nScmColor[AX_ISP_SCM_COLOR_SIZE];              /* target color. Accuracy: S7.2 Range: [-512, 511] */
    AX_U16  nScmCenterY;                                   /* color mask center Y. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nScmCenterUv[AX_ISP_SCM_MASK_CENTER_UV_SIZE];  /* scm color mask center [0]:U, [1]:V. Accuracy: S7.2 Range: [-512, 511] */
    AX_U16  nScmRadius[AX_ISP_SCM_MASK_SIZE];              /* scm color mask radius [0]:Y, [1]:U, [2]:V. Accuracy: U7.2 Range: [0, 511] */
    AX_U8   nScmSmooth[AX_ISP_SCM_MASK_SIZE];              /* scm color mask transition gradient. Accuracy: U3 Range: [0, 7] */
} AX_ISP_IQ_SCM_MANUAL_T;

typedef struct {
    AX_U8   nParamGrpNum;                                                               /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32  nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];                                         /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_S16  nScmColor[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_SCM_COLOR_SIZE];              /* target color. Accuracy: S7.2 Range: [-512, 511] */
    AX_U16  nScmCenterY[AX_ISP_AUTO_TABLE_MAX_NUM];                                    /* color mask center Y. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16  nScmCenterUv[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_SCM_MASK_CENTER_UV_SIZE];  /* scm color mask center [0]:U, [1]:V. Accuracy: S7.2 Range: [-512, 511] */
    AX_U16  nScmRadius[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_SCM_MASK_SIZE];               /* scm color mask radius [0]:Y, [1]:U, [2]:V. Accuracy: U7.2 Range: [0, 511] */
    AX_U8   nScmSmooth[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_SCM_MASK_SIZE];              /* scm color mask transition gradient. Accuracy: U3 Range: [0, 7] */
} AX_ISP_IQ_SCM_AUTO_T;

typedef struct {
    AX_U8                   nScmEn;        /* scm on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nAutoMode;      /* for lux auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nRefMode;       /* choose ref mode. Accuracy: U1 Range: [0, 1] */
    AX_U8                   nScmIoFlag;    /* scm color mask in/out flag. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_SCM_MANUAL_T  tManualParam;
    AX_ISP_IQ_SCM_AUTO_T    tAutoParam;
} AX_ISP_IQ_SCM_PARAM_T;

/************************************************************************************
 *  HS2DLUT IQ Param: HSV CONTROL
 ************************************************************************************/
#define AX_ISP_HS2DLUT_H_SIZE            (24)
#define AX_ISP_HS2DLUT_V_SIZE            (16)
#define AX_ISP_HS2DLUT_REF_SEG_NUM_MAX   (12)
#define AX_ISP_HS2DLUT_CCT_SEG_NUM_MAX   (16)

typedef struct {
    AX_U16  nHueLut[AX_ISP_HS2DLUT_H_SIZE][AX_ISP_HS2DLUT_V_SIZE];    /* hue lut. Accuracy: U9.7 Range: [0, 46080] */
    AX_U16  nSatLut[AX_ISP_HS2DLUT_H_SIZE][AX_ISP_HS2DLUT_V_SIZE];    /* saturation lut. Accuracy: U1.15 Range: [0, 32768] */
} AX_ISP_IQ_HS2DLUT_T;

typedef struct {
    AX_ISP_IQ_HS2DLUT_T         tHs2dLut;
} AX_ISP_IQ_HS2DLUT_MANUAL_T;

typedef struct
{
    AX_U32                      nCctStart;  /* Accuracy: U14. Range: [0, 1000000]. */
    AX_U32                      nCctEnd;    /* Accuracy: U14. Range: [0, 1000000]. */
    AX_ISP_IQ_HS2DLUT_T         tHs2dLut;
} AX_ISP_IQ_HS2DLUT_CCT_TABLE_T;

typedef struct
{
    AX_U32                      nRefStartVal;           /* Accuracy: U22.10. Range: [0x400, 0xFFFFFFFF]. */
    AX_U32                      nRefEndVal;             /* Accuracy: U22.10. Range: [0x400, 0xFFFFFFFF]. */
    AX_U8                       nCctListNum;            /* Accuracy: U8. Range: range: [1, AX_ISP_HS2DLUT_CCT_SEG_NUM_MAX] */
    AX_ISP_IQ_HS2DLUT_CCT_TABLE_T  tCctTbl[AX_ISP_HS2DLUT_CCT_SEG_NUM_MAX];
} AX_ISP_IQ_HS2DLUT_REF_TABLE_T;

typedef struct
{
    AX_U8 nRefListNum;                  /* Accuracy: U8. Range: [1, AX_ISP_HS2DLUT_REF_SEG_NUM_MAX] */
    AX_ISP_IQ_HS2DLUT_REF_TABLE_T       tRefTbl[AX_ISP_HS2DLUT_REF_SEG_NUM_MAX];
} AX_ISP_IQ_HS2DLUT_AUTO_T;

typedef struct {
    AX_U8                       nHs2dLutEn;    /* hsvc on-off. Accuracy: U1. Range: [0, 1] */
    AX_U8                       nAutoMode;     /* hsvc mode. 0:manual. 1:auto*/
    AX_U8                       nRefMode;      /* hsvc ref mode. 0:use lux as ref, 1:use gain as ref, 1:gain is default*/
    AX_U8                       nConvergeSpeed;     /* Accuracy: U8. Range: [0, 10]. */
    AX_U32                      nGainTrigger;       /* Accuracy: U22.10. Range: [0x400, 0xFFFFFFFF]. */
    AX_U32                      nLuxTrigger;        /* Accuracy: U22.10. Range: [0x400, 0xFFFFFFFF]. */
    AX_U32                      nCctTrigger;        /* Accuracy: U14. Range: [0, 1000000]. */
    AX_ISP_IQ_HS2DLUT_MANUAL_T  tManualParam;
    AX_ISP_IQ_HS2DLUT_AUTO_T    tAutoParam;
} AX_ISP_IQ_HS2DLUT_PARAM_T;

/************************************************************************************
 *  YCPROC IQ Param: COLOR PROCESS
 ************************************************************************************/

typedef struct {
    AX_U8                               nYCprocEn;      /* ycproc on-off. Accuracy: U1 Range: [0, 1] */
    AX_U16                              nBrightness;    /* adjust brightness. Accuracy: U4.8 Range: [0, 4095] */
    AX_S16                              nContrast;      /* adjust contrast. Accuracy: S4.8 Range: [-4096, 4095] */
    AX_U16                              nSaturation;    /* adjust saturation. Accuracy: U4.12 Range: [0, 65535] */
    AX_S16                              nHue;           /* adjust hue. Accuracy: S0.15 Range: [-32768, 32767] */
} AX_ISP_IQ_YCPROC_PARAM_T;

/************************************************************************************
 *  CCMP IQ Param: CHROMA COMP
 ************************************************************************************/
#define AX_ISP_CCMP_Y_SIZE             (29)
#define AX_ISP_CCMP_SAT_SIZE           (23)

typedef struct {
    AX_U16      nChromaCompY[AX_ISP_CCMP_Y_SIZE];       /* ccmp y lut. Accuracy: U1.9 Range: [0, 512] */
    AX_U16      nChromaCompSat[AX_ISP_CCMP_SAT_SIZE];   /* ccmp sat lut. Accuracy: U1.9 Range: [0, 512] */
} AX_ISP_IQ_CCMP_MANUAL_T;

typedef struct {
    AX_U8       nParamGrpNum;                                                               /* Accuracy: U8.0 Range: [0, AX_ISP_REF_AUTOTBL_CCMP_EXPRESS_NUM] */
    AX_U32      nRefVal[AX_ISP_REF_AUTOTBL_CCMP_EXPRESS_NUM];                               /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16      nChromaCompY[AX_ISP_REF_AUTOTBL_CCMP_EXPRESS_NUM][AX_ISP_CCMP_Y_SIZE];      /* ccmp y lut. Accuracy: U1.9 Range: [0, 512] */
    AX_U16      nChromaCompSat[AX_ISP_REF_AUTOTBL_CCMP_EXPRESS_NUM][AX_ISP_CCMP_SAT_SIZE];  /* ccmp sat lut. Accuracy: U1.9 Range: [0, 512] */
} AX_ISP_IQ_CCMP_AUTO_T;

typedef struct {
    AX_U8                       nChromaCompEn;  /* ccmp enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nAutoMode;      /* for lux auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nRefMode;       /* choose ref mode. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_CCMP_MANUAL_T     tManualParam;
    AX_ISP_IQ_CCMP_AUTO_T       tAutoParam;
} AX_ISP_IQ_CCMP_PARAM_T;

/************************************************************************************
 *  YCRT IQ Param
 ************************************************************************************/
#define AX_ISP_YCRT_SIZE         (2)

typedef struct {
    AX_U8       nYcrtEn;                            /* ycrt on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8       nSignalRangeMode;                   /* only supports full range mode, default:0. Accuracy: U2 Range: [0, 2] */
    AX_U16      nYrtInputRange[AX_ISP_YCRT_SIZE];   /* y-range input. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16      nYrtOutputRange[AX_ISP_YCRT_SIZE];  /* y-range output. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16      nCrtInputRange[AX_ISP_YCRT_SIZE];   /* uv-range input. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16      nCrtOutputRange[AX_ISP_YCRT_SIZE];  /* uv-range output. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16      nClipLevelY[AX_ISP_YCRT_SIZE];      /* yclip. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16      nClipLevelUV[AX_ISP_YCRT_SIZE];     /* cclip. Accuracy: S7.2 Range: [-512, 511] */
} AX_ISP_IQ_YCRT_PARAM_T;

/************************************************************************************
 *  LDC IQ Param
 ************************************************************************************/
#define AX_ISP_LDC_V2_MATRIX_V_SIZE     (3)
#define AX_ISP_LDC_V2_MATRIX_H_SIZE     (3)
#define AX_ISP_LDC_V2_COEFF_MAX_NUM     (8)

typedef enum
{
    AX_ISP_IQ_LDC_TYPE_V1,     /* lens distortion correction version 1 */
    AX_ISP_IQ_LDC_TYPE_V2,     /* lens distortion correction version 2 */
} AX_ISP_IQ_LDC_TYPE_E;

typedef struct
{
     AX_BOOL bAspect;          /* whether aspect ration is keep. Accuracy: U1.0 Range: [0, 1]*/
     AX_S16  nXRatio;          /* field angle ration of horizontal, valid when bAspect = 0. Accuracy: S16.0 Range: [0, 100] */
     AX_S16  nYRatio;          /* field angle ration of vertical, valid when bAspect = 0. Accuracy: S16.0 Range: [0, 100] */
     AX_S16  nXYRatio;         /* field angle ration of all,valid when bAspect = 1. Accuracy: S16.0 Range: [0, 100] */
     AX_S16  nCenterXOffset;   /* horizontal offset of the image distortion center relative to image center. Accuracy: S16.0 Range: [-511, 511] */
     AX_S16  nCenterYOffset;   /* vertical offset of the image distortion center relative to image center. Accuracy: S16.0 Range: [-511, 511] */
     AX_S16  nDistortionRatio; /* LDC distortion ratio. [-10000, 0): pincushion distortion; (0, 10000]: barrel distortion. Accuracy: S16.0 Range: [-10000, 10000] */
     AX_S8   nSpreadCoef;      /* LDC spread coefficient. Accuracy: S8.0 Range: [-18, 18] */
} AX_ISP_IQ_LDC_V1_PARAM_T;

typedef struct
{
     AX_U32  nMatrix[AX_ISP_LDC_V2_MATRIX_V_SIZE][AX_ISP_LDC_V2_MATRIX_H_SIZE];  /* Camera Internal Parameter Matrix, {{nXFocus, 0, nXCenter}, {0, nYFocus, nYCenter}, {0, 0, 1}}; Accuracy: have 2 decimal numbers, real value = nMatrix / 100; Range: [0, 0xFFFFFFFF] */
     AX_S64  nDistortionCoeff[AX_ISP_LDC_V2_COEFF_MAX_NUM];                      /* Distortion Coefficients = (k1, k2, p1, p2, k3, k4, k5, k6) Accuracy: have 6 decimal numbers, real value = nDistortionCoeff / 1000000; Range: [-0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF] */
} AX_ISP_IQ_LDC_V2_PARAM_T;

typedef struct
{
    AX_U8 nLdcEnable;                       /* LDC enable, Accuracy: U8.0 Range: [0, 1] */
    AX_U8 nType;                            /* LDC type, Accuracy: U8.0 Range: [AX_ISP_IQ_LDC_TYPE_V1, AX_ISP_IQ_LDC_TYPE_V2] */
    AX_ISP_IQ_LDC_V1_PARAM_T tLdcV1Param;   /* LDC V1 Param */
    AX_ISP_IQ_LDC_V2_PARAM_T tLdcV2Param;   /* LDC V2 Param */
} AX_ISP_IQ_LDC_PARAM_T;


/************************************************************************************
 *  DIS Param Config
 ************************************************************************************/
#define AX_ISP_DIS_POS_WEIGHT_SIZE     (16)

typedef enum
{
    AX_ISP_IQ_DIS_TYPE_V1,
} AX_ISP_IQ_DIS_TYPE_E;

typedef struct {
    AX_U8        bSWCalcEnable;       // u1, 0: use HW offset 1: use projection calculate offset
    AX_U8        nDelayFrameNum;      /* delay num, the more, the smoother. defalut 0. Accuracy: U8 Range: [0, 15] */
    AX_U8        nHistoryFrameNum;    /* history frame, the more, the smoother. defalut 2. Accuracy: U8 Range: [1, 16] */
    AX_U8        nCropRatio;          /* crop ratio for warped image, defalut 205. Accuracy: U0.8 Range: [127, 255] */
    AX_U8        nFramePosWeights[AX_ISP_DIS_POS_WEIGHT_SIZE]; /* Accuracy: U8.0 Range[0, 255], default [1, 1, ...], the weight of history frame position */
    AX_U32       nSadThreshold;       /*  u32, threshord for valid offset,defalut 0xFFFFFF. Accuracy: U32 Range: [0, 0xFFFFFFFF] */
} AX_ISP_IQ_DIS_V1_PARAM_T;

typedef struct {
    AX_U8                       bDisEnable;          /* EIS enable, 0: disable, 1:enable, default:0. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nDisType;            /* DIS type, Accuracy: U8.0 Range: [0, AX_ISP_IQ_DIS_TYPE_V1] */
    AX_ISP_IQ_DIS_V1_PARAM_T    tDisV1Param;         /* DIS version 1 */
} AX_ISP_IQ_DIS_PARAM_T;

/************************************************************************************
 *  ME Stat Info
 ************************************************************************************/
#define AX_ME_PROJ_NUM            (5)
#define AX_ME_PROJ_DIV_NUM     (5)
#define AX_ME_SAD_NUM              (256)

typedef struct {
    AX_U32 nSad[AX_ME_PROJ_NUM * AX_ME_PROJ_DIV_NUM][AX_ME_SAD_NUM]; /* Accuracy: U24.0 Range: [0, 16777215] */
    AX_S8  nOffset[AX_ME_PROJ_NUM][AX_ME_PROJ_DIV_NUM]; /* Accuracy: S7.0 Range: [-128, 127] */
} AX_ME_STAT_T;

typedef struct {
    AX_ME_STAT_T tMeHStat;
    AX_ME_STAT_T tMeVStat;
} AX_ISP_ME_STAT_INFO_T;

/************************************************************************************
 *  ME Param Config
 ************************************************************************************/
#define AX_ISP_ME_LUT_NUM   (33)
#define AX_ISP_ME_PROJ_SHIFT_BIT_DIRECTION_NUM   (2)

typedef struct {
    AX_U16 nRoiOffsetH;       /* Accuracy: U14.0, Range: [0, 4096], default 0 */
    AX_U16 nRoiOffsetV;       /* Accuracy: U14.0, Range: [0, 2160], default 0 */
    AX_U8  nRoiRegionNumH;    /* Accuracy: U3.0, Range: [1, 5], default 1 */
    AX_U8  nRoiRegionNumV;    /* Accuracy: U3.0, Range: [1, 5], default 1 */
    AX_U16 nRoiRegionSizeH;       /* Accuracy: U14.0, Range: [256, 4096], default 256 */
    AX_U16 nRoiRegionSizeV;       /* Accuracy: U14.0, Range: [256, 2160], default 256 */
    AX_U16  nSearchRange;      /* Accuracy: U9.0, Range: [0, 256], default 128 */
} AX_ISP_IQ_ME_STAT_ROI_T;

typedef struct {
    AX_U8  nEnable;               /* Accuracy: U1.0 Range: [0, 1] */
    AX_U8  nLutEnable;            /* Accuracy: U1.0 Range: [0, 1], default 0 */
    AX_U16 nLut[AX_ISP_ME_LUT_NUM];  /* Accuracy: U8.2 Range: [0, 1023] */
    AX_U8  nProjShiftBit[AX_ISP_ME_PROJ_SHIFT_BIT_DIRECTION_NUM];      /* Accuracy: U3.0 Range: [0, 4], [0]:H, [1]:V, default 0 */
    AX_U8  nScaleRatio;           /* Accuracy: U3.0 Range: [0, 2], default 0 */
    AX_ISP_IQ_ME_STAT_ROI_T tGridRoiH;
    AX_ISP_IQ_ME_STAT_ROI_T tGridRoiV;
} AX_ISP_IQ_ME_PARAM_T;


/************************************************************************************
 *  AE Stat Config
 ************************************************************************************/
#define AX_AE_GIRD_NUM              (2)
#define AX_AE_PREHDR_NUM            (2)
#define AX_AE_ENABLE_NUM            (2)
#define AX_AE_GRID0_ROW             (54)
#define AX_AE_GRID0_COL             (72)
#define AX_AE_GRID1_ROW             (256)
#define AX_AE_GRID1_COL             (8)
#define AX_AE_GRID_CHN              (4)
#define AX_AE_HIST_CHN              (4)
#define AX_AE_HIST_LOG_BIN          (64)
#define AX_AE_HIST_LINEAR_BIN       (256)
#define AX_AE_HIST_WEIGHT_BLK_ROW   (16)
#define AX_AE_HIST_WEIGHT_BLK_COL   (16)

typedef enum {
    AX_ISP_AE0_STAT_ITP_PREHDR_DPC  = 0,
    AX_ISP_AE0_STAT_IFE_PREHDR_BLC  = 1,
    AX_ISP_AE0_STAT_PREHDR_POS_MAX  = 2,
} AX_ISP_AE0_STAT_PREHDR_POS_E;

typedef enum {
    AX_ISP_AE1_STAT_ITP_PREHDR_DPC  = 0,
    AX_ISP_AE1_STAT_IFE_PREHDR_BLC  = 1,
    AX_ISP_AE1_STAT_PREHDR_POS_MAX  = 2
} AX_ISP_AE1_STAT_PREHDR_POS_E;

typedef enum {
    AX_ISP_AE_STAT_ITP_PSTHDR_LSC       = 0,    /* after lsc output ae stat data */
    AX_ISP_AE_STAT_ITP_PSTHDR_WBC       = 1,    /* after wbc output ae stat data */
    AX_ISP_AE_STAT_ITP_PSTHDR_POS_MAX   = 2
} AX_ISP_AE_STAT_PSTHDR_POS_E;

typedef struct {
    AX_ISP_AE0_STAT_PREHDR_POS_E eAe0StatPos;   /* AE0 stat position, default value is BPC0 used for long frame */
    AX_ISP_AE1_STAT_PREHDR_POS_E eAe1StatPos;   /* AE1 stat position default value is BPC1 used for shoft frame*/
} AX_ISP_AE_STAT_PREHDR_POS_T;

typedef struct {
    AX_ISP_AE_STAT_PSTHDR_POS_E ePstHdrPos;     /* pstHdr stat position */
    AX_ISP_AE_STAT_PREHDR_POS_T tPreHdrPos;     /* preHdr stat position */
} AX_ISP_IQ_AE_STAT_POS_T;

typedef struct {
    AX_U16 nRoiOffsetH;                         /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1 ] */
    AX_U16 nRoiOffsetV;                         /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max - 1] */
    AX_U16 nRoiRegionNumH;                      /* horiRegionNum, Accuracy: U7.0, Range: Grid0_Roi[1, 72], Grid1_Roi[1, 8] */
    AX_U16 nRoiRegionNumV;                      /* vertRegionNum, Accuracy: U10.0, Range: Grid0_Roi[1, 54], Grid1_Roi[1, 256] */
    AX_U16 nRoiRegionW;                         /* regionW, must be even, Accuracy: U16.0, nRoiOffsetH + (nRoiRegionNumH * nRoiRegionW) <= hsize */
    AX_U16 nRoiRegionH;                         /* regionH, must be even, Accuracy: U16.0, nRoiOffsetV + (nRoiRegionNumV * nRoiRegionH) <= vsize */
} AX_ISP_IQ_AE_STAT_ROI_T;

typedef struct {
    AX_U32 nRThr;                               /* AE RThr (YThr for nGridMode=0). Accuracy: pstHDR: U16.4 Range: [0, 2^20-1], preHDR: U12.4 Range: [0, 2^16-1] */
    AX_U32 nBThr;                               /* AE BThr. Accuracy: pstHDR: U16.4 Range: [0, 2^20-1], preHDR: U12.4 Range: [0, 2^16-1] */
    AX_U32 nGrThr;                              /* AE GrThr. Accuracy: pstHDR: U16.4 Range: [0, 2^20-1], preHDR: U12.4 Range: [0, 2^16-1] */
    AX_U32 nGbThr;                              /* AE GbThr. Accuracy: pstHDR: U16.4 Range: [0, 2^20-1], preHDR: U12.4 Range: [0, 2^16-1] */
} AX_ISP_IQ_AE_STAT_THR_T;

typedef struct {
    AX_U16 nRoiOffsetH;                         /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1] */
    AX_U16 nRoiOffsetV;                         /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max -1] */
    AX_U16 nRoiWidth;                           /* RoiWidth, Accuracy: U13.0, Range: [0, image_width_max], nRoiOffsetH + nRoiWidth <= hsize */
    AX_U16 nRoiHeight;                          /* RoiHeight, Accuracy: U10.0, Range: [0, image_high_max], nRoiOffsetV + nRoiHeight <= vsize  */
} AX_ISP_IQ_AE_HIST_ROI_T;

typedef struct {
    AX_U8                   nEnable[AX_AE_ENABLE_NUM];                                           /* nEnable[0]: ae stat prehdr enable, nEnable[1]: ae stat psthdr enable, Accuracy: U1.0 Range: [0, 1]*/
    AX_U8                   nIspGainEnable;                                                      /* nIspGainEnable 0: not need * isp_gain. nIspGainEnable 1 : need * isp_gain., Accuracy: U1.0 Range: [0, 1] */
    AX_U8                   nSkipNum;                                                            /* 3A STAT Skip Num. 0:no frame skip, 1:1/2 frame skip for 3a stat, 2:2/3 frame skip for 3a stat, Accuracy: U1.0 Range: [0, 2]  */
    AX_ISP_IQ_AE_STAT_POS_T tAeStatPos;                                                          /* ae stat position */
    AX_U8                   nGridMode[AX_AE_GIRD_NUM];                                           /* Grid Mode. 0: Y(1ch), 1: RGGB(4ch) Accuracy: U1.0 Range: [0, 1] */
    AX_U16                  nGridYcoeff[AX_AE_GRID_CHN];                                         /* Grid Ycoeff(R, Gr, Gb, B).  Accuracy: U0.12 Range: [0, 4095], sum <= 4096 */
    AX_U8                   nHistMode;                                                           /* Hist Mode. 0: Y(1ch), 1: YRGB(4ch), 2:RGGB(4ch) Accuracy: U2.0 Range: [0, 2] */
    AX_U8                   nHistLinearBinNum;                                                   /* Hist Bin Num. 0: 256, 1: 512, 2:1024, Accuracy: U2.0 Range: [0, 2], 1,2 is only available when nHistMode=0 */
    AX_U16                  nHistYcoeff[AX_AE_HIST_CHN];                                         /* Hist Ycoeff(R, Gr, Gb, B).  Accuracy: U0.12 Range: [0, 4095], sum <= 4096 */
    AX_U8                   nHistWeight[AX_AE_HIST_WEIGHT_BLK_ROW * AX_AE_HIST_WEIGHT_BLK_COL];  /* Hist Weight, Accuracy: U8.0, Range: [0, 255], 16 x 16 block */
    AX_ISP_IQ_AE_STAT_ROI_T tGridRoi[AX_AE_GIRD_NUM];
    AX_ISP_IQ_AE_STAT_THR_T tSatThr[AX_AE_GIRD_NUM];
    AX_ISP_IQ_AE_HIST_ROI_T tHistRoi;
} AX_ISP_IQ_AE_STAT_PARAM_T;

/************************************************************************************
 *  AE Stat Info
 ************************************************************************************/
typedef struct {
    AX_U32 nBin[AX_AE_HIST_CHN];                                        /* linear/log hist, Accuracy: U30.0, Range: [0, 2^30-1] */
} AX_AE_HIST_BIN_T;

typedef struct {
    AX_U8            nValid;                                            /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_AE_HIST_BIN_T nLinearHist[AX_AE_HIST_LINEAR_BIN];
    AX_AE_HIST_BIN_T nLogHist[AX_AE_HIST_LOG_BIN];
} AX_AE_HIST_STAT_T;

typedef struct {
    AX_U32 nGridSum[AX_AE_GRID_CHN];                                    /* pixel sum, Accuracy: preHDR: U28.4, pstHDR: U32, Range: [0, 2^32-1] */
    AX_U16 nGridNum[AX_AE_GRID_CHN];                                    /* pixel num, Accuracy: U16.0, Range: [0, 2^16-1] */
} AX_AE_GRID_STATS_T;

typedef struct {
    AX_U8              nValid;                                          /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_U8              nChnNum;                                         /* current channel number, 1: Y 4: RGGB or YRGB Accuracy: U3.0 Range: [0, 4] */
    AX_U8              nZoneRowSize;                                    /* vertRegionNum, must be even, Accuracy: U10.0, Range: [2, 54] */
    AX_U8              nZoneColSize;                                    /* horiRegionNum, must be even, Accuracy: U7.0, Range:[2, 72] */
    AX_U16             nOffsetH;                                        /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1] */
    AX_U16             nOffsetV;                                        /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max -1] */
    AX_U16             nGridWidth;                                      /* grid size width, must be even, Accuracy: U16.0, Range: [2, image_width_max] */
    AX_U16             nGridHeight;                                     /* grid size height, must be even, Accuracy: U16.0, Range: [2, image_high_max] */
    AX_AE_GRID_STATS_T tGridStats[AX_AE_GRID0_ROW * AX_AE_GRID0_COL];
} AX_AE_GRID_STAT0_T;

typedef struct {
    AX_U8              nValid;                                          /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_U8              nChnNum;                                         /* current channel number, 1: Y 4: RGGB or YRGB Accuracy: U3.0 Range: [0, 4] */
    AX_U16             nZoneRowSize;                                    /* vertRegionNum, must be even, Accuracy: U10.0, Range: [2, 512] */
    AX_U16             nZoneColSize;                                    /* horiRegionNum, must be even, Accuracy: U4.0, Range: [2, 8] */
    AX_U16             nOffsetH;                                        /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1] */
    AX_U16             nOffsetV;                                        /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max -1] */
    AX_U16             nGridWidth;                                      /* grid size width, must be even, Accuracy: U16.0, Range: [2, image_width_max] */
    AX_U16             nGridHeight;                                     /* grid size height, must be even, Accuracy: U16.0, Range: [2, image_high_max] */
    AX_AE_GRID_STATS_T tGridStats[AX_AE_GRID1_ROW * AX_AE_GRID1_COL];
} AX_AE_GRID_STAT1_T;

typedef struct {
    AX_U8                   nEnable;
    AX_AE_GRID_STAT0_T      tAeGrid0Stat;                               /* for AE */
    AX_AE_GRID_STAT1_T      tAeGrid1Stat;                               /* for Flicker , grid mode = 0 (1 ch, Y) */
    AX_AE_HIST_STAT_T       tAeHistStat;                                /* hist stat */
} AX_AE_GRID_HIST_STAT_T;

typedef struct {
    AX_U64                  nSeqNum;                                    /* frame seq num , Accuracy: U64.0, Range: [0, 2^64-1] */
    AX_U64                  nTimestamp;                                 /* frame timestamp , Accuracy: U64.0, Range: [0, 2^64-1] */
    AX_U64                  nUserData;                                  /* user data */
    AX_U32                  nSkipNum;                                   /* Algorithm running interval */
    AX_AE_GRID_HIST_STAT_T  tAeStatInfo0[AX_AE_PREHDR_NUM];             /* before hdr */
    AX_AE_GRID_HIST_STAT_T  tAeStatInfo1;                               /* after hdr */
} AX_ISP_AE_STAT_INFO_T;

/************************************************************************************
 *  AWB Stat Config
 ************************************************************************************/
#define AX_AWB_GRID_ROW                 (54)
#define AX_AWB_GRID_COL                 (72)
#define AX_AWB_GRID_CHN                 (4)
#define AX_AWB_GRID_LUMA_CHN            (4)
#define AX_AWB_GRID_LUMA_THR_NUM        (3)
#define AX_AWB_WIN_NUM                  (4)
#define AX_AWB_WIN_CHN                  (3)
#define AX_AWB_WIN_COORD_NUM            (2)
#define AX_AWB_WIN_BOUND_NUM            (2)
#define AX_AWB_WIN_LUMA_CHN             (3)
#define AX_AWB_WIN_LUMA_CHN_THR_NUM     (4)

typedef enum {
    AX_ISP_AWB_STAT_ITP_PSTHDR_LSC              = 0,                    /* pstHDR in ITP */
    AX_ISP_AWB_STAT_ITP_PSTHDR_RAW2DNR          = 1,                    /* pstHDR in ITP */
    AX_ISP_AWB_STAT_ITP_PREHDR_DPC_LONG_FRAME   = 2,                    /* preHDR in ITP */
    AX_ISP_AWB_STAT_ITP_PREHDR_DPC_SHORT_FRAME  = 3,                    /* preHDR in ITP */
    AX_ISP_AWB_STAT_ITP_POS_MAX                 = 4
} AX_ISP_IQ_AWB_STAT_POS_E;

typedef struct {
    AX_U16 nRoiOffsetH;                                                 /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max-1] */
    AX_U16 nRoiOffsetV;                                                 /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max-1] */
    AX_U16 nRoiRegionNumH;                                              /* Accuracy: U7.0, Range: [1, 72] */
    AX_U16 nRoiRegionNumV;                                              /* Accuracy: U6.0, Range: [1, 54] */
    AX_U16 nRoiRegionW;                                                 /* regionW, must be even, Accuracy: U10.0, Range: [16, 512], nRoiOffsetH + (nRoiRegionNumH * nRoiRegionW) <= hsize */
    AX_U16 nRoiRegionH;                                                 /* regionH, must be even, Accuracy: U10.0, Range: [16, 512], nRoiOffsetV + (nRoiRegionNumV * nRoiRegionH) <= vsize */
} AX_ISP_IQ_AWB_STAT_ROI_T;

typedef struct {
    AX_S32 nRThr;                                                       /* AWB RThr.  for tGridRoi(tLowThr/tHighThr): Accuracy: S16.4 Range: [-2^19, 2^19-1]   for tSatThr: Accuracy: U16.4 Range: [0, 2^20-1]  */
    AX_S32 nBThr;                                                       /* AWB BThr.  for tGridRoi(tLowThr/tHighThr): Accuracy: S16.4 Range: [-2^19, 2^19-1]   for tSatThr: Accuracy: U16.4 Range: [0, 2^20-1] */
    AX_S32 nGrThr;                                                      /* AWB GrThr. for tGridRoi(tLowThr/tHighThr): Accuracy: S16.4 Range: [-2^19, 2^19-1]   for tSatThr: Accuracy: U16.4 Range: [0, 2^20-1] */
    AX_S32 nGbThr;                                                      /* AWB GbThr. for tGridRoi(tLowThr/tHighThr): Accuracy: S16.4 Range: [-2^19, 2^19-1]   for tSatThr: Accuracy: U16.4 Range: [0, 2^20-1] */
    AX_S32 nYThr;                                                       /* AWB YThr.  for tGridRoi(tLowThr/tHighThr): Accuracy: S16.4 Range: [-2^19, 2^19-1]   for tSatThr: Accuracy: U16.4 Range: [0, 2^20-1] */
} AX_ISP_IQ_AWB_STAT_THR_T;

typedef struct {
    AX_ISP_IQ_AWB_STAT_THR_T tLowThr;
    AX_ISP_IQ_AWB_STAT_THR_T tHighThr;
    AX_S32                   nLumaThr[AX_AWB_GRID_LUMA_THR_NUM];        /* thr for luma slice . Accuracy: S16.4 Range: [-2^19, 2^19-1] */
} AX_ISP_IQ_AWB_GRID_THR_T;

typedef struct {
    AX_U16 nRoiOffsetH;                                                 /* ROI Offset(H) : U16 Range: [0, image_width_max - 1] */
    AX_U16 nRoiOffsetV;                                                 /* ROI Offset(V) : U16 Range: [0, image_width_max - 1] */
    AX_U16 nRoiSizeW;                                                   /* ROI Size(W) : U16 Range: [1, image_width_max ] */
    AX_U16 nRoiSizeH;                                                   /* ROI Size(H) : U16 Range: [1, image_width_max ] */
} AX_ISP_IQ_WIN_ROI_T;

typedef struct {
    AX_U16 nWbGain[AX_AWB_GRID_CHN];                                    /* NearGray WbGain (R, Gr, Gb, B) Accuracy: U4.8 Range: [0, 4095] */
    AX_S32 nWClip;                                                      /* NearGray WhiteClip Accuracy: S16.4 Range: [-2^19, 2^19 - 1] */
    AX_S16 nSlope[AX_AWB_WIN_COORD_NUM][AX_AWB_WIN_BOUND_NUM];          /* NearGray BoundarySlope 0: B-G(X) 1: R-G(Y) 0: upper bound 1: lower bound Accuracy: S8.4 Range: [-2^11, 2^11-1] */
    AX_S32 nOffset[AX_AWB_WIN_COORD_NUM][AX_AWB_WIN_BOUND_NUM];         /* NearGray BoundaryOffset 0: B-G(X) 1: R-G(Y) 0: upper bound 1: lower bound  Accuracy: S16.4 Range: [-2^19 2^19-1] */
} AX_ISP_IQ_WIN_NEAR_GRAY_T;

typedef struct {
    AX_U8                     nEnable;                                  /* AWB Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_U8                     nIspGainEnable;                           /* nIspGainEnable 0: not need * isp_gain. nIspGainEnable 1 : need * isp_gain., Accuracy: U1.0 Range: [0, 1] */
    AX_U8                     nSkipNum;                                 /* 3A STAT Skip Num. 0:no frame skip, 1:1/2 frame skip for 3a stat, 2:2/3 frame skip for 3a stat, Accuracy: U1.0 Range: [0, 2]  */
    AX_U8                     nGridMode;                                /* AWB GridMode. Accuracy: U2.0 Range: [0, 3], 0:RGB, 1:RGGB, 2: RGGB*Luma4ch, 3:RGGB*Luma2ch  */
    AX_ISP_IQ_AWB_STAT_POS_E  eAwbStatPos;                              /* AWB STAT position.Accuracy: U3.0 Range: [0, 2], 0:after LSC, 1:after RAW2DNR 3: after BPC0 4: after BPC1 */
    AX_U16                    nGridYcoeff[AX_AWB_GRID_CHN];             /* AWB Grid Ycoeff(R, Gr, Gb, B).  Accuracy: U0.12 Range: [0, 4095], sum <= 4096 */
    AX_ISP_IQ_AWB_STAT_ROI_T  tGridRoi;
    AX_ISP_IQ_AWB_STAT_THR_T  tSatThr;
    AX_ISP_IQ_AWB_GRID_THR_T  tGridThr;
    AX_ISP_IQ_WIN_ROI_T       tWinRoi[AX_AWB_WIN_NUM];
    AX_U16                    nWinYcoeff[AX_AWB_GRID_CHN];               /* AWB Win Ycoeff(R, Gr, Gb, B).  Accuracy: U0.12 Range: [0, 4095], sum <= 4096 */
    AX_S32                    nWinLumaThr[AX_AWB_WIN_LUMA_CHN_THR_NUM];  /* window luma thr. Accuracy: S16.4 Range: [-2^19, 2^19-1] */
    AX_ISP_IQ_WIN_NEAR_GRAY_T tWinNG;
} AX_ISP_IQ_AWB_STAT_PARAM_T;

/************************************************************************************
 *  AWB Stat Info
 ************************************************************************************/
typedef struct {
    AX_U64 nUnSatGridSum[AX_AWB_GRID_LUMA_CHN][AX_AWB_GRID_CHN];        /* not saturated pixel sum, Accuracy: U34.4, Range: [0, 2^38-1] */
    AX_U32 nUnSatGridNum[AX_AWB_GRID_LUMA_CHN];                         /* not saturated pixel num, Accuracy: U18 , Range: [0, 2^18-1] */
    AX_U64 nSatGridSum[AX_AWB_GRID_CHN];                                /* saturated pixel sum, Accuracy: U34.4, Range: [0, 2^38-1] */
    AX_U32 nSatGridNum;                                                 /* saturated pixel count, Accuracy: U18, Range: [0, 2^18-1] */
} AX_AWB_GRID_STATS_T;

typedef struct {
    AX_U64 nLSWinSum[AX_AWB_WIN_LUMA_CHN][AX_AWB_WIN_CHN];               /* luma sliced pixel sum, Accuracy: U38.4, Range: [0, 2^42 -1] */
    AX_U32 nLSWinNum[AX_AWB_WIN_LUMA_CHN];                               /* luma sliced pixel num, Accuracy: U22, Range: [0, 2^22-1] */
    AX_U64 nNGWinSum[AX_AWB_WIN_LUMA_CHN][AX_AWB_WIN_CHN];               /* neargray pixel sum, Accuracy: U38.4, 2^42-1] */
    AX_U32 nNGWinNum[AX_AWB_WIN_LUMA_CHN];                               /* neargray pixel num, Accuracy: U22, Range: [0, 2^22-1] */
} AX_AWB_WIN_STATS_T;

typedef struct {
    AX_U8               nValid;                                         /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_U8               nZoneRowSize;                                   /* vertRegionNum, must be even, Accuracy: U10.0, Range: [2, 54] */
    AX_U8               nZoneColSize;                                   /* horiRegionNum, must be even, Accuracy: U7.0, Range:[2, 72] */
    AX_U16              nOffsetH;                                       /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1] , used for 3A algorithm in small face exposure*/
    AX_U16              nOffsetV;                                       /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max -1] , used for 3A algorithm in small face exposure*/
    AX_U16              nGridWidth;                                     /* grid size width, must be even, Accuracy: U16.0, Range: [2, image_width_max], used for 3A algorithm in small face exposure */
    AX_U16              nGridHeight;                                    /* grid size height, must be even, Accuracy: U16.0, Range: [2, image_high_max], used for 3A algorithm in small face exposure */
    AX_AWB_GRID_STATS_T tAwbGridStats[AX_AWB_GRID_ROW * AX_AWB_GRID_COL];
} AX_AWB_GRID_STATS_INFO_T;

typedef struct {
    AX_U8               nValid;                                         /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_AWB_WIN_STATS_T  tAwbWinStats[AX_AWB_WIN_NUM];
} AX_AWB_WIN_STATS_INFO_T;

typedef struct {
    AX_U64                      nSeqNum;                                /* frame seq num , Accuracy: U64.0, Range: [0, 2^64-1]*/
    AX_U64                      nTimestamp;                             /* frame timestamp , Accuracy: U64.0, Range: [0, 2^64-1] */
    AX_U64                      nUserData;                              /* user data */
    AX_U32                      nSkipNum;                               /* Algorithm running interval , Accuracy: U32.0, Range: [0, 2^32-1] */
    AX_AWB_GRID_STATS_INFO_T    tAwbGridStats;
    AX_AWB_WIN_STATS_INFO_T     tAwbWinStats;
} AX_ISP_AWB_STAT_INFO_T;

/************************************************************************************
 *  AF Stat Config
 ************************************************************************************/
#define AX_AF_ROI_NUM_MAX               (180)
#define AX_AF_HSTAT_NUM                 (2)
#define AX_AF_VSTAT_NUM                 (2)
#define AX_AF_STAT_NUM                  (AX_AF_HSTAT_NUM + AX_AF_VSTAT_NUM)
#define AX_AF_GAMMA_LUT_NUM             (33)
#define AX_AF_WEIGHT_LUT_NUM            (8)
#define AX_AF_CORING_LUT_NUM            (16)
#define AX_AF_DRC_LUT_NUM               (17)
#define AX_AF_IIR_COEF_NUM              (10)
#define AX_AF_FIR_COEF_NUM              (13)
#define AX_AF_IIR_REF_LIST_SIZE         (32)

typedef enum {
    AX_ISP_AF_STAT_ITP_PSTHDR_RLTM              = 0,                    /* after itp rltm */
    AX_ISP_AF_STAT_ITP_PSTHDR_WBC               = 1,                    /* after itp wbc */
    AX_ISP_AF_STAT_ITP_PSTHDR_HDR               = 2,                    /* after itp hdr */
    AX_ISP_AF_STAT_ITP_PREHDR_DPC_LONG_FRAME    = 3,                    /* itp bpc long frame */
    AX_ISP_AF_STAT_ITP_PREHDR_DPC_SHORT_FRAME   = 4,                    /* itp bpc short frame */
    AX_ISP_AF_STAT_IFE_PREHDR_BLC_LONG_FRAME    = 5,                    /* ife long frame */
    AX_ISP_AF_STAT_IFE_PREHDR_BLC_SHORT_FRAME   = 6,                    /* ife short frame */
    AX_ISP_AF_STAT_POSITION_MAX
} AX_ISP_IQ_AF_STAT_POS_E;

typedef struct {
    AX_U8 nfv_mode[AX_AF_STAT_NUM];                                     /* Accuracy: U1, Range: [0, 1], 0:SUM, 1:PEAK */
} AX_ISP_IQ_AF_MODE_T;

typedef struct {
    AX_U16 nCoeffR;                                                     /* Accuracy: U0.12, Range: [0, 4095] */
    AX_U16 nCoeffGb;                                                    /* Accuracy: U0.12, Range: [0, 4095] */
    AX_U16 nCoeffGr;                                                    /* Accuracy: U0.12, Range: [0, 4095] */
    AX_U16 nCoeffB;                                                     /* Accuracy: U0.12, Range: [0, 4095], nCoeffR + nCoeffGr + nCoeffGb + nCoeffB <= 4096 */
} AX_ISP_IQ_AF_BAYER2Y_T;

typedef struct {
    AX_U8  nGammaEnable;                                                /* Accuracy: U1.0, Range: [0, 1], 0:Disable Gamma,   1:Enable.  */
    AX_U16 nGammaLut[AX_AF_GAMMA_LUT_NUM];                              /* Accuracy: U8.4, Range: [0, 4095] */
} AX_ISP_IQ_AF_GAMMA_T;

typedef struct {
    AX_U8 nScaleEnable;                                                 /* Accuracy: U1.0, Range: [0, 1], 0:Disable Downsample,   1:Enable Downsample.  */
    AX_U8 nScaleRatio;                                                  /* Accuracy: U3.0, Range: [1, 3], Downsample Ratio.   */
    AX_U8 nScaleWeight[AX_AF_WEIGHT_LUT_NUM];                           /* Accuracy: U1.7, Range: [0, 255] */
} AX_ISP_IQ_AF_DOWNSCALE_T;

typedef struct {
    AX_U32 nCoringThr[AX_AF_STAT_NUM];                                  /* Accuracy: U8.10, Range:[0, 2^18-1], suggest 18 numbers: {2^0, 2^1, ..., 2^17} */
    AX_U16 nCoringGain[AX_AF_STAT_NUM];                                 /* Accuracy: U5.7, Range:[0, 4095] */
    AX_U8  nCoringLut[AX_AF_STAT_NUM][AX_AF_CORING_LUT_NUM];            /* Accuracy: U5.0, Range[0, 31], nCoringLut[i] <= nCoringLut[i+1] */
    AX_U8  nCoringPixSumflg[AX_AF_STAT_NUM];                            /* Accuracy: U1.0, Range:[0, 1] */
} AX_ISP_IQ_AF_CORING_T;

typedef struct {
    AX_U8  nLdgRange[AX_AF_HSTAT_NUM];                                  /* Accuracy: U3.0, Range: [0, 2] */
    AX_U8  nLdgEnable[AX_AF_STAT_NUM];                                  /* Accuracy: U1.0, Range: [0, 1], 0:Disable, 1:Enable */
    AX_U8  nLdgSlope[AX_AF_STAT_NUM];                                   /* Accuracy: U0.8, Range: [0, 255] */
    AX_U8  nLdgLimit[AX_AF_STAT_NUM];                                   /* Accuracy: U1.7, Range: [0, 255] */
    AX_U16 nLdgThr[AX_AF_STAT_NUM];                                     /* Accuracy: U8.4, Range: [0, 4095] */
    AX_U16 nBrightPixNumThr[AX_AF_STAT_NUM];                            /* Accuracy: U8.4, Range: [0, 4095] */
} AX_ISP_IQ_AF_BRIGHTPIX_T;

typedef struct {
    AX_U16 nRoiOffsetH;                                                 /* Accuracy: U16.0, Range: [32, image_width_max -1], horiOffset, must be even */
    AX_U16 nRoiOffsetV;                                                 /* Accuracy: U16.0, Range: [16, image_high_max -1], vertOffset, must be even */
    AX_U16 nRoiRegionNumH;                                              /* Accuracy: U6.0, Range: [1, 20], horiRegionNum,(nRoiRegionNumH * nRoiRegionNumV ) % 4 == 0 */
    AX_U16 nRoiRegionNumV;                                              /* Accuracy: U6.0, Range: [1, 64], vertRegionNum, nRoiRegionNumH * nRoiRegionNumV <= 180 */
    AX_U16 nRoiRegionW;                                                 /* Accuracy: U11.0, Range: [16, 512], nRoiOffsetH + nRoiRegionNumH * nRoiRegionW <= hsize */
    AX_U16 nRoiRegionH;                                                 /* Accuracy: U11.0, Range: [8, 512], nRoiOffsetV + nRoiRegionNumV * nRoiRegionH <= vsize */
} AX_ISP_IQ_AF_ROI_T;

typedef struct {
    AX_U8 nFirEnable[AX_AF_HSTAT_NUM];                                  /* Accuracy: U1.0 Range: [0, 1] nFirEnable[0]: H1 nFirEnable[0]: H2 ,0:Disable FIR,  1:Enable FIR */
    AX_U8 nV1iirRefId;                                                  /* Accuracy: U6.0, Range:[0, 31] */
    AX_U8 nV2iirRefId;                                                  /* Accuracy: U6.0, Range:[0, 31] */
    AX_U8 nH1IirRefId;                                                  /* Accuracy: U6.0, Range:[0, 31] */
    AX_U8 nH2IirRefId;                                                  /* Accuracy: U6.0, Range:[0, 31]*/
} AX_ISP_IQ_AF_FLT_T;

typedef struct {
    AX_U8  nDrcEnable;                                                  /* Accuracy: U1.0 Range: [0, 1] 0:Disable Drc,  1:Enable Drc */
    AX_U16 nDrcLut[AX_AF_DRC_LUT_NUM];                                  /* Accuracy: U8.4, Range: [0, 4095] */
} AX_ISP_IQ_AF_DRC_T;

/* Luma Depend Curve for Spotlight or Noise Suppress */
typedef struct {
    AX_U8  nLumaLowStartTh;                                             /* Accuracy: U8.0,  Range:[0, 255]  nLumaLowStartTh <= nLumaLowEndTh <= nLumaHighStartTh <= nLumaHighEndTh */
    AX_U8  nLumaLowEndTh;                                               /* Accuracy: U8.0,  Range:[0, 255]  nLumaLowStartTh <= nLumaLowEndTh <= nLumaHighStartTh <= nLumaHighEndTh */
    AX_U8  nLumaHighStartTh;                                            /* Accuracy: U8.0,  Range:[0, 255] nLumaLowStartTh <= nLumaLowEndTh <= nLumaHighStartTh <= nLumaHighEndTh*/
    AX_U8  nLumaHighEndTh;                                              /* Accuracy: U8.0,  Range:[0, 255] nLumaLowStartTh <= nLumaLowEndTh <= nLumaHighStartTh <= nLumaHighEndTh */
    AX_U32 nLumaLowMinRatio;                                            /* Accuracy: U1.20, Range:[1, 0x200000] */
    AX_U32 nLumaHighMinRatio;                                           /* Accuracy: U1.20, Range:[1, 0x200000] */
}AX_ISP_IQ_AF_LUMA_SUPPRESS_USER_CURVE_T;

/* Spotlight or Noise Suppress */
typedef struct {
    AX_U8                                   nSuppressMode;              /* Accuracy: U1.0  Range:[0,   1] 0:UserDefine, 1:Ax Default */
    AX_ISP_IQ_AF_LUMA_SUPPRESS_USER_CURVE_T tLumaSuppressUserCurve;
}AX_ISP_IQ_AF_LUMA_SUPPRESS_T;

typedef struct {
    AX_U8 nSquareMode;                                                  /* Accuracy: U1.0 Range:[0, 1] 0: Linear mode, 1: Square mode*/
} AX_ISP_IQ_AF_PEAK_ENHANCE_T;

typedef struct {
    AX_U8                           nAfEnable;                          /* AF Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_AF_STAT_POS_E         eAfStatPos;
    AX_ISP_IQ_AF_MODE_T             tAfMode;
    AX_ISP_IQ_AF_BAYER2Y_T          tAfBayer2Y;
    AX_ISP_IQ_AF_GAMMA_T            tAfGamma;
    AX_ISP_IQ_AF_DOWNSCALE_T        tAfScaler;
    AX_ISP_IQ_AF_FLT_T              tAfFilter;
    AX_ISP_IQ_AF_CORING_T           tAfCoring;
    AX_ISP_IQ_AF_BRIGHTPIX_T        tAfBrightPix;
    AX_ISP_IQ_AF_DRC_T              tAfDrc;
    AX_ISP_IQ_AF_ROI_T              tAfRoi;
    AX_ISP_IQ_AF_LUMA_SUPPRESS_T    tAfLumaSuppress;
    AX_ISP_IQ_AF_PEAK_ENHANCE_T     tAfPeakEnhance;
} AX_ISP_IQ_AF_STAT_PARAM_T;

/* Bandpass Filter for Reference, with the Coefficients and Bandpass Info. */
typedef struct {
    AX_U32 nStartFreq;                                                  /* Accuracy:U1.20 Range:[1, 2^21-1] */
    AX_U32 nEndFreq;                                                    /* Accuracy:U1.20 Range:[1, 2^21-1] */
    AX_S16 nIirCoefList[AX_AF_IIR_COEF_NUM];                            /* Accuracy:S2.12, Range:[-16384, 16383]. */
} AX_ISP_IQ_AF_IIR_REF_T;

/* Frequently Used Bandpass Filter List for Reference.  */
typedef struct {
    AX_U8                  nV1iirRefNum;                                /* Accuracy: U7.0, Range:[1, 32] */
    AX_U8                  nV2iirRefNum;                                /* Accuracy: U7.0, Range:[1, 32] */
    AX_U8                  nH1IirRefNum;                                /* Accuracy: U7.0, Range:[1, 32] */
    AX_U8                  nH2IirRefNum;                                /* Accuracy: U7.0, Range:[1, 32] */
    AX_ISP_IQ_AF_IIR_REF_T tV1IirRefList[AX_AF_IIR_REF_LIST_SIZE];
    AX_ISP_IQ_AF_IIR_REF_T tV2IirRefList[AX_AF_IIR_REF_LIST_SIZE];
    AX_ISP_IQ_AF_IIR_REF_T tH1IirRefList[AX_AF_IIR_REF_LIST_SIZE];
    AX_ISP_IQ_AF_IIR_REF_T tH2IirRefList[AX_AF_IIR_REF_LIST_SIZE];
} AX_ISP_IQ_AF_IIR_REF_LIST_T;

/************************************************************************************
 *  AF Stat Info
 ************************************************************************************/
typedef struct {
    AX_U32 nPixSum;                                                     /* pixel sum (V1, V2, H1, H2) x180, Accuracy: U26.4, Range: [0, 2^30-1] */
    AX_U32 nPixCount;                                                   /* pixel count (V1, V2, H1, H2)x180, Accuracy: U18, Range: [0, 2^18-1] */
    AX_U64 nFocusValue;                                                 /* focus value  (V1, V2, H1, H2) x180, Accuracy: U31.10, Range: [0, 2^41-1] */
    AX_U32 nBrightPixCount;                                             /* high luma pixel num (V1, V2, H1, H2) x180, Accuracy: U18, Range: [0, 2^18-1] */
    AX_U64 nFocusValueLumaSuppress;                                     /* focus value luma suppress (V1, V2, H1, H2) x180, Accuracy: U32.30, Range: [0, 0x3fffffffffffffff] */
} AX_AF_GRID_STATS_T;

typedef struct {
    AX_U8               nValid;                                         /* current frame is valid for 3A algorithm, 1: AX_TRUE 0:AX_FALSE Accuracy: U1.0 Range: [0, 1] */
    AX_U8               nZoneRowSize;                                   /* vertRegionNum, Accuracy: U6.0, Range: [1, 64], (nZoneRowSize * nZoneColSize ) % 4 == 0  */
    AX_U8               nZoneColSize;                                   /* horiRegionNum, Accuracy: U6.0, Range: [1, 20], nZoneRowSize * nZoneColSize <= 180 */
    AX_U16              nOffsetH;                                       /* horiOffset, must be even, Accuracy: U16.0, Range: [0, image_width_max -1] */
    AX_U16              nOffsetV;                                       /* vertOffset, must be even, Accuracy: U16.0, Range: [0, image_high_max -1] */
    AX_U16              nGridWidth;                                     /* grid size width, must be even, Accuracy: U16.0, Range: [2, image_width_max] */
    AX_U16              nGridHeight;                                    /* grid size height, must be even, Accuracy: U16.0, Range: [2, image_high_max] */
    AX_AF_GRID_STATS_T  tAfRoiV1[AX_AF_ROI_NUM_MAX];
    AX_AF_GRID_STATS_T  tAfRoiV2[AX_AF_ROI_NUM_MAX];
    AX_AF_GRID_STATS_T  tAfRoiH1[AX_AF_ROI_NUM_MAX];
    AX_AF_GRID_STATS_T  tAfRoiH2[AX_AF_ROI_NUM_MAX];
} AX_AF_STATS_INFO_T;

typedef struct {
    AX_U64                  nSeqNum;                                    /* frame seq num , Accuracy: U64.0, Range: [0, 2^64-1]*/
    AX_U64                  nTimestamp;                                 /* frame timestamp , Accuracy: U64.0, Range: [0, 2^64-1] */
    AX_AF_STATS_INFO_T      tAfStatInfo;
} AX_ISP_AF_STAT_INFO_T;

typedef struct {
    AX_U8  nType;                                                      /* Types of converted data, 0: nFocusValue, 1: nFocusValueLumaSuppress */
    AX_F32 fCoeff;                                                     /* Conversion coefficient,  Accuracy:F32 Range: >= 1.0 */
    AX_U64 nMaxValueV1;                                                /* The maximum valueV during the focusing process, Accuracy:U31.10 Range:[0, 2^41-1] */
    AX_U64 nMaxValueV2;                                                /* The maximum valueV during the focusing process, Accuracy:U31.10 Range:[0, 2^41-1] */
    AX_U64 nMaxValueH1;                                                /* The maximum valueh1 during the focusing process, Accuracy:U31.10 Range:[0, 2^41-1] */
    AX_U64 nMaxValueH2;                                                /* The maximum valueh2 during the focusing process, Accuracy:U31.10 Range:[0, 2^41-1] */
    AX_ISP_AF_STAT_INFO_T* pStat;
} AX_ISP_AF_STAT_INFO_IN_T;

typedef struct {
    AX_U16  nFocusValueV1;                                              /* focus value in V1  Accuracy: U16, Range: [0, 2^16-1] */
    AX_U16  nFocusValueV2;                                              /* focus value in V2  Accuracy: U16, Range: [0, 2^16-1] */
    AX_U16  nFocusValueH1;                                              /* focus value in H1 Accuracy: U16, Range: [0, 2^16-1] */
    AX_U16  nFocusValueH2;                                              /* focus value in H2 Accuracy: U16, Range: [0, 2^16-1] */
} AX_ISP_AF_GRID_STAT_T;

typedef struct {
    AX_U8  nZoneRowSize;                                                /* vertRegionNum, Accuracy: U6.0, Range: [1, 64] */
    AX_U8  nZoneColSize;                                                /* horiRegionNum, Accuracy: U6.0, Range: [1, 20] */
    AX_ISP_AF_GRID_STAT_T tAfStatInfo[AX_AF_ROI_NUM_MAX];
} AX_ISP_AF_STAT_INFO_OUT_T;

/************************************************************************************
 *  SCENE IQ Param
 ************************************************************************************/
#define AX_ISP_AUTO_SCENE_MAX_NUM (4)

typedef enum {
    AX_AI_DISABLE   = 0,                                                         /* AI-ISP disable */
    AX_AI_ENABLE    = 1,                                                         /* AI-ISP enable */
} AX_ISP_AI_WORK_MODE_E;

typedef struct {
    AX_U8                          nAiWorkMode;                                 /* ainr work mode, Accuracy: U8.0 Range: [AX_AI_DISABLE, AX_AI_ENABLE] */
} AX_ISP_IQ_SCENE_MANUAL_T;

typedef struct {
    AX_U8   nSceneNum;                                                          /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_SCENE_MAX_NUM] */
    AX_U32  nDelta;                                                             /* Accuracy: U22.10 Range: [0, 2147483647] <= */
    AX_U32  nRefValStart[AX_ISP_AUTO_SCENE_MAX_NUM];                            /* Accuracy: U22.10 Range: [0, 4294967295] <= */
    AX_U32  nRefValEnd[AX_ISP_AUTO_SCENE_MAX_NUM];                              /* Accuracy: U22.10 Range: [0, 4294967295] <= */
    AX_U8   nAiWorkMode[AX_ISP_AUTO_SCENE_MAX_NUM];                           	/* ainr work mode, mapping reference AX_ISP_AI_WORK_MODE_E, Accuracy: U8.0 Range: [AX_AI_DISABLE, AX_AI_ENABLE] */
} AX_ISP_IQ_SCENE_AUTO_T;

typedef struct {
    AX_U8                      nAutoMode;                                      /* for auto or manual adjust mode. Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_SCENE_MANUAL_T   tManualParam;
    AX_ISP_IQ_SCENE_AUTO_T     tAutoParam;
} AX_ISP_IQ_SCENE_PARAM_T;

/************************************************************************************
 *  sensor default Param
 ************************************************************************************/
typedef struct _AX_SENSOR_DEFAULT_PARAM_T_ {
    AX_ISP_IQ_DPC_PARAM_T       *ptDpc;
    AX_ISP_IQ_BLC_PARAM_T       *ptBlc;

    AX_ISP_IQ_LSC_PARAM_T       *ptLsc;
    AX_ISP_IQ_WB_GAIN_PARAM_T   *ptWbGain;
    AX_ISP_IQ_RLTM_PARAM_T      *ptRltm;
    AX_ISP_IQ_DEHAZE_PARAM_T    *ptDehaze;
    AX_ISP_IQ_DEMOSAIC_PARAM_T  *ptDemosaic;
    AX_ISP_IQ_FCC_PARAM_T       *ptFcc;
    AX_ISP_IQ_GIC_PARAM_T       *ptGic;
    AX_ISP_IQ_CC_PARAM_T        *ptCc;
    AX_ISP_IQ_GAMMA_PARAM_T     *ptGamma;
    AX_ISP_IQ_CA_PARAM_T        *ptCa;
    AX_ISP_IQ_CSC_PARAM_T       *ptCsc;
    AX_ISP_IQ_DEPURPLE_PARAM_T  *ptDepurple;
    AX_ISP_IQ_HDR_PARAM_T       *ptHdr;
    AX_ISP_IQ_SHARPEN_PARAM_T   *ptSharpen;
    AX_ISP_IQ_SCM_PARAM_T       *ptScm;
    AX_ISP_IQ_YNR_PARAM_T       *ptYnr;
    AX_ISP_IQ_YCPROC_PARAM_T    *ptYcproc;
    AX_ISP_IQ_CNR_PARAM_T       *ptCnr;
    AX_ISP_IQ_CCMP_PARAM_T      *ptCcmp;
    AX_ISP_IQ_HS2DLUT_PARAM_T   *ptHs2dlut;
    AX_ISP_IQ_YCRT_PARAM_T      *ptYcrt;
    AX_ISP_IQ_CLP_PARAM_T       *ptClp;
    AX_ISP_IQ_RAW2DNR_PARAM_T   *ptRaw2dnr;
    AX_ISP_IQ_AINR_PARAM_T      *ptAinr;
    AX_ISP_IQ_YUV3DNR_PARAM_T   *ptYuv3dnr;
    AX_ISP_IQ_DIS_PARAM_T       *ptDis;
    AX_ISP_IQ_ME_PARAM_T        *pIspMeParam;
    AX_ISP_IQ_SCENE_PARAM_T     *ptScene;

    AX_ISP_IQ_LDC_PARAM_T       *ptLdc;
    AX_ISP_IQ_NUC_PARAM_T       *ptNuc;
} AX_SENSOR_DEFAULT_PARAM_T;

/************************************************************************************
 *  ISP IQ API
 ************************************************************************************/
AX_S32 AX_ISP_IQ_SetBlcParam(AX_U8 nPipeId, AX_ISP_IQ_BLC_PARAM_T *pIspBlcParam);
AX_S32 AX_ISP_IQ_GetBlcParam(AX_U8 nPipeId, AX_ISP_IQ_BLC_PARAM_T *pIspBlcParam);

AX_S32 AX_ISP_IQ_SetDrcEnable(AX_U8 nPipeId, AX_BOOL bEnable);
AX_S32 AX_ISP_IQ_GetDrcEnable(AX_U8 nPipeId, AX_BOOL *pbEnable);

AX_S32 AX_ISP_IQ_SetNucParam(AX_U8 nPipeId, AX_ISP_IQ_NUC_PARAM_T *pIspNucParam);
AX_S32 AX_ISP_IQ_GetNucParam(AX_U8 nPipeId, AX_ISP_IQ_NUC_PARAM_T *pIspNucParam);

AX_S32 AX_ISP_IQ_SetDpcParam(AX_U8 nPipeId, AX_ISP_IQ_DPC_PARAM_T *pIspDpcParam);
AX_S32 AX_ISP_IQ_GetDpcParam(AX_U8 nPipeId, AX_ISP_IQ_DPC_PARAM_T *pIspDpcParam);

AX_S32 AX_ISP_IQ_SetHdrParam(AX_U8 nPipeId, AX_ISP_IQ_HDR_PARAM_T *pIspHdrParam);
AX_S32 AX_ISP_IQ_GetHdrParam(AX_U8 nPipeId, AX_ISP_IQ_HDR_PARAM_T *pIspHdrParam);

AX_S32 AX_ISP_IQ_SetAinrParam(AX_U8 nPipeId, AX_ISP_IQ_AINR_PARAM_T *pIspAinrParam);
AX_S32 AX_ISP_IQ_GetAinrParam(AX_U8 nPipeId, AX_ISP_IQ_AINR_PARAM_T *pIspAinrParam);
AX_S32 AX_ISP_IQ_GetAinrCapability(AX_U8 nPipeId, AX_ISP_IQ_AINR_CAP_TABLE_T *pIspAiNrCapability);
AX_S32 AX_ISP_IQ_SetAinrLevel(AX_U8 nPipeId, AX_ISP_IQ_AINR_LEVEL_T *pIspAinrLevel);
AX_S32 AX_ISP_IQ_GetAinrLevel(AX_U8 nPipeId, AX_ISP_IQ_AINR_LEVEL_T *pIspAinrLevel);

AX_S32 AX_ISP_IQ_SetRaw2dnrParam(AX_U8 nPipeId, AX_ISP_IQ_RAW2DNR_PARAM_T *pIspRaw2dnrParam);
AX_S32 AX_ISP_IQ_GetRaw2dnrParam(AX_U8 nPipeId, AX_ISP_IQ_RAW2DNR_PARAM_T *pIspRaw2dnrParam);

AX_S32 AX_ISP_IQ_SetLscParam(AX_U8 nPipeId, AX_ISP_IQ_LSC_PARAM_T *pIspLscParam);
AX_S32 AX_ISP_IQ_GetLscParam(AX_U8 nPipeId, AX_ISP_IQ_LSC_PARAM_T *pIspLscParam);

AX_S32 AX_ISP_IQ_SetWbGainParam(AX_U8 nPipeId, AX_ISP_IQ_WB_GAIN_PARAM_T *pIspWbGainParam);
AX_S32 AX_ISP_IQ_GetWbGainParam(AX_U8 nPipeId, AX_ISP_IQ_WB_GAIN_PARAM_T *pIspWbGainParam);

AX_S32 AX_ISP_IQ_SetRltmParam(AX_U8 nPipeId, AX_ISP_IQ_RLTM_PARAM_T *pIspRltmParam);
AX_S32 AX_ISP_IQ_GetRltmParam(AX_U8 nPipeId, AX_ISP_IQ_RLTM_PARAM_T *pIspRltmParam);
AX_S32 AX_ISP_IQ_BuildBasicScurve(AX_U8 nPipeId, AX_ISP_IQ_SCURVE_PARAM_T *pIspScurveParam,
                                    AX_U16 nSCurveList[AX_ISP_RLTM_SCURVE_MAX_LEN]);

AX_S32 AX_ISP_IQ_SetDemosaicParam(AX_U8 nPipeId, AX_ISP_IQ_DEMOSAIC_PARAM_T *pIspDemosaicParam);
AX_S32 AX_ISP_IQ_GetDemosaicParam(AX_U8 nPipeId, AX_ISP_IQ_DEMOSAIC_PARAM_T *pIspDemosaicParam);

AX_S32 AX_ISP_IQ_SetFccParam(AX_U8 nPipeId, AX_ISP_IQ_FCC_PARAM_T *pIspFccParam);
AX_S32 AX_ISP_IQ_GetFccParam(AX_U8 nPipeId, AX_ISP_IQ_FCC_PARAM_T *pIspFccParam);

AX_S32 AX_ISP_IQ_SetGicParam(AX_U8 nPipeId, AX_ISP_IQ_GIC_PARAM_T *pIspGicParam);
AX_S32 AX_ISP_IQ_GetGicParam(AX_U8 nPipeId, AX_ISP_IQ_GIC_PARAM_T *pIspGicParam);

AX_S32 AX_ISP_IQ_SetDepurpleParam(AX_U8 nPipeId, AX_ISP_IQ_DEPURPLE_PARAM_T *pIspDepurpleParam);
AX_S32 AX_ISP_IQ_GetDepurpleParam(AX_U8 nPipeId, AX_ISP_IQ_DEPURPLE_PARAM_T *pIspDepurpleParam);

AX_S32 AX_ISP_IQ_SetCcParam(AX_U8 nPipeId, AX_ISP_IQ_CC_PARAM_T *pIspCcParam);
AX_S32 AX_ISP_IQ_GetCcParam(AX_U8 nPipeId, AX_ISP_IQ_CC_PARAM_T *pIspCcParam);

AX_S32 AX_ISP_IQ_SetGammaParam(AX_U8 nPipeId, AX_ISP_IQ_GAMMA_PARAM_T *pIspGammaParam);
AX_S32 AX_ISP_IQ_GetGammaParam(AX_U8 nPipeId, AX_ISP_IQ_GAMMA_PARAM_T *pIspGammaParam);

AX_S32 AX_ISP_IQ_SetDehazeParam(AX_U8 nPipeId, AX_ISP_IQ_DEHAZE_PARAM_T *pIspDehazeParam);
AX_S32 AX_ISP_IQ_GetDehazeParam(AX_U8 nPipeId, AX_ISP_IQ_DEHAZE_PARAM_T *pIspDehazeParam);

AX_S32 AX_ISP_IQ_SetCscParam(AX_U8 nPipeId, AX_ISP_IQ_CSC_PARAM_T *pIspCscParam);
AX_S32 AX_ISP_IQ_GetCscParam(AX_U8 nPipeId, AX_ISP_IQ_CSC_PARAM_T *pIspCscParam);

AX_S32 AX_ISP_IQ_SetCaParam(AX_U8 nPipeId, AX_ISP_IQ_CA_PARAM_T *pIspCaParam);
AX_S32 AX_ISP_IQ_GetCaParam(AX_U8 nPipeId, AX_ISP_IQ_CA_PARAM_T *pIspCaParam);

AX_S32 AX_ISP_IQ_SetClpParam(AX_U8 nPipeId, AX_ISP_IQ_CLP_PARAM_T *pIspClpParam);
AX_S32 AX_ISP_IQ_GetClpParam(AX_U8 nPipeId, AX_ISP_IQ_CLP_PARAM_T *pIspClpParam);

AX_S32 AX_ISP_IQ_SetYuv3dnrParam(AX_U8 nPipeId, AX_ISP_IQ_YUV3DNR_PARAM_T *pIspYuv3dnrParam);
AX_S32 AX_ISP_IQ_GetYuv3dnrParam(AX_U8 nPipeId, AX_ISP_IQ_YUV3DNR_PARAM_T *pIspYuv3dnrParam);

AX_S32 AX_ISP_IQ_SetShpParam(AX_U8 nPipeId, AX_ISP_IQ_SHARPEN_PARAM_T *pIspShpParam);
AX_S32 AX_ISP_IQ_GetShpParam(AX_U8 nPipeId, AX_ISP_IQ_SHARPEN_PARAM_T *pIspShpParam);

AX_S32 AX_ISP_IQ_SetYnrParam(AX_U8 nPipeId, AX_ISP_IQ_YNR_PARAM_T *pIspYnrParam);
AX_S32 AX_ISP_IQ_GetYnrParam(AX_U8 nPipeId, AX_ISP_IQ_YNR_PARAM_T *pIspYnrParam);

AX_S32 AX_ISP_IQ_SetCnrParam(AX_U8 nPipeId, AX_ISP_IQ_CNR_PARAM_T *pIspCnrParam);
AX_S32 AX_ISP_IQ_GetCnrParam(AX_U8 nPipeId, AX_ISP_IQ_CNR_PARAM_T *pIspCnrParam);

AX_S32 AX_ISP_IQ_SetScmParam(AX_U8 nPipeId, AX_ISP_IQ_SCM_PARAM_T *pIspScmParam);
AX_S32 AX_ISP_IQ_GetScmParam(AX_U8 nPipeId, AX_ISP_IQ_SCM_PARAM_T *pIspScmParam);

AX_S32 AX_ISP_IQ_SetHs2dlutParam(AX_U8 nPipeId, AX_ISP_IQ_HS2DLUT_PARAM_T *pIspHs2dlutParam);
AX_S32 AX_ISP_IQ_GetHs2dlutParam(AX_U8 nPipeId, AX_ISP_IQ_HS2DLUT_PARAM_T *pIspHs2dlutParam);

AX_S32 AX_ISP_IQ_SetYcprocParam(AX_U8 nPipeId, AX_ISP_IQ_YCPROC_PARAM_T *pIspYcprocParam);
AX_S32 AX_ISP_IQ_GetYcprocParam(AX_U8 nPipeId, AX_ISP_IQ_YCPROC_PARAM_T *pIspYcprocParam);

AX_S32 AX_ISP_IQ_SetCcmpParam(AX_U8 nPipeId, AX_ISP_IQ_CCMP_PARAM_T *pIspCcmpParam);
AX_S32 AX_ISP_IQ_GetCcmpParam(AX_U8 nPipeId, AX_ISP_IQ_CCMP_PARAM_T *pIspCcmpParam);

AX_S32 AX_ISP_IQ_SetYcrtParam(AX_U8 nPipeId, AX_ISP_IQ_YCRT_PARAM_T *pIspYcrtParam);
AX_S32 AX_ISP_IQ_GetYcrtParam(AX_U8 nPipeId, AX_ISP_IQ_YCRT_PARAM_T *pIspYcrtParam);

AX_S32 AX_ISP_IQ_SetLdcParam(AX_U8 nPipeId, AX_ISP_IQ_LDC_PARAM_T *pIspLDCParam);
AX_S32 AX_ISP_IQ_GetLdcParam(AX_U8 nPipeId, AX_ISP_IQ_LDC_PARAM_T *pIspLDCParam);

AX_S32 AX_ISP_IQ_SetDisParam(AX_U8 nPipeId, AX_ISP_IQ_DIS_PARAM_T *pIspDisParam);
AX_S32 AX_ISP_IQ_GetDisParam(AX_U8 nPipeId, AX_ISP_IQ_DIS_PARAM_T *pIspDisParam);

AX_S32 AX_ISP_IQ_SetMeParam(AX_U8 nPipeId, AX_ISP_IQ_ME_PARAM_T *pIspMeParam);
AX_S32 AX_ISP_IQ_GetMeParam(AX_U8 nPipeId, AX_ISP_IQ_ME_PARAM_T *pIspMeParam);
AX_S32 AX_ISP_IQ_GetMeStatInfo(AX_U8 nPipeId, AX_ISP_ME_STAT_INFO_T *pIspMeStatInfo);

AX_S32 AX_ISP_IQ_SetAeStatParam(AX_U8 nPipeId, const AX_ISP_IQ_AE_STAT_PARAM_T *pAeStatParam);
AX_S32 AX_ISP_IQ_GetAeStatParam(AX_U8 nPipeId, AX_ISP_IQ_AE_STAT_PARAM_T *pAeStatParam);

AX_S32 AX_ISP_IQ_SetAwbStatParam(AX_U8 nPipeId, const AX_ISP_IQ_AWB_STAT_PARAM_T *pAwbStatParam);
AX_S32 AX_ISP_IQ_GetAwbStatParam(AX_U8 nPipeId, AX_ISP_IQ_AWB_STAT_PARAM_T *pAwbStatParam);

AX_S32 AX_ISP_IQ_SetAfStatParam(AX_U8 nPipeId, const AX_ISP_IQ_AF_STAT_PARAM_T *pAfStatParam);
AX_S32 AX_ISP_IQ_GetAfStatParam(AX_U8 nPipeId, AX_ISP_IQ_AF_STAT_PARAM_T *pAfStatParam);

AX_S32 AX_ISP_IQ_SetAfIirRefList(AX_U8 nPipeId, const AX_ISP_IQ_AF_IIR_REF_LIST_T *pIirRefList);
AX_S32 AX_ISP_IQ_GetAfIirRefList(AX_U8 nPipeId, AX_ISP_IQ_AF_IIR_REF_LIST_T *pIirRefList);

AX_S32 AX_ISP_IQ_GetAeStatistics(AX_U8 nPipeId, AX_ISP_AE_STAT_INFO_T *pAeStat);
AX_S32 AX_ISP_IQ_GetAwbStatistics(AX_U8 nPipeId, AX_ISP_AWB_STAT_INFO_T *pAwbStat);
AX_S32 AX_ISP_IQ_GetAfStatistics(AX_U8 nPipeId, AX_ISP_AF_STAT_INFO_T *pAfStat);

AX_S32 AX_ISP_IQ_FocusValueConvert(AX_U8 nPipeId, const AX_ISP_AF_STAT_INFO_IN_T *pAfStatIn, AX_ISP_AF_STAT_INFO_OUT_T *pFvStat);

AX_S32 AX_ISP_IQ_SetSceneParam(AX_U8 nPipeId, AX_ISP_IQ_SCENE_PARAM_T *pIspSceneParam);
AX_S32 AX_ISP_IQ_GetSceneParam(AX_U8 nPipeId, AX_ISP_IQ_SCENE_PARAM_T *pIspSceneParam);

#ifdef __cplusplus
}
#endif
#endif  //_AX_ISP_IQ_API_H_

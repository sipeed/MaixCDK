/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_VO_H__
#define __COMMON_VO_H__

#include "ax_sys_api.h"
#include "ax_base_type.h"
#include "ax_vo_api.h"

#ifndef SAMPLE_PRT
#define SAMPLE_PRT(fmt,...)   \
{ \
    printf("[SAMPLE-VO][%s-%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(x, a)           ((((x) + ((a) - 1)) / a) * a)
#endif
#define ALIGN_DOWN(x, a)         (((x) / (a)) * (a))

#define SAMPLE_VO_DEV0 0
#define SAMPLE_VO_DEV1 1
#define SAMPLE_VO_DEV_MAX 2
#define SAMPLE_FB_PER_DEV_MAX   2
#define SAMPLE_VO_FRAME_MAX 50
#define SAMPLE_VO_SYNC_USER_MAX 4

#define VO_FILE_BLOCK_SIZE 1024
//#define VO_PATH_LEN (64)
#define VO_NAME_LEN (128)

typedef enum axSAMPLE_VO_MODE_E {
    VO_MODE_1MUX = 0,
    VO_MODE_2MUX = 1,
    VO_MODE_4MUX = 2,
    VO_MODE_8MUX = 3,
    VO_MODE_9MUX = 4,
    VO_MODE_16MUX = 5,
    VO_MODE_25MUX = 6,
    VO_MODE_36MUX = 7,
    VO_MODE_49MUX = 8,
    VO_MODE_64MUX = 9,
    VO_MODE_BUTT
} SAMPLE_VO_MODE_E;

typedef struct axSAMPLE_COMM_VO_LAYER_CONFIG_S {
    AX_S32                   s32InitFlag;

    /* for layer */
    VO_LAYER                 u32VoLayer;
    AX_U32                   u32LayerPoolId;
    VO_DEV                   bindVoDev[SAMPLE_VO_DEV_MAX];
    AX_VO_VIDEO_LAYER_ATTR_T stVoLayerAttr;
    AX_S32                   s32BatchProcFlag;

    /* for chnnel */
    SAMPLE_VO_MODE_E         enVoMode;
    AX_IMG_FORMAT_E          enChnFrmFmt;
    AX_U32                   u32ChnFrameRate;
    AX_U32                   u32ChnFrameNr;
    AX_U32                   u32ChnFrameOut;
    AX_U32                   u32FifoDepth;
    AX_U32                   u32ChnPoolId;
    AX_FRAME_COMPRESS_INFO_T chnCompressInfo;
    AX_CHAR                  chnFileName[VO_NAME_LEN];
    AX_U64                   u64KeepChnPrevFrameBitmap0;
    AX_U64                   u64KeepChnPrevFrameBitmap1;
    AX_U32                   u32ChnNr;

    /* for sys link */
    AX_MOD_INFO_T            stSrcMod;
    AX_MOD_INFO_T            stDstMod;
} SAMPLE_VO_LAYER_CONFIG_S;

typedef struct axSAMPLE_FB_CONFIG_S {
    AX_S32                   u32Index;

    AX_U32                   u32ResoW;
    AX_U32                   u32ResoH;
    AX_U32                   u32Fmt;

    AX_U32                   u32ColorKeyEn;
    AX_U32                   u32ColorKeyInv;
    AX_U32                   u32ColorKey;
} SAMPLE_FB_CONFIG_S;

typedef struct axSAMPLE_COMM_VO_GRAPHIC_CONFIG_S {
    AX_S32                   s32InitFlag;

    AX_U32                   u32FbNum;
    SAMPLE_FB_CONFIG_S       stFbConf[SAMPLE_FB_PER_DEV_MAX];

    VO_DEV                   bindVoDev;
} SAMPLE_VO_GRAPHIC_CONFIG_S;

typedef struct axSAMPLE_COMM_VO_CURSOR_CONFIG_S {
    AX_S32                   s32InitFlag;
    AX_U32                   u32CursorLayerEn;
    VO_DEV                   bindVoDev;
    AX_U32                   u32FBIndex;
    AX_U32                   u32X;
    AX_U32                   u32Y;
    AX_U32                   u32Width;
    AX_U32                   u32Height;
} SAMPLE_VO_CURSOR_CONFIG_S;

typedef struct axSAMPLE_COMM_VO_DEV_CONFIG_S {
    AX_S32                  s32InitFlag;

    VO_DEV                  u32VoDev;
    AX_VO_MODE_E            enMode;
    AX_VO_INTF_TYPE_E       enVoIntfType;
    AX_VO_INTF_SYNC_E       enIntfSync;
    AX_VO_OUT_FMT_E         enVoOutfmt;
    AX_U32                  u32SyncIndex;

    AX_BOOL                 setCsc;
    AX_VO_CSC_T             vo_csc;

    AX_BOOL                 bWbcEn;
    AX_U32                  u32WbcFrmaeNr;
    AX_VO_WBC_ATTR_T        stWbcAttr;
} SAMPLE_VO_DEV_CONFIG_S;

typedef struct axSAMPLE_VO_CONFIG_S {
    AX_U32                  u32BindMode;  /* 0:SINGLE TO SINGLE, 1: SINGLE TO MULTI */
    AX_U32                  u32FifoDepth;

    /* for vo device */
    AX_U32                  u32VDevNr;
    SAMPLE_VO_DEV_CONFIG_S  stVoDev[SAMPLE_VO_DEV_MAX];

    AX_U32                  u32LayerNr;

    /* for video layer */
    SAMPLE_VO_LAYER_CONFIG_S stVoLayer[SAMPLE_VO_DEV_MAX];

    /* for graphic layer */
    SAMPLE_VO_GRAPHIC_CONFIG_S stGraphicLayer[SAMPLE_VO_DEV_MAX];

    /* for cursor layer */
    SAMPLE_VO_CURSOR_CONFIG_S stCursorLayer;
} SAMPLE_VO_CONFIG_S;

AX_S32 SAMPLE_COMM_VO_StartDev(SAMPLE_VO_DEV_CONFIG_S *pstVoDevConf);
AX_S32 SAMPLE_COMM_VO_StopDev(SAMPLE_VO_DEV_CONFIG_S *pstVoDevConf);

AX_S32 SAMPLE_COMM_VO_StartLayer(SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf);
AX_S32 SAMPLE_COMM_VO_StopLayer(SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf);

AX_S32 SAMPLE_COMM_VO_StartChn(SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf);
AX_S32 SAMPLE_COMM_VO_StopChn(SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf);

AX_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig);
AX_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig);

AX_S32 SAMPLE_VO_WIN_INFO(AX_U32 u32LayerWidth, AX_U32 u32LayerHeight, SAMPLE_VO_MODE_E enMode, AX_U32 *u32Row,
                          AX_U32 *u32Col, AX_U32 *u32Width, AX_U32 *u32Height);

#endif
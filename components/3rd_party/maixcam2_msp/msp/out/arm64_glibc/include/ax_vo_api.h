/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_VO_H_
#define _AX_VO_H_

#include "ax_global_type.h"
#include "ax_sys_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AX_VO_WIDTH_MAX         (4096)
#define AX_VO_HEIGHT_MAX        (4096)
#define AX_VO_DEV_MAX           (2)
#define AX_VO_LAYER_MAX         (32)
#define AX_VO_CHN_MAX           (16)
#define AX_VO_MODE_MAX          (128)
#define AX_VO_GRAPH_LAYER_MAX   (2)

#define AX_VO_LUMA_MAX          (100)
#define AX_VO_CONTRAST_MAX      (100)
#define AX_VO_HUE_MAX           (100)
#define AX_VO_SATUATURE_MAX     (100)

#define AX_ID_VO_PUB_SMOD       (0x8A)
#define AX_ERR_VO_INVALID_DEVID               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_INVALID_DEVID)
#define AX_ERR_VO_INVALID_CHNID               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_INVALID_CHNID)
#define AX_ERR_VO_ILLEGAL_PARAM               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_VO_NULL_PTR                    AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NULL_PTR)
#define AX_ERR_VO_NOT_PERM                    AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NOT_PERM)
#define AX_ERR_VO_NODEV                       AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_UNEXIST)
#define AX_ERR_VO_NOMEM                       AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NOMEM)

#define AX_ERR_VO_DEV_OP_FAIL                 AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x80)
#define AX_ERR_VO_LAYER_OP_FAIL               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x81)
#define AX_ERR_VO_CHN_OP_FAIL                 AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x82)
#define AX_ERR_VO_INVALID_LAYERID             AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x83)
#define AX_ERR_VO_OPEN_FILE_FAIL              AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x84)
#define AX_ERR_VO_WBC_OP_FAIL                 AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x85)
#define AX_ERR_VO_WBC_INVALID_FD              AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x86)
#define AX_ERR_VO_HDMI_OP_FAIL                AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x87)
#define AX_ERR_VO_MODE_NOT_FOUND              AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x89)

typedef AX_U32 VO_DEV;
typedef AX_U32 VO_LAYER;
typedef AX_U32 GRAPHIC_LAYER;
typedef AX_U32 VO_CHN;
typedef AX_S32 VO_WBC;
typedef AX_COLORKEY_T AX_FB_COLORKEY_T;

/* fb ioctls  0x46 is 'F'	*/
#define AX_FBIOPUT_CURSOR_POS   _IOW('F', 0x21, AX_FB_CURSOR_POS_T)
#define AX_FBIOPUT_CURSOR_RES   _IOW('F', 0x22, AX_FB_CURSOR_RES_T)
#define AX_FBIOPUT_CURSOR_SHOW  _IOW('F', 0x23, AX_U16)
#define AX_FBIOPUT_ALPHA        _IOW('F', 0x24, AX_U32)
#define AX_FBIOGET_CURSORINFO   _IOR('F', 0x25, AX_FB_CURSOR_INFO_T)
#define AX_FBIOGET_TYPE         _IOR('F', 0x26, AX_U16)
#define AX_FBIOGET_COLORKEY     _IOR('F', 0x27, AX_FB_COLORKEY_T)
#define AX_FBIOPUT_COLORKEY     _IOW('F', 0x28, AX_FB_COLORKEY_T)

typedef enum {
    AX_VO_MODE_OFFLINE,
    AX_VO_MODE_ONLINE,
    AX_VO_MODE_BUTT
} AX_VO_MODE_E;

typedef enum {
    AX_VO_INTF_DPI,
    AX_VO_INTF_BT601,
    AX_VO_INTF_BT656,
    AX_VO_INTF_BT1120,
    AX_VO_INTF_DSI,
    AX_VO_INTF_LVDS,
    AX_VO_INTF_BUTT
} AX_VO_INTF_TYPE_E;

typedef enum {
    AX_VO_OUTPUT_576P50,                /* 720  x  576 at 50 Hz */
    AX_VO_OUTPUT_576I50,                /* 720  x  576 at 50 Hz, interlace. */
    AX_VO_OUTPUT_480P60,                /* 720  x  480 at 60 Hz */
    AX_VO_OUTPUT_480I60,                /* 720  x  480 at 60 Hz, interlace. */
    AX_VO_OUTPUT_720P25,                /* 1280 x  720 at 25 Hz */
    AX_VO_OUTPUT_720P30,                /* 1280 x  720 at 30 Hz */
    AX_VO_OUTPUT_720P50,                /* 1280 x  720 at 50 Hz */
    AX_VO_OUTPUT_720P60,                /* 1280 x  720 at 60 Hz */

    AX_VO_OUTPUT_1080P24,               /* 1920 x 1080 at 24 Hz */
    AX_VO_OUTPUT_1080P25,               /* 1920 x 1080 at 25 Hz */
    AX_VO_OUTPUT_1080P30,               /* 1920 x 1080 at 30 Hz */
    AX_VO_OUTPUT_1080P50,               /* 1920 x 1080 at 50 Hz */
    AX_VO_OUTPUT_1080P60,               /* 1920 x 1080 at 60 Hz */

    AX_VO_OUTPUT_640x480_60,            /* VESA 640 x 480 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_800x480_60,            /* 800  x  480 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_800x600_60,            /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1024x600_60,           /* VESA 1024 x 600 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1366x768_60,           /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1280x800_60,           /* VESA 1280 x 800 at 60 Hz (non-interlaced) CVT Compliant */
    AX_VO_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    AX_VO_OUTPUT_1600x1200_60,          /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    AX_VO_OUTPUT_1680x1050_60,          /* VESA 1680 x 1050 at 60 Hz (non-interlaced) CVT Compliant */
    AX_VO_OUTPUT_1920x1200_60,          /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking) */
    AX_VO_OUTPUT_2560x1600_60,          /* VESA 2560 x 1600 at 60 Hz (non-interlaced) CVT Compliant */

    AX_VO_OUTPUT_3840x2160_24,          /* 3840 x 2160 at 24 Hz */
    AX_VO_OUTPUT_3840x2160_25,          /* 3840 x 2160 at 25 Hz */
    AX_VO_OUTPUT_3840x2160_30,          /* 3840 x 2160 at 30 Hz */
    AX_VO_OUTPUT_3840x2160_50,          /* 3840 x 2160 at 50 Hz */
    AX_VO_OUTPUT_3840x2160_60,          /* 3840 x 2160 at 60 Hz */
    AX_VO_OUTPUT_4096x2160_24,          /* 4096 x 2160 at 24 Hz */
    AX_VO_OUTPUT_4096x2160_25,          /* 4096 x 2160 at 25 Hz */
    AX_VO_OUTPUT_4096x2160_30,          /* 4096 x 2160 at 30 Hz */
    AX_VO_OUTPUT_4096x2160_50,          /* 4096 x 2160 at 50 Hz */
    AX_VO_OUTPUT_4096x2160_60,          /* 4096 x 2160 at 60 Hz */

    AX_VO_OUTPUT_720x1280_60,           /* For MIPI DSI Tx 720 x1280 at 60 Hz */
    AX_VO_OUTPUT_1080x1920_60,          /* For MIPI DSI Tx 1080x1920 at 60 Hz */
    AX_VO_OUTPUT_1080x1920_30,          /* For MIPI DSI Tx 1080x1920 at 60 Hz */
    AX_VO_OUTPUT_USER,                  /* User timing. */

    AX_VO_OUTPUT_BUTT
} AX_VO_INTF_SYNC_E;

typedef enum {
    AX_VO_OUT_FMT_UNUSED = 0,
    AX_VO_OUT_FMT_RGB565,
    AX_VO_OUT_FMT_RGB666,
    AX_VO_OUT_FMT_RGB666LP,
    AX_VO_OUT_FMT_RGB888,
    AX_VO_OUT_FMT_YUV422,
    AX_VO_OUT_FMT_BUTT
} AX_VO_OUT_FMT_E;

typedef struct axVO_SYNC_INFO_T {
    AX_BOOL bSynm;      /* sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    AX_BOOL bIop;       /* interlaced or progressive display(0:i; 1:p) */
    AX_U16 u16Vact;     /* vertical active area */
    AX_U16 u16Vbb;      /* vertical back blank porch */
    AX_U16 u16Vfb;      /* vertical front blank porch */
    AX_U16 u16Hact;     /* herizontal active area */
    AX_U16 u16Hbb;      /* herizontal back blank porch */
    AX_U16 u16Hfb;      /* herizontal front blank porch */
    AX_U16 u16Hmid;     /* bottom herizontal active area */
    AX_U16 u16Bvact;    /* bottom vertical active area */
    AX_U16 u16Bvbb;     /* bottom vertical back blank porch */
    AX_U16 u16Bvfb;     /* bottom vertical front blank porch */
    AX_U16 u16Hpw;      /* horizontal pulse width */
    AX_U16 u16Vpw;      /* vertical pulse width */
    AX_U32 u32Pclk;     /* pixel clock, in kHz */
    AX_BOOL bIdv;       /* inverse data valid of output */
    AX_BOOL bIhs;       /* inverse horizontal synch signal */
    AX_BOOL bIvs;       /* inverse vertical synch signal */
} AX_VO_SYNC_INFO_T;

typedef struct axVO_PUB_ATTR_T {
    AX_VO_MODE_E enMode;
    AX_VO_INTF_TYPE_E enIntfType;
    AX_VO_OUT_FMT_E   enIntfFmt;
    AX_VO_INTF_SYNC_E enIntfSync;
    AX_VO_SYNC_INFO_T stSyncInfo;
} AX_VO_PUB_ATTR_T;

typedef struct axVO_RESO_T {
    AX_U32 u32Width;
    AX_U32 u32Height;
    AX_U32 u32RefreshRate;
} AX_VO_RESO_T;

typedef struct axVO_SIZE_T {
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_VO_SIZE_T;

typedef struct axVO_RECT_T {
    AX_U32 u32X;
    AX_U32 u32Y;
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_VO_RECT_T;

typedef enum axVO_PART_MODE_E {
    AX_VO_PART_MODE_SINGLE = 0,
    AX_VO_PART_MODE_MULTI = 1,
    AX_VO_PART_MODE_BUTT
} AX_VO_PART_MODE_E;

typedef enum axVO_BLEND_MODE_E {
    AX_VO_BLEND_MODE_DEFAULT = 0,
    AX_VO_BLEND_MODE_INDEPENDENT = 1,
    AX_VO_BLEND_MODE_GLOBAL = 2,
    AX_VO_BLEND_MODE_BUTT
} AX_VO_BLEND_MODE_E;

typedef enum {
    AX_VO_LAYER_SYNC_NORMAL,
    AX_VO_LAYER_SYNC_SHUTTLE,
    AX_VO_LAYER_SYNC_GROUPING,
    AX_VO_LAYER_SYNC_PRIMARY,
    AX_VO_LAYER_SYNC_BUTT,
} AX_VO_LAYER_SYNC_MODE_E;

typedef enum {
    AX_VO_LAYER_WB_POOL,
    AX_VO_LAYER_WB_INPLACE,
    AX_VO_LAYER_WB_BUF_BUTT,
} AX_VO_LAYER_WB_MODE_E;

typedef enum {
    AX_VO_LAYER_OUT_TO_FIFO = 1,
    AX_VO_LAYER_OUT_TO_LINK = 2,
    AX_VO_LAYER_DISPATCH_MODE_BUTT
} AX_VO_LAYER_DISPATCH_MODE_E;

typedef enum axVO_ENGINE_MODE_E {
    AX_VO_ENGINE_MODE_AUTO = 0,
    AX_VO_ENGINE_MODE_FORCE,
    AX_VO_ENGINE_MODE_BUTT
} AX_VO_ENGINE_MODE_E;

typedef struct axVO_VIDEO_LAYER_ATTR_T {
    AX_VO_SIZE_T stImageSize;             /* RW; Canvas size of the video layer */
    AX_FRAME_COMPRESS_INFO_T stCompressInfo;
    AX_IMG_FORMAT_E enPixFmt;
    AX_VO_LAYER_SYNC_MODE_E enSyncMode;
    AX_U32 u32PrimaryChnId;
    AX_U32 u32FifoDepth;
    AX_U32 u32BkClr;
    AX_U32 u32DispatchMode;
    AX_VO_LAYER_WB_MODE_E enWBMode;
    AX_U32 u32InplaceChnId;
    AX_POOL u32PoolId;
    AX_VO_PART_MODE_E enPartMode;
    AX_VO_BLEND_MODE_E enBlendMode;
    AX_VO_ENGINE_MODE_E enEngineMode;
    AX_U32 u32EngineId;
    AX_U32 u32Toleration;
    AX_F32 f32FrmRate;
} AX_VO_VIDEO_LAYER_ATTR_T;

typedef enum {
    AX_VO_CSC_MATRIX_IDENTITY = 0,
    AX_VO_CSC_MATRIX_BT601_TO_BT601,
    AX_VO_CSC_MATRIX_BT601_TO_BT709,
    AX_VO_CSC_MATRIX_BT709_TO_BT709,
    AX_VO_CSC_MATRIX_BT709_TO_BT601,
    AX_VO_CSC_MATRIX_BT601_TO_RGB_PC,
    AX_VO_CSC_MATRIX_BT709_TO_RGB_PC,
    AX_VO_CSC_MATRIX_RGB_TO_BT601_PC,
    AX_VO_CSC_MATRIX_RGB_TO_BT709_PC,
    AX_VO_CSC_MATRIX_RGB_TO_BT2020_PC,
    AX_VO_CSC_MATRIX_BT2020_TO_RGB_PC,
    AX_VO_CSC_MATRIX_RGB_TO_BT601_TV,
    AX_VO_CSC_MATRIX_RGB_TO_BT709_TV,
    AX_VO_CSC_MATRIX_BUTT
} AX_VO_CSC_MATRIX_E;

typedef struct {
    AX_VO_CSC_MATRIX_E enCscMatrix;
    AX_U32 u32Luma;         /* luminance: 0 ~ 100 */
    AX_U32 u32Contrast;     /* contrast : 0 ~ 100 */
    AX_U32 u32Hue;          /* hue : 0 ~ 100 */
    AX_U32 u32Satuature;    /* satuature: 0 ~ 100 */
} AX_VO_CSC_T;

typedef struct axVO_CHN_ATTR_T {
    AX_VO_RECT_T stRect;
    AX_U32 u32FifoDepth;
    AX_U32 u32Priority;
    AX_BOOL bKeepPrevFr;
    AX_BOOL bInUseFrOutput;
} AX_VO_CHN_ATTR_T;

typedef struct axVO_QUERY_STATUS_T {
    AX_U32 u32ChnBufUsed;
} AX_VO_QUERY_STATUS_T;

typedef enum axVO_WBC_SOURCE_TYPE_E {
    AX_VO_WBC_SOURCE_DEV = 0x0,
    AX_VO_WBC_SOURCE_VIDEO = 0x1,
    AX_VO_WBC_SOURCE_BUTT
} AX_VO_WBC_SOURCE_TYPE_E;

typedef enum axVO_WBC_MODE_E {
    AX_VO_WBC_MODE_NORMAL = 0,
    AX_VO_WBC_MODE_DROP_REPEAT,
    AX_VO_WBC_MODE_BUTT,
} AX_VO_WBC_MODE_E;

typedef struct axVO_WBC_ATTR_T {
    AX_VO_WBC_SOURCE_TYPE_E enSourceType;
    AX_VO_WBC_MODE_E enMode;
    AX_U32 u32FifoDepth;
    AX_F32 f32FrameRate;
} AX_VO_WBC_ATTR_T;

#define VO_DISPLAY_MODE_FLAG_PHSYNC         (1<<0)
#define VO_DISPLAY_MODE_FLAG_NHSYNC         (1<<1)
#define VO_DISPLAY_MODE_FLAG_PVSYNC         (1<<2)
#define VO_DISPLAY_MODE_FLAG_NVSYNC         (1<<3)
#define VO_DISPLAY_MODE_FLAG_INTERLACE      (1<<4)

#define VO_DISPLAY_TYPE_LVDS        7
#define VO_DISPLAY_TYPE_HDMIA       11
#define VO_DISPLAY_TYPE_VIRTUAL     15 /* Can be BT(601/656/1120) or DPI */
#define VO_DISPLAY_TYPE_DSI         16

typedef struct axVO_DISPLAY_MODE_T {
    AX_U16 u16ModesNum;

    struct {
        AX_U16 u16Width;
        AX_U16 u16Height;
        AX_U16 u16Refresh;
        AX_U16 u16Type;
        AX_U32 u32Flags;
    } stModes[AX_VO_MODE_MAX];
} AX_VO_DISPLAY_MODE_T;

typedef struct axFB_CURSOR_POS_T {
    AX_U16 u16X;
    AX_U16 u16Y;
} AX_FB_CURSOR_POS_T;

typedef struct axFB_CURSOR_RES_T {
    AX_U32 u32Width;		/* Size of image */
    AX_U32 u32Height;
} AX_FB_CURSOR_RES_T;

typedef struct axFB_CURSOR_INFO_T {
    AX_U16 u16Enable;
    AX_FB_CURSOR_POS_T stHot;
    AX_FB_CURSOR_RES_T stRes;
} AX_FB_CURSOR_INFO_T;

/* Device Relative Settings */

AX_S32 AX_VO_Init(AX_VOID);
AX_S32 AX_VO_Deinit(AX_VOID);

AX_S32 AX_VO_SetPubAttr(VO_DEV VoDev, const AX_VO_PUB_ATTR_T *pstPubAttr);
AX_S32 AX_VO_GetPubAttr(VO_DEV VoDev, AX_VO_PUB_ATTR_T *pstPubAttr);

AX_S32 AX_VO_SetCSC(VO_DEV VoDev, const AX_VO_CSC_T *pstVideoCSC);
AX_S32 AX_VO_GetCSC(VO_DEV VoDev, AX_VO_CSC_T *pstVideoCSC);

AX_S32 AX_VO_Enable(VO_DEV VoDev);
AX_S32 AX_VO_Disable(VO_DEV VoDev);

AX_S32 AX_VO_EnumMode(VO_DEV VoDev, AX_VO_DISPLAY_MODE_T *pstMode);

/* Video Relative Settings */
AX_S32 AX_VO_CreateVideoLayer(VO_LAYER *u32VoLayer);
AX_S32 AX_VO_DestroyVideoLayer(VO_LAYER u32VoLayer);

AX_S32 AX_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const AX_VO_VIDEO_LAYER_ATTR_T *pstLayerAttr);
AX_S32 AX_VO_GetVideoLayerAttr(VO_LAYER VoLayer, AX_VO_VIDEO_LAYER_ATTR_T *pstLayerAttr);

AX_S32 AX_VO_SetVideoLayerCSC(VO_LAYER VoLayer, const AX_VO_CSC_T *pstVideoCSC);
AX_S32 AX_VO_GetVideoLayerCSC(VO_LAYER VoLayer, AX_VO_CSC_T *pstVideoCSC);

AX_S32 AX_VO_BatchBegin(VO_LAYER VoLayer);
AX_S32 AX_VO_BatchEnd(VO_LAYER VoLayer);

AX_S32 AX_VO_EnableVideoLayer(VO_LAYER VoLayer);
AX_S32 AX_VO_DisableVideoLayer(VO_LAYER VoLayer);

AX_S32 AX_VO_BindVideoLayer(VO_LAYER VoLayer, VO_DEV VoDev);
AX_S32 AX_VO_UnBindVideoLayer(VO_LAYER VoLayer, VO_DEV VoDev);

AX_S32 AX_VO_BindGraphicLayer(GRAPHIC_LAYER GraphicLayer, VO_DEV VoDev);
AX_S32 AX_VO_UnBindGraphicLayer(GRAPHIC_LAYER GraphicLayer, VO_DEV VoDev);

/* This interface obtains the FD corresponding to the layer for the select operation. Closing operation is not allowed */
AX_S32 AX_VO_GetLayerFd(VO_LAYER VoLayer, AX_S32 *s32Fd);
AX_S32 AX_VO_GetLayerFrame(VO_LAYER VoLayer, AX_VIDEO_FRAME_T *pstVoFrame, AX_S32 s32MS);
AX_S32 AX_VO_ReleaseLayerFrame(VO_LAYER VoLayer, const AX_VIDEO_FRAME_T *pstVoFrame);

/* Channel Relative Operations */

AX_S32 AX_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn);
AX_S32 AX_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn);

AX_S32 AX_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const AX_VO_CHN_ATTR_T *pstChnAttr);
AX_S32 AX_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, AX_VO_CHN_ATTR_T *pstChnAttr);

AX_S32 AX_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, AX_VIDEO_FRAME_T *pstVoFrame, AX_S32 s32MS);

AX_S32 AX_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn);
AX_S32 AX_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn);

AX_S32 AX_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, AX_VO_QUERY_STATUS_T *pstStatus);
AX_S32 AX_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, AX_BOOL bClrAll);


AX_S32 AX_VO_GetChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, AX_VIDEO_FRAME_T *pstVoFrame, AX_S32 s32MS);
AX_S32 AX_VO_ReleaseChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, const AX_VIDEO_FRAME_T *pstVoFrame);

/* Writeback Relative Operations */
AX_S32 AX_VO_EnableWBC(VO_WBC VoWbc);
AX_S32 AX_VO_DisableWBC(VO_WBC VoWbc);
AX_S32 AX_VO_SetWBCAttr(VO_WBC VoWbc, const AX_VO_WBC_ATTR_T *pstWbcAttr);
AX_S32 AX_VO_GetWBCAttr(VO_WBC VoWbc, AX_VO_WBC_ATTR_T *pstWbcAttr);
AX_S32 AX_VO_GetWBCFrame(VO_WBC VoWbc, AX_VIDEO_FRAME_T *pstVFrame, AX_S32 s32MilliSec);
AX_S32 AX_VO_ReleaseWBCFrame(VO_WBC VoWbc, const AX_VIDEO_FRAME_T *pstVFrame);
AX_S32 AX_VO_GetWbcFd(VO_WBC VoWbc, AX_S32 *s32Fd);
AX_S32 AX_VO_CscMatrix(const AX_VIDEO_FRAME_T *ptSrc, AX_VIDEO_FRAME_T *ptDst, const AX_CSC_MATRIX_T *ptCscMatrix);

#ifdef __cplusplus
}
#endif

#endif /* _AX_VO_H_ */


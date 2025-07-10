/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_IVPS_TYPE_H_
#define _AX_IVPS_TYPE_H_
#include "ax_global_type.h"
#include "ax_pool_type.h"

/* When the common error code of AX_ERR_CODE_E cannot meet the requirements,
 the module error code can be extended in the following places,
 and the value should start with ox80 */

#define IVPS_SUCC 0x00

#define AX_SUB_ID_IVPS         0x01
#define AX_SUB_ID_RGN          0x02
#define AX_SUB_ID_GDC          0x03
#define AX_SUB_ID_VPP          0x04
#define AX_SUB_ID_TDP          0x05

#define AX_ERROR_IVPS(e) AX_DEF_ERR(AX_ID_IVPS, AX_SUB_ID_IVPS, (e))
#define AX_ERROR_RGN(e)  AX_DEF_ERR(AX_ID_IVPS, AX_SUB_ID_RGN, (e))
#define AX_ERROR_GDC(e)  AX_DEF_ERR(AX_ID_IVPS, AX_SUB_ID_GDC, (e))


/*******************************IVPS ERROR CODE************************************/
/* IVPS module is not exist, maybe IVPS module is not initial */
#define AX_ERR_IVPS_INVALID_MODID                 AX_ERROR_IVPS(AX_ERR_INVALID_MODID)
/* device node is invalid, plz check /dev/tdp* */
#define AX_ERR_IVPS_INVALID_DEVID                 AX_ERROR_IVPS(AX_ERR_INVALID_DEVID)
/* this channel is not exist */
#define AX_ERR_IVPS_INVALID_CHNID                 AX_ERROR_IVPS(AX_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg. an illegal enumeration value */
#define AX_ERR_IVPS_ILLEGAL_PARAM                 AX_ERROR_IVPS(AX_ERR_ILLEGAL_PARAM)
/* used a NULL point */
#define AX_ERR_IVPS_NULL_PTR                      AX_ERROR_IVPS(AX_ERR_NULL_PTR)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define AX_ERR_IVPS_BAD_ADDR                      AX_ERROR_IVPS(AX_ERR_BAD_ADDR)
/* system is not ready,had not initialed or loaded*/
#define AX_ERR_IVPS_SYS_NOTREADY                  AX_ERROR_IVPS(AX_ERR_SYS_NOTREADY)
/* ivps channel is busy */
#define AX_ERR_IVPS_BUSY                          AX_ERROR_IVPS(AX_ERR_BUSY)
/* initilize error */
#define AX_ERR_IVPS_NOT_INIT                      AX_ERROR_IVPS(AX_ERR_NOT_INIT)
/* try to enable or initialize system, device or channel, before configing attribute */
#define AX_ERR_IVPS_NOT_CONFIG                    AX_ERROR_IVPS(AX_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define AX_ERR_IVPS_NOT_SUPPORT                   AX_ERROR_IVPS(AX_ERR_NOT_SUPPORT)
/* operation is not permitted, eg. try to change stati attribute */
#define AX_ERR_IVPS_NOT_PERM                      AX_ERROR_IVPS(AX_ERR_NOT_PERM)
/* channel exists */
#define AX_ERR_IVPS_EXIST                         AX_ERROR_IVPS(AX_ERR_EXIST)
/* the channle is not existed */
#define AX_ERR_IVPS_UNEXIST                       AX_ERROR_IVPS(AX_ERR_UNEXIST)
/* failure caused by malloc memory */
#define AX_ERR_IVPS_NOMEM                         AX_ERROR_IVPS(AX_ERR_NOMEM)
/* failure caused by getting block from pool */
#define AX_ERR_IVPS_NOBUF                         AX_ERROR_IVPS(AX_ERR_NOBUF)
/* buffer contains no data */
#define AX_ERR_IVPS_BUF_EMPTY                     AX_ERROR_IVPS(AX_ERR_BUF_EMPTY)
/* buffer contains fresh data */
#define AX_ERR_IVPS_BUF_FULL                      AX_ERROR_IVPS(AX_ERR_BUF_FULL)
/* failed to read as fifo is empty */
#define AX_ERR_IVPS_QUEUE_EMPTY                   AX_ERROR_IVPS(AX_ERR_QUEUE_EMPTY)
/* failed to write as fifo is full */
#define AX_ERR_IVPS_QUEUE_FULL                    AX_ERROR_IVPS(AX_ERR_QUEUE_FULL)
/* wait timeout failed */
#define AX_ERR_IVPS_TIMED_OUT                     AX_ERROR_IVPS(AX_ERR_TIMED_OUT)
/* process termination */
#define AX_ERR_IVPS_FLOW_END                      AX_ERROR_IVPS(AX_ERR_FLOW_END)
/* for ivps unknown error */
#define AX_ERR_IVPS_UNKNOWN                       AX_ERROR_IVPS(AX_ERR_UNKNOWN)


/*******************************IVPS RGN ERROR CODE*********************************/
/* this region handle id invalid */
#define AX_ERR_IVPS_RGN_INVALID_GRPID             AX_ERROR_RGN(AX_ERR_INVALID_GRPID)
/* this region has been attached and in busy state */
#define AX_ERR_IVPS_RGN_BUSY                      AX_ERROR_RGN(AX_ERR_BUSY)
/* this region has been destroy */
#define AX_ERR_IVPS_RGN_UNEXIST                   AX_ERROR_RGN(AX_ERR_UNEXIST)
/* at lease one parameter is illagal */
#define AX_ERR_IVPS_RGN_ILLEGAL_PARAM             AX_ERROR_RGN(AX_ERR_ILLEGAL_PARAM)
/* failure caused by getting block from pool */
#define AX_ERR_IVPS_RGN_NOBUF                     AX_ERROR_RGN(AX_ERR_NOBUF)

#define AX_IVPS_MIN_IMAGE_WIDTH 32
#define AX_IVPS_MAX_IMAGE_WIDTH 8192

#define AX_IVPS_MIN_IMAGE_HEIGHT 32
#define AX_IVPS_MAX_IMAGE_HEIGHT 8192

#define AX_IVPS_MAX_GRP_NUM 20
#define AX_IVPS_MAX_OUTCHN_NUM 5
#define AX_IVPS_MAX_FILTER_NUM_PER_OUTCHN 2
#define AX_IVPS_INVALID_FRMRATE (-1)

#define AX_IVPS_MAX_POLYGON_POINT_NUM  10
#define AX_IVPS_MIN_POLYGON_POINT_NUM  3

typedef AX_S32 IVPS_GRP;
typedef AX_S32 IVPS_CHN;
typedef AX_S32 IVPS_FILTER;

typedef AX_S32 IVPS_RGN_GRP;
typedef AX_U32 IVPS_RGB;

typedef enum
{
    AX_IVPS_CHN_FLIP_NONE = 0,
    AX_IVPS_CHN_FLIP,
    AX_IVPS_CHN_MIRROR,
    AX_IVPS_CHN_FLIP_AND_MIRROR,
    AX_IVPS_CHN_FLIP_BUTT
} AX_IVPS_CHN_FLIP_MODE_E;

typedef struct
{
    AX_S16 nX;
    AX_S16 nY;
    AX_U16 nW;
    AX_U16 nH;
} AX_IVPS_RECT_T;

typedef struct
{
    AX_S16 nX;
    AX_S16 nY;
} AX_IVPS_POINT_T;

typedef struct
{
    AX_U16 nW;
    AX_U16 nH;
} AX_IVPS_SIZE_T;

typedef struct
{
    AX_S32 nHorRatio; /* RW; range: [-100000, 100000] */
    AX_S32 nVerRatio; /* RW; range: [-100000, 100000] */
} AX_IVPS_POINT_RATIO_T;

typedef struct
{
    AX_F32 fX;
    AX_F32 fY;
} AX_IVPS_POINT_NICE_T;

/* mosiac block size */
typedef enum
{
    AX_IVPS_MOSAIC_BLK_SIZE_2 = 0, /* block size 2*2 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_4,     /* block size 4*4 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_8,     /* block size 8*8 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_16,    /* block size 16*16 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_32,    /* block size 32*32 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_64,    /* block size 64*64 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_BUTT
} AX_IVPS_MOSAIC_BLK_SIZE_E;

typedef enum
{
    AX_IVPS_ROTATION_0   = 0,
    AX_IVPS_ROTATION_90  = 1,
    AX_IVPS_ROTATION_180 = 2,
    AX_IVPS_ROTATION_270 = 3,
    AX_IVPS_ROTATION_BUTT
} AX_IVPS_ROTATION_E;

typedef enum
{
    AX_IVPS_ASPECT_RATIO_STRETCH = 0, /* Fill buffer according to output image size */
    AX_IVPS_ASPECT_RATIO_AUTO = 1,    /* Fill buffer according to horizontal and vertical alignment specified by eAligns[] params */
    AX_IVPS_ASPECT_RATIO_MANUAL = 2,
    AX_IVPS_ASPECT_RATIO_BUTT
} AX_IVPS_ASPECT_RATIO_E;

typedef enum
{
/*
if (src_w / src_h < dst_w / dst_h)
    w_ratio = h_ratio = dst_h * 1.0 / src_h
else
    w_ratio = h_ratio = dst_w * 1.0 / src_w

Example1:
    src: 1920x1080  dst: 1920x1140
    IVPS_ASPECT_RATIO_HORIZONTAL_CENTER
    IVPS_ASPECT_RATIO_VERTICAL_BOTTOM
    then output image:
    |   fill area (0, 0, 1920, 60)      |  0
    |-----------------------------------| 60
    |                                   |
    |   Image area(0, 60, 1920, 1140)   |
    |                                   |
    |                                   | 1140

Example2:
    src: 1920x1080  dst: 1980x1080
    IVPS_ASPECT_RATIO_HORIZONTAL_LEFT
    IVPS_ASPECT_RATIO_VERTICAL_CENTER
    then output image:
    0                            1920   1980
    |                              |      |
    |      Image Area              | Fill |
    |      (0, 0, 1920, 1080)      | Area |
    |                              |      |
*/
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER = 0,
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_LEFT = 1,
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_RIGHT = 2,
    AX_IVPS_ASPECT_RATIO_VERTICAL_CENTER = AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER,
    AX_IVPS_ASPECT_RATIO_VERTICAL_TOP = AX_IVPS_ASPECT_RATIO_HORIZONTAL_LEFT,
    AX_IVPS_ASPECT_RATIO_VERTICAL_BOTTOM = AX_IVPS_ASPECT_RATIO_HORIZONTAL_RIGHT,
} AX_IVPS_ASPECT_RATIO_ALIGN_E;

/*
 * AX_IVPS_ENGINE_SCL: scaler module. This module must be placed on the channel and cannot exist independently.
 *                     There must be a previous module(GDC or VPP) with online link relationship between them.
 */
typedef enum
{
    AX_IVPS_ENGINE_SCL = 0,
    AX_IVPS_ENGINE_TDP,
    AX_IVPS_ENGINE_GDC,
    AX_IVPS_ENGINE_VPP,
    AX_IVPS_ENGINE_VO,
    AX_IVPS_ENGINE_BUTT
} AX_IVPS_ENGINE_E;

/*
 * AX_IVPS_PIPELINE_DEFAULT is the mainstream pipeline that we currently support. The mode is as follows:
 *                             |--> Pipe1Filter0 Pipe1Filter1 (chn 0)
 *                             |--> Pipe2Filter0 Pipe2Filter1 (chn 1)
 * Pipe0Filter0 Pipe0Filter1-->|--> Pipe3Filter0 Pipe3Filter1 (chn 2)
 *                             |--> Pipe4Filter0 Pipe4Filter1 (chn 3)
 *                             |--> Pipe5Filter0 Pipe5Filter1 (chn 4)
 */
typedef enum
{
    AX_IVPS_PIPELINE_DEFAULT = 0,
    AX_IVPS_PIPELINE_BUTT
} AX_IVPS_PIPELINE_E;

typedef enum
{
    AX_IVPS_SCL_TYPE_AUTO = 0,     /* Select the appropriate scaling algorithm based on current hardware support */
    AX_IVPS_SCL_TYPE_BILINEAR,     /* Bilinear interpolation scaling algorithm, commonly used */
    AX_IVPS_SCL_TYPE_PHASE_FILTER, /* Phase filtering scaling algorithm, better than Bilinear */
    AX_IVPS_SCL_TYPE_BUTT
} AX_IVPS_SCL_TYPE_E;

typedef enum
{
    AX_IVPS_SCL_INPUT_SHARE = 0, /* Share with vpp in pipeline. If the scaler is not enough, you can choose it. There may be a delay in the response. */
    AX_IVPS_SCL_INPUT_MONOPOLY,  /* Monopolize a scaler. If other users also choose this type, you will share this scaler with them. */
    AX_IVPS_SCL_INPUT_BUTT
} AX_IVPS_SCL_INPUT_E;

typedef struct
{
    /* src frame rate (<= 0: no FRC control) */
    AX_S32 nSrcFrameRate;
    /* dst frame rate (<= 0, or nSrcFrameRate must be set) */
    AX_S32 nDstFrameRate;
} AX_IVPS_FRAME_RATE_CTRL_T;

typedef struct
{
    AX_IVPS_ASPECT_RATIO_E eMode;
    AX_U32 nBgColor;
    /* YUV color:
    31       23      15      7       0
    |--------|---Y---|---U---|---V---|
    */
    AX_IVPS_ASPECT_RATIO_ALIGN_E eAligns[2]; /* IVPS_ASPECT_RATIO_ALIGN: [0]: HORIZONTAL [1]: VERTICAL */
    AX_IVPS_RECT_T tRect;                    /* Should be set in ASPECT_RATIO_MANUAL mode */
} AX_IVPS_ASPECT_RATIO_T;

typedef struct
{
    AX_IVPS_ASPECT_RATIO_T tAspectRatio;
    AX_IVPS_SCL_TYPE_E eSclType;
    AX_IVPS_SCL_INPUT_E eSclInput;
    AX_U8 nAlpha;                            /* RW; range: [0, 255]; 0: transparent, 255: opaque */
} AX_IVPS_CROP_RESIZE_ATTR_T;

typedef enum
{
    AX_IVPS_FRC_RATIO = 0,
    AX_IVPS_FRC_ABS,
    AX_IVPS_FRC_BUTT
} AX_IVPS_FRC_MODE_E;

typedef enum
{
    AX_IVPS_PTS_DEFAULT = 0,
    AX_IVPS_PTS_SMOOTH,
    AX_IVPS_PTS_BUTT
} AX_IVPS_PTS_MODE_E;
/***************************************************************************************************************/
/*                                                   TDP                                                       */
/***************************************************************************************************************/
/*
* AX_IVPS_TDP_CFG_T
* This configuration is specific to TDP engine.
* This engine can support many functions,
* such as mirror, flip, rotation, scale, mosaic, osd and so on.
* If in eCompressMode, stride and width should be 128 pixels aligned.
* @bVoOsd: Use VO hardware to assist TDP to draw OSD.
*/
typedef struct
{
    AX_IVPS_ROTATION_E eRotation;
    AX_BOOL bMirror;
    AX_BOOL bFlip;
    AX_BOOL bVoOsd;
    AX_BOOL bRotationOnly;
} AX_IVPS_TDP_CFG_T;

/***************************************************************************************************************/
/*                                                   GDC                                                       */
/***************************************************************************************************************/
#define AX_IVPS_GDC_MAX_IMAGE_WIDTH  8192
#define AX_IVPS_GDC_MAX_IMAGE_HEIGHT 8192
#define AX_IVPS_GDC_MIN_IMAGE_WIDTH  256
#define AX_IVPS_GDC_MIN_IMAGE_HEIGHT 256
#define AX_IVPS_FISHEYE_MAX_RGN_NUM  9
#define AX_IVPS_GDC_MAX_HANDLE_NUM  32

typedef AX_S32 GDC_HANDLE;

typedef enum
{
    AX_IVPS_GDC_BYPASS = 0,  /* no gdc correction for all whole frame */
    AX_IVPS_GDC_FISHEYE,     /* gdc correction for fisheye */
    AX_IVPS_GDC_MAP_USER,    /* customize mesh table by user */
    AX_IVPS_GDC_BUTT
} AX_IVPS_GDC_TYPE_E;

typedef enum
{
    AX_IVPS_FISHEYE_MOUNT_MODE_DESKTOP = 0, /* desktop mount mode */
    AX_IVPS_FISHEYE_MOUNT_MODE_CEILING = 1, /* ceiling mount mode */
    AX_IVPS_FISHEYE_MOUNT_MODE_WALL = 2,    /* wall mount mode */
    AX_IVPS_FISHEYE_MOUNT_MODE_BUTT
} AX_IVPS_FISHEYE_MOUNT_MODE_E;

typedef enum
{
    AX_IVPS_FISHEYE_VIEW_MODE_PANORAMA = 0, /* panorama mode of gdc correction; desktop and Ceiling only support 360 panorama; wall only support 180 panorama */
    AX_IVPS_FISHEYE_VIEW_MODE_NORMAL = 1,   /* normal mode of gdc correction */
    AX_IVPS_FISHEYE_VIEW_MODE_BYPASS = 2,   /* no gdc correction */
    AX_IVPS_FISHEYE_VIEW_MODE_BUTT
} AX_IVPS_FISHEYE_VIEW_MODE_E;

typedef struct
{
    AX_IVPS_FISHEYE_VIEW_MODE_E eViewMode; /* RW; range: [0, 3]; gdc view mode */
    AX_U16 nInRadius;                      /* RW; range: [0, nOutRadius); inner radius of gdc correction region */
    AX_U16 nOutRadius;                     /* RW; range: [0, 0.75 * MAX(width, height) of input picture]; out radius of gdc correction region */
    AX_U16 nPan;                           /* RW; range: [0, 360]; active if bRoiXY = 0 */
    AX_U16 nTilt;                          /* RW; range: [0, 360]; active if bRoiXY = 0 */
    AX_U16 nCenterX;                       /* RW; range: (0, width of input picture); x-coordinate of the centre point of correction region; active if bRoiXY = 1 */
    AX_U16 nCenterY;                       /* RW; range: (0, height of input picture); y-coordinate of the centre point of correction region; active if bRoiXY = 1 */
    AX_U16 nHorZoom;                       /* RW; range: [1, 5265] in normal mode, otherwise [1, 4095]; horizontal zoom of correction region */
    AX_U16 nVerZoom;                       /* RW; range: [1, 4095] vertical zoom of correction region */
    AX_IVPS_RECT_T tOutRect;               /* RW; out imge rectangle attribute */
} AX_IVPS_FISHEYE_RGN_ATTR_T;

typedef struct
{
    AX_BOOL bBgColor;                        /* RW; range: [0, 1]; whether use background color or not; not support now */
    AX_U32 nBgColor;                         /* RW; range: [0, 0xffffff]; background color RGB888; not support now */
    AX_S16 nHorOffset;                       /* RW; range: [-511, 511]; the horizontal offset between image center and physical center of len*/
    AX_S16 nVerOffset;                       /* RW; range: [-511, 511];  the vertical offset between image center and physical center of len */
    AX_U8 nTrapezoidCoef;                    /* RW; range: [0, 32]; strength coefficient of trapezoid correction */
    AX_S16 nFanStrength;                     /* RW; range: [-760, 760]; strength coefficient of fan correction */
    AX_IVPS_FISHEYE_MOUNT_MODE_E eMountMode; /* RW; range: [0, 2]; gdc mount mode */
    AX_U8 nRgnNum;                           /* RW; range: [1, 9]; gdc correction region number */
    AX_BOOL bRoiXY;                          /* RW; range: [0, 1]; 0: Polar coordinates with nPan and nTilt; 1: Planar coordinates with nCenterX and nCenterY */
    AX_IVPS_FISHEYE_RGN_ATTR_T tFisheyeRgnAttr[AX_IVPS_FISHEYE_MAX_RGN_NUM]; /* RW; attribution of gdc correction region */
} AX_IVPS_FISHEYE_ATTR_T;

/*
 * AX_IVPS_MAP_USER_ATTR_T
 * nMeshWidth * (nMeshNumH - 1) >= DstWidth
 * nMeshHeight * (nMeshNumV - 1) >= DstHeight
 */
typedef struct
{
    AX_U16 nMeshStartX;        /* RW; range: [0, output width]; x-coordinate of output picture in the mesh table */
    AX_U16 nMeshStartY;        /* RW; range: [0, output height]; y-coordinate of output picture in the mesh table */
    AX_U16 nMeshWidth;         /* RW; range: [16, 256]; 16 aligned; width of mesh block */
    AX_U16 nMeshHeight;        /* RW; range: [16, 256]; 16 aligned; height of mesh block */
    AX_U8 nMeshNumH;           /* RW; range: [33, 64]; number of mesh block in horizontal direction; Must be equal with nMeshNumV */
    AX_U8 nMeshNumV;           /* RW; range: [33, 64]; number of mesh block in vertical direction; Must be equal with nMeshNumH */
    AX_S32 *pUserMap;          /* RW; X-map and Y-map are crisscross arrangement; e.g. X-map[32bit] Y-map[32bit]... */
    AX_U64 nMeshTablePhyAddr;  /* RO; this variable is used internal */
} AX_IVPS_MAP_USER_ATTR_T;

typedef struct
{
    AX_IVPS_GDC_TYPE_E eGdcType;
    union {
        AX_IVPS_FISHEYE_ATTR_T tFisheyeAttr; /* RW; attribution of gdc fisheye */
        AX_IVPS_MAP_USER_ATTR_T tMapUserAttr;/* RW; attribution of gdc user map */
    };
    AX_U16 nSrcWidth;                        /* RW; range: [2, 8192]; 2 pixels aligned; width of input picture */
    AX_U16 nSrcHeight;                       /* RW; range: [2, 8192]; 2 pixels aligned; height of input picture */
    AX_U16 nDstStride;                       /* RW; range: [128, 8192]; 128 pixels aligned; format of output picture */
    AX_U16 nDstWidth;                        /* RW; range: [2, 8192]; 2 pixels aligned; width of output picture */
    AX_U16 nDstHeight;                       /* RW; range: [2, 8192]; 2 pixels aligned; height of output picture */
    AX_IMG_FORMAT_E eDstFormat;              /* RW; format of output picture; only support NV12 */
} AX_IVPS_GDC_ATTR_T;

typedef enum
{
    AX_IVPS_DEWARP_BYPASS = 0,     /* only support crop, rotation, mirror, flip or scaling */
    AX_IVPS_DEWARP_MAP_USER,       /* user defined map */
    AX_IVPS_DEWARP_PERSPECTIVE,    /* affine or perspective transformation */
    AX_IVPS_DEWARP_LDC,            /* lens distortion correction */
    AX_IVPS_DEWARP_LDC_V2,         /* lens distortion correction version 2 */
    AX_IVPS_DEWARP_LDC_PERSPECTIVE,/* LDC and PERSPECTIVE are done together */
    AX_IVPS_DEWARP_BUTT
} AX_IVPS_DEWARP_TYPE_E;

/*
 * AX_IVPS_PERSPECTIVE_ATTR_T
 * Perspective Matrix =
 *   [m(0,0),   m(0,1),   m(0,2)],
 *   [m(1,0),   m(1,1),   m(1,2)],
 *   [m(2,0),   m(2,1),   m(2,2)],
 * If [m(2,0), m(2,1), m(2,2)] = [0, 0, 1], the transformation is affine.
 * Note:
 * 1. m(2,2) is not 0.
 * 2. The matrix element has 6 decimal numbers.
 * i.e. If the element is 954301, 954301/1000000 = 0.954301,
 * the real value is 0.954301.
 */
typedef struct
{
    AX_S64 nMatrix[9];
} AX_IVPS_PERSPECTIVE_ATTR_T;

typedef struct
{
    AX_BOOL bAspect;         /* whether aspect ration is keep */
    AX_S16 nXRatio;          /* Range: [0, 100], field angle ration of horizontal, valid when bAspect = 0. */
    AX_S16 nYRatio;          /* Range: [0, 100], field angle ration of vertical, valid when bAspect = 0. */
    AX_S16 nXYRatio;         /* Range: [0, 100], field angle ration of all,valid when bAspect = 1. */
    AX_S16 nCenterXOffset;   /* Range: [-511, 511], horizontal offset of the image distortion center relative to image center. */
    AX_S16 nCenterYOffset;   /* Range: [-511, 511], vertical offset of the image distortion center relative to image center. */
    AX_S16 nDistortionRatio; /* Range: [-10000, 10000], LDC distortion ratio. [-10000, 0): pincushion distortion; (0, 10000]: barrel distortion */
    AX_S8 nSpreadCoef;       /* Range: [-18, 18], LDC spread coefficient */
} AX_IVPS_LDC_ATTR_T;

/*
 * AX_IVPS_LDC_V2_ATTR_T
 * Camera Matrix(internal parameter matrix) =
 *   [nXFocus,     0,        nXCenter],
 *   [0,          nYFocus,   nYCenter],
 *   [0,           0,              1 ],
 * The element has 2 decimal numbers.
 * i.e. If the element is 192029, 192029/100 = 1920.29,
 * the real value is 1920.29.
 *
 * Distortion Coefficients =
 *  (k1, k2, p1, p2, k3, k4, k5, k6)
 * The element has 6 decimal numbers.
 * i.e. If the element is 954301, 954301/1000000 = 0.954301,
 * the real value is 0.954301.
 */
typedef struct
{
    AX_U32 nXFocus;
    AX_U32 nYFocus;
    AX_U32 nXCenter;
    AX_U32 nYCenter;
    AX_S64 nDistortionCoeff[8];
} AX_IVPS_LDC_V2_ATTR_T;

/*
 * AX_IVPS_GDC_CFG_T
 * This configuration is specific to GDC engine.
 * If GDC has many types, such as bypass, user-defined map, distortion correction, affine or perspective transformation.
 * In bypass type, GDC only support crop, rotation, mirror, flip or scaling.
 * If FBC is enabled, stride and width should be 128 pixels aligned.
 * If FBC is disabled, only stride should be 128 pixels aligned, while
 * width and height should be 2 pixels aligned.
 */
typedef struct
{
    AX_IVPS_DEWARP_TYPE_E eDewarpType;

    AX_IVPS_ROTATION_E eRotation;
    AX_BOOL bHwRotation; /* Only support 90/270 degree rotation */
    AX_BOOL bMirror;
    AX_BOOL bFlip;

    union {
        /* AX_IVPS_DEWARP_MAP_USER */
        AX_IVPS_MAP_USER_ATTR_T tMapUserAttr;
        /* AX_IVPS_DEWARP_LDC */
        AX_IVPS_LDC_ATTR_T tLdcAttr;
        /* AX_IVPS_DEWARP_LDC_V2 */
        AX_IVPS_LDC_V2_ATTR_T tLdcV2Attr;
    };
    /* AX_IVPS_DEWARP_PERSPECTIVE */
    AX_IVPS_PERSPECTIVE_ATTR_T tPerspectiveAttr;
} AX_IVPS_GDC_CFG_T;

typedef struct
{
    AX_BOOL bOutRect;           /* RW; whether output rectangle is enabled */
    AX_IVPS_RECT_T tOutRect;    /* RW; 2 pixels aligned; output rectangle info */

    AX_IVPS_DEWARP_TYPE_E eDewarpType;
	union {
		AX_IVPS_PERSPECTIVE_ATTR_T tPerspectiveAttr;
		AX_IVPS_MAP_USER_ATTR_T tMapUserAttr;
	};
} AX_IVPS_DEWARP_ATTR_T;

typedef struct
{
    AX_POOL_SOURCE_E ePoolSrc; /* RW; pool allocation method; 0(default): common pool; 1: private pool; 2: user pool */
    AX_U8 nFrmBufNum;          /* RW; private pool frame buffer count; active in private pool */
    AX_POOL PoolId;            /* RW; user_pool_id; active in user pool */
} AX_IVPS_POOL_ATTR_T;

typedef struct {
    AX_BOOL bEnable;
    AX_COORD_E eCoordMode;
    AX_IVPS_RECT_T tCropRect;
    AX_S8 nSnsId;
} AX_IVPS_CROP_INFO_T;

typedef struct
{
    AX_FRAME_RATE_CTRL_T tFRC;
    AX_U16 nDstPicWidth;
    AX_U16 nDstPicHeight;
    AX_U16 nDstPicStride;
    AX_IMG_FORMAT_E eDstPicFormat;
    AX_IVPS_ASPECT_RATIO_T tAspectRatio;
    AX_U8 nOutFifoDepth;
    AX_U32 nFRC; /* Reserved */
} AX_IVPS_CHN_ATTR_T;

/*
 * The style of rectangle is like below
 * [            ]
 *
 * [            ]
 * if bEnable is AX_TRUE, then LineWidth should be non-zero
 */
typedef struct
{
    AX_BOOL bEnable;
    AX_U8 nHorLength;
    AX_U8 nVerLength;
} AX_IVPS_CORNER_RECT_ATTR_T;

typedef enum
{
    AX_IVPS_SCALE_RANGE_0 = 0, /* scale range <  8/64  */
    AX_IVPS_SCALE_RANGE_1,     /* scale range >= 8/64  */
    AX_IVPS_SCALE_RANGE_2,     /* scale range >= 16/64 */
    AX_IVPS_SCALE_RANGE_3,     /* scale range >= 24/64 */
    AX_IVPS_SCALE_RANGE_4,     /* scale range >= 32/64 */
    AX_IVPS_SCALE_RANGE_5,     /* scale range >= 40/64 */
    AX_IVPS_SCALE_RANGE_6,     /* scale range >= 48/64 */
    AX_IVPS_SCALE_RANGE_7,     /* scale range >= 56/64 */
    AX_IVPS_SCALE_RANGE_8,     /* scale range > 1      */
    AX_IVPS_SCALE_RANGE_BUTT
} AX_IVPS_SCALE_RANGE_TYPE_E;

typedef struct {
    AX_IVPS_SCALE_RANGE_TYPE_E eHorScaleRange;
    AX_IVPS_SCALE_RANGE_TYPE_E eVerScaleRange; /* Reserved */
} AX_IVPS_SCALE_RANGE_T;

typedef enum
{
    AX_IVPS_COEF_LEVEL_0 = 0, /* coefficient level 0 */
    AX_IVPS_COEF_LEVEL_1,     /* coefficient level 1 */
    AX_IVPS_COEF_LEVEL_2,     /* coefficient level 2 */
    AX_IVPS_COEF_LEVEL_3,     /* coefficient level 3 */
    AX_IVPS_COEF_LEVEL_4,     /* coefficient level 4 */
    AX_IVPS_COEF_LEVEL_5,     /* coefficient level 5 */
    AX_IVPS_COEF_LEVEL_BUTT,
} AX_IVPS_COEF_LEVEL_E;


typedef struct {
    AX_IVPS_COEF_LEVEL_E eHorLuma;   /* horizontal luminance coefficient level            */
    AX_IVPS_COEF_LEVEL_E eHorChroma; /* horizontal chrominance coefficient level Reserved */
    AX_IVPS_COEF_LEVEL_E eVerLuma;   /* vertical luminance coefficient level     Reserved */
    AX_IVPS_COEF_LEVEL_E eVerChroma; /* vertical chrominance coefficient level   Reserved */
} AX_IVPS_SCALE_COEF_LEVEL_T;

#endif

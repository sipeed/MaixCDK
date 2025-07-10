/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AVS_API_H_
#define _AX_AVS_API_H_

#include "ax_global_type.h"
#include "ax_sys_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/* error code define */
typedef enum
{
    AX_ID_AVS_INNER    = 0x01,
    AX_ID_AVS_COMMON  = 0x02,
    /* reserved */
} AX_AVS_SUB_ID_E;

#define AX_ERR_AVS_INVALID_CHNID                AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_INVALID_CHNID) // 0x160204
#define AX_ERR_AVS_INVALID_PIPEID               AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_INVALID_PIPEID) // 0x160205
#define AX_ERR_AVS_INVALID_STITCHGRPID          AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_INVALID_STITCHGRPID) // 0x160206

#define AX_ERR_AVS_ILLEGAL_PARAM                AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_ILLEGAL_PARAM) // 0x16020A
#define AX_ERR_AVS_NULL_PTR                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NULL_PTR) // 0x16020B
#define AX_ERR_AVS_BAD_ADDR                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_BAD_ADDR) // 0x16020C

#define AX_ERR_AVS_SYS_NOTREADY                 AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_SYS_NOTREADY) // 0x160210
#define AX_ERR_AVS_BUSY                         AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_BUSY) // 0x160211
#define AX_ERR_AVS_NOT_INIT                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOT_INIT) // 0x160212
#define AX_ERR_AVS_NOT_CONFIG                   AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOT_CONFIG) // 0x160213
#define AX_ERR_AVS_NOT_SUPPORT                  AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOT_SUPPORT) // 0x160214
#define AX_ERR_AVS_NOT_PERM                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOT_PERM) // 0x160215
#define AX_ERR_AVS_EXIST                        AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_EXIST) // 0x160216
#define AX_ERR_AVS_UNEXIST                      AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_UNEXIST) // 0x160217
#define AX_ERR_AVS_NOMEM                        AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOMEM) // 0x160218
#define AX_ERR_AVS_NOBUF                        AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOBUF) // 0x160219
#define AX_ERR_AVS_NOT_MATCH                    AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_NOT_MATCH) // 0x16021A

#define AX_ERR_AVS_BUF_EMPTY                    AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_BUF_EMPTY) // 0x160220
#define AX_ERR_AVS_BUF_FULL                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_BUF_FULL) // 0x160221
#define AX_ERR_AVS_QUEUE_EMPTY                  AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_QUEUE_EMPTY) // 0x160222
#define AX_ERR_AVS_QUEUE_FULL                   AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_QUEUE_FULL) // 0x160223

#define AX_ERR_AVS_TIMEOUT                      AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_TIMED_OUT) // 0x160227
#define AX_ERR_AVS_FLOW_END                     AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_FLOW_END) // 0x160228
#define AX_ERR_AVS_UNKNOWN                      AX_DEF_ERR(AX_ID_AVS, AX_ID_AVS_COMMON, AX_ERR_UNKNOWN) // 0x160229

/* attribute define */
#define AX_AVS_MAX_GRP_NUM                      4
#define AX_AVS_PIPE_NUM                         4
#define AX_AVS_MAX_CHN_NUM                      1
#define AX_AVS_MAX_IN_WIDTH                     4096
#define AX_AVS_MAX_IN_HEIGHT                    4096
#define AX_AVS_MIN_IN_WIDTH                     1280
#define AX_AVS_MIN_IN_HEIGHT                    720
#define AX_AVS_MAX_OUT_HEIGHT                   8192
#define AX_AVS_MAX_OUT_HEIGHT                   8192
#define AX_AVS_MIN_OUT_HEIGHT                   256
#define AX_AVS_MIN_OUT_HEIGHT                   256
#define AX_AVS_CUBE_MAP_SURFACE_NUM             6

typedef AX_S32 AX_AVS_GRP;
typedef AX_S32 AX_AVS_PIPE;
typedef AX_S32 AX_AVS_CHN;

typedef struct axAVS_POINT_T
{
    AX_S32                      s32X;
    AX_S32                      s32Y;
} AX_AVS_POINT_T;

typedef struct axAVS_FOV_T
{
    AX_F32                      u32FOVX;
    AX_F32                      u32FOVY;
} AX_AVS_FOV_T;

typedef enum axAVS_MODE_E
{
    AVS_MODE_BLEND = 0,
    AVS_MODE_NOBLEND_VER,
    AVS_MODE_NOBLEND_HOR,
    AVS_MODE_NOBLEND_QR,
    AVS_MODE_BUTT
} AX_AVS_MODE_E;

typedef struct axAVS_GRP_PIPE_MESH_T
{
    AX_VOID*                            pstVirAddr[AX_AVS_PIPE_NUM];
    AX_U32                              s32MeshSize[AX_AVS_PIPE_NUM];
} AX_AVS_GRP_PIPE_MESH_T;

typedef struct axAVS_GRP_MASK_T
{
    AX_VOID*                            pstVirAddr[AX_AVS_PIPE_NUM - 1];
    AX_U32                              s32MaskSize[AX_AVS_PIPE_NUM - 1];
} AX_AVS_GRP_MASK_T;

typedef struct axAVS_PIPE_SEAM_INFO_T
{
    AX_U16                              u16X;
    AX_U16                              u16Y;
    AX_U16                              u16W;
    AX_U16                              u16H;
} AX_AVS_PIPE_SEAM_INFO_T;

typedef struct axAVS_GRP_TRANSFORM_PARAM_T
{
    AX_AVS_GRP_PIPE_MESH_T      stPipeMesh;
    AX_AVS_PIPE_SEAM_INFO_T     stSeamInfo[AX_AVS_PIPE_NUM - 1];
    AX_AVS_GRP_MASK_T           stMask;
} AX_AVS_GRP_TRANSFORM_PARAM_T;


/*
 * AX_AVS_INTRINSIC_PARAM_T
 * Camera Matrix(internal parameter matrix) =
 *   [fx,          0,        -cx],
 *   [0,          fy,        -cy],
 *   [0,           0,         1 ],
 * Distortion Coefficients =
 *  (k1, k2, p1, p2, k3, k4, k5, k6)
 * Normal Distortion Coefficients =
 *  (cx, cy, d_ratio, x_ratio, y_ratio)
 * Note:
 * The matrix element has 6 decimal numbers.
 * i.e. If the element is 954301, 954301/1000000 = 0.954301,
 * the real value is 0.954301.
 */
typedef struct axAVS_INTRINSIC_PARAM_T
{
    AX_F32                              nCameraMatrix[9];
    AX_F32                              nDistortionCoeff[8];
    union
    {
        AX_F32                              nNormalDistortionCoeff[8];
        AX_F32                              nFisheyeDistortionCoeff[8];
    };
} AX_AVS_INTRINSIC_PARAM_T;

/*
 * AX_AVS_EXTRINSIC_PARAM_T
 * Transform Param =
 *  (Tx，Ty, Tz, Tyaw, Tpitch)
 * Rotation Param =
 *  (Yaw, Pitch, Roll)
 */
typedef struct axAVS_EXTRINSIC_PARAM_T
{
    AX_F32                              nTranslationParam[5];
    AX_F32                              nRotationParam[3];
} AX_AVS_EXTRINSIC_PARAM_T;

typedef enum axAVS_CAMERA_TYPE_E
{
    AVS_CAMERA_TYPE_PINHOLE = 0,
    AVS_CAMERA_TYPE_FISHEYE,
    AVS_CAMERA_TYPE_BUTT
} AX_AVS_CAMERA_TYPE_E;

typedef struct axAVS_GRP_CAMERA_PARAM_T
{
    AX_AVS_CAMERA_TYPE_E                enCameraType;
    AX_AVS_EXTRINSIC_PARAM_T            stPipeExtrinsic[AX_AVS_PIPE_NUM];
    AX_AVS_INTRINSIC_PARAM_T            stPipeIntrinsic[AX_AVS_PIPE_NUM];
    AX_S32                              s32ImgWidth;
    AX_S32                              s32ImgHeight;
    AX_AVS_FOV_T                        stImgFOV[AX_AVS_PIPE_NUM];
    AX_S32                              s32PanoWidth;
    AX_S32                              s32PanoHeight;
    AX_AVS_FOV_T                        stPanoFOV;
} AX_AVS_GRP_CAMERA_PARAM_T;

typedef enum axAVS_CALIBRATION_MODE_E
{
    AVS_CALIBRATION_PARAM_TRANSFORM = 0,
    AVS_CALIBRATION_PARAM_CAMERA,
    AVS_CALIBRATION_PARAM_FILE,
    AVS_CALIBRATION_PARAM_BUTT
} AX_AVS_CALIBRATION_MODE_E;

typedef enum axAVS_GAIN_E
{
    AVS_GAIN_MODE_MANUAL = 0,
    AVS_GAIN_MODE_AUTO,
    AVS_GAIN_MODE_BUTT
} AX_AVS_GAIN_MODE_E;

typedef struct axAVS_GAIN_ATTR_T
{
    AX_AVS_GAIN_MODE_E          enGainMode;
    AX_S32                      s32Coef[AX_AVS_PIPE_NUM];
} AX_AVS_GAIN_ATTR_T;

typedef enum axAVS_PROJECTION_MODE_E
{
    AVS_PROJECTION_EQUIRECTANGULER = 0,
    AVS_PROJECTION_RECTLINEAR = 10,
    AVS_PROJECTION_CYLINDRICAL = 20,
    AVS_PROJECTION_CUBE_MAP = 30,
    AVS_PROJECTION_BUTT
} AX_AVS_PROJECTION_MODE_E;


/* unit: 0.01° */
typedef struct axAVS_ROTATION_T
{
    AX_F32                      f32Yaw;
    AX_F32                      f32Pitch;
    AX_F32                      f32Roll;
} AX_AVS_ROTATION_T;

typedef struct axAVS_CUBE_MAP_ATTR_T
{
    AX_BOOL                     bBgColor;
    AX_U32                      u32BgColor;
    AX_U32                      u32SurfaceLength;
    AX_AVS_POINT_T              stStartPoint[AX_AVS_CUBE_MAP_SURFACE_NUM];
} AX_AVS_CUBE_MAP_ATTR_T;

typedef struct axAVS_OUTPUT_ATTR_T
{
    AX_AVS_PROJECTION_MODE_E    enPrjMode;
    AX_AVS_POINT_T              stCenter;
    AX_AVS_FOV_T                stFOV;
    AX_AVS_ROTATION_T           stRotation;
    AX_AVS_CUBE_MAP_ATTR_T      stCubeMapAttr;
    AX_U32                      u32Width;
    AX_U32                      u32Height;
} AX_AVS_OUTPUT_ATTR_T;

typedef struct axAVS_FRAME_RATE_CTRL_T
{
    AX_S32                      s32SrcFrameRate;
    AX_S32                      s32DstFrameRate;
} AX_AVS_FRAME_RATE_CTRL_T;

typedef enum axAVS_BLEND_MODE_E
{
    AVS_BLEND_ALPHA = 0,
    AVS_BLEND_PYRAMID,
    AVS_BLEND_EQUAL,
    AVS_BLEND_BUTT
} AX_AVS_BLEND_MODE_E;

typedef struct axAVS_GRP_ATTR_T
{
    AX_AVS_MODE_E               enMode; /* RW; Mode of AVS. */
    AX_U32                      u32PipeNum; /* RW; Range:[2 ~ AX_AVS_PIPE_NUM]; Number of pipe. */
    AX_BOOL                     bSyncPipe; /* RW; Range:[2 ~ AX_AVS_PIPE_NUM]; Number of pipe. */
    AX_AVS_BLEND_MODE_E         enBlendMode; /* RW; Mode of blend. Valid when in AVS_MODE_BLEND enMode. */
    AX_BOOL                     bDynamicSeam; /* RW; Enable dynamical seam finder */

    AX_AVS_CALIBRATION_MODE_E   enCalibrationMode; /* W; Calibration Param Mode. */
    union
    {
        AX_AVS_GRP_TRANSFORM_PARAM_T    stGrpTransformParam; /* W; Transform param mode. Deprecatied do not use. */
        AX_AVS_GRP_CAMERA_PARAM_T       stGrpCameraParam; /* W; Calibration param mode. Recommened use. */
        AX_CHAR*                        pGrpCalibrationFile; /* W; Calibration param file mode. */
    };

    AX_AVS_GAIN_ATTR_T          stGainAttr; /* RW; Luma and chroma gain attribute. Not support now */
    AX_AVS_OUTPUT_ATTR_T        stOutAttr; /* RW; Attribute of AVS output. Not support now. */
    AX_AVS_FRAME_RATE_CTRL_T    stFrameRate; /* RW; FrameRate control of AVS Group. Not support now. */
} AX_AVS_GRP_ATTR_T;

typedef struct axAVS_CHN_ATTR_T
{
    AX_AVS_OUTPUT_ATTR_T        stOutAttr;
    AX_U32                      u32Depth;
    AX_BOOL                     bBlockEnable;
    AX_FRAME_COMPRESS_INFO_T    stCompressInfo;
} AX_AVS_CHN_ATTR_T;

AX_S32 AX_AVS_CreateGrp(AX_AVS_GRP AVSGrp, const AX_AVS_GRP_ATTR_T *pstGrpAttr);
AX_S32 AX_AVS_DestroyGrp(AX_AVS_GRP AVSGrp);
AX_S32 AX_AVS_StartGrp(AX_AVS_GRP AVSGrp);
AX_S32 AX_AVS_StopGrp(AX_AVS_GRP AVSGrp);
AX_S32 AX_AVS_ResetGrp(AX_AVS_GRP AVSGrp);
AX_S32 AX_AVS_GetGrpAttr(AX_AVS_GRP AVSGrp, AX_AVS_GRP_ATTR_T * pstGrpAttr);
AX_S32 AX_AVS_SetGrpAttr(AX_AVS_GRP AVSGrp, const AX_AVS_GRP_ATTR_T *pstGrpAttr);
AX_S32 AX_AVS_SendPipeFrame(AX_AVS_GRP AVSGrp, AX_AVS_PIPE AVSPipe, const AX_VIDEO_FRAME_INFO_T *pstVideoFrame, AX_S32 s32MilliSec);

AX_S32 AX_AVS_SetDebugFifoDepth(AX_AVS_GRP AVSGrp, AX_AVS_PIPE AVSPipe, AX_S32 s32DebugFifoDepth);
AX_S32 AX_AVS_GetDebugFifoFrame(AX_AVS_GRP AVSGrp, AX_AVS_PIPE AVSPipe, AX_VIDEO_FRAME_INFO_T *pstVideoFrame);
AX_S32 AX_AVS_ReleaseDebugFifoFrame(AX_AVS_GRP AVSGrp, AX_AVS_PIPE AVSPipe, const AX_VIDEO_FRAME_INFO_T *pstVideoFrame);
AX_S32 AX_AVS_SetChnAttr(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn, AX_AVS_CHN_ATTR_T *pstChnAttr);
AX_S32 AX_AVS_GetChnAttr(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn, AX_AVS_CHN_ATTR_T *pstChnAttr);
AX_S32 AX_AVS_EnableChn(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn);
AX_S32 AX_AVS_DisableChn(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn);
AX_S32 AX_AVS_GetChnFrame(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn, AX_VIDEO_FRAME_INFO_T *pstVideoFrame, AX_S32 s32MilliSec);
AX_S32 AX_AVS_ReleaseChnFrame(AX_AVS_GRP AVSGrp, AX_AVS_CHN AVSChn, const AX_VIDEO_FRAME_INFO_T *pstVideoFrame);

#ifdef __cplusplus
}
#endif

#endif // _AX_AVS_API_H

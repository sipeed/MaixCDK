/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_VIN_API_INT_H__
#define __AX_VIN_API_INT_H__
#include "ax_vin_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum _AX_VIN_PRIV_FRAME_SOURCE_ID_E_ {
    AX_VIN_PRIV_RAW_SOURCE_ID_MIN                   = -1,
    AX_VIN_PRIV_SOURCE_ID_IFE             = 0,   /* write out from IFE, for user mode */
    AX_VIN_PRIV_SOURCE_ID_ITP             = 1,   /* write out from ITP, for user mode */
    AX_VIN_PRIV_SOURCE_ID_NRLITE_PST_DFRAME,
    AX_VIN_PRIV_SOURCE_ID_NRLITE_PST_RESIDUE,
    AX_VIN_PRIV_SOURCE_ID_NRLITE_PST_MASK,
    AX_VIN_PRIV_SOURCE_ID_RAW2DNR_MASK,
    AX_VIN_PRIV_SOURCE_ID_RGB_CLC,
    AX_VIN_PRIV_SOURCE_ID_YUV3DNR_MASK,
    AX_VIN_PRIV_SOURCE_ID_LCE_MASK,
    AX_VIN_PRIV_SOURCE_ID_YUV,
    AX_VIN_PRIV_SOURCE_ID_MAX
} AX_VIN_PRIV_FRAME_SOURCE_ID_E;

typedef enum _AX_VIN_PRIV_PIPE_DUMP_NODE_E_ {
    AX_VIN_PRIV_PIPE_DUMP_NODE_MIN                  = -1,
    AX_VIN_PRIV_PIPE_DUMP_NODE_IFE                  = 0x1,          /* write data from ife to ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_HDR_MASK             = 0x2,          /* write hdr data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_NRLITE_PRE_DFRAME    = 0x4,
    AX_VIN_PRIV_PIPE_DUMP_NODE_AI_3DNR              = 0x8,          /* write ai-3dnr data frame data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_WBC                  = 0x10,         /* write raw post data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_RLTM                 = 0x20,         /* write raw post data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_RGB_CLC              = 0x40,         /* write raw post data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_RGB_DEHAZE           = 0x80,         /* write raw post data into ddr */
    AX_VIN_PRIV_PIPE_DUMP_NODE_MAIN                 = 0x100,         /* write data from main chn */
    AX_VIN_PRIV_PIPE_DUMP_NODE_SUB1                 = 0x200,         /* write data from sub1 chn */
    AX_VIN_PRIV_PIPE_DUMP_NODE_SUB2                 = 0x400,         /* write data from sub2 chn */
    AX_VIN_PRIV_PIPE_DUMP_NODE_NRLITE_PST_REF       = 0x800,         /* write data from nrlite_pst ref */
    AX_VIN_PRIV_PIPE_DUMP_NODE_RAW_SCALER           = 0x1000,        /* write data from raw scaler */
    AX_VIN_PRIV_PIPE_DUMP_NODE_AI_3DNR_PREV_MASK    = 0x1400,        /* write data from ai3dnr prev mask */
    AX_VIN_PRIV_PIPE_DUMP_NODE_MAX                  = 0xFFFF,        /* write data from sub2 chn */
} AX_VIN_PRIV_PIPE_DUMP_NODE_E;

typedef enum _AX_VIN_PRIV_ME_STORE_MODE_ {
    AX_VIN_PRIV_ME_MERGED_1x1 = 0,
    AX_VIN_PRIV_ME_UNMERGED   = 1, /* not support now */
} AX_VIN_PRIV_ME_STORE_MODE_E;

typedef struct _AX_VIN_PRIV_ME_STORE_INFO_ {
    AX_BOOL MeStoreEnable;
    AX_U8 nDepth;
    AX_VIN_PRIV_ME_STORE_MODE_E mode;
} AX_VIN_PRIV_ME_STORE_INFO;

typedef enum _AX_ME_FIND_STATUS {
    AX_ME_FIND_SUCCESS,
    AX_ME_NOT_FOUND,
    AX_ME_NOT_READY,
} AX_ME_FIND_STATUS;

AX_S32 AX_VIN_PRIV_Init(AX_VOID);
AX_S32 AX_VIN_PRIV_Deinit(AX_VOID);

AX_S32 AX_VIN_PRIV_GetImageBuf(AX_U8 nPipeId, AX_VIN_FRAME_SOURCE_ID_E rawChId, AX_SNS_HDR_FRAME_E eSnsFrame,
                          AX_IMG_INFO_T *pImgInfo, AX_S32 nTimeOutMs);
AX_S32 AX_VIN_PRIV_ReleaseImageBuf(AX_U8 nPipeId, AX_VIN_FRAME_SOURCE_ID_E rawChId, AX_SNS_HDR_FRAME_E eSnsFrame,
                              AX_IMG_INFO_T *pImgInfo);
AX_S32 AX_VIN_PRIV_GetYuvFrame(AX_U8 nPipeId, AX_VIN_CHN_ID_E eChnId, AX_IMG_INFO_T *pImgInfo,
                                        AX_S32 nTimeOutMs);
AX_S32 AX_VIN_PRIV_ReleaseYuvFrame(AX_U8 nPipeId, AX_VIN_CHN_ID_E eChnId, const AX_IMG_INFO_T *pImgInfo);

AX_S32 AX_VIN_PRIV_SetPipeFrameSource(AX_U8 nPipeId, AX_VIN_PRIV_FRAME_SOURCE_ID_E eSrcId, AX_VIN_FRAME_SOURCE_TYPE_E  eSrcType);

AX_S32 AX_VIN_PRIV_GetPipeFrameSource(AX_U8 nPipeId, AX_VIN_PRIV_FRAME_SOURCE_ID_E eSrcId, AX_VIN_FRAME_SOURCE_TYPE_E *eSrcType);

AX_S32 AX_VIN_PRIV_SetPipeDumpAttr(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eDumpNode, AX_VIN_DUMP_QUEUE_TYPE_E eDumpType, const AX_VIN_DUMP_ATTR_T *ptDumpAttr);
AX_S32 AX_VIN_PRIV_GetPipeDumpAttr(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eDumpNode, AX_VIN_DUMP_QUEUE_TYPE_E eDumpType, AX_VIN_DUMP_ATTR_T *ptDumpAttr);

AX_S32 AX_VIN_PRIV_GetRawFrame(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eRawSrcId, AX_SNS_HDR_FRAME_E eSnsFrame,
                          AX_IMG_INFO_T *pImgInfo, AX_S32 nTimeOutMs);
AX_S32 AX_VIN_PRIV_ReleaseRawFrame(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eRawSrcId, AX_SNS_HDR_FRAME_E eSnsFrame,
                              const AX_IMG_INFO_T *pImgInfo);

AX_S32 AX_VIN_PRIV_SendRawFrame(AX_U8 nPipeId, AX_VIN_PRIV_FRAME_SOURCE_ID_E eSrcId, AX_S8 nFrameNum,
                           const AX_IMG_INFO_T *pImgInfo[],
                           AX_S32 nTimeOutMs);

AX_S32 AX_VIN_PRIV_GetRGBFrame(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eRawSrcId, AX_IMG_INFO_T *pImgInfo, AX_S32 nTimeOutMs);

AX_S32 AX_VIN_PRIV_ReleaseRGBFrame(AX_U8 nPipeId, AX_VIN_PRIV_PIPE_DUMP_NODE_E eRawSrcId, const AX_IMG_INFO_T *pImgInfo);

AX_S32 AX_VIN_PRIV_SendRGBFrame(AX_U8 nPipeId, const AX_VIDEO_FRAME_T *pFrameInfo,
                           AX_S32 nTimeOutMs);

AX_S32 AX_VIN_PRIV_SetMeStatInfo(AX_U8 nPipeId, AX_VIN_PRIV_ME_STORE_INFO me_info);
AX_ME_FIND_STATUS AX_VIN_PRIV_FindMeStat(AX_U8 nPipeId, AX_U64 u64SeqNum, AX_U32 nBlkId);

AX_S32 AX_VIN_PRIV_GetPartitionInfo(AX_S32 pipe, void *pPartitionInfo);

AX_S32 AX_VIN_PRIV_GetIDrcParams(AX_S32 nPipeId, void *pIDrcParams);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __AX_VIN_API_INT_H__

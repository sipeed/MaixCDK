/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_VDEC_API_H_
#define _AX_VDEC_API_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_vdec_type.h"

#ifdef __cplusplus
extern "C" {
#endif

AX_S32 AX_VDEC_Init (const AX_VDEC_MOD_ATTR_T *pstModAttr);
AX_S32 AX_VDEC_Deinit (AX_VOID);

AX_S32 AX_VDEC_ExtractStreamHeaderInfo(const AX_VDEC_STREAM_T *pstStreamBuf, AX_PAYLOAD_TYPE_E enVideoType,
                                       AX_VDEC_BITSTREAM_INFO_T *pstBitStreamInfo);

AX_S32 AX_VDEC_CreateGrp (AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_ATTR_T *pstGrpAttr);
AX_S32 AX_VDEC_CreateGrpEx (AX_VDEC_GRP *VdGrp, const AX_VDEC_GRP_ATTR_T *pstGrpAttr);
AX_S32 AX_VDEC_DestroyGrp (AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_GetGrpAttr (AX_VDEC_GRP VdGrp, AX_VDEC_GRP_ATTR_T *pstGrpAttr);
AX_S32 AX_VDEC_SetGrpAttr (AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_ATTR_T *pstGrpAttr);

AX_S32 AX_VDEC_StartRecvStream (AX_VDEC_GRP VdGrp, const AX_VDEC_RECV_PIC_PARAM_T *pstRecvParam);
AX_S32 AX_VDEC_StopRecvStream (AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_QueryStatus (AX_VDEC_GRP VdGrp,AX_VDEC_GRP_STATUS_T *pstGrpStatus);

AX_S32 AX_VDEC_ResetGrp (AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_SetGrpParam (AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_PARAM_T* pstGrpParam);
AX_S32 AX_VDEC_GetGrpParam (AX_VDEC_GRP VdGrp, AX_VDEC_GRP_PARAM_T* pstGrpParam);

AX_S32 AX_VDEC_SelectGrp(AX_VDEC_GRP_SET_INFO_T *pstGrpSet, AX_S32 s32MilliSec);

/* s32MilliSec: -1 is block,0 is no block,other positive number is timeout */
AX_S32 AX_VDEC_SendStream (AX_VDEC_GRP VdGrp, const AX_VDEC_STREAM_T *pstStream, AX_S32 s32MilliSec);

AX_S32 AX_VDEC_GetFrame (AX_VDEC_GRP VdGrp, AX_VIDEO_FRAME_INFO_T *pstFrameInfo, AX_S32 s32MilliSec);
AX_S32 AX_VDEC_ReleaseFrame (AX_VDEC_GRP VdGrp,const AX_VIDEO_FRAME_INFO_T *pstFrameInfo);

AX_S32 AX_VDEC_GetUserData (AX_VDEC_GRP VdGrp, AX_VDEC_USERDATA_T *pstUserData);
AX_S32 AX_VDEC_ReleaseUserData (AX_VDEC_GRP VdGrp, const AX_VDEC_USERDATA_T * pstUserData);

AX_S32 AX_VDEC_SetUserPic (AX_VDEC_GRP VdGrp, const AX_VDEC_USRPIC_T *pstUsrPic);
AX_S32 AX_VDEC_EnableUserPic (AX_VDEC_GRP VdGrp);
AX_S32 AX_VDEC_DisableUserPic (AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_SetDisplayMode(AX_VDEC_GRP VdGrp, AX_VDEC_DISPLAY_MODE_E enDisplayMode);
AX_S32 AX_VDEC_GetDisplayMode(AX_VDEC_GRP VdGrp, AX_VDEC_DISPLAY_MODE_E *penDisplayMode);

AX_S32 AX_VDEC_AttachPool (AX_VDEC_GRP VdGrp, AX_POOL PoolId);
AX_S32 AX_VDEC_DetachPool (AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_JpegDecodeOneFrame(AX_VDEC_DEC_ONE_FRM_T *pstParam);

AX_S32 AX_VDEC_GetStreamBufInfo(AX_VDEC_GRP VdGrp, AX_VDEC_STREAM_BUF_INFO_T *pstStreamBufInfo);
AX_S32 AX_VDEC_GetVuiParam(AX_VDEC_GRP VdGrp, AX_VDEC_VUI_PARAM_T *pstVuiParam);

#ifdef __cplusplus
}
#endif

#endif

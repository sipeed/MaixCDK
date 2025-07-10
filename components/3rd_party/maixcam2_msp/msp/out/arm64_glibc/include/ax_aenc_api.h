/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AENC_HAL_H_
#define _AX_AENC_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_codec_comm.h"
#include "ax_comm_aio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Common define */
#define AX_AENC_MAX_CHN_NUM                 16     /* support 16 channel output */
#define AX_MAX_AUDIO_FRAME_NUM              300

typedef enum {
    AX_ID_AENC_NULL    = 0x01,

    AX_ID_AENC_BUTT,
} AX_AENC_SUB_ID_E;

/* error code */
#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif
#define AX_ERR_AENC_INVALID_CHNID               AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_INVALID_CHNID)
#define AX_ERR_AENC_NULL_PTR                    AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_NULL_PTR)
#define AX_ERR_AENC_EXIST                       AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_EXIST)
#define AX_ERR_AENC_NOMEM                       AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_NOMEM)
#define AX_ERR_AENC_NOT_SUPPORT                 AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_NOT_SUPPORT)
#define AX_ERR_AENC_UNEXIST                     AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_UNEXIST)
#define AX_ERR_AENC_NOBUF                       AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_NOBUF)
#define AX_ERR_AENC_NOT_PERM                    AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_NOT_PERM)
#define AX_ERR_AENC_BUF_FULL                    AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_BUF_FULL)
#define AX_ERR_AENC_BUF_EMPTY                   AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_BUF_EMPTY)
#define AX_ERR_AENC_SYS_NOTREADY                AX_DEF_ERR(AX_ID_AENC, AX_ID_AENC_NULL, AX_ERR_SYS_NOTREADY)

typedef AX_S32 AENC_CHN;

typedef struct axAENC_CHN_ATTR_T {
    AX_PAYLOAD_TYPE_E   enType;             /*payload type ()*/
    AX_U32              u32PtNumPerFrm;
    AX_U32              u32BufSize;         /*buf size [2~MAX_AUDIO_FRAME_NUM]*/
    AX_VOID             *pValue;            /*point to attribute of definite audio encoder*/
    AX_LINK_MODE_E       enLinkMode;
} AX_AENC_CHN_ATTR_T;

typedef struct axAENC_ENCODER_T {
    AX_PAYLOAD_TYPE_E  enType;
    AX_U32          u32MaxFrmLen;
    AX_S8           aszName[17];            /* encoder type,be used to print proc information */
    AX_S32(*pfnOpenEncoder)(AX_VOID *pEncoderAttr,
                            AX_VOID **ppEncoder);           /* pEncoder is the handle to control the encoder */
    AX_S32(*pfnEncodeFrm)(AX_VOID *pEncoder, const AX_AUDIO_FRAME_T *pstData,
                          AX_U8 *pu8Outbuf, AX_U32 *pu32OutLen);
    AX_S32(*pfnCloseEncoder)(AX_VOID *pEncoder);
} AX_AENC_ENCODER_T;

typedef struct axAENC_G726_ENCODER_ATTR_T {
    AX_U32 u32BitRate;
} AX_AENC_G726_ENCODER_ATTR_T;

typedef struct axAENC_G723_ENCODER_ATTR_T {
    AX_U32 u32BitRate;
} AX_AENC_G723_ENCODER_ATTR_T;

AX_S32 AX_AENC_Init();
AX_S32 AX_AENC_DeInit();

AX_S32 AX_AENC_CreateChn(AENC_CHN aeChn, const AX_AENC_CHN_ATTR_T *pstAttr);
AX_S32 AX_AENC_DestroyChn(AENC_CHN aeChn);

AX_S32 AX_AENC_SendFrame(AENC_CHN aeChn, const AX_AUDIO_FRAME_T *pstFrm);

AX_S32 AX_AENC_GetStream(AENC_CHN aeChn, AX_AUDIO_STREAM_T *pstStream, AX_S32 s32MilliSec);
AX_S32 AX_AENC_ReleaseStream(AENC_CHN aeChn, const AX_AUDIO_STREAM_T *pstStream);

AX_S32 AX_AENC_RegisterEncoder(AX_S32 *ps32Handle, const AX_AENC_ENCODER_T *pstEncoder);
AX_S32 AX_AENC_UnRegisterEncoder(AX_S32 s32Handle);

#ifdef __cplusplus
}
#endif

#endif

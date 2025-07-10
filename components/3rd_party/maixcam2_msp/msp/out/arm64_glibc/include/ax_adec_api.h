/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_ADEC_HAL_H_
#define _AX_ADEC_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_sys_api.h"
#include "ax_codec_comm.h"
#include "ax_comm_aio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Common define */
#define AX_ADEC_MAX_CHN_NUM                 16     /* support 16 channel output */

typedef enum {
    AX_ID_ADEC_NULL    = 0x01,

    AX_ID_ADEC_BUTT,
} AX_ADEC_SUB_ID_E;

/* error code */
#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif
/* invlalid device ID */
#define AX_ERR_ADEC_INVALID_DEVID     AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define AX_ERR_ADEC_INVALID_CHNID     AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define AX_ERR_ADEC_ILLEGAL_PARAM     AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_ILLEGAL_PARAM)
/* channel exists */
#define AX_ERR_ADEC_EXIST             AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_EXIST)
/* channel unexists */
#define AX_ERR_ADEC_UNEXIST           AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_UNEXIST)
/* using a NULL point */
#define AX_ERR_ADEC_NULL_PTR          AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define AX_ERR_ADEC_NOT_CONFIG        AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define AX_ERR_ADEC_NOT_SUPPORT       AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define AX_ERR_ADEC_NOT_PERM          AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define AX_ERR_ADEC_NOMEM             AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NOMEM)
/* failure caused by malloc buffer */
#define AX_ERR_ADEC_NOBUF             AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_NOBUF)
/* no data in buffer */
#define AX_ERR_ADEC_BUF_EMPTY         AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_BUF_EMPTY)
/* no buffer for new data */
#define AX_ERR_ADEC_BUF_FULL          AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define AX_ERR_ADEC_SYS_NOTREADY      AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, AX_ERR_SYS_NOTREADY)
/* decoder internal err */
#define AX_ERR_ADEC_DECODER_ERR       AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, 0x80)
/* input buffer not enough to decode one frame */
#define AX_ERR_ADEC_BUF_LACK          AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, 0x81)
/* end of stream */
#define AX_ERR_ADEC_END_OF_STREAM     AX_DEF_ERR(AX_ID_ADEC, AX_ID_ADEC_NULL, 0x82)

typedef AX_S32 ADEC_CHN;

typedef enum axADEC_MODE_E
{
    AX_ADEC_MODE_PACK = 0,/*require input is valid dec pack(a
                            complete frame encode result),
                            e.g.the stream get from AENC is a
                            valid dec pack, the stream know actually
                            pack len from file is also a dec pack.
                            this mode is high-performance*/
    AX_ADEC_MODE_STREAM ,/*input is stream,low-performative,
                            if you couldn't find out whether a stream is
                            vaild dec pack,you could use
                            this mode*/
    AX_ADEC_MODE_BUTT
}AX_ADEC_MODE_E;

typedef struct axADEC_CH_ATTR_T
{
    AX_PAYLOAD_TYPE_E enType;
    AX_U32            u32BufSize;  /*buf size[2~MAX_AUDIO_FRAME_NUM]*/
    AX_ADEC_MODE_E    enMode;      /*decode mode*/
    AX_VOID           *pValue;
    AX_LINK_MODE_E    enLinkMode;
    AX_U32            u32MaxFrameSize; /*0 means use default(aac g726: 32768, opus: 384000, others: 4096)*/
}AX_ADEC_CHN_ATTR_T;

typedef struct axADEC_DECODER_T
{
    AX_PAYLOAD_TYPE_E  enType;
    AX_S8           aszName[17];
    AX_S32          (*pfnOpenDecoder)(AX_VOID *pDecoderAttr, AX_VOID **ppDecoder); /*struct ppDecoder is packed by user,user malloc and free memory for this struct */
    AX_S32          (*pfnDecodeFrm)(AX_VOID *pDecoder, AX_U8 **pu8Inbuf,AX_S32 *ps32LeftByte,
                        AX_U16 *pu16Outbuf,AX_U32 *pu32OutLen,AX_U32 *pu32Chns);
    AX_S32          (*pfnGetFrmInfo)(AX_VOID *pDecoder, AX_VOID *pInfo);
    AX_S32          (*pfnCloseDecoder)(AX_VOID *pDecoder);
    AX_S32          (*pfnResetDecoder)(AX_VOID *pDecoder);
} AX_ADEC_DECODER_T;

typedef struct axADEC_G726_DECODER_ATTR_T {
    AX_U32 u32BitRate;
} AX_ADEC_G726_DECODER_ATTR_T;

typedef struct axADEC_G723_DECODER_ATTR_T {
    AX_U32 u32BitRate;
} AX_ADEC_G723_DECODER_ATTR_T;

/* ADEC function api. */
AX_S32 AX_ADEC_Init();
AX_S32 AX_ADEC_DeInit();

AX_S32 AX_ADEC_CreateChn(ADEC_CHN adChn, const AX_ADEC_CHN_ATTR_T *pstAttr);
AX_S32 AX_ADEC_DestroyChn(ADEC_CHN adChn);

AX_S32 AX_ADEC_SendStream(ADEC_CHN adChn, const AX_AUDIO_STREAM_T *pstStream, AX_BOOL bBlock);

AX_S32 AX_ADEC_ClearChnBuf(ADEC_CHN adChn);

AX_S32 AX_ADEC_GetFrame(ADEC_CHN adChn, AX_AUDIO_FRAME_T *pstFrmInfo, AX_BOOL bBlock);
AX_S32 AX_ADEC_ReleaseFrame(ADEC_CHN adChn, const AX_AUDIO_FRAME_T *pstFrmInfo);
AX_S32 AX_ADEC_SendEndOfStream(ADEC_CHN adChn, AX_BOOL bInstant);

AX_S32 AX_ADEC_RegisterDecoder(AX_S32 *ps32Handle, const AX_ADEC_DECODER_T *pstDecoder);
AX_S32 AX_ADEC_UnRegisterDecoder(AX_S32 s32Handle);

AX_S32 AX_ADEC_AttachPool(ADEC_CHN adChn, AX_POOL PoolId);
AX_S32 AX_ADEC_DetachPool(ADEC_CHN adChn);

#ifdef __cplusplus
}
#endif

#endif

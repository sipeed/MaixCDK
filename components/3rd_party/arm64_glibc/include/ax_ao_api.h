/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AO_HAL_H_
#define _AX_AO_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_comm_aio.h"
#include "ax_audio_process.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AX_ID_AO_NULL    = 0x02,

    AX_ID_AO_BUTT,
} AX_AO_SUB_ID_E;

#define AX_ERR_AO_INVALID_CHNID               AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_INVALID_CHNID)
#define AX_ERR_AO_NULL_PTR                    AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_NULL_PTR)
#define AX_ERR_AO_EXIST                       AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_EXIST)
#define AX_ERR_AO_NOMEM                       AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_NOMEM)
#define AX_ERR_AO_NOT_SUPPORT                 AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_NOT_SUPPORT)
#define AX_ERR_AO_UNEXIST                     AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_UNEXIST)
#define AX_ERR_AO_NOBUF                       AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_NOBUF)
#define AX_ERR_AO_NOT_PERM                    AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_NOT_PERM)
#define AX_ERR_AO_BUF_FULL                    AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_BUF_FULL)
#define AX_ERR_AO_BUF_EMPTY                   AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_BUF_EMPTY)
#define AX_ERR_AO_SYS_NOTREADY                AX_DEF_ERR(AX_ID_AO, AX_ID_AO_NULL, AX_ERR_SYS_NOTREADY)

typedef AX_S32 AO_CARD;
typedef AX_S32 AO_DEV;

typedef struct axAO_ATTR_T {
    AX_AUDIO_SAMPLE_RATE_E  enSamplerate;   /* sample rate */
    AX_AUDIO_BIT_WIDTH_E    enBitwidth;      /* bitwidth */
    AX_AUDIO_SOUND_MODE_E   enSoundmode;     /*audio frame momo or stereo mode*/
    AX_U32                  u32ChnCnt;      /* channle number on FS, valid value:1/2/4/8 */
    AX_U32                  u32PeriodSize;      /* point num per frame (80/160/240/320/480/1024/2048/4096)*/
    AX_U32                  u32PeriodCount;      /* frame num in buf */
    AX_U32                  U32Depth;
    AX_LINK_MODE_E          enLinkMode;
    AX_BOOL                 bInsertSilence;
} AX_AO_ATTR_T;

typedef struct axAO_DEV_STATE_T {
    AX_U32                  u32DevTotalNum;    /* total number of device buffer */
    AX_U32                  u32DevFreeNum;     /* free number of device buffer */
    AX_U32                  u32DevBusyNum;     /* busy number of device buffer */
    AX_LONG                 longPcmBufDelay;   /* the delay of pcm in terms of frames */
} AX_AO_DEV_STATE_T;

/* AO function api. */
AX_S32 AX_AO_Init();
AX_S32 AX_AO_DeInit();

AX_S32 AX_AO_SetPubAttr(AO_CARD aoCardId,AO_DEV aoDevId, const AX_AO_ATTR_T *pstAttr);
AX_S32 AX_AO_GetPubAttr(AO_CARD aoCardId, AO_DEV aoDevId, AX_AO_ATTR_T *pstAttr);

AX_S32 AX_AO_EnableDev(AO_CARD aoCardId, AO_DEV aoDevId);
AX_S32 AX_AO_DisableDev(AO_CARD aoCardId, AO_DEV aoDevId);

AX_S32 AX_AO_SendFrame(AO_CARD aoCardId, AO_DEV aoDevId, AX_AUDIO_FRAME_T *pstFrame, AX_AUDIO_FRAME_T *pstMixFrame, AX_F32 f32MixWeight, AX_S32 s32MilliSec);

AX_S32 AX_AO_EnableResample(AO_CARD aoCardId, AO_DEV aoDevId, AX_AUDIO_SAMPLE_RATE_E enInSampleRate);
AX_S32 AX_AO_DisableResample(AO_CARD aoCardId, AO_DEV aoDevId);

AX_S32 AX_AO_SetDnVqeAttr(AO_CARD aoCardId,AO_DEV aoDevId, const AX_AP_DNVQE_ATTR_T *pstVqeAttr);
AX_S32 AX_AO_GetDnVqeAttr(AO_CARD aoCardId, AO_DEV aoDevId, AX_AP_DNVQE_ATTR_T *pstVqeConfig);

AX_S32 AX_AO_ClearDevBuf(AO_CARD aoCardId, AO_DEV aoDevId);
AX_S32 AX_AO_QueryDevStat(AO_CARD aoCardId, AO_DEV aoDevId, AX_AO_DEV_STATE_T *pstStatus);

AX_S32 AX_AO_PauseRecvFrame(AO_CARD aoCardId, AO_DEV aoDevId);
AX_S32 AX_AO_ResumeRecvFrame(AO_CARD aoCardId, AO_DEV aoDevId);

AX_S32 AX_AO_SaveFile(AO_CARD aoCardId, AO_DEV aoDevId, const AX_AUDIO_SAVE_FILE_INFO_T *pstSaveFileInfo);
AX_S32 AX_AO_QueryFileStatus(AO_CARD aoCardId, AO_DEV aoDevId, AX_AUDIO_FILE_STATUS_T* pstFileStatus);

AX_S32 AX_AO_SetVqeVolume(AO_CARD aoCardId, AO_DEV aoDevId, AX_F64 f64VqeVolume);
AX_S32 AX_AO_GetVqeVolume(AO_CARD aoCardId, AO_DEV aoDevId, AX_F64 *pf64VqeVolume);

AX_S32 AX_AO_SetVqeMute(AO_CARD aoCardId, AO_DEV aoDevId, AX_BOOL bEnable, const AX_AUDIO_FADE_T *pstFade);
AX_S32 AX_AO_GetVqeMute(AO_CARD aoCardId, AO_DEV aoDevId, AX_BOOL *pbEnable, AX_AUDIO_FADE_T *pstFade);

#ifdef __cplusplus
}
#endif

#endif

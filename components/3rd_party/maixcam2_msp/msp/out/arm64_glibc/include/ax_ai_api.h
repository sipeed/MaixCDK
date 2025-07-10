/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AI_HAL_H_
#define _AX_AI_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_sys_api.h"
#include "ax_comm_aio.h"
#include "ax_audio_process.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AX_ID_AI_NULL    = 0x02,

    AX_ID_AI_BUTT,
} AX_AI_SUB_ID_E;

#define AX_ERR_AI_INVALID_CHNID               AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_INVALID_CHNID)
#define AX_ERR_AI_NULL_PTR                    AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_NULL_PTR)
#define AX_ERR_AI_EXIST                       AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_EXIST)
#define AX_ERR_AI_NOMEM                       AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_NOMEM)
#define AX_ERR_AI_NOT_SUPPORT                 AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_NOT_SUPPORT)
#define AX_ERR_AI_UNEXIST                     AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_UNEXIST)
#define AX_ERR_AI_NOBUF                       AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_NOBUF)
#define AX_ERR_AI_NOT_PERM                    AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_NOT_PERM)
#define AX_ERR_AI_BUF_FULL                    AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_BUF_FULL)
#define AX_ERR_AI_BUF_EMPTY                   AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_BUF_EMPTY)
#define AX_ERR_AI_SYS_NOTREADY                AX_DEF_ERR(AX_ID_AI, AX_ID_AI_NULL, AX_ERR_SYS_NOTREADY)

typedef AX_S32 AI_CARD;
typedef AX_S32 AI_DEV;

typedef enum {
    AX_AI_MIC_MIC = 0,
    AX_AI_MIC_REF = 1,
    AX_AI_REF_MIC = 2,
    AX_AI_INTERNAL_MIC_NULL = 3,
    AX_AI_INTERNAL_NULL_MIC = 4,
    AX_AI_DOORBELL = 5,
    AX_AI_MIC_MIC_DEAL = 6,
    AX_AI_BUTT,
} AX_AI_LAYOUT_MODE_E;

typedef struct axAI_ATTR_T {
    AX_AUDIO_SAMPLE_RATE_E  enSamplerate;   /* sample rate */
    AX_AUDIO_BIT_WIDTH_E    enBitwidth;      /* bitwidth */
    AX_U32                  u32ChnCnt;      /* channle number on FS, valid value:1/2/4/8 */
    AX_U32                  u32PeriodSize;      /* point num per frame (80/160/240/320/480/1024/2048/4096)*/
    AX_U32                  u32PeriodCount;      /* frame num in buf */
    AX_U32                  U32Depth;
    AX_LINK_MODE_E          enLinkMode;
    AX_AI_LAYOUT_MODE_E     enLayoutMode;
    AX_F32                  f2mic_distance;
} AX_AI_ATTR_T;

typedef struct axAED_ATTR_T {
    AX_BOOL bDbDetection;
} AX_AED_ATTR_T;

typedef struct axAED_RESULT_INFO {
    AX_S32 s32Db;
} AX_AED_RESULT_INFO;

/* AI function api. */
AX_S32 AX_AI_Init();
AX_S32 AX_AI_DeInit();

AX_S32 AX_AI_SetPubAttr(AI_CARD aiCardId,AI_DEV aiDevId, const AX_AI_ATTR_T *pstAttr);
AX_S32 AX_AI_GetPubAttr(AI_CARD aiCardId, AI_DEV aiDevId,AX_AI_ATTR_T *pstAttr);

AX_S32 AX_AI_EnableDev(AI_CARD aiCardId, AI_DEV aiDevId);
AX_S32 AX_AI_DisableDev(AI_CARD aiCardId, AI_DEV aiDevId);

AX_S32 AX_AI_GetFrame(AI_CARD aiCardId, AI_DEV aiDevId, AX_AUDIO_FRAME_T* aiFrame, AX_S32 s32MilliSec);
AX_S32 AX_AI_ReleaseFrame(AI_CARD aiCardId, AI_DEV aiDevId,AX_AUDIO_FRAME_T* aiFrame);

AX_S32 AX_AI_EnableResample(AI_CARD aiCardId, AI_DEV aiDevId, AX_AUDIO_SAMPLE_RATE_E enOutSampleRate);
AX_S32 AX_AI_DisableResample(AI_CARD aiCardId, AI_DEV aiDevId);

AX_S32 AX_AI_SetUpTalkVqeAttr(AI_CARD aiCardId, AI_DEV aiDevId, const AX_AP_UPTALKVQE_ATTR_T *pstVqeAttr);
AX_S32 AX_AI_GetUpTalkVqeAttr(AI_CARD aiCardId, AI_DEV aiDevId, AX_AP_UPTALKVQE_ATTR_T *pstVqeAttr);

AX_S32 AX_AI_AttachPool(AI_CARD aiCardId, AI_DEV aiDevId, AX_POOL PoolId);
AX_S32 AX_AI_DetachPool(AI_CARD aiCardId, AI_DEV aiDevId);

AX_S32 AX_AI_SaveFile(AI_CARD aiCardId, AI_DEV aiDevId, const AX_AUDIO_SAVE_FILE_INFO_T *pstSaveFileInfo);
AX_S32 AX_AI_QueryFileStatus(AI_CARD aiCardId, AI_DEV aiDevId, AX_AUDIO_FILE_STATUS_T* pstFileStatus);

AX_S32 AX_AI_SetVqeVolume(AI_CARD aiCardId, AI_DEV aiDevId, AX_F64 f64VqeVolume);
AX_S32 AX_AI_GetVqeVolume(AI_CARD aiCardId, AI_DEV aiDevId, AX_F64 *pf64VqeVolume);

AX_S32 AX_AI_SetAedAttr(AI_CARD aiCardId, AI_DEV aiDevId, AX_AED_ATTR_T *pstAedAttr);
AX_S32 AX_AI_GetAedAttr(AI_CARD aiCardId, AI_DEV aiDevId, AX_AED_ATTR_T *pstAedAttr);

AX_S32 AX_AI_EnableAed(AI_CARD aiCardId, AI_DEV aiDevId);
AX_S32 AX_AI_DisableAed(AI_CARD aiCardId, AI_DEV aiDevId);

AX_S32 AX_AI_GetAedResult(AI_CARD aiCardId, AI_DEV aiDevId, AX_AED_RESULT_INFO *pstAedResultInfo);

#ifdef __cplusplus
}
#endif

#endif

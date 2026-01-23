/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_OPUS_HAL_H_
#define _AX_OPUS_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_aenc_api.h"
#include "ax_adec_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SC_OPUS_MAX_PACKET      1500

typedef enum axOPUS_APPLICATION_E {
    AX_OPUS_APPLICATION_UNKNOWN = 0,
    AX_OPUS_APPLICATION_VOIP = 2048,
    AX_OPUS_APPLICATION_AUDIO = 2049,
    AX_OPUS_APPLICATION_RESTRICTED_LOWDELAY = 2051,

    AX_OPUS_APPLICATION_BUTT,
} AX_OPUS_APPLICATION_E;

typedef struct axAENC_OPUS_ENCODER_ATTR_T {
    AX_OPUS_APPLICATION_E enApplication;
    AX_U32 u32SamplingRate; /* 8000, 12000, 16000, 24000 and 48000*/
    AX_S32 s32Channels; /* 1 or 2 */
    AX_S32 s32BitrateBps;
    AX_F32 f32FramesizeInMs; /* 2.5, 5, 10, 20, 40, 60, 80, 100, 120 */
} AX_AENC_OPUS_ENCODER_ATTR_T;

typedef struct axADEC_OPUS_DECODER_ATTR_T {
    AX_U32 u32SamplingRate; /* 8000, 12000, 16000, 24000 and 48000*/
    AX_S32 s32Channels; /* 1 or 2 */
} AX_ADEC_OPUS_DECODER_ATTR_T;

AX_S32 AX_AENC_OpusInit();
AX_S32 AX_AENC_OpusDeInit();
AX_S32 AX_ADEC_OpusInit();
AX_S32 AX_ADEC_OpusDeInit();

#ifdef __cplusplus
}
#endif

#endif

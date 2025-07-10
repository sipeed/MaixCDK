/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AAC_HAL_H_
#define _AX_AAC_HAL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_aenc_api.h"
#include "ax_adec_api.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum axAAC_TYPE_E {
    AX_AAC_TYPE_NONE = -1,
    AX_AAC_TYPE_NULL_OBJECT = 0,
    AX_AAC_TYPE_AAC_LC = 2,       /**< Low Complexity object                     */
    AX_AAC_TYPE_ER_AAC_LD = 23,   /**< Error Resilient(ER) AAC LowDelay object   */
    AX_AAC_TYPE_ER_AAC_ELD = 39,  /**< AAC Enhanced Low Delay                    */

    AX_AAC_TYPE_BUTT,
} AX_AAC_TYPE_E;

typedef enum axAAC_CHANNEL_MODE_E {
    AX_AAC_CHANNEL_MODE_INVALID = -1,
    AX_AAC_CHANNEL_MODE_UNKNOWN = 0,
    AX_AAC_CHANNEL_MODE_1 = 1,         /**< C */
    AX_AAC_CHANNEL_MODE_2 = 2,         /**< L+R */
    AX_AAC_CHANNEL_MODE_1_2 = 3,       /**< C, L+R */
    AX_AAC_CHANNEL_MODE_1_2_1 = 4,     /**< C, L+R, Rear */
    AX_AAC_CHANNEL_MODE_1_2_2 = 5,     /**< C, L+R, LS+RS */
    AX_AAC_CHANNEL_MODE_1_2_2_1 = 6,   /**< C, L+R, LS+RS, LFE */

    AX_AAC_CHANNEL_MODE_BUTT,
} AX_AAC_CHANNEL_MODE_E;

typedef struct axAENC_AAC_ENCODER_ATTR_T {
    AX_AAC_TYPE_E enAacType;
    AX_AAC_TRANS_TYPE_E enTransType;
    AX_AAC_CHANNEL_MODE_E enChnMode;
    AX_U32 u32GranuleLength;
    AX_U32 u32SampleRate;
    AX_U32 u32BitRate;
    AX_U8 u8ConfBuf[64]; /*!< Configuration buffer in binary format as an
                          AudioSpecificConfig or StreamMuxConfig according to the
                          selected transport type. */
} AX_AENC_AAC_ENCODER_ATTR_T;

typedef struct axADEC_AAC_DECODER_ATTR_T {
    AX_AAC_TRANS_TYPE_E enTransType;
    AX_U8 **u8Conf;
    AX_U32 u32ConfLen;
} AX_ADEC_AAC_DECODER_ATTR_T;

AX_S32 AX_AENC_FaacInit();
AX_S32 AX_AENC_FaacDeInit();
AX_S32 AX_ADEC_FaacInit();
AX_S32 AX_ADEC_FaacDeInit();

AX_S32 AX_AENC_FdkInit();
AX_S32 AX_AENC_FdkDeInit();
AX_S32 AX_ADEC_FdkInit();
AX_S32 AX_ADEC_FdkDeInit();

#ifdef __cplusplus
}
#endif

#endif

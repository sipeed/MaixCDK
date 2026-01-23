/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_CODEC_COMM_H__
#define __AX_CODEC_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


typedef enum {
    AX_FRAME_TYPE_NONE = 0,
    AX_FRAME_TYPE_AUTO,
    AX_FRAME_TYPE_IDR,
    AX_FRAME_TYPE_I,
    AX_FRAME_TYPE_P,
    AX_FRAME_TYPE_B,

    AX_FRAME_TYPE_BUTT,
} AX_FRAME_TYPE_E;

typedef enum axAAC_TRANS_TYPE_E {
    AX_AAC_TRANS_TYPE_UNKNOWN = -1, /**< Unknown format.            */
    AX_AAC_TRANS_TYPE_RAW = 0,  /**< "as is" access units (packet based since there is
                                obviously no sync layer) */
    AX_AAC_TRANS_TYPE_ADTS = 2, /**< ADTS bitstream format.     */

    AX_AAC_TRANS_TYPE_BUTT,
} AX_AAC_TRANS_TYPE_E;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of __AX_CODEC_COMM_H__ */


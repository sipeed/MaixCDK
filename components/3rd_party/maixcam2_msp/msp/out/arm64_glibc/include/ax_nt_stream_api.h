/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _DEF_NT_STREAM_API_H_
#define _DEF_NT_STREAM_API_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 AX_NT_StreamInit(AX_U32 nStreamPort);
AX_S32 AX_NT_StreamDeInit(void);
AX_S32 AX_NT_SetStreamSource(AX_U8 pipe);

#ifdef __cplusplus
}
#endif

#endif //_DEF_NT_STREAM_API_H_

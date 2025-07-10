/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_VDEC_CMD_H__
#define __COMMON_VDEC_CMD_H__


#include <stdint.h>
#include <unistd.h>

#include "ax_vdec_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "common_vdec_utils.h"

#include "ax_vdec_api.h"
#include "common_arg_parse.h"

#define VDEC_CMD_MAX_ARG_LEN (100)

AX_S32 OpenTestHooks(const SAMPLE_VDEC_CONTEXT_T *pstVdecCtx);


AX_S32 VdecDefaultParamsSet(SAMPLE_VDEC_CMD_PARAM_T *pstCmdPara);
AX_S32 VdecCmdLineParseAndCheck(AX_S32 argc, AX_CHAR **argv, SAMPLE_VDEC_CMD_PARAM_T *pstPara, AX_VDEC_GRP VdGrp, AX_BOOL bLink);

#ifdef __cplusplus
}
#endif
#endif

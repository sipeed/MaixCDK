/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_ARG_PARSE_H__
#define __COMMON_ARG_PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ax_global_type.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


typedef struct _SAMPLE_OPTION
{
    AX_CHAR *long_opt;
    AX_S8 short_opt;
    AX_BOOL enable;
} SAMPLE_OPTION_T;

typedef struct _SAMPLE_PARAMETER
{
    AX_S32 cnt;
    AX_CHAR *argument;
    AX_S8 short_opt;
    AX_CHAR *longOpt;
    AX_S32 enable;
} SAMPLE_PARAMETER_T;


#define AX_OPTION_NAME_LEN                (32)

typedef struct axSAMPLE_OPTION_NAME_T {
    AX_CHAR name[AX_OPTION_NAME_LEN];
} SAMPLE_OPTION_NAME_T;



enum TBCfgCallbackResult {
    TB_CFG_OK,
    TB_CFG_ERROR = 500,
    TB_CFG_INVALID_BLOCK = 501,
    TB_CFG_INVALID_PARAM = 502,
    TB_CFG_INVALID_VALUE = 503,
    TB_CFG_INVALID_CODE = 504,
    TB_CFG_DUPLICATE_BLOCK = 505
};

enum TBCfgCallbackParam {
    TB_CFG_CALLBACK_BLK_START = 300,
    TB_CFG_CALLBACK_VALUE = 301,
};

typedef enum TBCfgCallbackResult(*TBCfgCallback)(char *, char *, char *, enum TBCfgCallbackParam, void *);


AX_S32 SampleGetOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option, SAMPLE_PARAMETER_T * parameter);
AX_S32 SampleParseDelim(AX_CHAR *optArg, AX_CHAR delim);
AX_S32 SampleOptionsFill(SAMPLE_OPTION_T *p, int offset, AX_CHAR *long_opt, AX_S8 short_opt, AX_BOOL enable);
AX_BOOL TBParseConfig(char *filename, TBCfgCallback callback, void *cb_param);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif
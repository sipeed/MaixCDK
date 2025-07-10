/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_VIN_ERROR_CODE_H_
#define _AX_VIN_ERROR_CODE_H_

#include "ax_global_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
|----------------------------------------------------------------|
||   FIXED   |   MOD_ID    | SUB_MODULE_ID |   ERR_ID            |
|----------------------------------------------------------------|
|<--8bits----><----8bits---><-----8bits---><------8bits------->|
******************************************************************************/

/* VIN Error Code Base: 0x8011xxxx */
typedef enum {
    AX_ID_VIN_NULL      = 0x01,
    AX_ID_VIN_BUTT,
} AX_VIN_SUB_ID_E;

typedef enum {
    AX_ERR_CODE_VIN_FAILED                   = 0x80,//VIN Failed
    AX_ERR_CODE_VIN_PIPE_CREATE_ALREADY,
    AX_ERR_CODE_VIN_DRIVER_NOT_LOAD,
    AX_ERR_CODE_VIN_SNS_UNREGISTER,
    AX_ERR_CODE_VIN_WAIT_ABNORMAL,
    AX_ERR_CODE_VIN_CMM_NOT_ENOUGH,
} AX_VIN_ERR_CODE_E;

/* Success */
#define AX_VIN_SUCCESS                      (0)

/* Common Err Code */
#define AX_ERR_VIN_NULL_PTR                 AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NULL_PTR)
#define AX_ERR_VIN_INVALID_DEVID            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_DEVID)
#define AX_ERR_VIN_INVALID_PIPEID           AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_PIPEID)
#define AX_ERR_VIN_INVALID_CHNID            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_CHNID)
#define AX_ERR_VIN_ILLEGAL_PARAM            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_VIN_NOT_SUPPORT              AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_SUPPORT)
#define AX_ERR_VIN_NOMEM                    AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOMEM)
#define AX_ERR_VIN_TIMEOUT                  AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_TIMED_OUT)
#define AX_ERR_VIN_RES_EMPTY                AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_VIN_NOT_INIT                 AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_INIT)
#define AX_ERR_VIN_ATTR_NOT_CFG             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_CONFIG)
#define AX_ERR_VIN_INVALID_ADDR             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_BAD_ADDR)
#define AX_ERR_VIN_OBJ_EXIST                AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_EXIST)
#define AX_ERR_VIN_OBJ_UNEXIST              AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_UNEXIST)
#define AX_ERR_VIN_STATUS_MISMATCH          AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_PERM)


/* Private Err Code */
#define AX_ERR_VIN_FAILED                   AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_FAILED)
#define AX_ERR_VIN_SNS_UNREGISTER           AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_SNS_UNREGISTER)
#define AX_ERR_VIN_PIPE_CREATE_ALREADY      AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_PIPE_CREATE_ALREADY)
#define AX_ERR_VIN_WAIT_ABNORMAL            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_WAIT_ABNORMAL)
#define AX_ERR_VIN_CMM_NOT_ENOUGH           AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_CMM_NOT_ENOUGH)

#define AX_VIN_ERR_CODE(MODULE, ERR_CODE)   AX_DEF_ERR(AX_ID_VIN, MODULE, ERR_CODE)

#ifdef __cplusplus
}
#endif

#endif //_AX_VIN_ERROR_CODE_H_

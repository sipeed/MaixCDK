/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_ISP_ERROR_CODE_H_
#define _AX_ISP_ERROR_CODE_H_

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

/* ISP Error Code Base: 0x8001xxxx */
typedef enum {
    AX_ID_ISP_NULL      = 0x01,
    AX_ID_ISP_SIF       = 0x02,
    AX_ID_ISP_DPC       = 0x03,
    AX_ID_ISP_BLC       = 0x04,
    AX_ID_ISP_DS        = 0x05,
    AX_ID_ISP_FPN       = 0x06,
    AX_ID_ISP_GBL       = 0x07,
    AX_ID_ISP_AE        = 0x08,
    AX_ID_ISP_AF        = 0x09,
    AX_ID_ISP_AWB       = 0x0A,
    AX_ID_ISP_LSC       = 0x0B,
    AX_ID_ISP_WBC       = 0x0C,
    AX_ID_ISP_RLTM      = 0x0D,
    AX_ID_ISP_DEM       = 0x0E,
    AX_ID_ISP_GIC       = 0x0F,
    AX_ID_ISP_GAMMA     = 0x10,
    AX_ID_ISP_DEHAZE    = 0x11,
    AX_ID_ISP_CSC       = 0x12,
    AX_ID_ISP_OFL       = 0x13,
    AX_ID_ISP_REMAP     = 0x14,
    AX_ID_ISP_HDR       = 0x15,
    AX_ID_ISP_RAW3DNR   = 0x16,
    AX_ID_ISP_DEPURPLE  = 0x17,
    AX_ID_ISP_CC        = 0x18,
    AX_ID_ISP_CBLUT     = 0x19,
    AX_ID_ISP_LCE       = 0x1A,
    AX_ID_ISP_PYRA      = 0x1B,
    AX_ID_ISP_YUVSCALER = 0x1C,
    AX_ID_ISP_CA        = 0x1D,
    AX_ID_ISP_AINR      = 0x1E,
    AX_ID_ISP_AICE      = 0x1F,
    AX_ID_ISP_3DLUT     = 0x20,
    AX_ID_ISP_SCENE     = 0x21,
    AX_ID_ISP_FCC       = 0x22,
    AX_ID_ISP_RAW2DNR   = 0x23,
    AX_ID_ISP_YUV3DNR   = 0x24,
    AX_ID_ISP_LDC       = 0x25,
    AX_ID_ISP_CLP       = 0x26,
    AX_ID_ISP_ME_PRE    = 0x27,
    AX_ID_ISP_ME_POST   = 0x28,
    AX_ID_ISP_NUC       = 0x29,
    AX_ID_ISP_DIS       = 0x2A,
    AX_ID_ISP_BUTT,
} AX_ISP_SUB_ID_E;

typedef enum {
    _AX_ERR_ISP_FAILED                   = 0x80,//ISP Failed
    _AX_ERR_ISP_ALREADY_OPEN,
    _AX_ERR_ISP_PARAM_NOT_CFG,
    _AX_ERR_ISP_MOD_IMPL_FAILED,
    _AX_ERR_ISP_MOD_CONFIG_INVALID,
    _AX_ERR_ISP_ALGO_CREATE_FAILED,
    _AX_ERR_ISP_ALGO_DESTROY_FAILED,
    _AX_ERR_ISP_ALGO_NOT_REGISTER,
    _AX_ERR_ISP_ALGO_INIT_FAILED,
    _AX_ERR_ISP_ALGO_DEINIT_FAILED,
    _AX_ERR_ISP_ALGO_PROCESS_FAILED,
    _AX_ERR_ISP_ALGO_AUTOTBL_REF_BOUND_FAILED,
    _AX_ERR_ISP_DRIVER_NOT_LOAD,
    _AX_ERR_ISP_SNS_NOT_REGISTER,
    _AX_ERR_ISP_NOT_EVEN,
    _AX_ERR_ISP_NOT_ODD,
    _AX_ERR_ISP_STRUCT_NOT_MATCHED,
} AX_ISP_ERR_CODE_E;

/* Success */
#define AX_ISP_SUCCESS                      (0)

/* Common Err Code */
#define AX_ERR_ISP_NULL_PTR                 AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NULL_PTR)
#define AX_ERR_ISP_INVALID_DEVID            AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_DEVID)
#define AX_ERR_ISP_INVALID_PIPEID           AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_PIPEID)
#define AX_ERR_ISP_INVALID_CHNID            AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_CHNID)
#define AX_ERR_ISP_ILLEGAL_PARAM            AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_ISP_NOT_SUPPORT              AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_SUPPORT)
#define AX_ERR_ISP_NOMEM                    AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOMEM)
#define AX_ERR_ISP_TIMEOUT                  AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_TIMED_OUT)
#define AX_ERR_ISP_RES_EMPTY                AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_ISP_NOT_INIT                 AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_INIT)
#define AX_ERR_ISP_ATTR_NOT_CFG             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_CONFIG)
#define AX_ERR_ISP_INVALID_ADDR             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_BAD_ADDR)
#define AX_ERR_ISP_OBJ_EXIST                AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_EXIST)
#define AX_ERR_ISP_OBJ_UNEXIST              AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_UNEXIST)
#define AX_ERR_ISP_STATUS_MISMATCH          AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_PERM)

/* ISP Err Code */
#define AX_ERR_ISP_FAILED                   AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_FAILED)
#define AX_ERR_ISP_ALREADY_OPEN             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALREADY_OPEN)
#define AX_ERR_ISP_MOD_IMPL_FAILED          AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_MOD_IMPL_FAILED)
#define AX_ERR_ISP_MOD_CONFIG_INVALID       AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_MOD_CONFIG_INVALID)
#define AX_ERR_ISP_ALGO_CREATE_FAILED       AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALGO_CREATE_FAILED)
#define AX_ERR_ISP_ALGO_DESTROY_FAILED      AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALGO_DESTROY_FAILED)
#define AX_ERR_ISP_ALGO_INIT_FAILED         AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALGO_INIT_FAILED)
#define AX_ERR_ISP_ALGO_DEINIT_FAILED       AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALGO_DEINIT_FAILED)
#define AX_ERR_ISP_ALGO_PROCESS_FAILED      AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_ALGO_PROCESS_FAILED)
#define AX_ERR_ISP_DRIVER_NOT_LOAD          AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_DRIVER_NOT_LOAD)
#define AX_ERR_ISP_SNS_UNREGISTER           AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_SNS_NOT_REGISTER)
#define AX_ERR_ISP_NOT_EVEN                 AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_NOT_EVEN)
#define AX_ERR_ISP_NOT_ODD                  AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_NOT_ODD)
#define _AX_ERR_ISP_STRUCT_NOT_MATCHED      AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, _AX_ERR_ISP_STRUCT_NOT_MATCHED)

#define AX_ISP_ERR_CODE(MODULE, ERR_CODE)   AX_DEF_ERR(AX_ID_ISP, MODULE, ERR_CODE)

#ifdef __cplusplus
}
#endif

#endif //_AX_ISP_ERROR_CODE_H_

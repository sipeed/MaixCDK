/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_MODEL_API_
#define _AX_MODEL_API_

#include "ax_base_type.h"
#include "ax_global_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* error code define */
#define AX_MODEL_OPEN_MAP_FAIL       AX_DEF_ERR(AX_ID_SYS, 0x07, 0x80)
#define AX_MODEL_IMG_MMAP_FAIL       AX_DEF_ERR(AX_ID_SYS, 0x07, 0x81)
#define AX_MODEL_DUM_MMAP_FAIL       AX_DEF_ERR(AX_ID_SYS, 0x07, 0x82)
#define AX_MODEL_LOAD_IMG_TIMEOUT    AX_DEF_ERR(AX_ID_SYS, 0x07, 0x83)
#define AX_MODEL_IMG_DATA_ERR        AX_DEF_ERR(AX_ID_SYS, 0x07, 0x84)
#define AX_MODEL_NO_MODEL_PATITION   AX_DEF_ERR(AX_ID_SYS, 0x07, 0x85)
#define AX_MODEL_OPEN_MEM_FILE       AX_DEF_ERR(AX_ID_SYS, 0x07, 0x86)
#define AX_MODEL_NOT_SUCH_MODE_FILE  AX_DEF_ERR(AX_ID_SYS, 0x07, 0x87)
#define AX_MODEL_OTHER_ERROR         AX_DEF_ERR(AX_ID_SYS, 0x07, 0x88)

/* Models pac head info */
#define PAC_MAGIC             (0x5C6D8E9F)
#define PAC_VERSION           (1)
#define MAX_MD5_LEN           (32)
#define MAX_PROD_NAME_LEN     (32)
#define MAX_PROD_VER_LEN      (50)
#define MAX_FILE_ID_LEN       (32)
#define MAX_FILE_TYPE_LEN     (32)
#define MAX_FILE_NAME_LEN     (256)

typedef struct {
    AX_U64  u64Base;
    AX_U64  u64Size;
    AX_CHAR szPartID[72];
} BLOCK_T;

typedef struct {
    AX_CHAR szID[MAX_FILE_ID_LEN];
    AX_CHAR szType[MAX_FILE_TYPE_LEN];
    AX_CHAR szFile[MAX_FILE_NAME_LEN];
    AX_U64  u64CodeOffset;
    AX_U64  u64CodeSize;
    BLOCK_T tBlock;
    AX_U32  u32Flag;
    AX_U32  u32Select;
    AX_U32  u32Reserved[8];
} MODEL_PAC_FILE_T;

typedef struct {
    AX_U32 u32Magic;    /* 0x5C6D8E9F */
    AX_U32 u32PacVer;        /* 1 */
    AX_U64 u64PacSize;     /* pac file size */
    AX_CHAR szProdName[MAX_PROD_NAME_LEN];
    AX_CHAR szProdVer[MAX_PROD_VER_LEN];
    AX_U32  u32FileOffset;    /* offset of PAC_FILE_T */
    AX_U32  u32FileCount;
    AX_U32  u32Auth;          /* 0: no auth, 1: md5, 2: crc16 */
    AX_U32  u32Crc16;
    AX_CHAR szMd5[MAX_MD5_LEN];
    AX_U32  u32Reserved;
} MODEL_PAC_HEAD_T;

typedef struct {
    AX_U64 u64PhysAddr;
    AX_U64 u64Size;
} AX_MODEL_INFO_T;

/* GET MODELS API*/
AX_S32 AX_SYS_GetModelsInfo(AX_MODEL_INFO_T *pModelInfo, AX_CHAR *pName, AX_U32 u32Timeout);
AX_S32 AX_SYS_ModelMemRelease(AX_VOID);

#ifdef __cplusplus
}
#endif

#endif //_AX_MODEL_API_
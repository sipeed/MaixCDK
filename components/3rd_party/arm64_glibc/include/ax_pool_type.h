/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_POOL_TYPE_H_
#define _AX_POOL_TYPE_H_
#include "ax_global_type.h"
#include "ax_base_type.h"

#define AX_INVALID_POOLID  (-1U)
#define AX_INVALID_BLOCKID (0)

#define AX_MAX_POOLS 64
#define AX_MAX_COMM_POOLS 16
#define AX_MAX_BLKS_PER_POOL  128

#define AX_MAX_POOL_NAME_LEN 32
#define AX_MAX_PARTITION_NAME_LEN 32
#define AX_MAX_PARTITION_COUNT 16

/* error code define */
#define AX_ERR_POOL_ILLEGAL_PARAM   AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_ILLEGAL_PARAM) //0x800B010A
#define AX_ERR_POOL_NULL_PTR        AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_NULL_PTR)      //0x800B010B
#define AX_ERR_POOL_NOTREADY        AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_SYS_NOTREADY)  //0x800B0110
#define AX_ERR_POOL_BUSY            AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_BUSY)          //0x800B0111
#define AX_ERR_POOL_NOT_SUPPORT     AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_NOT_SUPPORT)   //0x800B0114
#define AX_ERR_POOL_NOT_PERM        AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_NOT_PERM)      //0x800B0115
#define AX_ERR_POOL_UNEXIST         AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_UNEXIST)       //0x800B0117
#define AX_ERR_POOL_NOMEM           AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_NOMEM)         //0x800B0118
#define AX_ERR_POOL_UNKNOWN         AX_DEF_ERR(AX_ID_SYS, 0x01, AX_ERR_UNKNOWN)       //0x800B0129
#define AX_ERR_POOL_MMAP_FAIL       AX_DEF_ERR(AX_ID_SYS, 0x01, 0x80)                 //0x800B0180
#define AX_ERR_POOL_MUNMAP_FAIL     AX_DEF_ERR(AX_ID_SYS, 0x01, 0x81)                 //0x800B0181
#define AX_ERR_POOL_BLKFREE_FAIL    AX_DEF_ERR(AX_ID_SYS, 0x01, 0x82)                 //0x800B0182
#define AX_ERR_POOL_COPY_TO_USER    AX_DEF_ERR(AX_ID_SYS, 0x01, 0x83)                 //0x800B0183
#define AX_ERR_POOL_COPY_FROM_USER  AX_DEF_ERR(AX_ID_SYS, 0x01, 0x84)                 //0x800B0184



typedef AX_U32 AX_POOL;
typedef AX_U32 AX_BLK;

typedef enum {
    AX_POOL_CACHE_MODE_NONCACHE = 0,
    AX_POOL_CACHE_MODE_CACHED = 1,
    AX_POOL_CACHE_MODE_BUTT
} AX_POOL_CACHE_MODE_E;

typedef enum {
    AX_POOL_SOURCE_COMMON = 0,
    AX_POOL_SOURCE_PRIVATE = 1,
    AX_POOL_SOURCE_USER = 2,
    AX_POOL_SOURCE_BUTT
} AX_POOL_SOURCE_E;

typedef struct {
    AX_U64 MetaSize;
    AX_U64 BlkSize;
    AX_U32 BlkCnt; /* range:(0,256] */
    AX_BOOL IsMergeMode; /* logically merged with common pool, make common pool bigger*/
    AX_POOL_CACHE_MODE_E CacheMode;
    AX_S8 PartitionName[AX_MAX_PARTITION_NAME_LEN];
    AX_S8 PoolName[AX_MAX_POOL_NAME_LEN] /* common pool will ignore this member, only used by user pool */;
} AX_POOL_CONFIG_T;

typedef struct {
    AX_POOL_CONFIG_T CommPool[AX_MAX_COMM_POOLS];
} AX_POOL_FLOORPLAN_T;

#endif //_AX_POOL_TYPE_H_

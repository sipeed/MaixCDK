/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_GZIP_API_H_
#define _AX_GZIP_API_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ax_base_type.h"
#include "ax_global_type.h"

typedef struct {
    AX_U8  cType[2];
    AX_U16 u16BlkNum;
    AX_U32 u32OutSize;
    AX_U32 u32InSize;
    AX_U32 u32Crc32;
} AX_GZIPD_HEADER_INFO_T;

typedef struct {
    AX_U64 pPhyAddr;
    AX_VOID *pVirAddr;
    AX_U32 u32BufSize;
} AX_GZIPD_BUF_INFO_T;

typedef struct  {
    AX_GZIPD_BUF_INFO_T stInBuf;
    AX_GZIPD_BUF_INFO_T stOutBuf;
    AX_U32 u32TileSize;
    AX_GZIPD_HEADER_INFO_T headerInfo;
    AX_U64 tilesNum;
    AX_U64 lastTileLen;
} AX_GZIPD_IO_PARAM_T;

typedef enum {
    AX_GZIPD_COMPLETE_SUCCESS,
    AX_GZIPD_COMPLETE_FAIL,
    AX_GZIPD_COMPLETE_MAX,
} AX_GZIPD_COMPLETE_STS_E;

typedef struct {
    AX_U32 u32Handle;
    AX_GZIPD_COMPLETE_STS_E u32Result;
} AX_GZIPD_RESULT_INFO_T;

#define AX_GZIPD_STATUS_OK          (0x0)
#define AX_GZIPD_STATUS_NO_INIT     AX_DEF_ERR(AX_ID_AXGZIPD, 0, 1) /* (0x802d0001) */
#define AX_GZIPD_STATUS_FAIL        AX_DEF_ERR(AX_ID_AXGZIPD, 0, 2) /* (0x802d0002) */
#define AX_GZIPD_STATUS_NO_MEM      AX_DEF_ERR(AX_ID_AXGZIPD, 0, 3) /* (0x802d0003) */
#define AX_GZIPD_INVALID_PARAM      AX_DEF_ERR(AX_ID_AXGZIPD, 0, 4) /* (0x802d0004) */
#define AX_GZIPD_WORK_FINISH        AX_DEF_ERR(AX_ID_AXGZIPD, 0, 5) /* (0x802d0005) */
#define AX_GZIPD_STATUS_BUSY        AX_DEF_ERR(AX_ID_AXGZIPD, 0, 6) /* (0x802d0006) */
#define AX_GZIPD_STATUS_IDLE        AX_DEF_ERR(AX_ID_AXGZIPD, 0, 7) /* (0x802d0007) */
#define AX_GZIPD_PART_COMPLETE      AX_DEF_ERR(AX_ID_AXGZIPD, 0, 8) /* (0x802d0008) */
#define AX_GZIPD_DEV_ERROR          AX_DEF_ERR(AX_ID_AXGZIPD, 0, 9) /* (0x802d0009) */
#define AX_GZIPD_NO_ENOUGH_RES      AX_DEF_ERR(AX_ID_AXGZIPD, 0, 10) /* (0x802d000A) */

AX_S32 AX_GZIPD_Init(AX_VOID);
AX_S32 AX_GZIPD_CreateHandle(AX_U32 *pHandle, AX_VOID *pGZIPData, AX_GZIPD_HEADER_INFO_T *pOutHeaderInfo);
AX_S32 AX_GZIPD_Config (AX_U32 handle, AX_GZIPD_IO_PARAM_T *pGzipdInputParam);
AX_U32 AX_GZIPD_TilesRun(AX_U32 handle, AX_U64 u64TilesStartPhyaddr, AX_U64 u64TilesDataBytesLen, AX_U32 *u32QueuedTilesNum);
AX_S32 AX_GZIPD_LastTileRun(AX_U32 handle, AX_U64 u64LastTilePhyAddr, AX_U32 u32LastTileSize);
AX_S32 AX_GZIPD_WaitFinish(AX_U32 handle, AX_GZIPD_RESULT_INFO_T *pResult);
AX_S32 AX_GZIPD_DestroyHandle(AX_U32 handle);
AX_S32 AX_GZIPD_Deinit(AX_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

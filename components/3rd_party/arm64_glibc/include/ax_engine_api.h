/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ax_engine_type.h>

DLLEXPORT const AX_CHAR* AX_ENGINE_GetVersion(AX_VOID);

DLLEXPORT AX_VOID AX_ENGINE_NPUReset(AX_VOID);

DLLEXPORT AX_S32 AX_ENGINE_Init(AX_ENGINE_NPU_ATTR_T* pNpuAttr);
DLLEXPORT AX_S32 AX_ENGINE_GetVNPUAttr(AX_ENGINE_NPU_ATTR_T* pNpuAttr);
DLLEXPORT AX_S32 AX_ENGINE_Deinit(AX_VOID);

DLLEXPORT AX_S32 AX_ENGINE_GetModelType(const AX_VOID* pData, AX_U32 nDataSize, AX_ENGINE_MODEL_TYPE_T* pModelType);

DLLEXPORT AX_S32 AX_ENGINE_CreateHandle(AX_ENGINE_HANDLE *pHandle, const AX_VOID *pData, AX_U32 nDataSize);
DLLEXPORT AX_S32 AX_ENGINE_CreateHandleV2(AX_ENGINE_HANDLE* pHandle, const AX_VOID* pData, AX_U32 nDataSize, AX_ENGINE_HANDLE_EXTRA_T* pExtraParam);
DLLEXPORT AX_S32 AX_ENGINE_DestroyHandle(AX_ENGINE_HANDLE nHandle);

DLLEXPORT AX_S32 AX_ENGINE_GetIOInfo(AX_ENGINE_HANDLE nHandle, AX_ENGINE_IO_INFO_T** pIO);
DLLEXPORT AX_S32 AX_ENGINE_GetGroupIOInfoCount(AX_ENGINE_HANDLE nHandle, AX_U32* pCount);
DLLEXPORT AX_S32 AX_ENGINE_GetGroupIOInfo(AX_ENGINE_HANDLE nHandle, AX_U32 nIndex, AX_ENGINE_IO_INFO_T** pIO);

DLLEXPORT AX_S32 AX_ENGINE_GetHandleModelType(AX_ENGINE_HANDLE nHandle, AX_ENGINE_MODEL_TYPE_T* pModelType);

DLLEXPORT AX_S32 AX_ENGINE_CreateContext(AX_ENGINE_HANDLE handle);
DLLEXPORT AX_S32 AX_ENGINE_CreateContextV2(AX_ENGINE_HANDLE nHandle, AX_ENGINE_CONTEXT_T* pContext);

DLLEXPORT AX_S32 AX_ENGINE_RunSync(AX_ENGINE_HANDLE handle, AX_ENGINE_IO_T* pIO);
DLLEXPORT AX_S32 AX_ENGINE_RunSyncV2(AX_ENGINE_HANDLE handle, AX_ENGINE_CONTEXT_T context, AX_ENGINE_IO_T* pIO);
DLLEXPORT AX_S32 AX_ENGINE_RunGroupIOSync(AX_ENGINE_HANDLE handle, AX_ENGINE_CONTEXT_T context, AX_U32 nIndex, AX_ENGINE_IO_T* pIO);

DLLEXPORT AX_S32 AX_ENGINE_SetAffinity(AX_ENGINE_HANDLE nHandle, AX_ENGINE_NPU_SET_T nNpuSet);
DLLEXPORT AX_S32 AX_ENGINE_GetAffinity(AX_ENGINE_HANDLE nHandle, AX_ENGINE_NPU_SET_T* pNpuSet);

DLLEXPORT AX_S32 AX_ENGINE_GetCMMUsage(AX_ENGINE_HANDLE nHandle, AX_ENGINE_CMM_INFO_T* pCMMInfo);

DLLEXPORT const AX_CHAR* AX_ENGINE_GetModelToolsVersion(AX_ENGINE_HANDLE nHandle);


#ifdef __cplusplus
}
#endif

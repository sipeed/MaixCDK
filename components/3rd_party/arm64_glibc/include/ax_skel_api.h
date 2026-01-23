/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_SKEL_API_H_
#define _AX_SKEL_API_H_

#include "ax_global_type.h"
#include "ax_skel_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* error code */
#define AX_SKEL_SUCC                        (0)
#define AX_ERR_SKEL_NULL_PTR                AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_NULL_PTR)
#define AX_ERR_SKEL_ILLEGAL_PARAM           AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_SKEL_NOT_INIT                AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_NOT_INIT)
#define AX_ERR_SKEL_QUEUE_EMPTY             AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_SKEL_QUEUE_FULL              AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_QUEUE_FULL)
#define AX_ERR_SKEL_UNEXIST                 AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_UNEXIST)
#define AX_ERR_SKEL_TIMEOUT                 AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_TIMED_OUT)
#define AX_ERR_SKEL_SYS_NOTREADY            AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_SYS_NOTREADY)
#define AX_ERR_SKEL_INVALID_HANDLE          AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_INVALID_CHNID)
#define AX_ERR_SKEL_NOMEM                   AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_NOMEM)
#define AX_ERR_SKEL_UNKNOWN                 AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_UNKNOWN)
#define AX_ERR_SKEL_NOT_SUPPORT             AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_NOT_SUPPORT)
#define AX_ERR_SKEL_INITED                  AX_DEF_ERR(AX_ID_SKEL, 1, AX_ERR_EXIST)

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize skel sdk
///
/// @param pstParam   [I]: initialize parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Init(const AX_SKEL_INIT_PARAM_T *pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief uninitialize skel sdk
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief create handle
///
/// @param pstParam   [I]: handle parameter
/// @param handle     [O]: handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Create(const AX_SKEL_HANDLE_PARAM_T *pstParam, AX_SKEL_HANDLE *pHandle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief destroy handle
///
/// @param pHandle    [I]: handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Destroy(AX_SKEL_HANDLE handle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register get algorithm result callback
///
/// @param pHandle    [I]: handle
/// @param callback   [I]: callback function
/// @param pUserData  [I]: private user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_RegisterResultCallback(AX_SKEL_HANDLE handle, AX_SKEL_RESULT_CALLBACK_FUNC callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief send frame
///
/// @param pHandle    [I]: handle
/// @param pstImage   [I]: image
/// @param ppstResult [O]: algorithm result
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_SendFrame(AX_SKEL_HANDLE handle, const AX_SKEL_FRAME_T *pstFrame, AX_S32 nTimeout);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief process image
///
/// @param pHandle    [I]: handle
/// @param ppstResult [O]: algorithm result
/// @param nTimeout   [I]: timeout
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_GetResult(AX_SKEL_HANDLE handle, AX_SKEL_RESULT_T **ppstResult, AX_S32 nTimeout);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief free memory
///
/// @param p           [I]: memory address
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Release(AX_VOID *p);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get capability
///
/// @param ppstCapability [O]: Capability
///
/// @return version info
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_GetCapability(const AX_SKEL_CAPABILITY_T **ppstCapability);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get version
///
/// @param NA
///
/// @return version info
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_GetVersion(const AX_SKEL_VERSION_INFO_T **ppstVersion);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get config
///
/// @param pstConfig           [I/O]: config
///
/// @return version info
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_GetConfig(AX_SKEL_HANDLE handle, const AX_SKEL_CONFIG_T **ppstConfig);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief set config
///
/// @param pstConfig           [I]: config
///
/// @return version info
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_SetConfig(AX_SKEL_HANDLE handle, const AX_SKEL_CONFIG_T *pstConfig);

//API for SKEL search
//////////////////////////////////////////////////////////////////////////////////////
/// @brief initialize search
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_Init(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief uninitialize search
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief create group for search
///
/// @param nGroupId   [I]: group id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_Create(AX_U64 nGroupId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief destroy group for search
///
/// @param nGroupId    [I]: group id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_Destroy(AX_U64 nGroupId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Insert feature for search
///
/// @param nGroupId    [I]: group id
/// @param pParam      [I]: feature param
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_InsertFeature(AX_U64 nGroupId, AX_SKEL_SEARCH_FEATURE_PARAM_T *pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Delete feature for search
///
/// @param nGroupId    [I]: group id
/// @param nObjectId   [I]: object id
/// @param ppInfo      [O]: info
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search_DeleteFeature(AX_U64 nGroupId, AX_U64 nObjectId, AX_VOID **ppInfo);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief search
///
/// @param nGroupId    [I]: group id
/// @param pstParam    [I]: param
/// @param ppstResult  [O]: result
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_SKEL_Search(AX_U64 nGroupId, AX_SKEL_SEARCH_PARAM_T *pstParam, AX_SKEL_SEARCH_RESULT_T **ppstResult);

#ifdef __cplusplus
}
#endif

#endif /* _AX_SKEL_API_H_ */

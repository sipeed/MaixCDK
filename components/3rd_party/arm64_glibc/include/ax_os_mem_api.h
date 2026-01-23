/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_OS_MEM_API_H
#define __AX_OS_MEM_API_H

#include <stdbool.h>

#include "ax_base_type.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    AX_U32 nId;
    AX_U32 nSize;
} AX_OS_MEM_RECORD_T;
/*****************************************************************************
*   Prototype    : AX_OS_MEM_Malloc
*   Description  : This API is used to malloc memory for usr space application
*   Parameters   :
*                  AX_U32         nId           module_id and sub_module_id
*                  size_t         nSize         malloc block size
*                :
*   Return Value : none zero value: success; Error: NULL
*   Spec         :
*
*****************************************************************************/
AX_VOID *AX_OS_MEM_Malloc(AX_U32 nId, size_t nSize);
/*****************************************************************************
*   Prototype    : AX_OS_MEM_Calloc
*   Description  : This API is used to calloc memory for usr space application
*   Parameters   :
*                  AX_U32         nId          module_id and sub_module_id
*                  size_t         nMemb        memory blocks number
*                  size_t         nSize        malloc block size
*                :
*   Return Value : none zero value: success; Error: NULL
*   Spec         :
*
*****************************************************************************/
AX_VOID *AX_OS_MEM_Calloc(AX_U32 nId, size_t nMemb, size_t nSize);
/*****************************************************************************
*   Prototype    : AX_OS_MEM_Free
*   Description  : This API is used to free memory for usr space application
*   Parameters   :
*                  AX_U32         nModID        module id and sub_module_id
*                  AX_VOID *      ptr          the memory space pointer will be free
*                :
*   Return Value : None
*   Spec         :
*
*****************************************************************************/
AX_VOID AX_OS_MEM_Free(AX_U32 nID, AX_VOID *ptr);
/*****************************************************************************
*   Prototype    : AX_OS_MEM_ModNodesGet
*   Description  : This API is used to get the alloc memory record of one module
*   Parameters   :
*                  AX_U32       nID              module id
*                  AX_OS_MEM_RECORD_T *ptRecord        pointer the ptr, the memory node info will be stored
*                  AX_U32       nNum          the max get memory node number
*                :
*   Return Value : the valid memory node number
*   Spec         :
*
*****************************************************************************/
AX_S32 AX_OS_MEM_ModNodesGet(AX_U32 nID, AX_OS_MEM_RECORD_T *ptRecord, AX_U32 nNum);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

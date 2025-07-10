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

#if defined __GNUC__
#define DLLEXPORT __attribute((visibility("default")))
#elif defined(_MSC_VER)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#if defined __GNUC__
#define DEPRECATED_BEFORE
#define DEPRECATED_AFTER __attribute__((deprecated))
#elif defined(_MSC_VER)
#pragma deprecated()
#define DEPRECATED_BEFORE __declspec(deprecated)
#define DEPRECATED_AFTER
#else
#define DEPRECATED_BEFORE
#define DEPRECATED_AFTER
#endif

#include <ax_base_type.h>

typedef AX_U32 AX_KERNEL_HANDLE;
typedef AX_VOID* AX_ENGINE_HANDLE;
typedef AX_VOID* AX_ENGINE_CONTEXT_T;
typedef AX_VOID* AX_ENGINE_EXECUTION_CONTEXT;
typedef AX_U32 AX_ENGINE_NPU_SET_T;


typedef enum _AX_ENGINE_TENSOR_LAYOUT_E
{
    AX_ENGINE_TENSOR_LAYOUT_UNKNOWN = 0,
    AX_ENGINE_TENSOR_LAYOUT_NHWC    = 1,
    AX_ENGINE_TENSOR_LAYOUT_NCHW    = 2,
} AX_ENGINE_TENSOR_LAYOUT_T;

typedef enum
{
    AX_ENGINE_MT_PHYSICAL           = 0,
    AX_ENGINE_MT_VIRTUAL            = 1,
    AX_ENGINE_MT_OCM                = 2,
} AX_ENGINE_MEMORY_TYPE_T;

typedef enum
{
    AX_ENGINE_DT_UNKNOWN            = 0,
    AX_ENGINE_DT_UINT8              = 1,
    AX_ENGINE_DT_UINT16             = 2,
    AX_ENGINE_DT_FLOAT32            = 3,
    AX_ENGINE_DT_SINT16             = 4,
    AX_ENGINE_DT_SINT8              = 5,
    AX_ENGINE_DT_SINT32             = 6,
    AX_ENGINE_DT_UINT32             = 7,
    AX_ENGINE_DT_FLOAT64            = 8,
    AX_ENGINE_DT_UINT10_PACKED      = 100,
    AX_ENGINE_DT_UINT12_PACKED      = 101,
    AX_ENGINE_DT_UINT14_PACKED      = 102,
    AX_ENGINE_DT_UINT16_PACKED      = 103,
} AX_ENGINE_DATA_TYPE_T;

typedef enum
{
    AX_ENGINE_CS_FEATUREMAP         = 0,
    AX_ENGINE_CS_RAW8               = 12,
    AX_ENGINE_CS_RAW10              = 1,
    AX_ENGINE_CS_RAW12              = 2,
    AX_ENGINE_CS_RAW14              = 11,
    AX_ENGINE_CS_RAW16              = 3,
    AX_ENGINE_CS_NV12               = 4,
    AX_ENGINE_CS_NV21               = 5,
    AX_ENGINE_CS_RGB                = 6,
    AX_ENGINE_CS_BGR                = 7,
    AX_ENGINE_CS_RGBA               = 8,
    AX_ENGINE_CS_GRAY               = 9,
    AX_ENGINE_CS_YUV444             = 10,
} AX_ENGINE_COLOR_SPACE_T;

typedef enum {
    AX_ENGINE_VIRTUAL_NPU_DISABLE   = 0,    // virtual npu disable
    AX_ENGINE_VIRTUAL_NPU_ENABLE    = 1,    // virtual npu enable
    AX_ENGINE_VIRTUAL_NPU_BUTT      = 2,
} AX_ENGINE_NPU_MODE_T;

typedef enum {
    AX_ENGINE_MODEL_TYPE0           = 0,
    AX_ENGINE_MODEL_TYPE1           = 1,
    AX_ENGINE_MODEL_TYPE_BUTT       = 2,
} AX_ENGINE_MODEL_TYPE_T;

typedef struct {
    AX_ENGINE_NPU_MODE_T eHardMode;
    AX_U32 reserve[8];
} AX_ENGINE_NPU_ATTR_T;

typedef struct _AX_ENGINE_IOMETA_EX_T
{
    AX_ENGINE_COLOR_SPACE_T eColorSpace;
    AX_U64 u64Reserved[18];
} AX_ENGINE_IOMETA_EX_T;

typedef struct _AX_ENGINE_IOMETA_T
{
    AX_CHAR *pName;
    AX_S32  *pShape;                        // YUV will be treated as 1-ch data
    AX_U8   nShapeSize;                     // dimension of shape
    AX_ENGINE_TENSOR_LAYOUT_T eLayout;
    AX_ENGINE_MEMORY_TYPE_T   eMemoryType;
    AX_ENGINE_DATA_TYPE_T     eDataType;
    AX_ENGINE_IOMETA_EX_T*    pExtraMeta;
    AX_U32 nSize;
    /*!
     * `nQuantizationValue` is the total amount of possible values
     * in quantization part of a data represented in `Q` method
     * eg. for U4Q12 data, nQuantizationValue = 2**12 = 4096
     */
    AX_U32 nQuantizationValue;
    /*!
     * when pStride is NULL, there is not stride limit
     *
     * when pStride is not NULL, it holds the number of elements in stride
     * of each dimension, the size of `pStride` always equal to `nShapeSize`
     * eg. index[0:3] shape[n,c,h,w] stride[chw,hw,w,1]
     * eg. index[0:3] shape[n,h,w,c] stride[hwc,wc,c,1]
     */
    AX_S32 *pStride;
#if defined(__aarch64__)
    AX_U64 u64Reserved[9];
#elif defined(__arm__)
    AX_U64 u64Reserved[11];
#endif
} AX_ENGINE_IOMETA_T;

typedef struct _AX_ENGINE_IO_INFO_T
{
    AX_ENGINE_IOMETA_T *pInputs;
    AX_U32 nInputSize;
    AX_ENGINE_IOMETA_T *pOutputs;
    AX_U32 nOutputSize;
    AX_U32 nMaxBatchSize;  // 0 for unlimited
    AX_BOOL bDynamicBatchSize;  // if true, any batch size <= nMaxBatchSize is supported
#if defined(__aarch64__)
    AX_U64 u64Reserved[11];
#elif defined(__arm__)
    AX_U64 u64Reserved[13];
#endif
} AX_ENGINE_IO_INFO_T;

typedef struct _AX_ENGINE_IO_BUFFER_T
{
    AX_U64 phyAddr;
    AX_VOID *pVirAddr;
    AX_U32 nSize;  // total size of memory
    /*!
     * pStride holds the number of elements in stride of each dimension
     * set pStride to NULL to disable stride function
     *
     * `nStrideSize` should be equal to `nShapeSize`
     * eg. index[0:3] shape[n,c,h,w] stride[chw,hw,w,1]
     * eg. index[0:3] shape[n,h,w,c] stride[hwc,wc,c,1]
     */
    AX_S32 *pStride;
    AX_U8 nStrideSize;
#if defined(__aarch64__)
    AX_U64 u64Reserved[11];
#elif defined(__arm__)
    AX_U64 u64Reserved[13];
#endif
} AX_ENGINE_IO_BUFFER_T;

typedef struct _AX_ENGINE_IO_SETTING_T
{
    AX_U32 nWbtIndex;
    AX_U64 u64Reserved[7];
}AX_ENGINE_IO_SETTING_T;

typedef struct _AX_ENGINE_IO_T
{
    AX_ENGINE_IO_BUFFER_T *pInputs;
    AX_U32 nInputSize;
    AX_ENGINE_IO_BUFFER_T *pOutputs;
    AX_U32 nOutputSize;
    AX_U32 nBatchSize;  // 0 for auto detection
    AX_ENGINE_IO_SETTING_T *pIoSetting;
#if defined(__aarch64__)
    AX_U64 u64Reserved[11];
#elif defined(__arm__)
    AX_U64 u64Reserved[13];
#endif
} AX_ENGINE_IO_T;

typedef struct {
    AX_ENGINE_NPU_SET_T nNpuSet;
    AX_S8 *pName;
    AX_U32 reserve[8];
} AX_ENGINE_HANDLE_EXTRA_T;

typedef struct _AX_ENGINE_CMM_INFO_T
{
    AX_U32 nCMMSize;
} AX_ENGINE_CMM_INFO_T;

#ifdef __cplusplus
}
#endif

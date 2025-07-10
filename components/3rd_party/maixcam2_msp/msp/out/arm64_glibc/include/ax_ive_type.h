/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_IVE_TYPE_H_
#define _AX_IVE_TYPE_H_

#include "ax_base_type.h"
#include "ax_global_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef AX_FAILURE
#define AX_FAILURE (-1)
#endif

#ifndef AX_SUCCESS
#define AX_SUCCESS 0
#endif


/*
* The fixed-point data type, will be used to
* represent float data in hardware calculations.
*/

/* u8bit */
typedef unsigned char AX_U1Q7;
typedef unsigned char AX_U4Q4;
typedef unsigned char AX_U3Q5;

/* u16bit */
typedef unsigned short AX_U1Q10;
typedef unsigned short AX_U1Q15;
typedef unsigned short AX_U8Q6;
typedef unsigned short AX_U0Q16;

/* u32bit */
typedef unsigned int AX_U0Q20;
typedef unsigned int AX_U14Q4;
typedef unsigned int AX_U11Q20;
typedef unsigned int AX_U7Q12;

/* s16bit */
typedef short AX_S1Q7;
typedef short AX_S1Q14;

/* s32bit */
typedef int AX_S6Q10;
typedef int AX_S3Q20;
typedef int AX_S8Q10;

/*
* IVE handle
*/
typedef AX_S32 AX_IVE_HANDLE;

#define AX_IVE_MAX_REGION_NUM  2048
#define AX_IVE_HIST_NUM        256
#define AX_IVE_MAP_NUM         256
#define AX_IVE_MAX_CORNER_NUM  500

/* Type of the AX_IVE_IMAGE_T */
typedef enum axIVE_IMAGE_TYPE_E {
    AX_IVE_IMAGE_TYPE_U8C1 = 0x0,
    AX_IVE_IMAGE_TYPE_S8C1 = 0x1,

    AX_IVE_IMAGE_TYPE_YUV420SP = 0x2, /* YUV420 SemiPlanar */
    AX_IVE_IMAGE_TYPE_YUV422SP = 0x3, /* YUV422 SemiPlanar */
    AX_IVE_IMAGE_TYPE_YUV420P = 0x4,  /* YUV420 Planar */
    AX_IVE_IMAGE_TYPE_YUV422P = 0x5,  /* YUV422 planar */

    AX_IVE_IMAGE_TYPE_S8C2_PACKAGE = 0x6,
    AX_IVE_IMAGE_TYPE_S8C2_PLANAR = 0x7,

    AX_IVE_IMAGE_TYPE_S16C1 = 0x8,
    AX_IVE_IMAGE_TYPE_U16C1 = 0x9,

    AX_IVE_IMAGE_TYPE_U8C3_PACKAGE = 0xa,
    AX_IVE_IMAGE_TYPE_U8C3_PLANAR = 0xb,

    AX_IVE_IMAGE_TYPE_S32C1 = 0xc,
    AX_IVE_IMAGE_TYPE_U32C1 = 0xd,

    AX_IVE_IMAGE_TYPE_S64C1 = 0xe,
    AX_IVE_IMAGE_TYPE_U64C1 = 0xf,

    AX_IVE_IMAGE_TYPE_BUTT

} AX_IVE_IMAGE_TYPE_E;

/* Definition of the AX_IVE_IMAGE_T */
typedef struct axIVE_IMAGE_T {
    AX_U64 au64PhyAddr[3];   /* RW;The physical address of the image */
    AX_U64 au64VirAddr[3];   /* RW;The virtual address of the image */
    AX_U32 au32Stride[3];    /* RW;The stride of the image */
    AX_U32 u32Width;         /* RW;The width of the image */
    AX_U32 u32Height;        /* RW;The height of the image */
    union {
        AX_IVE_IMAGE_TYPE_E enType; /* RW;The type of the image */
        AX_IMG_FORMAT_E enGlbType; /* RW;The type of the global image */
    };
} AX_IVE_IMAGE_T;

typedef AX_IVE_IMAGE_T AX_IVE_SRC_IMAGE_T;
typedef AX_IVE_IMAGE_T AX_IVE_DST_IMAGE_T;

/*
* Definition of the AX_IVE_MEM_INFO_T.
* This struct special purpose for input or ouput, such as Hist, CCL.
*/
typedef struct axIVE_MEM_INFO_T {
    AX_U64 u64PhyAddr; /* RW;The physical address of the memory */
    AX_U64 u64VirAddr; /* RW;The virtual address of the memory */
    AX_U32 u32Size;    /* RW;The size of memory */
} AX_IVE_MEM_INFO_T;
typedef AX_IVE_MEM_INFO_T AX_IVE_SRC_MEM_INFO_T;
typedef AX_IVE_MEM_INFO_T AX_IVE_DST_MEM_INFO_T;

/* Data struct */
typedef struct axIVE_DATA_T {
    AX_U64 u64PhyAddr; /* RW;The physical address of the data */
    AX_U64 u64VirAddr; /* RW;The virtaul address of the data */
    AX_U32 u32Stride; /* RW;The stride of 2D data by byte */
    AX_U32 u32Width;  /* RW;The width of 2D data by byte */
    AX_U32 u32Height; /* RW;The height of 2D data by byte */

    AX_U32 u32Reserved;
} AX_IVE_DATA_T;

typedef AX_IVE_DATA_T AX_IVE_SRC_DATA_T;
typedef AX_IVE_DATA_T AX_IVE_DST_DATA_T;

/*
* Hardware Engine
*/
typedef enum axIVE_HW_ENGINE_E {
    AX_IVE_ENGINE_IVE = 0,
    AX_IVE_ENGINE_TDP,
    AX_IVE_ENGINE_VGP,
    AX_IVE_ENGINE_VPP,
    AX_IVE_ENGINE_GDC,
    AX_IVE_ENGINE_DSP,
    AX_IVE_ENGINE_NPU,
    AX_IVE_ENGINE_CPU,
    AX_IVE_ENGINE_BUTT
} AX_IVE_ENGINE_E;

/*
* Definition of s16 point
*/
typedef struct axIVE_POINT_S16_T {
    AX_S16 s16X; /* RW;The X coordinate of the point */
    AX_S16 s16Y; /* RW;The Y coordinate of the point */
} AX_IVE_POINT_S16_T;

/* Definition of u16 point */
typedef struct axIVE_POINT_U16_T {
    AX_U16 u16X; /* RW;The X coordinate of the point */
    AX_U16 u16Y; /* RW;The Y coordinate of the point */
} AX_IVE_POINT_U16_T;

/* Definition of rectangle */
typedef struct axIVE_RECT_U16_T {
    AX_U16 u16X;      /* RW;The location of X axis of the rectangle */
    AX_U16 u16Y;      /* RW;The location of Y axis of the rectangle */
    AX_U16 u16Width;  /* RW;The width of the rectangle */
    AX_U16 u16Height; /* RW;The height of the rectangle */
} AX_IVE_RECT_U16_T;

/* Definition of the union of AX_IVE_8BIT_U */
typedef union axIVE_8BIT_U {
    AX_S16 s16Val;  //[-256,255]
    AX_U8 u8Val;    //[0,255]
} AX_IVE_8BIT_U;

typedef enum axIVE_ERR_CODE_E {
    AX_ERR_IVE_OPEN_FAILED = 0x50,     /* IVE open device failed */
    AX_ERR_IVE_INIT_FAILED = 0x51,     /* IVE init failed  */
    AX_ERR_IVE_NOT_INIT = 0x52,        /* IVE not init error */
    AX_ERR_IVE_SYS_TIMEOUT = 0x53,     /* IVE process timeout */
    AX_ERR_IVE_QUERY_TIMEOUT = 0x54,   /* IVE query timeout */
    AX_ERR_IVE_BUS_ERR = 0x55,         /* IVE bus error */

} AX_IVE_ERR_CODE_E;

#define AX_ID_IVE_SMOD  0x00
/************************************************IVE error code ***********************************/
/* Invalid device ID */
#define AX_ERR_IVE_INVALID_DEVID AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_INVALID_MODID)
/* Invalid channel ID */
#define AX_ERR_IVE_INVALID_CHNID AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_INVALID_CHNID)
/* At least one parameter is illegal. For example, an illegal enumeration value exists. */
#define AX_ERR_IVE_ILLEGAL_PARAM AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_ILLEGAL_PARAM)
/* The channel exists. */
#define AX_ERR_IVE_EXIST AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_EXIST)
/* The UN exists. */
#define AX_ERR_IVE_UNEXIST AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_UNEXIST)
/* A null point is used. */
#define AX_ERR_IVE_NULL_PTR AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NULL_PTR)
/* Try to enable or initialize the system, device, or channel before configuring attributes. */
#define AX_ERR_IVE_NOT_CONFIG AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NOT_CONFIG)
/* The operation is not supported currently. */
#define AX_ERR_IVE_NOT_SURPPORT AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NOT_SUPPORT)
/* The operation, changing static attributes for example, is not permitted. */
#define AX_ERR_IVE_NOT_PERM AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NOT_PERM)
/* A failure caused by the malloc memory occurs. */
#define AX_ERR_IVE_NOMEM AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NOMEM)
/* A failure caused by the malloc buffer occurs. */
#define AX_ERR_IVE_NOBUF AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_NOBUF)
/* The buffer is empty. */
#define AX_ERR_IVE_BUF_EMPTY AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_BUF_EMPTY)
/* No buffer is provided for storing new data. */
#define AX_ERR_IVE_BUF_FULL AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_BUF_FULL)
/* The system is not ready because it may be not initialized or loaded.
 * The error code is returned when a device file fails to be opened. */
#define AX_ERR_IVE_NOTREADY AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_SYS_NOTREADY)
/* The source address or target address is incorrect during the operations such as calling
copy_from_user or copy_to_user. */
#define AX_ERR_IVE_BADADDR AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_BADADDR)
/* The resource is busy during the operations such as destroying a VENC channel
without deregistering it. */
#define AX_ERR_IVE_BUSY AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_BUSY)
/* IVE open device error: */
#define AX_ERR_IVE_OPEN_FAILED AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_OPEN_FAILED)
/* IVE init error: */
#define AX_ERR_IVE_INIT_FAILED AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_INIT_FAILED)
/* IVE not init error:  */
#define AX_ERR_IVE_NOT_INIT AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_NOT_INIT)
/* IVE process timeout: */
#define AX_ERR_IVE_SYS_TIMEOUT AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_SYS_TIMEOUT)
/* IVE query timeout: */
#define AX_ERR_IVE_QUERY_TIMEOUT AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_QUERY_TIMEOUT)
/* IVE Bus error: */
#define AX_ERR_IVE_BUS_ERR AX_DEF_ERR(AX_ID_IVE, AX_ID_IVE_SMOD, AX_ERR_IVE_BUS_ERR)

/*
* DMA mode
*/
typedef enum axIVE_DMA_MODE_E {
    AX_IVE_DMA_MODE_DIRECT_COPY = 0x0,
    AX_IVE_DMA_MODE_INTERVAL_COPY = 0x1,
    AX_IVE_DMA_MODE_SET_3BYTE = 0x2,
    AX_IVE_DMA_MODE_SET_8BYTE = 0x3,
    AX_IVE_DMA_MODE_BUTT
} AX_IVE_DMA_MODE_E;

/*
* DMA control parameter
*/
typedef struct axIVE_DMA_CTRL_T {
    AX_IVE_DMA_MODE_E enMode;
    AX_U64 u64Val;      /* Used in memset mode */
    AX_U8 u8HorSegSize; /* Used in interval-copy mode, every row was segmented by u8HorSegSize bytes */
    AX_U8 u8ElemSize;   /* Used in interval-copy mode, the valid bytes copied in front of every segment
                        in a valid row, which 0<u8ElemSize<u8HorSegSize */
    AX_U8 u8VerSegRows; /* Used in interval-copy mode, copy one row in every u8VerSegRows */
    AX_U16 u16CrpX0;    /* Used in direct-copy mode, crop start point x-coordinate */
    AX_U16 u16CrpY0;    /* Used in direct-copy mode, crop start point y-coordinate */
} AX_IVE_DMA_CTRL_T;

/*
* Add control parameters
*/
typedef struct axIVE_ADD_CTRL_T {
    AX_U1Q7 u1q7X; /* x of "xA+yB" */
    AX_U1Q7 u1q7Y; /* y of "xA+yB" */
} AX_IVE_ADD_CTRL_T;

/*
* Type of the Sub output results
*/
typedef enum axIVE_SUB_MODE_E {
    AX_IVE_SUB_MODE_ABS = 0x0,   /* Absolute value of the difference */
    AX_IVE_SUB_MODE_SHIFT = 0x1, /* The output result is obtained by shifting the result one digit right
                                to reserve the signed bit. */
    AX_IVE_SUB_MODE_BUTT
} AX_IVE_SUB_MODE_E;

/*
* Sub control parameters
*/
typedef struct axIVE_SUB_CTRL_T {
    AX_IVE_SUB_MODE_E enMode;
} AX_IVE_SUB_CTRL_T;

/*
* Mse control parameters
*/
typedef struct axIVE_MSE_CTRL_T {
    AX_U1Q15 u1q15MseCoef; /* MSE coef, range: [0,65535]*/
} AX_IVE_MSE_CTRL_T;

/*
* HysEdge control struct
*/
typedef struct axIVE_HYS_EDGE_CTRL_T {
    AX_U16 u16LowThr;
    AX_U16 u16HighThr;
} AX_IVE_HYS_EDGE_CTRL_T;

/*
* CannyEdge control struct
*/
typedef struct axIVE_CANNY_EDGE_CTRL_T {
    AX_U8 u8Thr;
} AX_IVE_CANNY_EDGE_CTRL_T;

/*
* Region struct
*/
typedef struct axIVE_REGION_T {
    AX_U8 u8LabelStatus; /* 0: Labeled failed ; 1: Labeled successfully */
    AX_U32 u32Area;   /* Represented by the pixel number */
    AX_U16 u16Left;   /* Circumscribed rectangle left border */
    AX_U16 u16Right;  /* Circumscribed rectangle right border */
    AX_U16 u16Top;    /* Circumscribed rectangle top border */
    AX_U16 u16Bottom; /* Circumscribed rectangle bottom border */
} AX_IVE_REGION_T;

/*
* CCBLOB struct
*/
typedef struct axIVE_CCBLOB_T {
    AX_U16 u16RegionNum; /* Number of valid region */
    AX_IVE_REGION_T astRegion[AX_IVE_MAX_REGION_NUM]; /* Valid regions with 'u32Area>0' and 'label = ArrayIndex+1' */
} AX_IVE_CCBLOB_T;

/*
* Type of the CCL
*/
typedef enum axIVE_CCL_MODE_E {
    AX_IVE_CCL_MODE_8C = 0x0, /* 8-connected */
    AX_IVE_CCL_MODE_4C = 0x1, /* 4-connected */

    AX_IVE_CCL_MODE_BUTT
} AX_IVE_CCL_MODE_E;
/*
* CCL control struct
*/
typedef struct axIVE_CCL_CTRL_T {
    AX_IVE_CCL_MODE_E enMode; /* Mode */
} AX_IVE_CCL_CTRL_T;

/*
* Erode control parameter
*/
typedef struct axIVE_ERODE_CTRL_T {
    AX_U8 au8Mask[25]; /* The template parameter value must be 0 or 255. */
    AX_U32 threshold;  /* The threshold value range is 0-255. */
} AX_IVE_ERODE_CTRL_T;

/*
* Dilate control parameters
*/
typedef struct axIVE_DILATE_CTRL_T {
    AX_U8 au8Mask[25]; /* The template parameter value must be 0 or 255. */
    AX_U32 threshold;  /* The threshold value range is 0-255. */
} AX_IVE_DILATE_CTRL_T;

/*
* Filter control parameters
* You need to set these parameters when using the filter operator.
*/
typedef struct axIVE_FILTER_CTRL_T {
    AX_S8Q10 as8q10Mask[25]; /* Template parameter filter coefficient */
} AX_IVE_FILTER_CTRL_T;

/*
* Equalizehist control parameters
*/
typedef struct axIVE_EQUALIZE_HIST_CTRL_T {
    AX_U0Q20 u0q20HistEqualCoef; /* range: [0,1048575] */
} AX_IVE_EQUALIZE_HIST_CTRL_T;

/*
* Type of the Integ output results
*/
typedef enum axIVE_INTEG_OUT_CTRL_E {
    AX_IVE_INTEG_OUT_CTRL_COMBINE = 0x0,
    AX_IVE_INTEG_OUT_CTRL_SUM = 0x1,
    AX_IVE_INTEG_OUT_CTRL_SQSUM = 0x2,
    AX_IVE_INTEG_OUT_CTRL_BUTT
} AX_IVE_INTEG_OUT_CTRL_E;

/*
* Integ control parameters
*/
typedef struct axIVE_INTEG_CTRL_T {
    AX_IVE_INTEG_OUT_CTRL_E enOutCtrl;
} AX_IVE_INTEG_CTRL_T;

/*
* SOBEL control parameter
*/
typedef struct axIVE_SOBEL_CTRL_T {
    AX_S8Q10 as8q10Mask[25]; /* Template parameter sobel coefficient */
} AX_IVE_SOBEL_CTRL_T;

/*
* GMM2 control struct
*/
typedef struct axIVE_GMM2_CTRL_T {
    AX_U14Q4 u14q4InitVar; /* Initial Variance, range: [0,262143] */
    AX_U14Q4 u14q4MinVar;  /* Min  Variance, range: [0,262143] */
    AX_U14Q4 u14q4MaxVar;  /* Max  Variance, range: [0,262143] */
    AX_U1Q7 u1q7LearnRate;   /* Learning rate, range: [0,128] */
    AX_U1Q7 u1q7BgRatio;     /* Background ratio, range: [0,128] */
    AX_U4Q4 u4q4VarThr;      /* Variance Threshold, range: [0,255] */
    AX_U4Q4 u4q4VarThrCheck; /* Variance Threshold Check, range: [0,255] */
    AX_S1Q7 s1q7CT;          /* range: [-255,255] */
    AX_U8 u8Thr;             /* Output Threshold, range: [1,255] */
} AX_IVE_GMM2_CTRL_T;

/*
* Type of the Thresh mode.
*/
typedef enum axIVE_THRESH_MODE_E {
    AX_IVE_THRESH_MODE_BINARY = 0x0,    /* srcVal <= lowThr, dstVal = minVal; srcVal > lowThr, dstVal = maxVal. */
    AX_IVE_THRESH_MODE_TRUNC = 0x1,     /* srcVal <= lowThr, dstVal = srcVal; srcVal > lowThr, dstVal = maxVal. */
    AX_IVE_THRESH_MODE_TO_MINVAL = 0x2, /* srcVal <= lowThr, dstVal = minVal; srcVal > lowThr, dstVal = srcVal. */

    AX_IVE_THRESH_MODE_MIN_MID_MAX = 0x3, /* srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= highThr,
                                       dstVal = midVal; srcVal > highThr, dstVal = maxVal. */
    AX_IVE_THRESH_MODE_ORI_MID_MAX = 0x4, /* srcVal <= lowThr, dstVal = srcVal;  lowThr < srcVal <= highThr,
                                       dstVal = midVal; srcVal > highThr, dstVal = maxVal. */
    AX_IVE_THRESH_MODE_MIN_MID_ORI = 0x5, /* srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= highThr,
                                       dstVal = midVal; srcVal > highThr, dstVal = srcVal. */
    AX_IVE_THRESH_MODE_MIN_ORI_MAX = 0x6, /* srcVal <= lowThr, dstVal = minVal;  lowThr < srcVal <= highThr,
                                       dstVal = srcVal; srcVal > highThr, dstVal = maxVal. */
    AX_IVE_THRESH_MODE_ORI_MID_ORI = 0x7, /* srcVal <= lowThr, dstVal = srcVal;  lowThr < srcVal <= highThr,
                                       dstVal = midVal; srcVal > highThr, dstVal = srcVal. */

    AX_IVE_THRESH_MODE_BUTT
} AX_IVE_THRESH_MODE_E;

/*
* Thresh control parameters.
*/
typedef struct axIVE_THRESH_CTRL_T {
    AX_IVE_THRESH_MODE_E enMode;
    AX_U8 u8LowThr;  /* user-defined threshold,  0<=u8LowThr<=255 */
    AX_U8 u8HighThr; /* user-defined threshold, if enMode<AX_IVE_THRESH_MODE_MIN_MID_MAX, u8HighThr is not used,
                      else 0<=u8LowThr<=u8HighThr<=255; */
    AX_U8 u8MinVal;  /* Minimum value when tri-level thresholding */
    AX_U8 u8MidVal;  /* Middle value when tri-level thresholding, if enMode<2, u32MidVal is not used; */
    AX_U8 u8MaxVal;  /* Maxmum value when tri-level thresholding */
} AX_IVE_THRESH_CTRL_T;

/*
* Mode of 16BitTo8Bit
*/
typedef enum axIVE_16BIT_TO_8BIT_MODE_E {
    AX_IVE_16BIT_TO_8BIT_MODE_S16_TO_S8 = 0x0,
    AX_IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_ABS = 0x1,
    AX_IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS = 0x2,
    AX_IVE_16BIT_TO_8BIT_MODE_U16_TO_U8 = 0x3,

    AX_IVE_16BIT_TO_8BIT_MODE_BUTT
} AX_IVE_16BIT_TO_8BIT_MODE_E;

/*
* 16BitTo8Bit control parameters
*/
typedef struct axIVE_16BIT_TO_8BIT_CTRL_T {
    AX_IVE_16BIT_TO_8BIT_MODE_E enMode;
    AX_S1Q14 s1q14Gain; /* range: [-16383,16383] */
    AX_S16 s16Bias; /* range: [-16384,16383] */
} AX_IVE_16BIT_TO_8BIT_CTRL_T;

/*
* Type of the Map
*/
typedef enum axIVE_MAP_MODE_E {
    AX_IVE_MAP_MODE_U8 = 0x0,
    AX_IVE_MAP_MODE_S16 = 0x1,
    AX_IVE_MAP_MODE_U16 = 0x2,

    AX_IVE_MAP_MODE_BUTT
} AX_IVE_MAP_MODE_E;

/*
* Map control struct
*/
typedef struct axIVE_MAP_CTRL_T {
    AX_IVE_MAP_MODE_E enMode;
} AX_IVE_MAP_CTRL_T;

/*
* Map unsigned 8 bit LUT memory struct
*/
typedef struct axIVE_MAP_U8BIT_LUT_MEM_T {
    AX_U8 au8Map[AX_IVE_MAP_NUM];
} AX_IVE_MAP_U8BIT_LUT_MEM_T;

/*
* Map unsigned 16 bit LUT memory struct
*/
typedef struct axIVE_MAP_U16BIT_LUT_MEM_T {
    AX_U16 au16Map[AX_IVE_MAP_NUM];
} AX_IVE_MAP_U16BIT_LUT_MEM_T;

/*
* Map signed 16 bit LUT memory struct
*/
typedef struct axIVE_MAP_S16BIT_LUT_MEM_T {
    AX_S16 as16Map[AX_IVE_MAP_NUM];
} AX_IVE_MAP_S16BIT_LUT_MEM_T;

/*
* LBP compare mode
*/
typedef enum axIVE_LBP_CMP_MODE_E {
    AX_IVE_LBP_CMP_MODE_NORMAL = 0x0, /* P(x)-P(center)>= un8BitThr.s8Val, s(x)=1; else s(x)=0; */
    AX_IVE_LBP_CMP_MODE_ABS = 0x1,    /* Abs(P(x)-P(center))>=un8BitThr.u8Val, s(x)=1; else s(x)=0; */

    AX_IVE_LBP_CMP_MODE_BUTT
} AX_IVE_LBP_CMP_MODE_E;

/*
* LBP control parameters
*/
typedef struct axIVE_LBP_CTRL_T {
    AX_IVE_LBP_CMP_MODE_E enMode;
    AX_IVE_8BIT_U un8BitThr;
} AX_IVE_LBP_CTRL_T;

/*
* Corner memory struct
*/
typedef struct axIVE_CORNER_DET_MEM_T {
    AX_U16 u16CornerNum;
    AX_IVE_POINT_U16_T astCorner[AX_IVE_MAX_CORNER_NUM];
} AX_IVE_CORNER_DET_MEM_T;

/*
* Corner detection mode
*/
typedef enum axIVE_CORNER_DET_MODE_E {
    AX_IVE_CORNER_DET_MODE_HARRIS = 0x0, /* Harris corner detection */
    AX_IVE_CORNER_DET_MODE_ST = 0x1, /* Shi-Tomasi corner detection */

    AX_IVE_CORNER_DET_MODE_BUTT
} AX_IVE_CORNER_DET_MODE_E;

/*
* Corner detection control parameters
*/
typedef struct axIVE_CORNER_DET_CTRL_T {
    AX_IVE_CORNER_DET_MODE_E enMode;
    AX_U16 u16MaxCornerNum; /* Maximum number of corner output,range: [0, 500] */
    AX_S16 s16CornerThr; /* Corner hresh, range: [-32768, 32767] */
    AX_U8 u8Shift; /* Corner response shift, range: [0, 127] */
    AX_U1Q15 u1q15Harris; /* Harris corner detection coefficient */
    AX_U8 u8MatScale; /* Harris corner detection matrix shift bit, range: [0, 31]*/
} AX_IVE_CORNER_DET_CTRL_T;

/*
* Corner fpt_num parameters
*/
typedef struct axIVE_CORNER_FPT_NUM_CTRL_T {
    AX_U16 u16CornerFptNum;
} AX_IVE_CORNER_FPT_NUM_CTRL_T;

/*
* Type of the OrdStaFilter or Bernsen
*/
typedef enum axIVE_ORD_STAT_FILTER_BERNSEN_MODE_E {
    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_MIN = 0x0,
    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_MEDIAN = 0x1,
    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_MAX = 0x2,
    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_BERNSEN = 0x3,

    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_BUTT
} AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_E;

/*
* OrdStaFilter or Bernsen control parameters
*/
typedef struct axIVE_ORD_STAT_FILTER_BERNSEN_CTRL_T {
    AX_IVE_ORD_STAT_FILTER_BERNSEN_MODE_E enMode;
    AX_U8 u8Cmin; /* Bernsen local contrast threshold */
    AX_U8 u8MaxThr; /* The threshold value to judge corner based on corner response */
    AX_U8 u8Thr; /* ??? */
} AX_IVE_ORD_STAT_FILTER_BERNSEN_CTRL_T;

#pragma  pack (1)
/*
* LKOpticalFlow coordinate
*/
typedef struct axIVE_POINT_U11Q20_T {
    AX_U11Q20    u11q20Y;           /*Y coordinate*/
    AX_U11Q20    u11q20X;           /*X coordinate*/
}AX_IVE_POINT_U11Q20_T;

typedef AX_IVE_POINT_U11Q20_T AX_IVE_LK_OPTICAL_FLOW_POINT_MEM_T;

/*
* LKOpticalFlow DxDy
*/
typedef struct axIVE_DxDy_S3Q20_T {
    AX_S3Q20    s3q20Dy;           /*Y-direction component of the movement*/
    AX_S3Q20    s3q20Dx;           /*X-direction component of the movement*/
}AX_IVE_DxDy_S3Q20_T;

typedef AX_IVE_DxDy_S3Q20_T AX_IVE_LK_OPTICAL_FLOW_DXDY_MEM_T;

/*
* LKOpticalFlow PointDxDy mem info
*/
typedef struct axIVE_LK_OPTICAL_FLOW_POINT_DXDY_MEM_T {
    AX_IVE_LK_OPTICAL_FLOW_POINT_MEM_T u11q20Point;
    AX_IVE_LK_OPTICAL_FLOW_DXDY_MEM_T s3q20DxDy;
}IVE_LK_OPTICAL_FLOW_POINT_DXDY_MEM_T;

/*
* LKOpticalFlow Status mem info
*/
typedef struct axIVE_LK_OPTICAL_FLOW_STATUS_MEM_T {
    AX_U8       u8Status;          /*Result of tracking: 1-success; 0-failure*/
}IVE_LK_OPTICAL_FLOW_STATUS_MEM_T;

/*
* LKOpticalFlow movement
*/
typedef struct axIVE_MV_S2Q20_T {
    AX_U8       u8Status;          /*Result of tracking: 1-success; 0-failure*/
    AX_IVE_DxDy_S3Q20_T s3q20DxDy; /*XY-direction component of the movement*/
}AX_IVE_MV_S3Q20_T;

#pragma pack(0)

/*
* LKOpticalFlow control parameters
*/
typedef struct axIVE_LK_OPTICAL_FLOW_CTRL_T {
    AX_U16      u16CornerNum;           /*Number of the feature points,[1, 500]*/
    AX_U3Q5     u3q5PyraScale;          /*Scale factor used for pyramid lk-ofl. It's multiplied on pics[3](previous dx, dy) to map dx, dy from previous layer to current layer, [0, 128]*/
    AX_U8       u8MatScale;             /*In OFL, when calculate matrix G and b, bit-width is too large. Thus use this parameter to shrink bit-width for HW and clip-round. Set this value properly for better result, [0, 36]*/
    AX_U8Q6     u8q6GradCompensation;   /*This parameter should be set in consistent with the weight used to calculate gradients, [0, 16383]*/
    AX_S8Q10    s8q10FilterWeight[25];  /*This parameter is used to calculate gradients at specific feature points, [-262144, 262143]*/
    AX_U7Q12    u7q12FltGEpsilon;       /*Error set to judge whether determinant of matrix G is small enough. In other words, it judges whether this specific point has movement therefore can calculate ofl, [0, 524287]*/
    AX_U0Q16    u0q16IterEpsilon;       /*Condition used to jump out of iteration of single point ofl calculation. When res is less than ofl_iter_epsilon*ofl_iter_epsilon, end iteration. The larger this parameter is, the faster it will jump out of iteration, [0, 65535]*/
    AX_U8       u8MaxIter;              /*The maximum number of iteration for single point ofl calculation, [1, 15]*/
}AX_IVE_LK_OPTICAL_FLOW_CTRL_T;

/*
* Type of the LKOpticalFlow_Pyr
*/
typedef enum axIVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_E {
    AX_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_NONE = 0,   /*Output none*/
    AX_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_STATUS = 1, /*Output status*/

    AX_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_BUTT
} AX_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_E;

/*
* LKOpticalFlow_Pyr control parameters
*/
typedef struct axIVE_LK_OPTICAL_FLOW_PYR_CTRL_T {
    AX_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_E enOutMode;    /*Output mode*/
    AX_BOOL     bUseInitFlow;           /*where to use initial flow*/
    AX_U8       u8MaxLevel;             /*0<=u8MaxLevel<=3*/
    AX_U16      u16CornerNum;           /*Number of the feature points,[1, 500]*/
    AX_U3Q5     u3q5PyraScale;          /*Scale factor used for pyramid lk-ofl. It's multiplied on pics[3](previous dx, dy) to map dx, dy from previous layer to current layer, [0, 128]*/
    AX_U8       u8MatScale;             /*In OFL, when calculate matrix G and b, bit-width is too large. Thus use this parameter to shrink bit-width for HW and clip-round. Set this value properly for better result, [0, 36]*/
    AX_U8Q6     u8q6GradCompensation;   /*This parameter should be set in consistent with the weight used to calculate gradients, [0, 16383]*/
    AX_S8Q10    s8q10FilterWeight[25];  /*This parameter is used to calculate gradients at specific feature points, [-262144, 262143]*/
    AX_U7Q12    u7q12FltGEpsilon;       /*Error set to judge whether determinant of matrix G is small enough. In other words, it judges whether this specific point has movement therefore can calculate ofl, [0, 524287]*/
    AX_U0Q16    u0q16IterEpsilon;       /*Condition used to jump out of iteration of single point ofl calculation. When res is less than ofl_iter_epsilon*ofl_iter_epsilon, end iteration. The larger this parameter is, the faster it will jump out of iteration, [0, 65535]*/
    AX_U8       u8MaxIter;              /*The maximum number of iteration for single point ofl calculation, [1, 15]*/
}AX_IVE_LK_OPTICAL_FLOW_PYR_CTRL_T;

/*
* Sad mode
*/
typedef enum axIVE_SAD_MODE_E {
    AX_IVE_SAD_MODE_MB_4X4 = 0x0,   /* 4x4 */

    AX_IVE_SAD_MODE_BUTT
} AX_IVE_SAD_MODE_E;

/*
* Sad output ctrl
*/
typedef enum axIVE_SAD_OUT_CTRL_E {
    AX_IVE_SAD_OUT_CTRL_8BIT_BOTH = 0x0,  /* Output 8 bit sad and thresh */
    AX_IVE_SAD_OUT_CTRL_8BIT_SAD = 0x1,   /* Output 8 bit sad */
    AX_IVE_SAD_OUT_CTRL_THRESH = 0x2,     /* Output thresh */

    AX_IVE_SAD_OUT_CTRL_BUTT
} AX_IVE_SAD_OUT_CTRL_E;

/*
* Sad control parameters
*/
typedef struct axIVE_SAD_CTRL_T {
    AX_IVE_SAD_MODE_E enMode;
    AX_IVE_SAD_OUT_CTRL_E enOutCtrl;
    AX_U8 u8LowThr;  /* user-defined threshold, 0<=u8LowThr<=255 */
    AX_U8 u8HighThr; /* user-defined threshold, 0<=u8LowThr<=u8HighThr<=255; */
    AX_U8 u8MinVal;  /* Minimum value when tri-level thresholding */
    AX_U8 u8MidVal;  /* Middle value when tri-level thresholding */
    AX_U8 u8MaxVal;  /* Maxmum value when tri-level thresholding */
    AX_S8Q10 as8q10Mask[25]; /* filter coefficient */
} AX_IVE_SAD_CTRL_T;

#pragma  pack (1)
/*
* NCC dst memory struct
*/
typedef struct axIVE_NCC_DST_MEM_T {
    AX_U64 u64Numerator;
    AX_U64 u64QuadSum1;
    AX_U64 u64QuadSum2;
    AX_U8 u8Reserved[8];
} AX_IVE_NCC_DST_MEM_T;
#pragma  pack (0)

#ifdef __cplusplus
}
#endif
#endif/*_AX_IVE_TYPE_H_*/

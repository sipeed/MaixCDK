/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_IVE_API_H_
#define _AX_IVE_API_H_

#include "ax_ive_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
*   Prototype    : AX_IVE_Init
*   Description  : IVE module initialize
*   Parameters   : AX_VOID
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         :
*
*****************************************************************************/
AX_S32 AX_IVE_Init(AX_VOID);

/*****************************************************************************
*   Prototype    : AX_IVE_Exit
*   Description  : IVE module exit
*   Parameters   : AX_VOID
*   Return Value : AX_VOID
*   Spec         :
*
*****************************************************************************/
AX_VOID AX_IVE_Exit(AX_VOID);

/*****************************************************************************
*   Prototype    : AX_IVE_Query
*   Description  : This API is used to query the status of a called function by using the returned IveHandle of the function.
                   In block mode, the system waits until the function that is being queried is called.
                   In non-block mode, the current status is queried and no action is taken.
*   Parameters   : AX_IVE_HANDLE  IveHandle     IveHandle of a called function. It is entered by users.
*                  AX_BOOL       *pbFinish      Returned status
*                  AX_BOOL        bBlock        Flag indicating the block mode or non-block mode
*                  AX_BOOL  *pbFinish
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         :
*
*****************************************************************************/
AX_S32 AX_IVE_Query(AX_IVE_HANDLE IveHandle, AX_BOOL *pbFinish, AX_BOOL bBlock);

/*****************************************************************************
*   Prototype    : AX_IVE_DMA
*   Description  : Direct memory access (DMA):
*                  1.Direct memory copy;
*                    2. Copy with interval bytes;
*                  3. Memset using 3 bytes;
*                    4. Memset using 8 bytes;
*   Parameters   : AX_IVE_HANDLE       *pIveHandle        Returned handle ID of a task.
*                  AX_IVE_SRC_DATA_T   *pstSrc            Input source data.The input data is treated as U8C1 data.
*                  AX_IVE_DST_DATA_T   *pstDst            Output result data.
*                  AX_IVE_DMA_CTRL_T   *pstDmaCtrl        DMA control parameter.
*                  AX_BOOL              bInstant          Flag indicating whether to generate an interrupt.
*                                                         If the output result blocks the next operation,
*                                                         set bInstant to AX_TRUE.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 32x1 pixels to 1920x1080 pixels.
*                  The stride must be 16-byte-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_DMA(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_DATA_T *pstSrc, AX_IVE_DST_DATA_T *pstDst,
    AX_IVE_DMA_CTRL_T *pstDmaCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Add
*   Description  : Two gray images' Add operation.
*   Parameters   : AX_IVE_HANDLE       *pIveHandle      Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T  *pstSrc1         Augend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T  *pstSrc2         Addend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T  *pstDst          Output result of src1 plus src2
*                  AX_IVE_ADD_CTRL_T   *pstAddCtrl      Control parameter
*                  AX_BOOL              bInstant        For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_Add(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_ADD_CTRL_T *pstAddCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Sub
*   Description  : Two gray images' Sub operation.
*   Parameters   : AX_IVE_HANDLE       *pIveHandle   Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T  *pstSrc1      Minuend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T  *pstSrc2      Subtrahend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T  *pstDst       Output result of src1 minus src2
*                  AX_IVE_SUB_CTRL_T   *pstSubCtrl   Control parameter
*                  AX_BOOL              bInstant     For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_Sub(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_SUB_CTRL_T *pstSubCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_And
*   Description  : Binary images' And operation.
*   Parameters   : AX_IVE_HANDLE          *pIveHandle       Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T     *pstSrc1          The input source1. Only U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T     *pstSrc2          The input source2.Only U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T     *pstDst           Output result of " src1 & src2 ".
*                  AX_BOOL              bInstant         For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_And(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Or
*   Description  : Two binary images' Or operation.
*   Parameters   : AX_IVE_HANDLE          *pIveHandle    Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T     *pstSrc1       Input source1. Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T     *pstSrc2       Input source2. Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T     *pstDst        Output result src1 or src2
*                  AX_BOOL              bInstant        For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_Or(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_BOOL bInstant);
/*****************************************************************************
*   Prototype    : AX_IVE_Xor
*   Description  : Two binary images' Xor operation.
*   Parameters   : AX_IVE_HANDLE           *pIveHandle    Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T      *pstSrc1       The input source1.Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T      *pstSrc2       The input source2.
*                  AX_IVE_DST_IMAGE_T      *pstDst        Output result
*                  AX_BOOL               bInstant      For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_Xor(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Mse
*   Description  : Two gray images' Mse operation.
*   Parameters   : AX_IVE_HANDLE       *pIveHandle   Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T  *pstSrc1      Minuend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T  *pstSrc2      Subtrahend of the input source.Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T  *pstDst       Output result of src1 mse src2
*                  AX_IVE_MSE_CTRL_T   *pstMseCtrl   Control parameter
*                  AX_BOOL              bInstant     For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The types, widths, heights of two input sources must be the same.
*
*****************************************************************************/
AX_S32 AX_IVE_Mse(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_MSE_CTRL_T *pstMseCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_CannyHysEdge
*   Description  : The first part of canny Edge detection. Including step: gradient calculation,
*                  magnitude and angle calculation, hysteresis threshold, NMS(Non-Maximum Suppression)
*   Parameters   : AX_IVE_HANDLE              *pIveHandle           Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T         *pstSrc1              Input source1. Only the U8C1 input format is supported
*                  AX_IVE_SRC_IMAGE_T         *pstSrc2              Input source2. Only the U8C1 input format is supported
*                  AX_IVE_DST_IMAGE_T         *pstDst               Output result.
*                  AX_IVE_HYS_EDGE_CTRL_T     *pstHysEdgeCtrl       Control parameter.
*                  AX_BOOL                     bInstant             For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_CannyHysEdge(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_HYS_EDGE_CTRL_T *pstCannyHysEdgeCtrl, AX_BOOL bInstant);


/*****************************************************************************
*   Prototype    : AX_IVE_CannyEdge
*   Description  : The second part of canny Edge detection: trace strong edge by weak edge.
*   Parameters   : AX_IVE_HANDLE              *pIveHandle         Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T         *pstSrc             Input source. Only the U8C1 format is supported
*                  AX_IVE_DST_IMAGE_T         *pstDst             Output result.
*                  AX_IVE_CANNY_EDGE_CTRL_T   *pstCannyEdgeCtrl   Control parameter.
*                  AX_BOOL                     bInstant           For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_CannyEdge(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_CANNY_EDGE_CTRL_T *pstCannyEdgeCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_CCL
*   Description  : Connected Component Labeling. Only 8-Connected method is supported.
*   Parameters   : AX_IVE_HANDLE         *pIveHandle      Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T    *pstSrc          Input source
*                  AX_IVE_DST_IMAGE_T    *pstDst          Output result of label
*                  AX_IVE_MEM_INFO_T     *pstBlob         Output result of detected region;
*                  AX_IVE_CCL_CTRL_T     *pstCclCtrl      CCL control parameter
*                  AX_BOOL                bInstant        For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1280x720 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_CCL(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_DST_MEM_INFO_T *pstBlob, AX_IVE_CCL_CTRL_T *pstCclCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Erode
*   Parameters   : 5x5 template erode. Only the U8C1 binary image input is supported.Or else the result is not correct.
*   Input        : AX_IVE_HANDLE            *pIveHandle       Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T       *pstSrc           Input binary image, which consists of 0 or 255;
*                  AX_IVE_DST_IMAGE_T       *pstDst           Output result.
*                  AX_IVE_ERODE_CTRL_T      *pstErodeCtrl     Control parameters
*                  AX_BOOL                   bInstant         For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The input value, output value, and mask value must be 0 or 255.
*
*****************************************************************************/
AX_S32 AX_IVE_Erode(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_ERODE_CTRL_T *pstErodeCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Dilate
*   Description  : 5x5 template dilate. Only the U8C1 binary image input is supported.Or else the result is not expected.
*   Parameters   : AX_IVE_HANDLE          *pIveHandle          Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T     *pstSrc              Input binary image, which consists of 0 or 255;
*                  AX_IVE_DST_IMAGE_T     *pstDst              Output result.
*                  AX_IVE_DILATE_CTRL_T   *pstDilateCtrl       Control parameters.
*                  AX_BOOL                 bInstant            For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The input value, output value, and mask value must be 0 or 255.
*
*****************************************************************************/
AX_S32 AX_IVE_Dilate(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_DILATE_CTRL_T *pstDilateCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Filter
*   Description  : 5x5 template filter.
*   Parameters   : AX_IVE_HANDLE         *pIveHandle         Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T    *pstSrc             Input source data.
*                                                            The U8C1,SP420 and SP422 input formats are supported.
*                  AX_IVE_DST_IMAGE_T    *pstDst             Output result, of same type with the input.
*                  AX_IVE_FILTER_CTRL_T  *pstFltCtrl         Control parameters of filter
*                  AX_BOOL                bInstant           For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_Filter(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_FILTER_CTRL_T *pstFltCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Hist
*   Description  : Calculate the input gray image's histogram.
*   Parameters   : AX_IVE_HANDLE          *pIveHandle      Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T     *pstSrc          Input source data. Only the U8C1 input format is supported.
*                  AX_IVE_DST_MEM_INFO_T  *pstDst          Output result.
*                  AX_BOOL                 bInstant        For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_Hist(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_MEM_INFO_T *pstDst,
    AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_EqualizeHist
*   Description  : Enhance the input image's contrast through histogram equalization.
*   Parameters   : AX_IVE_HANDLE               *pIveHandle              Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T          *pstSrc                  Input source.Only U8C1 input format is supported.
*                  AX_IVE_DST_MEM_INFO_T       *pstDst                  Output result.
*                  AX_IVE_EQUALIZEHIST_CTRL_T  *pstEqualizeHistCtrl     EqualizeHist control parameter.
*                  AX_BOOL                      bInstant                For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_EqualizeHist(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_MEM_INFO_T *pstDst,
    AX_IVE_EQUALIZE_HIST_CTRL_T *pstEqualizeHistCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_INTEG
*   Description  : Calculate the input gray image's integral image.
*   Parameters   : AX_IVE_HANDLE        *pIveHandle        Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T   *pstSrc            Input source data.Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T   *pstDst            Output result.Can be U32C1 or U64C1, relied on the control parameter.
*                  AX_IVE_INTEG_CTRL_T  *pstIntegCtrl      Integ Control
*                  AX_BOOL               bInstant          For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 32x16 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*                  The pixel can be 32bit or 64 bit relied on the control parameter.
*
*****************************************************************************/
AX_S32 AX_IVE_Integ(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_INTEG_CTRL_T *pstIntegCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_MagAndAng
*   Description  : MagAndAng is used to extract the edge information.
*   Parameters   : AX_IVE_HANDLE              *pIveHandle         Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T         *pstSrc1            Input source1 data. Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T         *pstSrc2            Input source2 data. Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T         *pstDstMag          Output magnitude.
*                  AX_IVE_DST_IMAGE_T         *pstDstAng          Output angle.
*                                                                 If the output mode is set to magnitude only,
*                                                                 this item can be set to null.
*                  AX_IVE_MAG_AND_ANG_CTRL_T  *pstMagAndAngCtrl   Control parameters
*                  AX_BOOL                     bInstant           For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_MagAndAng(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2,
    AX_IVE_DST_IMAGE_T *pstDstMag, AX_IVE_DST_IMAGE_T *pstDstAng, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Sobel
*   Description  : SOBEL is used to extract the gradient information.
*   Parameters   : AX_IVE_HANDLE        *pIveHandle      Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T   *pstSrc          Input source data. Only the U8C1 input image is supported.
*                  AX_IVE_DST_IMAGE_T   *pstDst          The result of input image filtered by the input mask.
*                  AX_IVE_SOBEL_CTRL_T  *pstSobelCtrl    Control parameters
*                  AX_BOOL               bInstant        For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_Sobel(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_SOBEL_CTRL_T *pstSobelCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_GMM2
*   Description  : Separate foreground and background using GMM(Gaussian Mixture Model) method;
*                  Gray or RGB GMM are supported.
*   Parameters   : AX_IVE_HANDLE       *pIveHandle          Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T  *pstSrc              Only U8C1 or U8C3_PACKAGE input are supported.
*                  AX_IVE_DST_IMAGE_T  *pstFg               Output foreground (Binary) image.
*                  AX_IVE_DST_IMAGE_T  *pstBg               Output background image. With same type of pstSrc.
*                  AX_IVE_MEM_INFO_T   *pstModel            Model data.
*                  AX_IVE_GMM2_CTRL_T  *pstGmm2Ctrl         Control parameter.
*                  AX_BOOL              bInstant            For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1280x720 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_GMM2(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstFg,
 AX_IVE_DST_IMAGE_T *pstBg, AX_IVE_MEM_INFO_T *pstModel, AX_IVE_GMM2_CTRL_T *pstGmm2Ctrl, AX_BOOL bInstant);

 /*****************************************************************************
*   Prototype    : AX_IVE_Thresh
*   Description  : Thresh operation to the input image.
*   Parameters   : AX_IVE_HANDLE         *pIveHandle       Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T    *pstSrc           Input source data. Only the U8C1 input format is supported.
*                  AX_IVE_DST_IMAGE_T    *pstDst           Output result
*                  AX_IVE_THRESH_CTRL_T  *pstThrCtrl       Control parameters
*                  AX_BOOL                bInstant         For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_Thresh(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_THRESH_CTRL_T *pstThrCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_16BitTo8Bit
*   Description  : Scale the input 16bit data to the output 8bit data.
*   Parameters   : AX_IVE_HANDLE              *pIveHandle              Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T         *pstSrc                  Input source data.Only U16C1\S16C1 input is supported.
*                  AX_IVE_DST_IMAGE_T         *pstDst                  Output result
*                  AX_IVE_16BITTO8BIT_CTRL_T  *pst16BitTo8BitCtrl      Control parameter
*                  AX_BOOL                     bInstant                For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_16BitTo8Bit(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_16BIT_TO_8BIT_CTRL_T *pst16BitTo8BitCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_Map
*   Description  : Map a image to another through a lookup table.
*   Parameters   : AX_IVE_HANDLE              *pIveHandle              Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T         *pstSrc                  Input source data.Only U8C1 input is supported
*                  AX_IVE_SRC_MEM_INFO_T      *pstMap                  Input lookup table. Must be an U8 array of size 256
*                  AX_IVE_DST_IMAGE_T         *pstDst                  Output result
*                  AX_IVE_MAP_CTRL_T          *pstMapCtrl              Control parameter
*                  AX_BOOL                     bInstant                For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1080 pixels.
*                  The physical addresses of the input data and output data must be 8-byte-aligned.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_Map(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_SRC_MEM_INFO_T *pstMap,
    AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_MAP_CTRL_T *pstMapCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_LBP
*   Description  : LBP calculation using the original method and a extensional method.
*   Parameters   : AX_IVE_HANDLE         *pIveHandle     Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T    *pstSrc         Input source.Only the U8C1 inpu format is supported
*                  AX_IVE_DST_IMAGE_T    *pstDst         Output result
*                  AX_IVE_LBP_CTRL_T     *pstLbpCtrl     Control parameter
*                  AX_BOOL                bInstant       For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The physical addresses of the input data and output data must be 8-byte-aligned.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_LBP(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_LBP_CTRL_T *pstLbpCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_CornerDet
*   Description  : Corners detection using Shi-Tomasi-like or Harris method.
*   Parameters   : AX_IVE_HANDLE            *pIveHandle        Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T       *pstSrc1           Input source1 data
*                  AX_IVE_SRC_IMAGE_T       *pstSrc2           Input source2 data
*                  AX_IVE_DST_MEM_INFO_T    *pstCorner         Output result of corner coordinates
*                  AX_IVE_DST_IMAGE_T       *pstDst            Output result of corner responses
*                  AX_IVE_ST_CORNER_CTRL_T  *pstCornerCtrl     Control parameter
*                  AX_BOOL                   bInstant          For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1280x720 pixels.
*                  The physical addresses of the input data and output data must be 16-byte-aligned.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_CornerDet(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1, AX_IVE_SRC_IMAGE_T *pstSrc2, AX_IVE_DST_MEM_INFO_T *pstCorner,
     AX_IVE_DST_IMAGE_T *pstDst, AX_IVE_CORNER_DET_CTRL_T *pstCornerCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_OrdStatFilter_Bernsen
*   Description  : Order Statistic Filter or Bernsen thresh. It can be used as median\max\min value filter or bernsen thresh.
*   Parameters   : AX_IVE_HANDLE                          *pIveHandle             Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T                     *pstSrc                 Input source data. Only U8C1 input is supported
*                  AX_IVE_DST_IMAGE_T                     *pstDst                 Output result
*                  AX_IVE_ORD_STAT_FILTER_BERNSEN_CTRL_T  *pstOrdStatFltBernCtrl  Control parameter
*                  AX_BOOL                                 bInstant               For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1920x1024 pixels.
*                  The physical addresses of the input data and output data must be 8-byte-aligned.
*                  The stride must be 16-pixel-aligned.
*
*****************************************************************************/
AX_S32 AX_IVE_OrdStatFilter_Bernsen(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc, AX_IVE_DST_IMAGE_T *pstDst,
    AX_IVE_ORD_STAT_FILTER_BERNSEN_CTRL_T *pstOrdStatFltBernCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_LKOpticalFlow
*   Description  : Calculate single-layer LK Optical Flow.
*   Parameters   : AX_IVE_HANDLE                    *pIveHandle          Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T               *pstSrcPre           Pre-frame input source.Must be U8C1 image
*                  AX_IVE_SRC_IMAGE_T               *pstSrcCur           Cur-frame input source.Same size, type with pstPreSrc
*                  AX_IVE_SRC_MEM_INFO_T            *pstPoint            Intresting points coordinates in the cur-layer for LKOpticalFlow tracking
*                  AX_IVE_MEM_INFO_T                *pstMv               Accumlative movements of the interesting points in pre-layers
*                                                                        or init 0s for the first-layer as input.
*                  AX_IVE_LK_OPTICAL_FLOW_CTRL_T    *pstLkOptiFlowCtrl   Control parameters.
*                  AX_BOOL                           bInstant             For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1280x720 pixels.
*                  The stride must be 16-pixel-aligned.
*
****************************************************************************/
AX_S32 AX_IVE_LKOpticalFlow(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrcPre,
    AX_IVE_SRC_IMAGE_T *pstSrcCur, AX_IVE_SRC_MEM_INFO_T *pstPoint, AX_IVE_MEM_INFO_T *pstMv,
    AX_IVE_LK_OPTICAL_FLOW_CTRL_T *pstLkOptiFlowCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_LKOpticalFlowPyr
*   Description  : Calculate the LK optical flow of the pyramid.
*   Parameters   : AX_IVE_HANDLE                    *pIveHandle             Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T               astSrcPrevPyr[]         Pre-frame input source, with a size of u8MaxLevel.Must be U8C1 image
*                  AX_IVE_SRC_IMAGE_T               astSrcNextPyr[]         Next-frame input source, with a size of u8MaxLevel.Same size, type with pstPreSrc
*                  AX_IVE_SRC_MEM_INFO_T            *pstPrevPts             The initial feature point array of layer 0 (astSrcPrevPyr[0]) in the previous image pyramid
*                  AX_IVE_MEM_INFO_T                *pstNextPts             The feature point pstPrevPts calculates the LK optical flow through a pyramid to obtain the
*                                                                           coordinates corresponding to the 0 th layer of the next frame image pyramid (astSrcNextPyr [])
                   AX_IVE_DST_MEM_INFO_T            *pstStatus
*                  AX_IVE_LK_OPTICAL_FLOW_CTRL_T    *pstLkOptiFlowPyrCtrl   Control parameters.
*                  AX_BOOL                           bInstant               For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : The size of the input data ranges from 64x64 pixels to 1280x720 pixels.
*                  The stride must be 16-pixel-aligned.
*
****************************************************************************/
AX_S32 AX_IVE_LKOpticalFlowPyr(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T astSrcPrevPyr[],
    AX_IVE_SRC_IMAGE_T astSrcNextPyr[], AX_IVE_SRC_MEM_INFO_T *pstPrevPts, AX_IVE_MEM_INFO_T *pstNextPts,
    AX_IVE_DST_MEM_INFO_T *pstStatus, AX_IVE_LK_OPTICAL_FLOW_PYR_CTRL_T *pstLkOptiFlowPyrCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_SAD
*   Description  : Sum of absolute differences.
*   Parameters   : AX_IVE_HANDLE          *pIveHandle   Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T     *pstSrc1      The input source1.Only the U8C1 input format is supported
*                  AX_IVE_SRC_IMAGE_T     *pstSrc2      The input source2.Only the U8C1 input format is supported
*                  AX_IVE_DST_IMAGE_T     *pstSad       Output result of sad value.Only the U8C1 format is supported
*                  AX_IVE_DST_IMAGE_T     *pstThr       Output result of thresh.Only the U8C1 format is supported
*                  AX_IVE_SAD_CTRL_T      *pstSadCtrl   Control parameter
*                  AX_BOOL                 bInstant     For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : To do
*
*
*****************************************************************************/
AX_S32 AX_IVE_SAD(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1,
    AX_IVE_SRC_IMAGE_T *pstSrc2, AX_IVE_DST_IMAGE_T *pstSad,AX_IVE_DST_IMAGE_T *pstThr,
    AX_IVE_SAD_CTRL_T *pstSadCtrl, AX_BOOL bInstant);

/*****************************************************************************
*   Prototype    : AX_IVE_NCC
*   Description  : Calculate two gray images' NCC (Normalized Cross Correlation).
*   Parameters   : AX_IVE_HANDLE            *pIveHandle     Returned handle ID of a task
*                  AX_IVE_SRC_IMAGE_T       *pstSrc1        Input source1. Only the U8C1 input format is supported.
*                  AX_IVE_SRC_IMAGE_T       *pstSrc2        Input source2. Must be of the same type,size of source1.
*                  AX_IVE_DST_MEM_INFO_T    *pstDst         Output result
*                  AX_BOOL                   bInstant       For details, see AX_IVE_DMA.
*   Return Value : AX_SUCCESS: Success;Error codes: Failure.
*   Spec         : To do
*
*****************************************************************************/
AX_S32 AX_IVE_NCC(AX_IVE_HANDLE *pIveHandle, AX_IVE_SRC_IMAGE_T *pstSrc1,
    AX_IVE_SRC_IMAGE_T *pstSrc2, AX_IVE_DST_MEM_INFO_T *pstDst, AX_BOOL bInstant);

#ifdef __cplusplus
}
#endif

#endif //_AX_IVE_API_H_

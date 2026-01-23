/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_IVES_AP_H_80DEC0DA_BDEB_419F_85AD_3371389435C3_
#define _AX_IVES_AP_H_80DEC0DA_BDEB_419F_85AD_3371389435C3_

#include "ax_global_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_IVES_MAX_IMAGE_WIDTH (4096)
#define AX_IVES_MAX_IMAGE_HEIGHT (3072)
#define AX_IVES_MAX_REGION_NUM (254)

typedef enum {
    IVES_CCL_MODE_4C = 0x0, /* 4-connectivity */
    IVES_CCL_MODE_8C = 0x1, /* 8-connectivity */
    IVES_CCL_MODE_BUTT
} AX_IVES_CCL_MODE_E;

typedef struct axIVES_RECT_T {
    AX_U32 u32X;
    AX_U32 u32Y;
    AX_U32 u32W;
    AX_U32 u32H;
} AX_IVES_RECT_T;

typedef struct axIVES_MB_SIZE_T {
    AX_U32 u32W; /* x pixels */
    AX_U32 u32H; /* y pixels */
} AX_IVES_MB_SIZE_T;

typedef struct axIVES_BLK_SIZE_T {
    AX_U32 u32W;
    AX_U32 u32H;
} AX_IVES_BLK_SIZE_T;

typedef struct axIVES_REGION_T {
    AX_U32 u32Area;   /* Represented by the pixel number */
    AX_U16 u16Left;   /* Circumscribed rectangle left border */
    AX_U16 u16Top;    /* Circumscribed rectangle top border */
    AX_U16 u16Right;  /* Circumscribed rectangle right border */
    AX_U16 u16Bottom; /* Circumscribed rectangle bottom border */
} AX_IVES_REGION_T;

typedef struct axIVES_CCBLOB_T {
    AX_IVES_CCL_MODE_E enMode;                          /* [IN ]: CCL mode */
    AX_U32 u32AreaThrs;                                 /* [IN ]: threshold of area pixels */
    AX_U32 u32RegionNum;                                /* [OUT]: Number of valid region */
    AX_IVES_REGION_T arrRegion[AX_IVES_MAX_REGION_NUM]; /* [OUT]: Valid regions with 'u32Area>0' */
} AX_IVES_CCBLOB_T;

typedef AX_VIDEO_FRAME_T AX_IVES_IMAGE_T;

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize or uninitialize IVES device
///
/// @param N/A
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_Init(AX_VOID);
AX_S32 AX_IVES_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
///                                 MD
//////////////////////////////////////////////////////////////////////////////////////
typedef AX_S32 MD_CHN;

typedef enum axMD_ALG_MODE_E { AX_MD_MODE_REF = 0x0, AX_MD_MODE_BUTT } AX_MD_ALG_MODE_E;

typedef struct axMD_CHN_ATTR_T {
    MD_CHN mdChn;
    AX_MD_ALG_MODE_E enAlgMode;
    AX_IVES_MB_SIZE_T stMbSize;
    AX_IVES_RECT_T stArea;
    AX_U8 u8ThrY; /* threshold of Y */
} AX_MD_CHN_ATTR_T;

typedef struct axMD_MB_THR_T {
    AX_U32 u32Count; /* MB count */
    AX_U8 *pMbThrs;  /* 1 or 0 thresold of each MB */
} AX_MD_MB_THR_T;

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize or uninitialize MD module
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_MD_Init(AX_VOID);
AX_S32 AX_IVES_MD_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Create or destory MD channel
///
/// @param mdChn   [I]: specifies the MD channel to create
/// @param pstAttr [I]: specifies channel attribute.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_MD_CreateChn(MD_CHN mdChn, AX_MD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_MD_DestoryChn(MD_CHN mdChn);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Set or get channel attribute
///
/// @param mdChn   [I ]: specifies the MD channel created.
/// @param pstAttr [IO]: specifies the channel attribute to set [I] or get [O].
///        <NOTE>
///         - enAlgMode: static parameter, cannot be changed.
///         - stMbSize : static parameter, cannot be changed.
///         - stMbSize : static parameter, cannot be changed.
///         - stArea   : static parameter, cannot be changed.
///
/// @return 0 if success, otherwise failure
/// If static parametes are changed, AX_ERR_ILLEGAL_PARAM error will be returned.
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_MD_SetChnAttr(MD_CHN mdChn, AX_MD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_MD_GetChnAttr(MD_CHN mdChn, AX_MD_CHN_ATTR_T *pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Process image
///        The 1st input image is reference image, mb thresholds will return all 0.
///
/// @param mdChn    [I]: specifies the MD channel created.
/// @param pstCur   [I]: specifies the image to process
/// @param pstMbThr [O]: return the detected result.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_MD_Process(MD_CHN mdChn, AX_IVES_IMAGE_T *pstCur, AX_MD_MB_THR_T *pstMbThr, AX_IVES_CCBLOB_T *pstBlob);

//////////////////////////////////////////////////////////////////////////////////////
///                                 MD
//////////////////////////////////////////////////////////////////////////////////////
typedef AX_S32 OD_CHN;

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize or uninitialize OD module
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_OD_Init(AX_VOID);
AX_S32 AX_IVES_OD_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Create or destory OD channel
///
/// @param odChn   [I]: specifies the OD channel
/// @param pstAttr [I]: specifies the channel attribute.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
typedef struct axOD_CHN_ATTR_T {
    OD_CHN odChn;
    AX_IVES_RECT_T stArea; /* area of OD */
    AX_U32 u32FrameRate;   /* frame rate: 25/30/60 */

    AX_U8 u8ThrdY;       /* threshold of Y, range [0 - 255] */
    AX_U8 u8ConfidenceY; /* threshold confidence percent, range [0 - 100] */

    /*
        @param u32LuxThrd: specifies the lux threshold. U22.10
            - u32LuxThrd >= 1024, means if current lux < u32LuxThrd, mark od = 1
            - u32LuxThrd = 0, skip the beginning 3 seconds frames to estimate the scene lux.
        @param u32LuxDiff: specifies the lux changed threshold.
            - if u32LuxThrd >= 1024, u32LuxDiff is the absolute changed lux threshold, U22.10
                    it means:
                    if abs(current lux - last lux) > u32LuxDiff, then mark od = 1
            - if u32LuxThrd = 0, u32LuxDiff is the confidence of lux changed, range [0 - 100]
                    it means:
                    if (current lux < scene lux * u32LuxDiff/100.0), then mark od = 1

        Recommended to set:
            when lux is descended to 40% of scense lux, then mark od = 1
            u32LuxThrd =  0  // use scene lux to reference lux
            u32LuxDiff = 60  // lux descend confidenece is 40%
    */
    AX_U32 u32LuxThrd;
    AX_U32 u32LuxDiff;
} AX_OD_CHN_ATTR_T;

AX_S32 AX_IVES_OD_CreateChn(OD_CHN odChn, AX_OD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_OD_DestoryChn(OD_CHN odChn);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Set or get channel attribute
///
/// @param odChn   [I ]: specifies the OD channel created.
/// @param pstAttr [IO]: specifies the channel attribute to set [I] or get [O]
///
/// @return 0 if success, otherwise failure
/// If static parametes are changed, AX_ERR_ILLEGAL_PARAM error will be returned.
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_OD_SetChnAttr(OD_CHN odChn, AX_OD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_OD_GetChnAttr(OD_CHN odChn, AX_OD_CHN_ATTR_T *pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Process image
///
/// @param odChn   [I]: specifies the OD channel created.
/// @param pstCur  [I]: specifies the image to process
/// @param pResult [O]: return the detection result, 0: not occlusion, 1: occlusion
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
typedef struct axIVES_OD_IMAGE_T {
    AX_IVES_IMAGE_T *pstImg;
    AX_U32 u32Lux; /* current lux returned by AE, U22.10 */
} AX_IVES_OD_IMAGE_T;
AX_S32 AX_IVES_OD_Process(OD_CHN odChn, const AX_IVES_OD_IMAGE_T *pstCur, AX_U8 *pResult);


//////////////////////////////////////////////////////////////////////////////////////
///                         SCD (Scene Change Detection)
//////////////////////////////////////////////////////////////////////////////////////
typedef AX_S32 SCD_CHN;

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize or uninitialize SCD module
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_SCD_Init(AX_VOID);
AX_S32 AX_IVES_SCD_DeInit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
typedef struct axSCD_CHN_ATTR_T {
    SCD_CHN chn;
    AX_IVES_RECT_T stArea;
    AX_U8 u8Thrd;           /* range: [1 - 100], recommend: 60 */
    AX_U8 u8Confidence;     /* range: [1 - 100], recommend: 60 */
} AX_SCD_CHN_ATTR_T;
AX_S32 AX_IVES_SCD_CreateChn(SCD_CHN scdChn, const AX_SCD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_SCD_DestoryChn(SCD_CHN scdChn);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Set or get channel attribute
///
/// @param scdChn  [I ]: specifies the SCD channel created.
/// @param pstAttr [IO]: specifies the channel attribute to set [I] or get [O]
///
/// @return 0 if success, otherwise failure
/// If static parametes are changed, AX_ERR_ILLEGAL_PARAM error will be returned.
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_SCD_SetChnAttr(SCD_CHN scdChn, const AX_SCD_CHN_ATTR_T *pstAttr);
AX_S32 AX_IVES_SCD_GetChnAttr(SCD_CHN scdChn, AX_SCD_CHN_ATTR_T *pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Process image
///
/// @param scdChn  [I]: specifies the SCD channel created.
/// @param pstImg  [I]: specifies the image to process
/// @param pResult [O]: return the detection result, 0: not change, 1: changed
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_IVES_SCD_Process(SCD_CHN scdChn, const AX_IVES_IMAGE_T *pstImg, AX_U8 *pResult);

#ifdef __cplusplus
}
#endif

#endif /*  _AX_IVES_AP_H_80DEC0DA_BDEB_419F_85AD_3371389435C3_ */
/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_VENC_COMM_H__
#define __AX_VENC_COMM_H__

#include "ax_base_type.h"
#include "ax_codec_comm.h"
#include "ax_global_type.h"
#include "ax_venc_rc.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define AX_VENC_QP_HISGRM_NUM (52)

#define AX_MAX_VENC_CHN_NUM (16)

#define AX_MAX_VENC_GRP_NUM (AX_MAX_VENC_CHN_NUM / 2)

#define AX_MAX_VENC_ROI_NUM (8)
#define AX_MAX_JENC_ROI_NUM (8)

#define AX_MAX_VENC_OSD_NUM (8)

#define AX_MAX_VENC_NALU_NUM (32)

#define AX_MIN_VENC_PIC_WIDTH_H264 (144)
#define AX_MIN_VENC_PIC_WIDTH_H265 (136)
#define AX_MAX_VENC_PIC_WIDTH (5120)

#define AX_MIN_VENC_PIC_HEIGHT_H264 (144)
#define AX_MIN_VENC_PIC_HEIGHT_H265 (136)
#define AX_MAX_VENC_PIC_HEIGHT (5120)

#define AX_MIN_JENC_PIC_WIDTH (32)
#define AX_MAX_JENC_PIC_WIDTH (32768)

#define AX_MIN_JENC_PIC_HEIGHT (32)
#define AX_MAX_JENC_PIC_HEIGHT (32768)

#define AX_MIN_JENC_ROI_BLOCK_WIDTH  (16)
#define AX_MIN_JENC_ROI_BLOCK_HEIGHT (16)

#define AX_MAX_VENC_USER_DATA_SIZE (2048)
#define AX_MAX_JENC_USER_DATA_SIZE (4096)

#define AX_MIN_JENC_QFACTOR (1)
#define AX_MAX_JENC_QFACTOR (99)

#define AX_MAX_SVC_DELTA_QP  (51)
#define AX_MIN_SVC_DELTA_QP  (-51)
#define AX_MAX_SVC_ABS_QP (51)
#define AX_MIN_SVC_ABS_QP (0)

typedef AX_S32 VENC_CHN;
typedef AX_S32 VENC_GRP;

/*the attribute of h264e*/
typedef struct axVENC_ATTR_H264_T
{
    AX_BOOL bRcnRefShareBuf; /* Range:[0, 1]; Whether to enable the Share Buf of Rcn and Ref .*/
    // reserved
} AX_VENC_ATTR_H264_T;

/*the attribute of h265e*/
typedef struct axVENC_ATTR_H265_T
{
    AX_BOOL bRcnRefShareBuf; /* Range:[0, 1]; Whether to enable the Share Buf of Rcn and Ref .*/
    // reserved
} AX_VENC_ATTR_H265_T;

/* the attribute of the roi */
typedef struct axRECT_T
{
    AX_U32 u32X;
    AX_U32 u32Y;
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_RECT_T;

/* the param of the jpege */
typedef struct axVENC_JPEG_PARAM_T
{
    /* NonROI Attribute when do ROI configuration */
    AX_U32 u32Qfactor;  /* RW; Range:[1,99]; Qfactor value */
    AX_U8 u8YQt[64];    /* RW; Range:[1, 255]; Y quantization table */
    AX_U8 u8CbCrQt[64]; /* RW; Range:[1, 255]; CbCr quantization table */
    /* End of NonROI Attribute */

    /* ROI Attribute (only for ROI)*/
    AX_BOOL bEnableRoi;    /* RW; Range:[0, 1]; 0: Whether want to do ROI configuration */
    AX_BOOL bSaveNonRoiQt; /* RW; Range:[0, 1]; Which quantization table to save between RoiQt and nonRoiQt.*/
    AX_U32 u32RoiQfactor;  /* RW; Range:[1,99]; Qfactor value */
    AX_U8 u8RoiYQt[64];    /* RW; Range:[1, 255]; Y quantization table */
    AX_U8 u8RoiCbCrQt[64]; /* RW; Range:[1, 255]; CbCr quantization table */
    AX_BOOL bEnable[AX_MAX_VENC_ROI_NUM];     /* RW; Range:[0, 1]; Whether to enable this ROI */
    AX_RECT_T stRoiArea[AX_MAX_VENC_ROI_NUM]; /* RW; Region of an ROI*/
    /* End of ROI Attribute */

    /**
     *RW; the max MCU number is (picwidth + 15) >> 4 x (picheight +
     *15) >> 4 x 2]; MCU number of one ECS
     */
    AX_U32 u32MCUPerECS;
    AX_BOOL bDblkEnable; /* JPEG AX_TRUE: enable Deblock;AX_FALSE: disable Deblock. */
} AX_VENC_JPEG_PARAM_T;

/* the param of the mjpege */
typedef struct axVENC_MJPEG_PARAM_T
{
    AX_U8 u8YQt[64];  /* Range:[1, 255]; Y quantization table */
    AX_U8 u8CbQt[64]; /* Range:[1, 255]; Cb quantization table */
    AX_U8 u8CrQt[64]; /* Range:[1, 255]; Cr quantization table */
    /**
     * the max MCU number is
     * (picwidth + 15) >> 4 x (picheight + 15) >> 4 x 2]; MCU number of one ECS
     */
    AX_U32 u32MCUPerECS;
} AX_VENC_MJPEG_PARAM_T;

typedef struct axRES_SIZE_T
{
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_RES_SIZE_T;

/*the size of array is 2,that is the maximum*/
typedef struct axVENC_MPF_CFG_T
{
    AX_U8 u8LargeThumbNailNum;              /* Range:[0,2]; the large thumbnail pic num of the MPF */
    AX_RES_SIZE_T astLargeThumbNailSize[2]; /* The resolution of large ThumbNail*/
} AX_VENC_MPF_CFG_T;

typedef enum
{
    AX_VENC_PIC_RECEIVE_SINGLE = 0, /* single frame mode.*/
    AX_VENC_PIC_RECEIVE_MULTI,      /* multi slice mode */
    AX_VENC_PIC_RECEIVE_BUTT
} AX_VENC_PIC_RECEIVE_MODE_E;

/*the attribute of jpege*/
typedef struct axVENC_ATTR_JPEG_T
{
    //    AX_VENC_PIC_RECEIVE_MODE_E    enReceiveMode;  /* Config the receive mode*/ not support now
} AX_VENC_ATTR_JPEG_T;

/*the attribute of mjpege*/
typedef struct axVENC_ATTR_MJPEG_T
{
    //    AX_VENC_PIC_RECEIVE_MODE_E    enReceiveMode;  /*RW; Config the receive mode*/ not support now
} AX_VENC_ATTR_MJPEG_T;

/* Profile for initialization */
typedef enum
{
    AX_VENC_HEVC_MAIN_PROFILE = 0,
    AX_VENC_HEVC_MAIN_STILL_PICTURE_PROFILE = 1,
    AX_VENC_HEVC_MAIN_10_PROFILE = 2,
    AX_VENC_HEVC_MAINREXT = 3,
    /* H264 Defination*/
    AX_VENC_H264_BASE_PROFILE = 9,
    AX_VENC_H264_MAIN_PROFILE = 10,
    AX_VENC_H264_HIGH_PROFILE = 11,
    AX_VENC_H264_HIGH_10_PROFILE = 12
} AX_VENC_PROFILE_E;

/* Level for initialization */
typedef enum
{
    AX_VENC_HEVC_LEVEL_1 = 30,
    AX_VENC_HEVC_LEVEL_2 = 60,
    AX_VENC_HEVC_LEVEL_2_1 = 63,
    AX_VENC_HEVC_LEVEL_3 = 90,
    AX_VENC_HEVC_LEVEL_3_1 = 93,
    AX_VENC_HEVC_LEVEL_4 = 120,
    AX_VENC_HEVC_LEVEL_4_1 = 123,
    AX_VENC_HEVC_LEVEL_5 = 150,
    AX_VENC_HEVC_LEVEL_5_1 = 153,
    AX_VENC_HEVC_LEVEL_5_2 = 156,
    AX_VENC_HEVC_LEVEL_6 = 180,
    AX_VENC_HEVC_LEVEL_6_1 = 183,
    AX_VENC_HEVC_LEVEL_6_2 = 186,

    /* H264 Defination*/
    AX_VENC_H264_LEVEL_1 = 10,
    AX_VENC_H264_LEVEL_1_b = 99,
    AX_VENC_H264_LEVEL_1_1 = 11,
    AX_VENC_H264_LEVEL_1_2 = 12,
    AX_VENC_H264_LEVEL_1_3 = 13,
    AX_VENC_H264_LEVEL_2 = 20,
    AX_VENC_H264_LEVEL_2_1 = 21,
    AX_VENC_H264_LEVEL_2_2 = 22,
    AX_VENC_H264_LEVEL_3 = 30,
    AX_VENC_H264_LEVEL_3_1 = 31,
    AX_VENC_H264_LEVEL_3_2 = 32,
    AX_VENC_H264_LEVEL_4 = 40,
    AX_VENC_H264_LEVEL_4_1 = 41,
    AX_VENC_H264_LEVEL_4_2 = 42,
    AX_VENC_H264_LEVEL_5 = 50,
    AX_VENC_H264_LEVEL_5_1 = 51,
    AX_VENC_H264_LEVEL_5_2 = 52,
    AX_VENC_H264_LEVEL_6 = 60,
    AX_VENC_H264_LEVEL_6_1 = 61,
    AX_VENC_H264_LEVEL_6_2 = 62
} AX_VENC_LEVEL_E;

/* Tier for initialization */
typedef enum
{
    AX_VENC_HEVC_MAIN_TIER = 0,
    AX_VENC_HEVC_HIGH_TIER = 1,
} AX_VENC_TIER_E;

typedef enum
{
    AX_VENC_DUMP_NONE = 0,
    AX_VENC_DUMP_FRAME = 1,
    AX_VENC_DUMP_STREAM = 2,
    AX_VENC_DUMP_FRAME_STREAM = 3, /* dump both frame and stream */
} AX_VENC_DUMP_TYPE_E;

typedef enum
{
    /* enable h264/hevc encoder */
    AX_VENC_VIDEO_ENCODER = 1,
    /* enable jpeg/mjpeg encoder */
    AX_VENC_JPEG_ENCODER = 2,
    /* enable h264/h265/jpeg/mjpeg encoder */
    AX_VENC_MULTI_ENCODER = 3
} AX_VENC_ENCODER_TYPE_E;

typedef enum
{
    AX_VENC_SCHED_OTHER = 0, /* default linux time-sharing scheduling */
    AX_VENC_SCHED_FIFO = 3,  /* First in-first out scheduling */
    AX_VENC_SCHED_RR = 4,    /* Round-robin scheduling */
    AX_VENC_SCHED_BUTT
} AX_VENC_THREAD_SCHED_POLICY_E;

typedef struct axVENC_ENCODE_THREAD_ATTR_T
{
    /**
     * Range:[0, 1]; whether take scheduling attributes from the values specified by the attributes object or not,
     * only true user could change thread policy and priority
     * 0: inherit scheduling attributes from the creating thread,
     * 1: specified by attributes object.
     */
    AX_BOOL bExplicitSched;
    AX_VENC_THREAD_SCHED_POLICY_E enSchedPolicy; /* encode thread sched policy.*/
    AX_U32 u32SchedPriority;                     /* Range:[1, 99]; encode thread scheduling priority.*/
    AX_U32 u32TotalThreadNum;                    /* Range:[1, 9]; thread number of encoding all venc channels.*/
} AX_VENC_ENCODE_THREAD_ATTR_T;

typedef struct axVENC_MOD_ATTR_T
{
    AX_VENC_ENCODER_TYPE_E enVencType;
    AX_VENC_ENCODE_THREAD_ATTR_T stModThdAttr;
} AX_VENC_MOD_ATTR_T;

typedef struct axVENC_USR_DATA_T
{
    AX_BOOL bEnable;
    AX_U8 *pu8UsrData;
    AX_U32 u32DataSize;
} AX_VENC_USR_DATA_T;

typedef struct axVENC_RECT_T
{
    AX_S32 s32X;
    AX_S32 s32Y;
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_VENC_RECT_T;

typedef struct axVENC_CROP_INFO_T
{
    AX_BOOL bEnable;
    AX_VENC_RECT_T stRect;
} AX_VENC_CROP_INFO_T;

/* the attribute of the venc*/
typedef struct axVENC_ATTR_T
{
    AX_PAYLOAD_TYPE_E enType; /* the type of payload */

    /**
     * VENC: Max input image width [5120], in pixel
     * JENC: Max input image width [32768], in pixel
     */
    AX_U32 u32MaxPicWidth;
    /**
     * VENC: Max input image height [5120], in pixel
     * JENC: Max input image height [32768], in pixel
     */
    AX_U32 u32MaxPicHeight;

    AX_MEMORY_SOURCE_E enMemSource; /* memory source of stream buffer */
    AX_U32 u32BufSize;              /* stream buffer size*/
    /**
     * H.264: 9: Baseline; 10: Main; 11: High; 12: High 10;
     * H.265: 0: Main; 1: Main Still Picture; 2: Main 10;
     */
    AX_VENC_PROFILE_E enProfile;
    AX_VENC_LEVEL_E enLevel; /* HEVC level: 180 = level 6.0 * 30; H264 level: 51 = Level 5.1 */
    AX_VENC_TIER_E enTier;   /* HEVC: 0: Main tier、 1: High tier */
    /**
     * VENC: Range:H264:[144, 5120] H265:[136, 5120]; width of source image,must be even, in pixel
     * JENC: Range:[32, 32768]; width of source image,must be even, in pixel
     */
    AX_U32 u32PicWidthSrc;
    /**
     * VENC: Range:H264:[144, 5120] H265:[136, 5120]; height of source image,must be even, in pixel
     * JENC: Range:[32, 32768]; height of source image,must be even, in pixel
     */
    AX_U32 u32PicHeightSrc;

    AX_VENC_CROP_INFO_T stCropCfg; /* channel crop config */

    AX_ROTATION_E enRotation; /* rotation config */

    AX_LINK_MODE_E enLinkMode;
    AX_BOOL bDeBreathEffect; /* AX_FALSE: turn off de breathing AX_TRUE: turn on de breathing*/
    AX_BOOL bRefRingbuf; /* AX_FALSE: turn off refbuffer AX_TRUE: turn on refbuffer */

    /**
     * whether flush output queue immediately when try to destroy channel,
     * -1: wait until output queue empty,
     * 0: not wait (default),
     * >0: wait some time, in millisecond
     */
    AX_S32 s32StopWaitTime;

    AX_U8 u8InFifoDepth;  /* depth of input fifo */
    AX_U8 u8OutFifoDepth; /* depth of output fifo */

    AX_U32 u32SliceNum; /* RW; how many slices one frame will be cut, only support jpeg unlink-mode */
    union
    {
        AX_VENC_ATTR_H264_T stAttrH264e;   /* attributes of H264e */
        AX_VENC_ATTR_H265_T stAttrH265e;   /* attributes of H265e */
        AX_VENC_ATTR_MJPEG_T stAttrMjpege; /* attributes of Mjpeg */
        AX_VENC_ATTR_JPEG_T stAttrJpege;   /* attributes of jpeg  */
    };
} AX_VENC_ATTR_T;

/* the gop mode */
typedef enum axVENC_GOP_MODE_E
{
    AX_VENC_GOPMODE_NORMALP = 0, /* NORMALP */
    AX_VENC_GOPMODE_ONELTR = 1,  /* ONELTR */
    AX_VENC_GOPMODE_SVC_T = 2,   /* SVC-T */

    AX_VENC_GOPMODE_BUTT
} AX_VENC_GOP_MODE_E;

/**
 * QPFactor (quality preference) will be used in rate distorition optimization, higher value
 * mean lower quality and less bits. Typical suggested range is between 0.3 and 1
 */
typedef struct ax_VENC_GOP_PIC_CONFIG_T
{
    AX_S32 s32QpOffset; /*  QP offset will be added to the QP parameter to set the final QP */
    AX_F32 f32QpFactor;
} AX_VENC_GOP_PIC_CONFIG_T;

/**
 * QPFactor (quality preference) will be used in rate distorition optimization, higher value
 * mean lower quality and less bits. Typical suggested range is between 0.3 and 1
 */
typedef struct ax_VENC_GOP_PIC_SPECIAL_CONFIG_T
{
    AX_S32 s32QpOffset; /*  QP offset will be added to the QP parameter to set the final QP */
    AX_F32 f32QpFactor;
    /**
     * interval between two pictures using LTR as reference picture or
     * interval between two pictures coded as special frame
     */
    AX_S32 s32Interval;
} AX_VENC_GOP_PIC_SPECIAL_CONFIG_T;

/* the attribute of the normalp*/
typedef struct axVENC_GOP_NORMALP_T
{
    AX_VENC_GOP_PIC_CONFIG_T stPicConfig; /* normal P frame config */
} AX_VENC_GOP_NORMALP_T;

/* the attribute of the one long-term reference frame */
typedef struct axVENC_GOP_ONE_LTR_T
{
    AX_VENC_GOP_PIC_CONFIG_T stPicConfig;                /* normal P frame config */
    AX_VENC_GOP_PIC_SPECIAL_CONFIG_T stPicSpecialConfig; /* one long-term reference frame config */
} AX_VENC_GOP_ONE_LTR_T;

/* the attribute of the one long-term reference frame */
typedef struct axVENC_GOP_SVC_T_T
{
    AX_CHAR **s8SvcTCfg;
    AX_U32 u32GopSize;
} AX_VENC_GOP_SVC_T_T;

/* the attribute of the gop*/
typedef struct axVENC_GOP_ATTR_T
{
    AX_VENC_GOP_MODE_E enGopMode; /* Encoding GOP type */
    union
    {
        AX_VENC_GOP_NORMALP_T stNormalP; /*attributes of normal P*/
        AX_VENC_GOP_ONE_LTR_T stOneLTR;  /*attributes of one long-term reference frame */
        AX_VENC_GOP_SVC_T_T stSvcT;      /*attributes of svc-t */
    };

} AX_VENC_GOP_ATTR_T;

/* the attribute of the venc chnl*/
typedef struct axVENC_CHN_ATTR_T
{
    AX_VENC_ATTR_T stVencAttr;    /*the attribute of video encoder channel */
    AX_VENC_RC_ATTR_T stRcAttr;   /*the attribute of rate  ctrl */
    AX_VENC_GOP_ATTR_T stGopAttr; /*the attribute of gop */
} AX_VENC_CHN_ATTR_T;

/* the param of vui */
typedef struct axVENC_VUI_ASPECT_RATIO_T
{
    AX_U8 aspect_ratio_info_present_flag; /* RW; Range:[0,1]; If 1, aspectratio info belows will be encoded into vui */
    AX_U8 aspect_ratio_idc;               /* RW; Range:[0,255]; 17~254 is reserved,see the protocol for the meaning.*/
    AX_U8 overscan_info_present_flag;     /* RW; Range:[0,1]; If 1, oversacan info belows will be encoded into vui.*/
    AX_U8 overscan_appropriate_flag;      /* RW; Range:[0,1]; see the protocol for the meaning. */
    AX_U16 sar_width;                     /* RW; Range:(0, 65535]; see the protocol for the meaning. */
    AX_U16 sar_height;                    /* RW; Range:(0, 65535]; see the protocol for the meaning. */
} AX_VENC_VUI_ASPECT_RATIO_T;

typedef struct axVENC_VUI_TIME_INFO_T
{
    AX_U8 timing_info_present_flag; /* RW; Range:[0,1]; If 1, timing info belows will be encoded into vui.*/
    AX_U32 num_units_in_tick;       /* RW; Range:(0,4294967295]; see the H.264/H.265 protocol for the meaning */
    AX_U32 time_scale;              /* RW; Range:(0,4294967295]; see the H.264/H.265 protocol for the meaning */

    AX_U8 fixed_frame_rate_flag;          /* RW; Range:[0,1]; see the H.264 protocol for the meaning. */
    AX_U32 num_ticks_poc_diff_one_minus1; /* RW; Range:(0,4294967294]; see the H.265 protocol for the meaning */

} AX_VENC_VUI_TIME_INFO_T;

typedef struct axVENC_VIDEO_SIGNAL_T
{
    AX_U8 video_signal_type_present_flag; /* RW; Range:[0,1]; If 1, video singnal info will be encoded into vui. */
    AX_U8 video_format;          /* RW; H.264e Range:[0,7], H.265e Range:[0,5]; see the protocol for the meaning. */
    AX_U8 video_full_range_flag; /* RW; Range: {0,1}; see the protocol for the meaning.*/
    AX_U8 colour_description_present_flag; /* RO; Range: {0,1}; see the protocol for the meaning.*/
    AX_U8 colour_primaries;                /* RO; Range: [0,255]; see the protocol for the meaning. */
    AX_U8 transfer_characteristics;        /* RO; Range: [0,255]; see the protocol for the meaning. */
    AX_U8 matrix_coefficients;             /* RO; Range:[0,255]; see the protocol for the meaning. */
} AX_VENC_VUI_VIDEO_SIGNAL_T;

typedef struct axVENC_VUI_BITSTREAM_RESTRIC_T
{
    AX_U8 bitstream_restriction_flag; /* RW; Range: {0,1}; see the protocol for the meaning.*/
} AX_VENC_VUI_BITSTREAM_RESTRIC_T;

typedef struct axVENC_VUI_PARAM_T
{
    AX_VENC_VUI_ASPECT_RATIO_T stVuiAspectRatio;
    AX_VENC_VUI_TIME_INFO_T stVuiTimeInfo;
    AX_VENC_VUI_VIDEO_SIGNAL_T stVuiVideoSignal;
    AX_VENC_VUI_BITSTREAM_RESTRIC_T stVuiBitstreamRestric;
} AX_VENC_VUI_PARAM_T;

/*the nalu type of H264E*/
typedef enum
{
    AX_H264E_NALU_BSLICE = 0,   /*B SLICE types*/
    AX_H264E_NALU_PSLICE = 1,   /*P SLICE types*/
    AX_H264E_NALU_ISLICE = 2,   /*I SLICE types*/
    AX_H264E_NALU_IDRSLICE = 5, /*IDR SLICE types*/
    AX_H264E_NALU_SEI = 6,      /*SEI types*/
    AX_H264E_NALU_SPS = 7,      /*SPS types*/
    AX_H264E_NALU_PPS = 8,      /*PPS types*/

    AX_H264E_NALU_PREFIX_14 = 14, /*Prefix NAL unit */

    AX_H264E_NALU_BUTT
} AX_H264E_NALU_TYPE_E;

/*the nalu type of H265E*/
typedef enum
{
    AX_H265E_NALU_BSLICE = 0, /*B SLICE types*/
    AX_H265E_NALU_PSLICE = 1, /*P SLICE types*/
    AX_H265E_NALU_ISLICE = 2, /*I SLICE types*/

    AX_H265E_NALU_TSA_R = 3,

    AX_H265E_NALU_IDRSLICE = 19, /*IDR SLICE types*/
    AX_H265E_NALU_VPS = 32,      /*VPS types*/
    AX_H265E_NALU_SPS = 33,      /*SPS types*/
    AX_H265E_NALU_PPS = 34,      /*PPS types*/
    AX_H265E_NALU_SEI = 39,      /*SEI types*/

    AX_H265E_NALU_BUTT
} AX_H265E_NALU_TYPE_E;

/* Picture type for encoding */
typedef enum
{
    AX_VENC_INTRA_FRAME = 0,           /* I Frame */
    AX_VENC_PREDICTED_FRAME = 1,       /* P Frame */
    AX_VENC_BIDIR_PREDICTED_FRAME = 2, /* B Frame */
    AX_VENC_VIRTUAL_INTRA_FRAME = 3,   /* virtual I frame */
    AX_VENC_NOTCODED_FRAME             /* Used just as a return value */
} AX_VENC_PICTURE_CODING_TYPE_E;

/*the pack type of JPEGE*/
typedef enum
{
    AX_JPEGE_PACK_ECS = 5,      /*ECS types*/
    AX_JPEGE_PACK_APP = 6,      /*APP types*/
    AX_JPEGE_PACK_VDO = 7,      /*VDO types*/
    AX_JPEGE_PACK_PIC = 8,      /*PIC types*/
    AX_JPEGE_PACK_DCF = 9,      /*DCF types*/
    AX_JPEGE_PACK_DCF_PIC = 10, /*DCF PIC types*/
    AX_JPEGE_PACK_BUTT
} AX_JPEGE_PACK_TYPE_E;

/*the data type of VENC*/
typedef union
{
    AX_H264E_NALU_TYPE_E enH264EType; /* R; H264E NALU types*/
    AX_JPEGE_PACK_TYPE_E enJPEGEType; /* R; JPEGE pack types*/
    AX_H265E_NALU_TYPE_E enH265EType; /* R; H264E NALU types*/
} AX_VENC_DATA_TYPE_U;

/*the pack info of VENC*/
typedef struct axVENC_NALU_INFO_T
{
    AX_VENC_DATA_TYPE_U unNaluType; /* R; the nalu type*/
    AX_U32 u32NaluOffset;
    AX_U32 u32NaluLength;
} AX_VENC_NALU_INFO_T;

typedef struct axCHN_STREAM_STATUS_T
{
    AX_U32 u32TotalChnNum;                 /* Range:[0, AX_MAX_VENC_CHN_NUM], how many channels have stream. */
    AX_U32 au32ChnIndex[AX_MAX_VENC_CHN_NUM]; /* the channel id set of venc channel that has stream */
    AX_PAYLOAD_TYPE_E aenChnCodecType[AX_MAX_VENC_CHN_NUM]; /* channel payload type */
} AX_CHN_STREAM_STATUS_T;

typedef struct axVENC_SELECT_GRP_PARAM_T
{
    AX_U16 u16TotalChnNum;                /* Range:[0, AX_MAX_VENC_CHN_NUM), how many channels in grp. */
    AX_U16 u16ChnInGrp[AX_MAX_VENC_CHN_NUM]; /* the channel id set of group */
} AX_VENC_SELECT_GRP_PARAM_T;

/*Defines a stream packet*/
typedef struct axVENC_PACK_T
{
    AX_U64 ulPhyAddr; /* the physics address of stream */
    AX_U8 *pu8Addr;   /* the virtual address of stream */
    AX_U32 u32Len;    /* the length of stream */

    AX_U64 u64PTS;    /* PTS */
    AX_U64 u64SeqNum; /* sequence number of input frame */
    AX_U64 u64UserData;

    AX_PAYLOAD_TYPE_E enType;                   /* the type of payload*/
    AX_VENC_PICTURE_CODING_TYPE_E enCodingType; /* stream type */
    AX_U32 u32TemporalID;                       /* svc-t, layer id*/

    AX_U32 u32NaluNum;                                 /* the stream nalus num */
    AX_VENC_NALU_INFO_T stNaluInfo[AX_MAX_VENC_NALU_NUM]; /* the stream nalu Information */
} AX_VENC_PACK_T;

/*Defines the frame type and reference attributes of the H.264 frame skipping reference streams*/
typedef enum
{
    AX_BASE_IDRSLICE = 0,    /* the Idr frame at Base layer*/
    AX_BASE_PSLICE_REFTOIDR, /* the P frame at Base layer, referenced by other frames at Base layer and reference to Idr
                             frame*/
    AX_BASE_PSLICE_REFBYBASE,       /* the P frame at Base layer, referenced by other frames at Base layer*/
    AX_BASE_PSLICE_REFBYENHANCE,    /* the P frame at Base layer, referenced by other frames at Enhance layer*/
    AX_ENHANCE_PSLICE_REFBYENHANCE, /* the P frame at Enhance layer, referenced by other frames at Enhance layer*/
    AX_ENHANCE_PSLICE_NOTFORREF,    /* the P frame at Enhance layer ,not referenced*/
    AX_ENHANCE_PSLICE_BUTT
} AX_H264E_REF_TYPE_E;

typedef AX_H264E_REF_TYPE_E AX_H265E_REF_TYPE_E;

/*Defines the features of an H.264 stream*/
typedef struct axVENC_STREAM_INFO_H264_T
{
    AX_U32 u32PicBytesNum;     /* the coded picture stream byte number */
    AX_U32 u32Inter16x16MbNum; /* the inter16x16 macroblock num */
    AX_U32 u32Inter8x8MbNum;   /* the inter8x8 macroblock num */
    AX_U32 u32Intra16MbNum;    /* the intra16x16 macroblock num */
    AX_U32 u32Intra8MbNum;     /* the intra8x8 macroblock num */
    AX_U32 u32Intra4MbNum;     /* the inter4x4 macroblock num */

    /* Type of encoded frames in advanced frame skipping reference mode*/
    AX_H264E_REF_TYPE_E enRefType;
    /* Number of times that channel attributes or parameters (including RC parameters) are set*/
    AX_U32 u32UpdateAttrCnt;
    AX_U32 u32StartQp; /* the start Qp of encoded frames*/
    AX_U32 u32MeanQp;  /* the mean Qp of encoded frames*/
    AX_BOOL bPSkip;
} AX_VENC_STREAM_INFO_H264_T;

/*Defines the features of an H.265 stream*/
typedef struct axVENC_STREAM_INFO_H265_T
{
    AX_U32 u32PicBytesNum;     /* the coded picture stream byte number */
    AX_U32 u32Inter64x64CuNum; /* the inter64x64 cu num  */
    AX_U32 u32Inter32x32CuNum; /* the inter32x32 cu num  */
    AX_U32 u32Inter16x16CuNum; /* the inter16x16 cu num  */
    AX_U32 u32Inter8x8CuNum;   /* the inter8x8   cu num  */
    AX_U32 u32Intra32x32CuNum; /* the Intra32x32 cu num  */
    AX_U32 u32Intra16x16CuNum; /* the Intra16x16 cu num  */
    AX_U32 u32Intra8x8CuNum;   /* the Intra8x8   cu num  */
    AX_U32 u32Intra4x4CuNum;   /* the Intra4x4   cu num  */

    /* Type of encoded frames in advanced frame skipping reference mode*/
    AX_H265E_REF_TYPE_E enRefType;
    /* Number of times that channel attributes or parameters (including RC parameters) are set*/
    AX_U32 u32UpdateAttrCnt;
    AX_U32 u32StartQp; /* the start Qp of encoded frames*/
    AX_U32 u32MeanQp;  /* the mean Qp of encoded frames*/
    AX_BOOL bPSkip;
} AX_VENC_STREAM_INFO_H265_T;

/* the sse info*/
typedef struct axVENC_SSE_INFO_T
{
    AX_BOOL bSSEEn;   /* Range:[0,1]; Region SSE enable */
    AX_U32 u32SSEVal; /* Region SSE value */
} AX_VENC_SSE_INFO_T;

/* the advance information of the h264e */
typedef struct axVENC_STREAM_ADVANCE_INFO_H264_T
{
    AX_U32 u32ResidualBitNum;               /* the residual num */
    AX_U32 u32HeadBitNum;                   /* the head bit num */
    AX_U32 u32MadiVal;                      /* the madi value */
    AX_U32 u32MadpVal;                      /* the madp value */
    AX_F64 f64PSNRVal;                      /* the PSNR value */
    AX_U32 u32MseLcuCnt;                    /* the lcu cnt of the mse */
    AX_U32 u32MseSum;                       /* the sum of the mse */
    AX_VENC_SSE_INFO_T stSSEInfo[8];        /* the information of the sse */
    AX_U32 u32QpHstgrm[AX_VENC_QP_HISGRM_NUM]; /* the Qp histogram value */
    AX_U32 u32MoveScene16x16Num;            /* the 16x16 cu num of the move scene*/
    AX_U32 u32MoveSceneBits;                /* the stream bit num of the move scene */
} AX_VENC_STREAM_ADVANCE_INFO_H264_T;

/* the advance information of the h265e */
typedef struct axVENC_STREAM_ADVANCE_INFO_H265_T
{
    AX_U32 u32ResidualBitNum;               /* the residual num */
    AX_U32 u32HeadBitNum;                   /* the head bit num */
    AX_U32 u32MadiVal;                      /* the madi value */
    AX_U32 u32MadpVal;                      /* the madp value */
    AX_F64 f64PSNRVal;                      /* the PSNR value */
    AX_U32 u32MseLcuCnt;                    /* the lcu cnt of the mse */
    AX_U32 u32MseSum;                       /* the sum of the mse */
    AX_VENC_SSE_INFO_T stSSEInfo[8];        /* the information of the sse */
    AX_U32 u32QpHstgrm[AX_VENC_QP_HISGRM_NUM]; /* the Qp histogram value */
    AX_U32 u32MoveScene32x32Num;            /* the 32x32 cu num of the move scene*/
    AX_U32 u32MoveSceneBits;                /* the stream bit num of the move scene */
} AX_VENC_STREAM_ADVANCE_INFO_H265_T;

/*Defines the features of an jpege stream*/
typedef struct axVENC_STREAM_INFO_JPEG_T
{
    AX_U32 u32PicBytesNum;   /* the coded picture stream byte number */
    AX_U32 u32UpdateAttrCnt; /* Number of times that channel attributes or parameters
                              * (including RC parameters) are set */
    AX_U32 u32Qfactor;       /* image quality */
} AX_VENC_STREAM_INFO_JPEG_T;

/* the advance information of the Jpege */
typedef struct axVENC_STREAM_ADVANCE_INFO_JPEG_T
{
    // AX_U32 u32Reserved;
} AX_VENC_STREAM_ADVANCE_INFO_JPEG_T;

/*Defines the features of an strAX_AX_*/
typedef struct axVENC_STREAM_T
{
    AX_VENC_PACK_T stPack; /* stream pack attribute*/

    union
    {
        AX_VENC_STREAM_INFO_H264_T stH264Info; /* the stream info of h264*/
        AX_VENC_STREAM_INFO_JPEG_T stJpegInfo; /* the stream info of jpeg*/
        AX_VENC_STREAM_INFO_H265_T stH265Info; /* the stream info of h265*/
    };

    union
    {
        AX_VENC_STREAM_ADVANCE_INFO_H264_T stAdvanceH264Info; /* the stream info of h264*/
        AX_VENC_STREAM_ADVANCE_INFO_JPEG_T stAdvanceJpegInfo; /* the stream info of jpeg*/
        AX_VENC_STREAM_ADVANCE_INFO_H265_T stAdvanceH265Info; /* the stream info of h265*/
    };
} AX_VENC_STREAM_T;

/* the param of receive picture */
typedef struct axVENC_RECV_PIC_PARAM_T
{
    /**
     * RW; Range:[-1, 2147483647]; Number of frames received and encoded by the encoding channel,
     * 0 is not supported
     */
    AX_S32 s32RecvPicNum;
} AX_VENC_RECV_PIC_PARAM_T;

typedef struct axVENC_STREAM_BUF_INFO_T
{
    AX_U64 u64PhyAddr;
    AX_VOID *pUserAddr;
    AX_U32 u32BufSize;
} AX_VENC_STREAM_BUF_INFO_T;

typedef struct axVENC_ROI_ATTR_T
{
    AX_U32 u32Index;     /* RW; Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    AX_BOOL bEnable;     /* RW; Range:[0, 1]; Whether to enable this ROI */
    AX_BOOL bAbsQp;      /* RW; Range:[0, 1]; QP mode of an ROI. 0: relative QP. 1: absolute QP. (only for venc)*/
    AX_S32 s32RoiQp;     /* RW; Range: [-51, 51] when bAbsQp==0; [0, 51] when bAbsQp==1;  (only for venc)*/
    AX_RECT_T stRoiArea; /* RW; Region of an ROI*/
} AX_VENC_ROI_ATTR_T;

/* ROI struct */
typedef struct axVENC_ROI_ATTR_EX_T
{
    AX_U32 u32Index;     /* Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    AX_BOOL bEnable[3];  /* Range:[0, 1]; Subscript of array   0: I Frame; 1: P/B Frame; 2: VI Frame; other params are
                            the same. */
    AX_BOOL bAbsQp[3];   /* Range:[0, 1]; QP mode of an ROI.AX_FALSE: relative QP.AX_TURE: absolute QP.*/
    AX_S32 s32Qp[3];     /* Range:[-51, 51]; QP value,only relative mode can QP value less than 0. */
    AX_RECT_T stRect[3]; /* Region of an ROI*/
} AX_VENC_ROI_ATTR_EX_T;

typedef struct axJPEG_ROI_ATTR_T
{
    AX_U32 u32Index;     /* RW; Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    AX_BOOL bEnable;     /* RW; Range:[0, 1]; Whether to enable this ROI */
    AX_RECT_T stRoiArea; /* RW; Region of an ROI*/
} AX_JPEG_ROI_ATTR_T;

/* osd config */
typedef struct axVENC_OSD_ATTR_T
{
    AX_BOOL bEnable[AX_MAX_VENC_OSD_NUM]; /* RW; AX_TRUE: enable this OSD ; AX_FALSE: disable this OSD */
    AX_OSD_BMP_ATTR_T stOsdBmpAttr[AX_MAX_VENC_OSD_NUM]; /* RW; OSD configuration */
} AX_VENC_OSD_ATTR_T;

/* VENC CHANNEL STATUS struct */
typedef struct axVENC_CHN_STATUS_T
{
    AX_U32 u32LeftPics;         /* Number of frames yet to encode (until fifo empty) */
    AX_U32 u32LeftStreamBytes;  /* Number of bytes remaining in the bitstream buffer */
    AX_U32 u32LeftStreamFrames; /* Number of frames remaining in the bitstream buffer */
    AX_U32 u32CurPacks;         /* Number of current stream packets. not support now */
    AX_U32 u32LeftRecvPics;     /* Number of frames yet to recieve (total number specified at start). not support now */
    AX_U32 u32LeftEncPics;      /* Number of frames yet to encode (total number specified at start). not support now */
    AX_U32 u32Reserved;         /* Reserved */
} AX_VENC_CHN_STATUS_T;

/* the information of the user rc*/
typedef struct axUSER_RC_INFO_T
{
    AX_BOOL bQpMapValid;         /* Range:[0,1]; Indicates whether the QpMap mode is valid for the current frame*/
    AX_BOOL bIPCMMapValid;       /* Range:[0,1]; Indicates whether the IpcmMap mode is valid for the current frame*/
    AX_U32 u32BlkStartQp;        /* Range:[0,51];QP value of the first 16 x 16 block in QpMap mode */
    AX_U64 u64QpMapPhyAddr;      /* Physical address of the QP table in QpMap mode*/
    AX_S8 *pQpMapVirAddr;        /* virtaul address of the qpMap */
    AX_U64 u64IpcmMapPhyAddr;    /* Physical address of the IPCM table in QpMap mode*/
    AX_FRAME_TYPE_E enFrameType; /* Encoding frame type of the current frame */
    AX_U32 u32RoiMapDeltaSize;   /* size of QpDelta map (per frame) */
} AX_USER_RC_INFO_T;

/* the information of the user frame*/
typedef struct axUSER_FRAME_INFO_T
{
    AX_VIDEO_FRAME_INFO_T stUserFrame;
    AX_USER_RC_INFO_T stUserRcInfo;
} AX_USER_FRAME_INFO_T;

typedef enum
{
    AX_ERR_ENC_CREATE_CHAN_ERR = 0x80,   /* create encoder channel failed */
    AX_ERR_ENC_SET_PRIORITY_FAIL = 0x81, /* set encoder thread priority failed */
    AX_ERR_ENC_OUTBUF_OVERFLOW = 0x82, /* out buffer overflow */
    AX_ERR_ENC_HW_BUS_ERROR = 0x83, /* hardware bus error */
    AX_ERR_ENC_HW_TIMEOUT = 0x84, /* hardware timeout */
} AX_ENC_ERR_CODE_E;

typedef enum
{
    AX_STREAM_BUF_NON_CACHE = 0,
    AX_STREAM_BUF_CACHE = 1,
} AX_VENC_STREAM_BUF_TYPE_E;

typedef struct axJPEG_ENCODE_ONCE_PARAMS_T
{
    /* frame info, all IN */
    AX_U32 u32Width;
    AX_U32 u32Height;
    AX_IMG_FORMAT_E enImgFormat;
    AX_U32 u32PicStride[3];
    AX_U64 u64PhyAddr[3]; /* IN; frame physics address */
    AX_U64 u64VirAddr[3]; /* IN; frame virtual address */
    AX_U32 u32OutBufSize; /* IN; the size of stream buffer alloc by user */
    /* for crop */
    AX_S16 s16CropX;
    AX_S16 s16CropY;
    AX_S16 s16CropWidth;
    AX_S16 s16CropHeight;

    /* for rotation */
    AX_ROTATION_E enRotation; /* IN; rotation config */

    /* for thumbnail */
    AX_BOOL bThumbEnable;  /* IN; AX_FALSE: disable thumbnail encode; AX_TURE: enable thumbnail encode. */
    AX_U32 u32ThumbWidth;  /* IN; thumbnail width */
    AX_U32 u32ThumbHeight; /* IN; thumbnail height */

    /* definition for frame buffer compression */
    AX_FRAME_COMPRESS_INFO_T stCompressInfo; /* IN; frame buffer compression */

    /* stream info */
    AX_U64 ulPhyAddr; /* OUT; the physics address of stream */
    AX_U8 *pu8Addr;   /* OUT; the virtual address of stream */
    AX_U32 u32Len;    /* OUT; the length of stream */
    AX_VENC_STREAM_BUF_TYPE_E enStrmBufType; /* stream buffer type. */

    /* qFactor and qptable */
    AX_VENC_JPEG_PARAM_T stJpegParam;
} AX_JPEG_ENCODE_ONCE_PARAMS_T;

typedef enum
{
    AX_VENC_INTRA_REFRESH_ROW = 0,
    AX_VENC_INTRA_REFRESH_COLUMN,
    AX_VENC_INTRA_REFRESH_BUTT
} AX_VENC_INTRA_REFRESH_MODE_E;

typedef struct axVENC_INTRA_REFRESH_T
{
    AX_BOOL bRefresh;
    AX_U32 u32RefreshNum; /* Range:[1, gopLen]; how many frames it will take to do GDR */
    AX_U32 u32ReqIQp;
    AX_VENC_INTRA_REFRESH_MODE_E enIntraRefreshMode;
} AX_VENC_INTRA_REFRESH_T;

typedef struct axVENC_SLICE_SPLIT_T
{
    AX_BOOL bSplit;
    /**
     * [1, align_up(picHeight)/BLK_SIZE]: a slice should contain how many MCU/MB/CTU lines
     */
    AX_U32 u32LcuLineNum;
} AX_VENC_SLICE_SPLIT_T;

#define AX_VENC_SVC_MAX_RECT_NUM (32)

typedef enum {
    AX_VENC_SVC_RECT_TYPE0 = 0,
    AX_VENC_SVC_RECT_TYPE1,
    AX_VENC_SVC_RECT_TYPE2,
    AX_VENC_SVC_RECT_TYPE3,
    AX_VENC_SVC_RECT_TYPE4,
    AX_VENC_SVC_RECT_TYPE_BUTT
} AX_VENC_SVC_RECT_TYPE_E;

typedef struct axRECT_F
{
    float fX;      /* Range: [0.0, 1.0) */
    float fY;      /* Range: [0.0, 1.0) */
    float fWidth;  /* Range: (0.0, 1.0] */
    float fHeight; /* Range: (0.0, 1.0] */
} AX_VENC_RECT_F_T;

typedef struct {
    AX_S8 iQp;   /* qp range: relative deltaQp: [-51, 51] absolute absQp: [0, 51] */
    AX_S8 pQp;   /* qp range: relative deltaQp: [-51, 51] absolute absQp: [0, 51] */
} AX_VENC_SVC_MAP_PARAM_T;

typedef struct axVENC_SVC_PARAM_T
{
    AX_BOOL bAbsQp;          /* RW; Range:[0, 1]; QP mode of svc region. 0: relative QP. 1: absolute QP. (only for venc)*/
    AX_BOOL bSync;
    AX_U32 u32RectTypeNum;   /* rect type num in stQpCfg, [1, AX_VENC_SVC_RECT_TYPE_BUTT] */
    AX_VENC_SVC_MAP_PARAM_T  stQpCfg[AX_VENC_SVC_RECT_TYPE_BUTT];
    AX_VENC_SVC_MAP_PARAM_T  stBgQpCfg; /* sudjest >= 0 */
} AX_VENC_SVC_PARAM_T;

typedef struct axVENC_SVC_REGION_T
{
    AX_U32 u32RectNum;
    AX_VENC_RECT_F_T stRect[AX_VENC_SVC_MAX_RECT_NUM];
    AX_VENC_SVC_RECT_TYPE_E enRectType[AX_VENC_SVC_MAX_RECT_NUM];
    AX_U64 u64Pts;
} AX_VENC_SVC_REGION_T;

#define AX_ID_VENC_COMMON (0x02)

/* video encoder error code */
#define AX_ERR_VENC_CREATE_CHAN_ERR   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_CREATE_CHAN_ERR)
#define AX_ERR_VENC_SET_PRIORITY_FAIL AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_SET_PRIORITY_FAIL)

#define AX_ERR_VENC_NULL_PTR      AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NULL_PTR)
#define AX_ERR_VENC_ILLEGAL_PARAM AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_VENC_BAD_ADDR      AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BAD_ADDR)

#define AX_ERR_VENC_NOT_SUPPORT AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_SUPPORT)

#define AX_ERR_VENC_NOT_INIT AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_INIT)

#define AX_ERR_VENC_BUF_EMPTY   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BUF_EMPTY)
#define AX_ERR_VENC_BUF_FULL    AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BUF_FULL)
#define AX_ERR_VENC_QUEUE_EMPTY AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_VENC_QUEUE_FULL  AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_QUEUE_FULL)

#define AX_ERR_VENC_EXIST   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_EXIST)
#define AX_ERR_VENC_UNEXIST AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_UNEXIST)

#define AX_ERR_VENC_NOT_PERMIT AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_PERM)
#define AX_ERR_VENC_UNKNOWN    AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_UNKNOWN)
#define AX_ERR_VENC_TIMEOUT    AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_TIMED_OUT)
#define AX_ERR_VENC_FLOW_END   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_FLOW_END)
#define AX_ERR_VENC_BUSY       AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BUSY)

#define AX_ERR_VENC_ATTR_NOT_CFG AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_CONFIG)

#define AX_ERR_VENC_SYS_NOTREADY  AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_SYS_NOTREADY)
#define AX_ERR_VENC_INVALID_CHNID AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_INVALID_CHNID)
#define AX_ERR_VENC_NOMEM         AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOMEM)
#define AX_ERR_VENC_NOT_MATCH     AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_MATCH)

#define AX_ERR_VENC_INVALID_GRPID AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_INVALID_GRPID)

#define AX_ERR_VENC_OUTBUF_OVERFLOW AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_OUTBUF_OVERFLOW)
#define AX_ERR_VENC_HW_BUS_ERROR    AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_HW_BUS_ERROR)
#define AX_ERR_VENC_HW_TIMEOUT      AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_HW_TIMEOUT)

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of __AX_VENC_COMM_H__ */

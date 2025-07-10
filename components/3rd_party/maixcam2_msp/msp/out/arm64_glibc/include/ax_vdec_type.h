/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_VDEC_TYPE_H_
#define _AX_VDEC_TYPE_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_pool_type.h"
#include "ax_sys_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_VDEC_MAX_GRP_NUM         16

#define AX_VDEC_MAX_WIDTH           1920
#define AX_VDEC_MAX_HEIGHT          1920
#define AX_VDEC_MIN_WIDTH           48
#define AX_VDEC_MIN_HEIGHT          48

#define AX_JDEC_MAX_WIDTH           16384
#define AX_JDEC_MAX_HEIGHT          16384
#define AX_JDEC_MIN_WIDTH           48
#define AX_JDEC_MIN_HEIGHT          48

#define AX_VDEC_MAX_FRAME_BUF_CNT    32
#define AX_VDEC_MIN_FRAME_BUF_CNT    1

#define AX_MAX_VDEC_USER_DATA_SIZE (2048)

#define AX_MAX_VDEC_USER_DATA_CNT  (20)

/* 2 byte for length */
#define AX_MAX_JDEC_USER_DATA_SIZE (0xFFFF - 2)

#define AX_VDEC_ALIGN_NUM            8
#define ATTRIBUTE __attribute__((aligned (AX_VDEC_ALIGN_NUM)))

typedef AX_S32 AX_VDEC_GRP;

typedef struct axVDEC_MOD_ATTR_T {
    AX_U32 u32MaxGroupCount; /* 预留参数 */
 } AX_VDEC_MOD_ATTR_T;

typedef enum axVDEC_INPUT_MODE_E {
    /* send by NAL  */
    AX_VDEC_INPUT_MODE_NAL = 0,
    /* send by frame  */
    AX_VDEC_INPUT_MODE_FRAME,
    /* send by slice  */
    AX_VDEC_INPUT_MODE_SLICE,
    /* send by stream */
    AX_VDEC_INPUT_MODE_STREAM,
    /* One frame supports multiple packets sending. The current frame is
    considered to end when bEndOfFrame is equal to AX_TRUE */
    AX_VDEC_INPUT_MODE_COMPAT,
    AX_VDEC_INPUT_MODE_BUTT
} AX_VDEC_INPUT_MODE_E;

typedef enum axVIDEO_OUTPUT_ORDER_E {
    AX_VDEC_OUTPUT_ORDER_DISP = 0,
    AX_VDEC_OUTPUT_ORDER_DEC,
    AX_VDEC_OUTPUT_ORDER_BUTT
} AX_VDEC_OUTPUT_ORDER_E;

typedef struct axVDEC_GRP_ATTR_T {
    /* RW; video type(H264/JPEG/MJPEG) to be decoded   */
    AX_PAYLOAD_TYPE_E       enCodecType;
    /* RW; send by stream or by frame */
    AX_VDEC_INPUT_MODE_E    enInputMode;
    /* RW; frame to next module mode */
    AX_LINK_MODE_E          enLinkMode;
    /* RW; frames output order */
    AX_VDEC_OUTPUT_ORDER_E  enOutOrder;
    /* H264: input image width [AX_VDEC_MAX_WIDTH], in pixel */
    /* JPEG/MJPEG: input image width [AX_JDEC_MAX_WIDTH], in pixel */
    AX_U32                  u32PicWidth;
    /* H264: input image height [AX_VDEC_MAX_HEIGHT], in pixel */
    /* JPEG/MJPEG: input image height [AX_JDEC_MAX_HEIGHT], in pixel */
    AX_U32                  u32PicHeight;
    /* RW; frame height = pic height + padding size(lines) */
    AX_U32                  u32FrameHeight;
    /* RW; stream buffer size(Byte) */
    AX_U32                  u32StreamBufSize;
    /* RW; private/com/user vb for frame buffer */
    AX_POOL_SOURCE_E        enVdecVbSource;
    /* RW; frame buffer number*/
    AX_U32                  u32FrameBufCnt;
    /* RW; wait for output queue empty before executing destroy */
    AX_S32                  s32DestroyTimeout;
} AX_VDEC_GRP_ATTR_T;

typedef struct axVDEC_STREAM_T {
    /* W; time stamp */
    AX_U64  u64PTS;
    AX_U64  u64PrivateData;
    /* W; is the end of a frame */
    AX_BOOL bEndOfFrame;
    /* W; is the end of all stream */
    AX_BOOL bEndOfStream;
    /* W; is the current frame skip displayed. only valid by AX_VDEC_INPUT_MODE_FRAME */
    AX_BOOL bSkipDisplay;
    /* W; stream len */
    AX_U32  u32StreamPackLen;
    /* W; stream virture address */
    AX_U8 ATTRIBUTE *pu8Addr;
    /* W; stream physical address */
    AX_U64  u64PhyAddr;
} AX_VDEC_STREAM_T;

typedef enum axVIDEO_MODE_E {
    VIDEO_DEC_MODE_IPB = 0,
    VIDEO_DEC_MODE_IP,
    VIDEO_DEC_MODE_I,
    VIDEO_DEC_MODE_BUTT
} AX_VDEC_MODE_E;

typedef struct axVDEC_GRP_PARAM_T {
    AX_VDEC_MODE_E enVdecMode;
} AX_VDEC_GRP_PARAM_T;

/* the param of receive picture */
typedef struct axVDEC_RECV_PIC_PARAM_T {
    /**
     * RW; Range:[-1, 2147483647]; Number of frames received and decoded by the decoding channel,
     * 0 is not supported
     */
    AX_S32 s32RecvPicNum;
} AX_VDEC_RECV_PIC_PARAM_T;

typedef struct axVDEC_DECODE_ERROR_T {
    /* R; format error. eg: do not support filed */
    AX_S32 s32FormatErr;
    /* R; picture width or height is larger than chnnel width or height */
    AX_S32 s32PicSizeErrSet;
    /* R; unsupport the stream specification */
    AX_S32 s32StreamUnsprt;
    /* R; stream package error */
    AX_S32 s32PackErr;
    /* R; refrence num is not enough */
    AX_S32 s32RefErrSet;
    /* R; the buffer size of picture is not enough */
    AX_S32 s32PicBufSizeErrSet;
    /* R; the stream size is too big and and force discard stream */
    AX_S32 s32StreamSizeOver;
    /* R; the stream not released for too long time */
    AX_S32 s32VdecStreamNotRelease;
} AX_VDEC_DECODE_ERROR_T;

typedef struct axVDEC_GRP_STATUS_T {
    /* R; video type to be decoded */
    AX_PAYLOAD_TYPE_E enCodecType;
    /* R; left stream bytes waiting for decode */
    AX_U32  u32LeftStreamBytes;
    /* R; left frames waiting for decode,only valid for AX_VDEC_INPUT_MODE_FRAME */
    AX_U32  u32LeftStreamFrames;
    /* R; pics waiting for output */
    AX_U32  u32LeftPics;
    /* R; had started recv stream? */
    AX_BOOL bStartRecvStream;
    /* R; how many frames of stream has been received. valid when send by frame. */
    AX_U32  u32RecvStreamFrames;
    /* R; how many frames of stream has been decoded. valid when send by frame. */
    AX_U32  u32DecodeStreamFrames;
    AX_U32  u32PicWidth;
    AX_U32  u32PicHeight;
    /* R; input fifo is full? */
    AX_BOOL bInputFifoIsFull;
    /* R; information about decode error */
    AX_VDEC_DECODE_ERROR_T stVdecDecErr;
} AX_VDEC_GRP_STATUS_T;

typedef struct axVDEC_USERDATA_T {
    /* R; userdata data phy address */
    AX_U64          u64PhyAddr;
    /* R; userdata data len */
    AX_U32          u32UserDataCnt; /* total user data count */
    AX_U32          u32Len; /* total userdata len */
    AX_U32          u32BufSize; /* userdata buffer size */
    AX_U32          u32DataLen[AX_MAX_VDEC_USER_DATA_CNT];
    AX_BOOL         bValid;
    /* R; userdata data vir address */
    AX_U8 ATTRIBUTE *pu8Addr;
} AX_VDEC_USERDATA_T;

typedef enum axVDEC_DISPLAY_MODE_E {
    AX_VDEC_DISPLAY_MODE_PREVIEW = 0x0,
    AX_VDEC_DISPLAY_MODE_PLAYBACK = 0x1,
    AX_VDEC_DISPLAY_MODE_BUTT
} AX_VDEC_DISPLAY_MODE_E;

typedef struct axVDEC_GRP_SET_INFO_T {
    AX_U32 u32GrpCount;
    AX_VDEC_GRP VdGrp[AX_VDEC_MAX_GRP_NUM];
} AX_VDEC_GRP_SET_INFO_T;

typedef struct axVDEC_DEC_ONE_FRM_T {
    AX_VDEC_STREAM_T stStream;
    AX_VIDEO_FRAME_T stFrame;
} AX_VDEC_DEC_ONE_FRM_T;

typedef struct axVDEC_STREAM_BUF_INFO_T {
    AX_U64 phyStart;
    AX_U8  *virStart;
    AX_U32 totalSize;
    AX_U32 readAbleSize;
    AX_U32 writeAbleSize;
    AX_U32 readOffset;
    AX_U32 writeOffset;
} AX_VDEC_STREAM_BUF_INFO_T;

/************************************************************************************************************************/
typedef enum {
    AX_ID_VDEC_NULL    = 0x01,
    AX_ID_VDEC_BUTT,
} AX_VDEC_SUB_ID_E;

typedef struct axVdec_VUI_ASPECT_RATIO_T {
    AX_U8 aspect_ratio_info_present_flag; /* RW; Range:[0,1]; If 1, aspectratio info belows will be encoded into vui */
    AX_U8 aspect_ratio_idc;               /* RW; Range:[0,255]; 17~254 is reserved,see the protocol for the meaning.*/
    AX_U8 overscan_info_present_flag;     /* RW; Range:[0,1]; If 1, oversacan info belows will be encoded into vui.*/
    AX_U8 overscan_appropriate_flag;      /* RW; Range:[0,1]; see the protocol for the meaning. */
    AX_U16 sar_width;                     /* RW; Range:(0, 65535]; see the protocol for the meaning. */
    AX_U16 sar_height;                    /* RW; Range:(0, 65535]; see the protocol for the meaning. */
} AX_VDEC_VUI_ASPECT_RATIO_T;

typedef struct axVdec_VUI_TIME_INFO_T {
    AX_U8 timing_info_present_flag; /* RW; Range:[0,1]; If 1, timing info belows will be encoded into vui.*/
    AX_U32 num_units_in_tick;       /* RW; Range:(0,4294967295]; see the H.264/H.265 protocol for the meaning */
    AX_U32 time_scale;              /* RW; Range:(0,4294967295]; see the H.264/H.265 protocol for the meaning */

    AX_U8 fixed_frame_rate_flag;          /* RW; Range:[0,1]; see the H.264 protocol for the meaning. */
    AX_U32 num_ticks_poc_diff_one_minus1; /* RW; Range:(0,4294967294]; see the H.265 protocol for the meaning */
} AX_VDEC_VUI_TIME_INFO_T;

typedef struct axVdec_VIDEO_SIGNAL_T {
    AX_U8 video_signal_type_present_flag; /* RW; Range:[0,1]; If 1, video singnal info will be encoded into vui. */
    AX_U8 video_format;          /* RW; H.264e Range:[0,7], H.265e Range:[0,5]; see the protocol for the meaning. */
    AX_U8 video_full_range_flag; /* RW; Range: {0,1}; see the protocol for the meaning.*/
    AX_U8 colour_description_present_flag; /* RO; Range: {0,1}; see the protocol for the meaning.*/
    AX_U8 colour_primaries;                /* RO; Range: [0,255]; see the protocol for the meaning. */
    AX_U8 transfer_characteristics;        /* RO; Range: [0,255]; see the protocol for the meaning. */
    AX_U8 matrix_coefficients;             /* RO; Range:[0,255]; see the protocol for the meaning. */
} AX_VDEC_VUI_VIDEO_SIGNAL_T;

typedef struct axVdec_VUI_BITSTREAM_RESTRIC_T {
    AX_U8 bitstream_restriction_flag; /* RW; Range: {0,1}; see the protocol for the meaning.*/
} AX_VDEC_VUI_BITSTREAM_RESTRIC_T;

typedef struct axVdec_VUI_PARAM_T {
    AX_VDEC_VUI_ASPECT_RATIO_T      stVuiAspectRatio;
    AX_VDEC_VUI_TIME_INFO_T         stVuiTimeInfo;
    AX_VDEC_VUI_VIDEO_SIGNAL_T      stVuiVideoSignal;
    AX_VDEC_VUI_BITSTREAM_RESTRIC_T stVuiBitstreamRestric;
} AX_VDEC_VUI_PARAM_T;

typedef struct {
    AX_VIDEO_FRAME_INFO_T stFrmInfo;
    AX_BOOL bInstant;
    AX_BOOL bEnable;
} AX_VDEC_USRPIC_T;

typedef struct axVDEC_BITSTREAM_INFO_T {
    AX_U32 u32Width;
    AX_U32 u32Height;
    AX_U32 u32RefFramesNum;
    AX_U32 u32BitDepthY;
    AX_U32 u32BitDepthC;
    AX_U32 u32BufferCnt;
} AX_VDEC_BITSTREAM_INFO_T;

/* invlalid channel ID */
#define AX_ERR_VDEC_INVALID_GRPID       AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_INVALID_GRPID)
#define AX_ERR_VDEC_INVALID_CHNID       AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_INVALID_CHNID)
/* at lease one parameter is illegal ,eg, out of range  */
#define AX_ERR_VDEC_ILLEGAL_PARAM       AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_ILLEGAL_PARAM)
/* using a NULL pointer */
#define AX_ERR_VDEC_NULL_PTR            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NULL_PTR)
/* bad address,  eg. used for copy_from_user & copy_to_user   */
#define AX_ERR_VDEC_BAD_ADDR            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_BAD_ADDR)

/* system is not ready, not properly initialized or loaded driver */
#define AX_ERR_VDEC_SYS_NOTREADY        AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_SYS_NOTREADY)
/* system busy, cannot share use or destroy */
#define AX_ERR_VDEC_BUSY                AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_BUSY)
/* init error or not init at all */
#define AX_ERR_VDEC_NOT_INIT            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOT_INIT)
/* configuration required before using a system/device/channel */
#define AX_ERR_VDEC_NOT_CONFIG          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOT_CONFIG)
/* operation is not (yet) supported on this platform */
#define AX_ERR_VDEC_NOT_SUPPORT         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOT_SUPPORT)
/* operation is not permitted, eg, try to change a static attribute, or start without init */
#define AX_ERR_VDEC_NOT_PERM            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOT_PERM)
/* channel already exists */
#define AX_ERR_VDEC_EXIST               AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_EXIST)
/* the channle is not existing  */
#define AX_ERR_VDEC_UNEXIST             AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_UNEXIST)
/* failure caused by malloc memory from heap */
#define AX_ERR_VDEC_NOMEM               AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOMEM)
/* failure caused by borrow buffer from pool */
#define AX_ERR_VDEC_NOBUF               AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOBUF)

#define AX_ERR_VDEC_NOT_MATCH           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_NOT_MATCH)

/* no data in buffer */
#define AX_ERR_VDEC_BUF_EMPTY           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_BUF_EMPTY)
/* no buffer for new data */
#define AX_ERR_VDEC_BUF_FULL            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_BUF_FULL)

/* no data in queue */
#define AX_ERR_VDEC_QUEUE_EMPTY         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_QUEUE_EMPTY)
/* no queue for new data */
#define AX_ERR_VDEC_QUEUE_FULL          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_QUEUE_FULL)

/* wait timeout failed */
#define AX_ERR_VDEC_TIMED_OUT           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_TIMED_OUT)
/* process termination */
#define AX_ERR_VDEC_FLOW_END            AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_FLOW_END)
/* for vdec unknown error */
#define AX_ERR_VDEC_UNKNOWN             AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_UNKNOWN)

/* for vdec os failed */
#define AX_ERR_VDEC_OS                  AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, AX_ERR_OS_FAIL)

/* run error */
#define AX_ERR_VDEC_RUN_ERROR           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x80)
/* stream error */
#define AX_ERR_VDEC_STRM_ERROR          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x81)

#define AX_ERR_VDEC_NEED_REALLOC_BUF    AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x82)

#define AX_ERR_VDEC_NO_AVAILABLE_GRP    AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x83)

#define AX_ERR_VDEC_MAX_ERR_DEF         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0xFF)


#ifdef __cplusplus
}
#endif

#endif

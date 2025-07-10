/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_OPAL_TYPE_H_
#define _AX_OPAL_TYPE_H_

#include "ax_global_type.h"
#include "ax_base_type.h"

#ifdef __cplusplus
extern "C" {
#endif

// macro
// error code
#define AX_OPAL_SUCC (0)
#define AX_ERR_OPAL_GENERIC (-1)
#define AX_ERR_OPAL_NULL_PTR AX_DEF_ERR(0xFF, 1, AX_ERR_NULL_PTR)
#define AX_ERR_OPAL_ILLEGAL_PARAM AX_DEF_ERR(0xFF, 1, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_OPAL_NOT_INIT AX_DEF_ERR(0xFF, 1, AX_ERR_NOT_INIT)
#define AX_ERR_OPAL_QUEUE_EMPTY AX_DEF_ERR(0xFF, 1, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_OPAL_QUEUE_FULL AX_DEF_ERR(0xFF, 1, AX_ERR_QUEUE_FULL)
#define AX_ERR_OPAL_UNEXIST AX_DEF_ERR(0xFF, 1, AX_ERR_UNEXIST)
#define AX_ERR_OPAL_TIMEOUT AX_DEF_ERR(0xFF, 1, AX_ERR_TIMED_OUT)
#define AX_ERR_OPAL_SYS_NOTREADY AX_DEF_ERR(0xFF, 1, AX_ERR_SYS_NOTREADY)
#define AX_ERR_OPAL_INVALID_HANDLE AX_DEF_ERR(0xFF, 1, AX_ERR_INVALID_CHNID)
#define AX_ERR_OPAL_NOMEM AX_DEF_ERR(0xFF, 1, AX_ERR_NOMEM)
#define AX_ERR_OPAL_UNKNOWN AX_DEF_ERR(0xFF, 1, AX_ERR_UNKNOWN)
#define AX_ERR_OPAL_NOT_SUPPORT AX_DEF_ERR(0xFF, 1, AX_ERR_NOT_SUPPORT)
#define AX_ERR_OPAL_INITED AX_DEF_ERR(0xFF, 1, AX_ERR_EXIST)
#define AX_ERR_OPAL_NOT_PERM AX_DEF_ERR(0xFF, 1, AX_ERR_NOT_PERM)

// support Y/U/V three planes
#define AX_OPAL_MAX_COLOR_COMPONENT (3)

// max algorithm motion region count
#define AX_OPAL_MAX_ALGO_MD_REGION_COUNT (16)

// max osd string length
#define AX_OPAL_MAX_OSD_STR_LEN (128)

// osd privacy point num
#define AX_OPAL_OSD_PIVACY_POINT_MAX (4)

// AAC config biffer size
#define AX_OPAL_AUDIO_AAC_CONFIG_BUF_SIZE (64)

// max point num
#define AX_OPAL_POINT_MAX (10)

// tuning bin path
#define AX_OPAL_PATH_LEN (256)

// tuning bin num
#define AX_OPAL_TUNING_BIN_MAX (5)

// smart video coding region max
#define AX_OPAL_SVC_MAX_REGION_NUM (32)

// algo plate code length
#define AX_OPAL_ALGO_PLATE_CODE_LEN (16)

/// handle definition
typedef AX_VOID* AX_OPAL_HANDLE;

// enum
// chip type
typedef enum axOPAL_CHIP_TYPE_E {
    AX_OPAL_CHIP_TYPE_NONE = 0x00,
    AX_OPAL_CHIP_TYPE_AX620Q = 0x01,
    AX_OPAL_CHIP_TYPE_AX620QX = 0x02,
    AX_OPAL_CHIP_TYPE_AX630C = 0x04,
    AX_OPAL_CHIP_TYPE_AX631 = 0x05,
    AX_OPAL_CHIP_TYPE_AX620QZ = 0x06,
    AX_OPAL_CHIP_TYPE_AX620QP = 0x07,
    AX_OPAL_CHIP_TYPE_BUTT
} AX_OPAL_CHIP_TYPE_E;

// sensor sensor id
typedef enum axOPAL_SNS_ID_E {
    AX_OPAL_SNS_ID_0 = 0,
    AX_OPAL_SNS_ID_1,
    AX_OPAL_SNS_ID_BUTT
} AX_OPAL_SNS_ID_E;

// support sensor type
typedef enum axOPAL_SNS_TYPE_E {
    AX_OPAL_SNS_OS04A10, /* os04a10 */
    AX_OPAL_SNS_SC450AI, /* sc450ai */
    AX_OPAL_SNS_IMX678,  /* imx678 */
    AX_OPAL_SNS_SC200AI, /* sc200ai */
    AX_OPAL_SNS_SC500AI, /* sc500ai */
    AX_OPAL_SNS_SC850SL, /* sc850sl */
    AX_OPAL_SNS_C4395,   /* c4395 */
    AX_OPAL_SNS_GC4653,  /* gc4653 */
    AX_OPAL_SNS_MIS2032, /* mis2032 */
    AX_OPAL_SNS_OS12D40, /* os12d40 */
    AX_OPAL_SNS_TYPE_BUTT
} AX_OPAL_SNS_TYPE_E;

// sensor mode
typedef enum axOPAL_SNS_MODE_E {
    AX_OPAL_SNS_SDR_MODE = 1,
    AX_OPAL_SNS_HDR_MODE = 2,
    AX_OPAL_SNS_SDR_ONLY_MODE = 5,
    AX_OPAL_SNS_MODE_BUTT }
AX_OPAL_SNS_MODE_E;

// sensor mode
typedef enum axOPAL_SNS_DAYNIGHT_E {
    AX_OPAL_SNS_DAY_MODE = 0,
    AX_OPAL_SNS_NIGHT_MODE = 1,
    AX_OPAL_SNS_DAYNIGHT_BUTT
} AX_OPAL_SNS_DAYNIGHT_E;

// sensor rotation mode
typedef enum axOPAL_SNS_ROTATION_E {
    AX_OPAL_SNS_ROTATION_0,
    AX_OPAL_SNS_ROTATION_90,
    AX_OPAL_SNS_ROTATION_180,
    AX_OPAL_SNS_ROTATION_270,
    AX_OPAL_SNS_ROTATION_BUTT
} AX_OPAL_SNS_ROTATION_E;

// sensor soft photosensitivity status
typedef enum axOPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_E {
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_DAY,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_NIGHT,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_UNKOWN,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_BUTT
} AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_E;

// sensor soft photosensitivity type
typedef enum axOPAL_SNS_SOFT_PHOTOSENSITIVITY_TYPE_E {
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_NONE,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_IR,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_WARMLIGHT,
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_TYPE_BUTT
} AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_TYPE_E;

// video channel num
typedef enum axOPAL_VIDEO_CHN_E {
    AX_OPAL_VIDEO_CHN_0,
    AX_OPAL_VIDEO_CHN_1,
    AX_OPAL_VIDEO_CHN_2,
    AX_OPAL_VIDEO_CHN_3,
    AX_OPAL_VIDEO_CHN_4,
    AX_OPAL_VIDEO_CHN_BUTT
} AX_OPAL_VIDEO_CHN_E;

// video channel type
typedef enum axOPAL_VIDEO_CHN_TYPE_E {
    AX_OPAL_VIDEO_CHN_TYPE_H264,
    AX_OPAL_VIDEO_CHN_TYPE_H265,
    AX_OPAL_VIDEO_CHN_TYPE_MJPEG,
    AX_OPAL_VIDEO_CHN_TYPE_JPEG,
    AX_OPAL_VIDEO_CHN_TYPE_ALGO,
    AX_OPAL_VIDEO_CHN_TYPE_BUTT
} AX_OPAL_VIDEO_CHN_TYPE_E;

// video rc mode
typedef enum axOPAL_VIDEO_RC_MODE_E {
    AX_OPAL_VIDEO_RC_MODE_CBR,
    AX_OPAL_VIDEO_RC_MODE_VBR,
    AX_OPAL_VIDEO_RC_MODE_FIXQP,
    AX_OPAL_VIDEO_RC_MODE_AVBR,
    AX_OPAL_VIDEO_RC_MODE_CVBR,
    AX_OPAL_VIDEO_RC_MODE_BUTT
} AX_OPAL_VIDEO_RC_MODE_E;

// video nalu type
typedef enum axOPAL_VIDEO_NALU_TYPE_E {
    // H264
    AX_OPAL_VIDEO_H264_NALU_BSLICE = 0,
    AX_OPAL_VIDEO_H264_NALU_PSLICE = 1,
    AX_OPAL_VIDEO_H264_NALU_ISLICE = 2,
    AX_OPAL_VIDEO_H264_NALU_IDRSLICE = 5,
    AX_OPAL_VIDEO_VIDEO_H264_NALU_SEI = 6,
    AX_OPAL_VIDEO_H264_NALU_SPS = 7,
    AX_OPAL_VIDEO_H264_NALU_PPS = 8,
    AX_OPAL_VIDEO_H264_NALU_PREFIX_14 = 14,

    // H265
    AX_OPAL_VIDEO_H265_NALU_BSLICE = 0,
    AX_OPAL_VIDEO_H265_NALU_PSLICE = 1,
    AX_OPAL_VIDEO_H265_NALU_ISLICE = 2,
    AX_OPAL_VIDEO_H265_NALU_TSA_R = 3,
    AX_OPAL_VIDEO_H265_NALU_IDRSLICE = 19,
    AX_OPAL_VIDEO_H265_NALU_VPS = 32,
    AX_OPAL_VIDEO_H265_NALU_SPS = 33,
    AX_OPAL_VIDEO_H265_NALU_PPS = 34,
    AX_OPAL_VIDEO_H265_NALU_SEI = 39,
    AX_OPAL_VIDEO_NALU_TYPE_BUTT
} AX_OPAL_VIDEO_NALU_TYPE_E;

// smart video coding region type
typedef enum axOPAL_VIDEO_SVC_REGION_TYPE_E {
    AX_OPAL_VIDEO_SVC_REGION_TYPE0 = 0,
    AX_OPAL_VIDEO_SVC_REGION_TYPE1,
    AX_OPAL_VIDEO_SVC_REGION_TYPE2,
    AX_OPAL_VIDEO_SVC_REGION_TYPE3,
    AX_OPAL_VIDEO_SVC_REGION_TYPE4,
    AX_OPAL_VIDEO_SVC_REGION_TYPE_BUTT
} AX_OPAL_VIDEO_SVC_REGION_TYPE_E;

// algorithm type
typedef enum axOPAL_ALGO_TYPE_E {
    AX_OPAL_ALGO_TYPE_NONE = (0 << 0),
    AX_OPAL_ALGO_HVCP = (1 << 0),
    AX_OPAL_ALGO_FACE = (1 << 1),
    AX_OPAL_ALGO_MD = (1 << 2),
    AX_OPAL_ALGO_OD = (1 << 3),
} AX_OPAL_ALGO_TYPE_E;

// algorithm hvcfp object type
typedef enum axOPAL_ALGO_HVCFP_TYPE_E {
    AX_OPAL_ALGO_HVCFP_BODY,
    AX_OPAL_ALGO_HVCFP_VEHICLE,
    AX_OPAL_ALGO_HVCFP_CYCLE,
    AX_OPAL_ALGO_HVCFP_FACE,
    AX_OPAL_ALGO_HVCFP_PLATE,
    AX_OPAL_ALGO_HVCFP_TYPE_BUTT
} AX_OPAL_ALGO_HVCFP_TYPE_E;

// algorithm ives type
typedef enum axOPAL_ALGO_IVES_TYPE_E {
    AX_OPAL_ALGO_IVES_MD,
    AX_OPAL_ALGO_IVES_OD,
    AX_OPAL_ALGO_IVES_TYPE_BUTT
} AX_OPAL_ALGO_IVES_TYPE_E;

// algorithm face respirator type
typedef enum axOPAL_ALGO_FACE_RESPIRATOR_TYPE_E {
    AX_OPAL_ALGO_FACE_RESPIRATOR_NONE,
    AX_OPAL_ALGO_FACE_RESPIRATOR_COMMON,
    AX_OPAL_ALGO_FACE_RESPIRATOR_TYPE_BUTT
} AX_OPAL_ALGO_FACE_RESPIRATOR_TYPE_E;

// algorithm plate color type
typedef enum axOPAL_ALGO_PLATE_COLOR_TYPE_E {
    AX_OPAL_ALGO_PLATE_COLOR_BLUE,
    AX_OPAL_ALGO_PLATE_COLOR_YELLOW,
    AX_OPAL_ALGO_PLATE_COLOR_BLACK,
    AX_OPAL_ALGO_PLATE_COLOR_WHITE,
    AX_OPAL_ALGO_PLATE_COLOR_GREEN,
    AX_OPAL_ALGO_PLATE_COLOR_NEW_ENERGY,
    AX_OPAL_ALGO_PLATE_COLOR_UNKOWN,
    AX_OPAL_ALGO_PLATE_COLOR_TYPE_BUTT
} AX_OPAL_ALGO_PLATE_COLOR_TYPE_E;

// algorithm track status type
typedef enum axOPAL_ALGO_TRACK_STATUS_E {
    AX_OPAL_ALGO_TRACK_STATUS_NEW,
    AX_OPAL_ALGO_TRACK_STATUS_UPDATE,
    AX_OPAL_ALGO_TRACK_STATUS_LOST,
    AX_OPAL_ALGO_TRACK_STATUS_SELECT,
    AX_OPAL_ALGO_TRACK_STATUS_BUTT
} AX_OPAL_ALGO_TRACK_STATUS_E;

// algorithm push mode
typedef enum axOPAL_ALGO_PUSH_MODE_E {
    AX_OPAL_ALGO_PUSH_MODE_FAST = 1,
    AX_OPAL_ALGO_PUSH_MODE_INTERVAL = 2,
    AX_OPAL_ALGO_PUSH_MODE_BEST = 3,
    AX_OPAL_ALGO_PUSH_MODE_BUTT
} AX_OPAL_ALGO_PUSH_MODE_E;

// osd type
typedef enum axOPAL_OSD_TYPE_E {
    AX_OPAL_OSD_TYPE_TIME,
    AX_OPAL_OSD_TYPE_PICTURE,
    AX_OPAL_OSD_TYPE_STRING,
    AX_OPAL_OSD_TYPE_STRING_TOP,
    AX_OPAL_OSD_TYPE_PRIVACY,
    AX_OPAL_OSD_TYPE_BUTT
} AX_OPAL_OSD_TYPE_E;

/* osd privacy type */
typedef enum axOPAL_OSD_PRIVACY_TYPE_E {
    AX_OPAL_OSD_PRIVACY_TYPE_LINE,
    AX_OPAL_OSD_PRIVACY_TYPE_RECT,
    AX_OPAL_OSD_PRIVACY_TYPE_POLYGON,
    AX_OPAL_OSD_PRIVACY_TYPE_MOSAIC,
    AX_OPAL_OSD_PRIVACY_TYPE_BUTT
} AX_OPAL_OSD_PRIVACY_TYPE_E;

// osd align type
typedef enum axOPAL_OSD_ALIGN_E {
    AX_OPAL_OSD_ALIGN_LEFT_TOP,
    AX_OPAL_OSD_ALIGN_RIGHT_TOP,
    AX_OPAL_OSD_ALIGN_LEFT_BOTTOM,
    AX_OPAL_OSD_ALIGN_RIGHT_BOTTOM,
    AX_OPAL_OSD_ALIGN_BUTT
} AX_OPAL_OSD_ALIGN_E;

// osd date format
typedef enum axOPAL_OSD_DATETIME_FORMAT_E {
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDD1,         /* YYYY-MM-DD */
    AX_OPAL_OSD_DATETIME_FORMAT_MMDDYY1,         /* MM-DD-YYYY */
    AX_OPAL_OSD_DATETIME_FORMAT_DDMMYY1,         /* DD-MM-YYYY */
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDD2,         /* YYYY年MM月DD日 */
    AX_OPAL_OSD_DATETIME_FORMAT_MMDDYY2,         /* MM月DD日YYYY年 */
    AX_OPAL_OSD_DATETIME_FORMAT_DDMMYY2,         /* DD日MM月YYYY年 */
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDD3,         /* YYYY/MM/DD */
    AX_OPAL_OSD_DATETIME_FORMAT_MMDDYY3,         /* MM/DD/YYYY */
    AX_OPAL_OSD_DATETIME_FORMAT_DDMMYY3,         /* DD/MM/YYYY */
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDDWW1,       /* YYYY-MM-DD 星期几 */
    AX_OPAL_OSD_DATETIME_FORMAT_HHMMSS1,         /* HH:MM:SS */
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDDHHMMSS1,   /* YYYY-MM-DD HH:MM:SS */
    AX_OPAL_OSD_DATETIME_FORMAT_YYMMDDHHMMSSWW1, /* YYYY-MM-DD HH:MM:SS 星期几 */
    AX_OPAL_OSD_DATETIME_FORMAT_BUTT
} AX_OPAL_OSD_DATETIME_FORMAT_E;

// osd picture source
typedef enum axOPAL_OSD_PIC_SOURCE_E {
    AX_OPAL_OSD_PIC_SOURCE_FILE,
    AX_OPAL_OSD_PIC_SOURCE_BUFFER,
    AX_OPAL_OSD_PIC_SOURCE_BUTT
} AX_OPAL_OSD_PIC_SOURCE_TYPE_E;

// audio
// audio channel num
typedef enum axOPAL_AUDIO_CHN_E {
    AX_OPAL_AUDIO_CHN_0,
    AX_OPAL_AUDIO_CHN_1,
    AX_OPAL_AUDIO_CHN_2,
    AX_OPAL_AUDIO_CHN_3,
    AX_OPAL_AUDIO_CHN_4,
    AX_OPAL_AUDIO_CHN_BUTT
} AX_OPAL_AUDIO_CHN_E;

// audio sound mode
typedef enum axOPAL_AUDIO_SOUND_MODE_E {
    AX_OPAL_AUDIO_SOUND_MODE_MONO = 0,   /*mono*/
    AX_OPAL_AUDIO_SOUND_MODE_STEREO = 1, /*stereo*/
    AX_OPAL_AUDIO_SOUND_MODE_BUTT
} AX_OPAL_AUDIO_SOUND_MODE_E;

// audio layout mode
typedef enum axOPAL_AUDIO_LAYOUT_MODE_E {
    AX_OPAL_AUDIO_LAYOUT_MIC_MIC = 0,
    AX_OPAL_AUDIO_LAYOUT_MIC_REF = 1,
    AX_OPAL_AUDIO_LAYOUT_REF_MIC = 2,
    AX_OPAL_AUDIO_LAYOUT_INTERNAL_MIC_NULL = 3,
    AX_OPAL_AUDIO_LAYOUT_INTERNAL_NULL_MIC = 4,
    AX_OPAL_AUDIO_LAYOUT_DOORBELL = 5,
    AX_OPAL_AUDIO_LAYOUT_MODE_BUTT
} AX_OPAL_AUDIO_LAYOUT_MODE_E;

// audio bit width
typedef enum axOPAL_AUDIO_BIT_WIDTH_E {
    AX_OPAL_AUDIO_BIT_WIDTH_8 = 0,  /* 8bit width */
    AX_OPAL_AUDIO_BIT_WIDTH_16 = 1, /* 16bit width*/
    AX_OPAL_AUDIO_BIT_WIDTH_24 = 2, /* 24bit width*/
    AX_OPAL_AUDIO_BIT_WIDTH_32 = 3, /* 32bit width*/
    AX_OPAL_AUDIO_BIT_WIDTH_BUTT,
} AX_OPAL_AUDIO_BIT_WIDTH_E;

// audio sample rate
typedef enum axOPAL_AUDIO_SAMPLE_RATE_E {
    AX_OPAL_AUDIO_SAMPLE_RATE_8000 = 8000,   /* 8K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_12000 = 12000, /* 12K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_11025 = 11025, /* 11.025K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_16000 = 16000, /* 16K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_22050 = 22050, /* 22.050K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_24000 = 24000, /* 24K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_32000 = 32000, /* 32K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_44100 = 44100, /* 44.1K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_48000 = 48000, /* 48K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_64000 = 64000, /* 64K samplerate*/
    AX_OPAL_AUDIO_SAMPLE_RATE_96000 = 96000  /* 96K samplerate*/
} AX_OPAL_AUDIO_SAMPLE_RATE_E;

typedef enum axOPAL_AUDIO_AGC_MODE_E {
    // Adaptive mode intended for use if an analog volume control is available
    // on the capture device. It will require the user to provide coupling
    // between the OS mixer controls and AGC through the |stream_analog_level()|
    // functions.
    //
    // It consists of an analog gain prescription for the audio device and a
    // digital compression stage.
    AX_OPAL_AUDIO_AGC_MODE_ADAPTIVE_ANALOG = 0,

    // Adaptive mode intended for situations in which an analog volume control
    // is unavailable. It operates in a similar fashion to the adaptive analog
    // mode, but with scaling instead applied in the digital domain. As with
    // the analog mode, it additionally uses a digital compression stage.
    AX_OPAL_AUDIO_AGC_MODE_ADAPTIVE_DIGITAL,

    // Fixed mode which enables only the digital compression stage also used by
    // the two adaptive modes.
    //
    // It is distinguished from the adaptive modes by considering only a
    // short time-window of the input signal. It applies a fixed gain through
    // most of the input level range, and compresses (gradually reduces gain
    // with increasing level) the input signal at higher levels. This mode is
    // preferred on embedded devices where the capture signal level is
    // predictable, so that a known gain can be applied.
    AX_OPAL_AUDIO_AGC_MODE_FIXED_DIGITAL,

    AX_OPAL_AUDIO_AGC_MODE_BUTT
} AX_OPAL_AUDIO_AGC_MODE_E;

// audio noise suppression aggressiveness type
typedef enum axOPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_E {
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_LOW = 0,
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_MODERATE,
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_HIGH,
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_VERYHIGH,
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_BUTT
} AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_E;

// audio Acoustic Echo Canceller type
typedef enum axOPAL_AUDIO_AEC_TYPE_E {
    AX_OPAL_AUDIO_AEC_DISABLE,
    AX_OPAL_AUDIO_AEC_FLOAT,
    AX_OPAL_AUDIO_AEC_FIXED,
    AX_OPAL_AUDIO_AEC_TYPE_BUTT
} AX_OPAL_AUDIO_AEC_TYPE_E;

// audio Acoustic Echo Canceller routing mode
typedef enum axOPAL_AUDIO_AEC_ROUTING_MODE_E {
    AX_OPAL_AUDIO_AEC_QUITE_EARPIECE_OR_HEADSET,
    AX_OPAL_AUDIO_AEC_EARPIECE,
    AX_OPAL_AUDIO_AEC_LOUD_EARPIECE,
    AX_OPAL_AUDIO_AEC_SPEAKERPHONE,
    AX_OPAL_AUDIO_AEC_LOUD_SPEAKERPHONE,
    AX_OPAL_AUDIO_AEC_ROUTING_MODE_BUTT
} AX_OPAL_AUDIO_AEC_ROUTING_MODE_E;

// audio aec suppression level
typedef enum axOPAL_AUDIO_AEC_SUPPRESSION_LEVEL_E {
    AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_LOW,
    AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_MODERATE,
    AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_HIGH,
    AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_BUTT
} AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_E;

// audio aac type
typedef enum axOPAL_AUDIO_AAC_TYPE_E {
    AX_OPAL_AUDIO_AAC_TYPE_NONE = -1,
    AX_OPAL_AUDIO_AAC_TYPE_NULL_OBJECT = 0,
    AX_OPAL_AUDIO_AAC_TYPE_AAC_LC = 2,      /* Low Complexity object                     */
    AX_OPAL_AUDIO_AAC_TYPE_ER_AAC_LD = 23,  /* Error Resilient(ER) AAC LowDelay object   */
    AX_OPAL_AUDIO_AAC_TYPE_ER_AAC_ELD = 39, /* AAC Enhanced Low Delay                    */
    AX_OPAL_AUDIO_AAC_TYPE_BUTT,
} AX_OPAL_AUDIO_AAC_TYPE_E;

// audio aac channel mode
typedef enum axOPAL_AUDIO_AAC_CHN_MODE_E {
    AX_OPAL_AUDIO_AAC_CHN_MODE_INVALID = -1,
    AX_OPAL_AUDIO_AAC_CHN_MODE_UNKNOWN = 0,
    AX_OPAL_AUDIO_AAC_CHN_MODE_1 = 1,       /**< C */
    AX_OPAL_AUDIO_AAC_CHN_MODE_2 = 2,       /**< L+R */
    AX_OPAL_AUDIO_AAC_CHN_MODE_1_2 = 3,     /**< C, L+R */
    AX_OPAL_AUDIO_AAC_CHN_MODE_1_2_1 = 4,   /**< C, L+R, Rear */
    AX_OPAL_AUDIO_AAC_CHN_MODE_1_2_2 = 5,   /**< C, L+R, LS+RS */
    AX_OPAL_AUDIO_AAC_CHN_MODE_1_2_2_1 = 6, /**< C, L+R, LS+RS, LFE */
    AX_OPAL_AUDIO_AAC_CHN_MODE_BUTT,
} AX_OPAL_AUDIO_AAC_CHN_MODE_E;

// audio aac transport type
typedef enum axOPAL_AUDIO_AAC_TRANS_TYPE_E {
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_UNKNOWN = -1, /* Unknown format.            */
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_RAW = 0,      /* "as is" access units (packet based since there is obviously no sync layer) */
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_ADTS = 2,     /* ADTS bitstream format.     */
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_BUTT,
} AX_OPAL_AUDIO_AAC_TRANS_TYPE_E;

// audio file play status
typedef enum axOPAL_AUDIO_PLAY_FILE_STATUS_E {
    AX_OPAL_AUDIO_PLAY_FILE_STATUS_COMPLETE,
    AX_OPAL_AUDIO_PLAY_FILE_STATUS_STOP,
    AX_OPAL_AUDIO_PLAY_FILE_STATUS_ERROR,
    AX_OPAL_AUDIO_PLAY_FILE_STATUS_BUTT
} AX_OPAL_AUDIO_PLAY_FILE_STATUS_E;

// struct
// sensor ircut attribute
typedef struct axOPAL_SNS_IRCUT_ATTR_T {
    AX_F32 fIrCalibR;
    AX_F32 fIrCalibG;
    AX_F32 fIrCalibB;
    AX_F32 fNight2DayIrStrengthTh;
    AX_F32 fNight2DayIrDetectTh;
    AX_U32 nInitDayNightMode;
    AX_F32 fDay2NightLuxTh;
    AX_F32 fNight2DayLuxTh;
    AX_F32 fNight2DayBrightTh;
    AX_F32 fNight2DayDarkTh;
    AX_F32 fNight2DayUsefullWpRatio;
    AX_U32 nCacheTime;
} AX_OPAL_SNS_IRCUT_ATTR_T;

// sensor warm light attribute
typedef struct axOPAL_SNS_WARMLIGHT_ATTR_T {
    AX_U64 nOnLightSensitivity;
    AX_U64 nOnLightExpValMax;
    AX_U64 nOnLightExpValMid;
    AX_U64 nOnLightExpValMin;
    AX_U64 nOffLightSensitivity;
    AX_U64 nOffLightExpValMax;
    AX_U64 nOffLightExpValMid;
    AX_U64 nOffLightExpValMin;
} AX_OPAL_SNS_WARMLIGHT_ATTR_T;

// sensor soft photo sensitivity
typedef struct axOPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_T {
    AX_BOOL bAutoCtrl;
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_TYPE_E eType;

    union {
        AX_OPAL_SNS_IRCUT_ATTR_T stIrAttr;
        AX_OPAL_SNS_WARMLIGHT_ATTR_T stWarmAttr;
    };
} AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_T;

// sensor soft photo sensitivity result
typedef struct axOPAL_SNS_SOFT_PHOTOSENSITIVITY_RESULT_T {
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_TYPE_E eType;
    AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_STATUS_E eStatus;
    AX_VOID *pUserData;
} AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_RESULT_T;

// sensor hot noise balance
typedef struct axOPAL_SNS_HOTNOISEBALANCE_ATTR_T {
    AX_BOOL bEnable;
    AX_F32 fNormalThreshold;
    AX_F32 fBalanceThreshold;
    AX_CHAR *strSdrHotNoiseNormalModeBin;
    AX_CHAR *strSdrHotNoiseBalanceModeBin;
    AX_CHAR *strHdrHotNoiseNormalModeBin;
    AX_CHAR *strHdrHotNoiseBalanceModeBin;
} AX_OPAL_SNS_HOTNOISEBALANCE_ATTR_T;

// video encoder attribute
typedef struct axOPAL_VIDEO_ENCODER_ATTR_T {
    AX_OPAL_VIDEO_RC_MODE_E eRcMode;
    AX_U32 nQpLevel;      /* JPEG */
    AX_U32 nGop;          /* H264/H265 CBR/VBR/AVBR/CVBR/FIXQP */
    AX_U32 nBitrate;      /* H264/H265/MJPEG CBR/VBR/AVBR/CVBR/FIXQP */
    AX_U32 nMinQp;        /* H264/H265/MJPEG CBR/VBR/AVBR/CVBR/FIXQP */
    AX_U32 nMaxQp;        /* H264/H265/MJPEG CBR/VBR/AVBR/CVBR/FIXQP */
    AX_U32 nMinIQp;       /* H264/H265 CBR/VBR/AVBR/CVBR */
    AX_U32 nMaxIQp;       /* H264/H265 CBR/VBR/AVBR/CVBR */
    AX_U32 nMinIprop;     /* H264/H265 CBR/CVBR */
    AX_U32 nMaxIprop;     /* H264/H265 CBR/CVBR */
    AX_U32 nFixQp;        /* H264/H265 FIXQP */
    AX_S32 nIntraQpDelta; /* H264/H265 CBR/VBR/AVBR/CVBR */
    AX_S32 nDeBreathQpDelta; /* H264/H265 CBR/VBR/AVBR/CVBR */
    AX_U32 nIdrQpDeltaRange; /* H264/H265 CBR/VBR/AVBR/CVBR */
} AX_OPAL_VIDEO_ENCODER_ATTR_T;

// video channel attribute
typedef struct axOPAL_VIDEO_CHN_ATTR_T {
    AX_BOOL bEnable;
    AX_S32 nMaxWidth;
    AX_S32 nMaxHeight;
    AX_S32 nWidth;
    AX_S32 nHeight;
    AX_S32 nFramerate;
    AX_OPAL_VIDEO_CHN_TYPE_E eType;
    AX_OPAL_VIDEO_ENCODER_ATTR_T stEncoderAttr;
} AX_OPAL_VIDEO_CHN_ATTR_T;

// algorithm attribute
typedef struct axOPAL_ALGO_ATTR_T {
    AX_U32 nAlgoType;
    AX_CHAR *strDetectModelsPath;
} AX_OPAL_ALGO_ATTR_T;

// sensor color attribute
typedef struct axOPAL_SNS_COLOR_ATTR_T {
    AX_BOOL bManual;
    AX_F32 fBrightness;
    AX_F32 fSharpness;
    AX_F32 fContrast;
    AX_F32 fSaturation;
} AX_OPAL_SNS_COLOR_ATTR_T;

// sensor ldc attribute
typedef struct axOPAL_SNS_LDC_ATTR_T {
    AX_BOOL bEnable;
    AX_BOOL bAspect;
    AX_S16 nXRatio;
    AX_S16 nYRatio;
    AX_S16 nXYRatio;
    AX_S16 nDistortionRatio;
} AX_OPAL_SNS_LDC_ATTR_T;

// sensor dis attribute
typedef struct axOPAL_SNS_DIS_ATTR_T {
    AX_BOOL bEnable;
    AX_BOOL bMotionEst;
    AX_BOOL bMotionShare;
    AX_U8 nDelayFrameNum;
} AX_OPAL_SNS_DIS_ATTR_T;

// sensor ezoom attribute
typedef struct axOPAL_SNS_EZOOM_ATTR_T {
    AX_F32 fRatio;          // 1 - 8
    AX_S16 nCenterOffsetX;  // offset x from center
    AX_S16 nCenterOffsetY;  // offset y from center
} AX_OPAL_SNS_EZOOM_ATTR_T;

// sensor attribute
typedef struct axOPAL_VIDEO_SNS_ATTR_T {
    AX_S32 nWidth;
    AX_S32 nHeight;
    AX_BOOL bFlip;
    AX_BOOL bMirror;
    AX_F32 fFrameRate;
    AX_OPAL_SNS_MODE_E eMode;
    AX_OPAL_SNS_DAYNIGHT_E eDayNight;
    AX_OPAL_SNS_ROTATION_E eRotation;
    AX_OPAL_SNS_COLOR_ATTR_T stColorAttr;
    AX_OPAL_SNS_LDC_ATTR_T stLdcAttr;
    AX_OPAL_SNS_DIS_ATTR_T stDisAttr;
    AX_OPAL_SNS_EZOOM_ATTR_T stEZoomAttr;
} AX_OPAL_VIDEO_SNS_ATTR_T;

// video pipe attribute
typedef struct axOPAL_VIDEO_PIPE_ATTR_T {
    AX_OPAL_VIDEO_CHN_ATTR_T stVideoChnAttr[AX_OPAL_VIDEO_CHN_BUTT];
} AX_OPAL_VIDEO_PIPE_ATTR_T;

// video attribute
typedef struct axOPAL_VIDEO_ATTR_T {
    AX_BOOL bEnable;
    AX_CHAR szSnsConfigPath[AX_OPAL_PATH_LEN];
    AX_OPAL_ALGO_ATTR_T stAlgoAttr;
    AX_OPAL_VIDEO_SNS_ATTR_T stSnsAttr;
    AX_OPAL_VIDEO_PIPE_ATTR_T stPipeAttr;
} AX_OPAL_VIDEO_ATTR_T;

// audio aac encoder attribute
typedef struct axOPAL_AUDIO_AAC_ENCODER_ATTR_T {
    AX_OPAL_AUDIO_AAC_TYPE_E eAacType;
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_E eTransType;
} AX_OPAL_AUDIO_AAC_ENCODER_ATTR_T;

// audio aac decoder attribute
typedef struct axOPAL_AUDIO_AAC_DECODER_ATTR_T {
    AX_OPAL_AUDIO_AAC_TRANS_TYPE_E eTransType;
} AX_OPAL_AUDIO_AAC_DECODER_ATTR_T;

// audio default encoder attribute
typedef struct axOPAL_AUDIO_DEF_ENCODER_ATTR_T {
    AX_U8 nReserved;
} AX_OPAL_AUDIO_DEF_ENCODER_ATTR_T;

// audio default decoder attribute
typedef struct axOPAL_AUDIO_DEF_DECODER_ATTR_T {
    AX_U8 nReserved;
} AX_OPAL_AUDIO_DEF_DECODER_ATTR_T;

// audio automatic gain control attribute
typedef struct axOPAL_AUDIO_AGC_ATTR_T {
    AX_BOOL bEnable;
    AX_OPAL_AUDIO_AGC_MODE_E eAgcMode; /* only support AX_OPAL_AUDIO_AGC_MODE_FIXED_DIGITAL */
    AX_S16 nTargetLv;                  /* [-31 - 0], default is -3 */
    AX_S16 nGain;                      /*0 ~ 90 default 9*/
} AX_OPAL_AUDIO_AGC_ATTR_T;

// audio noise suppression attribute
typedef struct axOPAL_AUDIO_ANS_ATTR_T {
    AX_BOOL bEnable;
    AX_OPAL_AUDIO_ANS_AGGRESSIVENESS_LEVEL_E eLevel;
} AX_OPAL_AUDIO_ANS_ATTR_T;

// audio aec float attribute
typedef struct axOPAL_AUDIO_AEC_FLOAT_ATTR_T {
    AX_OPAL_AUDIO_AEC_SUPPRESSION_LEVEL_E eLevel;
} AX_OPAL_AUDIO_AEC_FLOAT_ATTR_T;

// audio aec fixed attribute
typedef struct axOPAL_AUDIO_AEC_FIXED_ATTR_T {
    AX_OPAL_AUDIO_AEC_ROUTING_MODE_E eMode;
} AX_OPAL_AUDIO_AEC_FIXED_ATTR_T;

// audio acoustic echo canceller attribute
typedef struct axOPAL_AUDIO_AEC_ATTR_T {
    AX_OPAL_AUDIO_AEC_TYPE_E eType;
    union {
        AX_OPAL_AUDIO_AEC_FLOAT_ATTR_T stFloatAttr;
        AX_OPAL_AUDIO_AEC_FIXED_ATTR_T stFixedAttr;
    };
} AX_OPAL_AUDIO_AEC_ATTR_T;

// voice activity detect attribute
typedef struct axOPAL_AUDIO_VAD_ATTR_T {
    AX_BOOL bEnable;
    AX_U32 nVadLevel;                 /*0 ~ 3 default 2*/
} AX_OPAL_AUDIO_VAD_ATTR_T;

// audio capture vqe attribute
typedef struct axOPAL_AUDIO_CAP_VQE_ATTR_T {
    AX_OPAL_AUDIO_AEC_ATTR_T stAecAttr;
    AX_OPAL_AUDIO_ANS_ATTR_T stAnsAttr;
    AX_OPAL_AUDIO_AGC_ATTR_T stAgcAttr;
    AX_OPAL_AUDIO_VAD_ATTR_T stVadAttr;
} AX_OPAL_AUDIO_CAP_VQE_ATTR_T;

// audio play vqe attribute
typedef struct axOPAL_AUDIO_PLAY_VQE_ATTR_T {
    AX_OPAL_AUDIO_ANS_ATTR_T stAnsAttr;
    AX_OPAL_AUDIO_AGC_ATTR_T stAgcAttr;
} AX_OPAL_AUDIO_PLAY_VQE_ATTR_T;

// audio encoder attribute
typedef struct axOPAL_AUDIO_ENCODER_ATTR_T {
    AX_S32 nAOT;
    AX_U32 nBitRate;
    AX_PAYLOAD_TYPE_E eType;
    AX_OPAL_AUDIO_BIT_WIDTH_E eBitWidth;
    AX_OPAL_AUDIO_SOUND_MODE_E eSoundMode;
    AX_OPAL_AUDIO_SAMPLE_RATE_E eSampleRate;
} AX_OPAL_AUDIO_ENCODER_ATTR_T;

// audio capture dev attribute
typedef struct axOPAL_AUDIO_CAP_DEV_ATTR_T {
    AX_U32 nCardId;
    AX_U32 nDeviceId;
    AX_F32 fVolume;
    AX_OPAL_AUDIO_LAYOUT_MODE_E eLayoutMode;
    AX_OPAL_AUDIO_CAP_VQE_ATTR_T stVqeAttr;
} AX_OPAL_AUDIO_CAP_DEV_ATTR_T;

// audio capture pipe attribute
typedef struct axOPAL_AUDIO_CAP_CHN_ATTR_T {
    AX_BOOL bEnable;
    AX_U32 nBitRate;
    AX_OPAL_AUDIO_SOUND_MODE_E eSoundMode;
    AX_PAYLOAD_TYPE_E eType;
    union {
        AX_OPAL_AUDIO_DEF_ENCODER_ATTR_T stDefEncoder;
        AX_OPAL_AUDIO_AAC_ENCODER_ATTR_T stAacEncoder;
    };
} AX_OPAL_AUDIO_CAP_CHN_ATTR_T;

// audio capture pipe attribute
typedef struct axOPAL_AUDIO_CAP_PIPE_ATTR_T {
    AX_OPAL_AUDIO_CAP_CHN_ATTR_T stAudioChnAttr[AX_OPAL_AUDIO_CHN_BUTT];
} AX_OPAL_AUDIO_CAP_PIPE_ATTR_T;

typedef struct axOPAL_AUDIO_CAP_ATTR_T {
    AX_BOOL bEnable;
    AX_OPAL_AUDIO_CAP_DEV_ATTR_T stDevAttr;
    AX_OPAL_AUDIO_CAP_PIPE_ATTR_T stPipeAttr;
} AX_OPAL_AUDIO_CAP_ATTR_T;

// audio play dev attribute
typedef struct axOPAL_AUDIO_PLAY_DEV_ATTR_T {
    AX_U32 nCardId;
    AX_U32 nDeviceId;
    AX_F32 fVolume;
    AX_OPAL_AUDIO_PLAY_VQE_ATTR_T stVqeAttr;
} AX_OPAL_AUDIO_PLAY_DEV_ATTR_T;

// audio play channel attribute
typedef struct axOPAL_AUDIO_PLAY_CHN_ATTR_T {
    AX_BOOL bEnable;
    AX_U32 nBitRate;
    AX_OPAL_AUDIO_SAMPLE_RATE_E eSampleRate;
    AX_OPAL_AUDIO_SOUND_MODE_E eSoundMode;
    AX_PAYLOAD_TYPE_E eType;
    union {
        AX_OPAL_AUDIO_DEF_DECODER_ATTR_T stDefDecoder;
        AX_OPAL_AUDIO_AAC_DECODER_ATTR_T stAacDecoder;
    };
} AX_OPAL_AUDIO_PLAY_CHN_ATTR_T;

// audio play pipe attribute
typedef struct axOPAL_AUDIO_PLAY_PIPE_ATTR_T {
    AX_OPAL_AUDIO_PLAY_CHN_ATTR_T stAudioChnAttr[AX_OPAL_AUDIO_CHN_BUTT];
} AX_OPAL_AUDIO_PLAY_PIPE_ATTR_T;

// audio play attribute
typedef struct axOPAL_AUDIO_PLAY_ATTR_T {
    AX_BOOL bEnable;
    AX_OPAL_AUDIO_PLAY_DEV_ATTR_T stDevAttr;
    AX_OPAL_AUDIO_PLAY_PIPE_ATTR_T stPipeAttr;
} AX_OPAL_AUDIO_PLAY_ATTR_T;

// audio dev common attribute
typedef struct axOPAL_AUDIO_DEV_COMM_ATTR_T {
    AX_OPAL_AUDIO_BIT_WIDTH_E eBitWidth;
    AX_OPAL_AUDIO_SAMPLE_RATE_E eSampleRate;
    AX_U32 nPeriodSize;
} AX_OPAL_AUDIO_DEV_COMM_ATTR_T;

// audio attribute
typedef struct axOPAL_AUDIO_ATTR_T {
    AX_OPAL_AUDIO_DEV_COMM_ATTR_T stDevCommAttr;
    AX_OPAL_AUDIO_CAP_ATTR_T stCapAttr;
    AX_OPAL_AUDIO_PLAY_ATTR_T stPlayAttr;
} AX_OPAL_AUDIO_ATTR_T;

// opal attribute
typedef struct AX_OPAL_ATTR_T {
    AX_CHAR szPipelineConfigPath[AX_OPAL_PATH_LEN];
    AX_CHAR szPoolConfigPath[AX_OPAL_PATH_LEN];
    AX_OPAL_VIDEO_ATTR_T stVideoAttr[AX_OPAL_SNS_ID_BUTT];
    AX_OPAL_AUDIO_ATTR_T stAudioAttr;
} AX_OPAL_ATTR_T;

// hotbalance attribute
typedef struct axOPAL_HOTBALANCE_ATTR_T {
    AX_BOOL bEnable;
} AX_OPAL_HOTBALANCE_ATTR_T;

// video packet
typedef struct axOPAL_VIDEO_PKT_T {
    AX_S32 nSnsId;
    AX_S32 nChnId;
    AX_PAYLOAD_TYPE_E eType;
    AX_OPAL_VIDEO_NALU_TYPE_E eNaluType;
    AX_BOOL bIFrame;
    AX_U8 *pData;
    AX_U32 nDataSize;
    AX_U64 u64Pts;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_VIDEO_PKT_T;

// video frame
typedef struct axOPAL_VIDEO_FRAME_T {
    AX_S32 nSnsId;
    AX_S32 nChnId;
    AX_VIDEO_FRAME_T stFrame;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_VIDEO_FRAME_T;

// buffer
typedef struct axOPAL_BUFFER_T {
    AX_U8 *pData;
    AX_U32 nDataSize;
} AX_OPAL_BUFFER_T;

// point coordinates
typedef struct axOPAL_POINT_T {
    AX_F32 fX;
    AX_F32 fY;
} AX_OPAL_POINT_T;

// polygon coordinates
typedef struct axOPAL_POLYGON_T {
    AX_U32 nPointNum;
    AX_OPAL_POINT_T stPoints[AX_OPAL_POINT_MAX];
} AX_OPAL_POLYGON_T;

// rect coordinates
typedef struct axOPAL_RECT_T {
    AX_F32 fX;
    AX_F32 fY;
    AX_F32 fW;
    AX_F32 fH;
} AX_OPAL_RECT_T;

// smart video coding map param
typedef struct axOPAL_VIDEO_SVC_MAP_PARAM_T {
    AX_S8 nIQp;
    AX_S8 nPQp;
} AX_OPAL_VIDEO_SVC_MAP_PARAM_T;

// smart video coding qp config
typedef struct axOPAL_VIDEO_SVC_QP_CFG_T {
    AX_BOOL bEnable;
    AX_OPAL_VIDEO_SVC_MAP_PARAM_T stQpMap;
} AX_OPAL_VIDEO_SVC_QP_CFG_T;

// smart video coding param
typedef struct axOPAL_VIDEO_SVC_PARAM_T {
    AX_BOOL bEnable;
    AX_BOOL bAbsQp;
    AX_BOOL bSync;
    AX_OPAL_VIDEO_SVC_MAP_PARAM_T stBgQpCfg;
    AX_OPAL_VIDEO_SVC_QP_CFG_T stQpCfg[AX_OPAL_VIDEO_SVC_REGION_TYPE_BUTT];
} AX_OPAL_VIDEO_SVC_PARAM_T;

// smart video coding region item
typedef struct axOPAL_VIDEO_SVC_REGION_ITEM_T {
    AX_OPAL_RECT_T stRect;
    AX_OPAL_VIDEO_SVC_REGION_TYPE_E eRegionType;
} AX_OPAL_VIDEO_SVC_REGION_ITEM_T;

// smart video coding region
typedef struct axOPAL_VIDEO_SVC_REGION_T {
    AX_U64 u64Pts;
    AX_U64 u64SeqNum;
    AX_U32 nItemSize;
    AX_OPAL_VIDEO_SVC_REGION_ITEM_T* pstItems;
} AX_OPAL_VIDEO_SVC_REGION_T;

// algorithm box
typedef struct axOPAL_ALGO_BOX_T {
    // normalized coordinates
    AX_F32 fX;
    AX_F32 fY;
    AX_F32 fW;
    AX_F32 fH;

    // image resolution
    AX_U32 nImgWidth;
    AX_U32 nImgHeight;
} AX_OPAL_ALGO_BOX_T;

// algorithm face pose blur
typedef struct axOPAL_ALGO_FACE_POSE_BLUR_T {
    AX_F32 fPitch;
    AX_F32 fYaw;
    AX_F32 fRoll;
    AX_F32 fBlur;
} AX_OPAL_ALGO_FACE_POSE_BLUR_T;

// algorithm face attribute
typedef struct axOPAL_ALGO_FACE_ATTR_T {
    AX_BOOL bExist;
    AX_U8 nAge;
    /* 0: female 1: male */
    AX_U8 nGender;
    AX_OPAL_ALGO_FACE_POSE_BLUR_T stFacePoseBlur;
    AX_OPAL_ALGO_FACE_RESPIRATOR_TYPE_E eRespirator;
} AX_OPAL_ALGO_FACE_ATTR_T;

// algorithm plate attribute
typedef struct axOPAL_ALGO_PLATE_ATTR_T {
    AX_BOOL bExist;
    AX_BOOL bValid;
    /* AX_CHAR: UTF8*/
    AX_CHAR szPlateCode[AX_OPAL_ALGO_PLATE_CODE_LEN];
    AX_OPAL_ALGO_PLATE_COLOR_TYPE_E ePlateColor;
} AX_OPAL_ALGO_PLATE_ATTR_T;

// algorithm vehicle attribute
typedef struct axOPAL_ALGO_VEHICLE_ATTR_T {
    AX_BOOL bExist;
    AX_OPAL_ALGO_PLATE_ATTR_T stPlateAttr;
} AX_OPAL_ALGO_VEHICLE_ATTR_T;

// algorithm image
typedef struct axOPAL_ALGO_IMG_T {
    AX_BOOL bExist;
    AX_U8 *pData;
    AX_U32 nDataSize;
    AX_U32 nWidth;
    AX_U32 nHeight;
} AX_OPAL_ALGO_IMG_T;

// algorithm hvcfp item
typedef struct axOPAL_ALGO_HVCFP_ITEM_T {
    AX_OPAL_ALGO_HVCFP_TYPE_E eType;
    AX_U64 u64FrameId;
    AX_U64 u64TrackId;
    AX_F32 fConfidence;
    AX_OPAL_ALGO_TRACK_STATUS_E eTrackStatus;
    AX_OPAL_ALGO_BOX_T stBox;
    AX_OPAL_ALGO_IMG_T stImg;
    AX_OPAL_ALGO_IMG_T stPanoramaImg;
    union {
        AX_OPAL_ALGO_VEHICLE_ATTR_T stVehicleAttr;
        AX_OPAL_ALGO_FACE_ATTR_T stFaceAttr;
        AX_OPAL_ALGO_PLATE_ATTR_T stPlateAttr;
    };
} AX_OPAL_ALGO_HVCFP_ITEM_T;

// algorithm hvcfp result
typedef struct axOPAL_ALGO_HVCFP_RESULT_T {
    AX_BOOL bValid;
    AX_S32 nSnsId;
    AX_U64 u64Pts;
    AX_U32 u64FrameId;
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_U32 nBodySize;
    AX_OPAL_ALGO_HVCFP_ITEM_T* pstBodys;
    AX_U32 nVehicleSize;
    AX_OPAL_ALGO_HVCFP_ITEM_T* pstVehicles;
    AX_U32 nCycleSize;
    AX_OPAL_ALGO_HVCFP_ITEM_T* pstCycles;
    AX_U32 nFaceSize;
    AX_OPAL_ALGO_HVCFP_ITEM_T* pstFaces;
    AX_U32 nPlateSize;
    AX_OPAL_ALGO_HVCFP_ITEM_T* pstPlates;
} AX_OPAL_ALGO_HVCFP_RESULT_T;

// algorithm ives item
typedef struct axOPAL_ALGO_IVES_ITEM_T {
    AX_OPAL_ALGO_IVES_TYPE_E eType;
    AX_U64 u64FrameId;
    AX_F32 fConfidence;
    AX_OPAL_ALGO_BOX_T stBox;
    AX_OPAL_ALGO_IMG_T stPanoramaImg;
} AX_OPAL_ALGO_IVES_ITEM_T;

// algorithm ives result
typedef struct axOPAL_ALGO_IVES_RESULT_T {
    AX_BOOL bValid;
    AX_S32 nSnsId;
    AX_U64 u64Pts;
    AX_U64 u64FrameId;
    AX_U32 nMdSize;
    AX_OPAL_ALGO_IVES_ITEM_T* pstMds;
    AX_U32 nOdSize;
    AX_OPAL_ALGO_IVES_ITEM_T* pstOds;
} AX_OPAL_ALGO_IVES_RESULT_T;

// algorithm result
typedef struct axOPAL_ALGO_RESULT_T {
    AX_OPAL_ALGO_HVCFP_RESULT_T stHvcfpResult;
    AX_OPAL_ALGO_IVES_RESULT_T stIvesResult;
    AX_VOID *pUserData;
} AX_OPAL_ALGO_RESULT_T;

// algorithm roi config
typedef struct axOPAL_ALGO_ROI_CONFIG_T {
    AX_BOOL bEnable;
    AX_OPAL_POLYGON_T stPolygon;
} AX_OPAL_ALGO_ROI_CONFIG_T;

// algorithm object fliter config
typedef struct axOPAL_ALGO_HVCFP_FILTER_CONFIG_T {
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_F32 fConfidence;
} AX_OPAL_ALGO_HVCFP_FILTER_CONFIG_T;

// algorithm track size
typedef struct axOPAL_ALGO_TRACK_SIZE_T {
    AX_U8 nTrackHumanSize;
    AX_U8 nTrackVehicleSize;
    AX_U8 nTrackCycleSize;
} AX_OPAL_ALGO_TRACK_SIZE_T;

// algorithm push strategy
typedef struct axOPAL_ALGO_PUSH_STRATEGY_T {
    AX_OPAL_ALGO_PUSH_MODE_E ePushMode;
    AX_U32 nInterval;
    AX_U32 nPushCount;
} AX_OPAL_ALGO_PUSH_STRATEGY_T;

// algorithm panorama
typedef struct axOPAL_ALGO_PANORAMA_T {
    AX_BOOL bEnable;
} AX_OPAL_ALGO_PANORAMA_T;

// algorithm face push filter config
typedef struct axOPAL_ALGO_FACE_PUSH_FILTER_CONFIG_T {
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_OPAL_ALGO_FACE_POSE_BLUR_T stFacePoseBlur;
} AX_OPAL_ALGO_FACE_PUSH_FILTER_CONFIG_T;

// algorithm common push filter config
typedef struct axOPAL_ALGO_COMMON_PUSH_FILTER_CONFIG_T {
    AX_F32 fQuality;
} AX_OPAL_ALGO_COMMON_PUSH_FILTER_CONFIG_T;

// algorithm push filter config
typedef struct {
    union {
        AX_OPAL_ALGO_FACE_PUSH_FILTER_CONFIG_T stFacePushFilterConfig;
        AX_OPAL_ALGO_COMMON_PUSH_FILTER_CONFIG_T stCommonPushFilterConfig;
    };
} AX_OPAL_ALGO_PUSH_FILTER_CONFIG_T;

// algorithm ae roi config
typedef struct axOPAL_ALGO_AE_ROI_CONFIG_T {
    AX_BOOL bEnable;
    AX_U32 nWidth;
    AX_U32 nHeight;
} AX_OPAL_ALGO_AE_ROI_CONFIG_T;

// algorithm hvcfp parameter
typedef struct axOPAL_ALGO_HVCFP_PARAM_T {
    AX_BOOL bEnable;
    AX_BOOL bPushActive;
    AX_U8 nCropEncoderQpLevel;
    AX_U8 nAlgoType;  // readonly
    AX_S32 nSrcFramerate;
    AX_S32 nDstFramerate;
    AX_OPAL_ALGO_ROI_CONFIG_T stRoiConfig;
    AX_OPAL_ALGO_TRACK_SIZE_T stTrackSize;
    AX_OPAL_ALGO_PUSH_STRATEGY_T stPushStrategy;
    AX_OPAL_ALGO_PANORAMA_T stPanoramaConfig;
    AX_OPAL_ALGO_HVCFP_FILTER_CONFIG_T stObjectFliterConfig[AX_OPAL_ALGO_HVCFP_TYPE_BUTT];
    AX_OPAL_ALGO_PUSH_FILTER_CONFIG_T stPushFliterConfig[AX_OPAL_ALGO_HVCFP_TYPE_BUTT];
    AX_OPAL_ALGO_AE_ROI_CONFIG_T stAeRoiConfig[AX_OPAL_ALGO_HVCFP_TYPE_BUTT];
} AX_OPAL_ALGO_HVCFP_PARAM_T;

// algorithm motion region
typedef struct axOPAL_ALGO_MD_REGION_T {
    AX_F32 fThreshold;
    AX_F32 fConfidence;
    AX_OPAL_RECT_T stRect;
} AX_OPAL_ALGO_MD_REGION_T;

// algorithm motion parameter
typedef struct axOPAL_ALGO_MD_PARAM_T {
    AX_BOOL bEnable;
    AX_BOOL bCapture;
    AX_U8 nRegionSize;
    AX_OPAL_ALGO_MD_REGION_T stRegions[AX_OPAL_MAX_ALGO_MD_REGION_COUNT];
} AX_OPAL_ALGO_MD_PARAM_T;

// algorithm occlusion parameter
typedef struct axOPAL_ALGO_OD_PARAM_T {
    AX_BOOL bEnable;
    AX_F32 fThreshold;
    AX_F32 fConfidence;
} AX_OPAL_ALGO_OD_PARAM_T;

// ives parameter
typedef struct axOPAL_ALGO_IVES_PARAM_T {
    AX_S32 nSrcFramerate;
    AX_S32 nDstFramerate;
    AX_OPAL_ALGO_MD_PARAM_T stMdParam;
    AX_OPAL_ALGO_OD_PARAM_T stOdParam;
} AX_OPAL_ALGO_IVES_PARAM_T;

// algorithm parameter
typedef struct axOPAL_ALGO_PARAM_T {
    AX_OPAL_ALGO_HVCFP_PARAM_T stHvcfpParam;
    AX_OPAL_ALGO_IVES_PARAM_T stIvesParam;
} AX_OPAL_ALGO_PARAM_T;

// osd picture attribute
typedef struct axOPAL_OSD_PIC_ATTR_T {
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_OPAL_OSD_PIC_SOURCE_TYPE_E eSource;
    union {
        AX_CHAR *pstrFileName;
        AX_OPAL_BUFFER_T stBuffer;
    };
} AX_OPAL_OSD_PIC_ATTR_T;

// osd string attribute
typedef struct axOPAL_OSD_STR_ATTR_T {
    AX_S32 nFontSize;
    AX_CHAR *pStr;
    AX_BOOL bInvEnable;
    AX_U32 nColorInv;
} AX_OPAL_OSD_STR_ATTR_T;

// osd datetime attribute
typedef struct axOPAL_OSD_DATETIME_ATTR_T {
    AX_S32 nFontSize;
    AX_OPAL_OSD_DATETIME_FORMAT_E eFormat;
    AX_BOOL bInvEnable;
    AX_U32 nColorInv;
} AX_OPAL_OSD_DATETIME_ATTR_T;

// osd privacy attribute
typedef struct axOPAL_OSD_PRIVACY_ATTR_T {
    AX_OPAL_OSD_PRIVACY_TYPE_E eType;
    AX_U32 nLineWidth;
    AX_BOOL bSolid;
    AX_OPAL_POINT_T stPoints[AX_OPAL_OSD_PIVACY_POINT_MAX];
} AX_OPAL_OSD_PRIVACY_ATTR_T;

// osd attibute
typedef struct axOPAL_OSD_ATTR_T {
    AX_BOOL bEnable;
    AX_OPAL_OSD_ALIGN_E eAlign;
    AX_S32 nXBoundary;
    AX_S32 nYBoundary;
    AX_U32 nARGB;
    AX_OPAL_OSD_TYPE_E eType;
    union {
        AX_OPAL_OSD_PIC_ATTR_T stPicAttr;
        AX_OPAL_OSD_STR_ATTR_T stStrAttr;
        AX_OPAL_OSD_DATETIME_ATTR_T stDatetimeAttr;
        AX_OPAL_OSD_PRIVACY_ATTR_T stPrivacyAttr;
    };
} AX_OPAL_OSD_ATTR_T;

// audio
// audio packet
typedef struct axOPAL_AUDIO_PKT_T {
    AX_U32 nBitRate;
    AX_PAYLOAD_TYPE_E eType;
    AX_OPAL_AUDIO_BIT_WIDTH_E eBitWidth;
    AX_OPAL_AUDIO_SAMPLE_RATE_E eSampleRate;
    AX_OPAL_AUDIO_SOUND_MODE_E eSoundMode;
    AX_U8 *pData;
    AX_U32 nDataSize;
    AX_U64 u64Pts;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_AUDIO_PKT_T;

// audio frame
typedef struct axOPAL_AUDIO_FRAME_T {
    AX_PAYLOAD_TYPE_E eType;
    AX_OPAL_AUDIO_BIT_WIDTH_E eBitWidth;
    AX_OPAL_AUDIO_SAMPLE_RATE_E eSampleRate;
    AX_U32 nChnCnt;
    AX_U8 *pData;
    AX_U32 nDataSize;
    AX_U64 u64Pts;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_AUDIO_FRAME_T;

// audio file play result
typedef struct axOPAL_AUDIO_PLAY_FILE_RESULT_T {
    AX_PAYLOAD_TYPE_E eType;
    AX_OPAL_AUDIO_PLAY_FILE_STATUS_E eStatus;
    AX_CHAR *pstrFileName;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_AUDIO_PLAY_FILE_RESULT_T;

// audio detect result
typedef struct axOPAL_AUDIO_DETECT_RESULT_T {
    AX_F32 fDb;
    AX_VOID *pUserData;
    AX_VOID *pPrivateData;
} AX_OPAL_AUDIO_DETECT_RESULT_T;

// video packet callback
typedef AX_VOID (*AX_OPAL_VIDEO_PKT_CALLBACK)(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_VIDEO_PKT_T* pstPkt);

// video algorithm callback
typedef AX_VOID (*AX_OPAL_VIDEO_ALGO_CALLBACK)(AX_S32 nSnsId, const AX_OPAL_ALGO_RESULT_T* pstResult);

// video sensor soft photosensitivity callback
typedef AX_VOID (*AX_OPAL_VIDEO_SNS_SOFTPHOTOSENSITIVITY_CALLBACK)(AX_S32 nSnsId, const AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_RESULT_T* pstResult);

// audio
typedef AX_VOID (*AX_OPAL_AUDIO_FRAME_CALLBACK)(const AX_OPAL_AUDIO_FRAME_T* pstFrame);
typedef AX_VOID (*AX_OPAL_AUDIO_PKT_CALLBACK)(AX_S32 nChnId, const AX_OPAL_AUDIO_PKT_T* pstPkt);
typedef AX_VOID (*AX_OPAL_AUDIO_PLAYFILERESULT_CALLBACK)(AX_S32 nChnId, const AX_OPAL_AUDIO_PLAY_FILE_RESULT_T* pstResult);

#ifdef __cplusplus
}
#endif

#endif // _AX_OPAL_TYPE_H_

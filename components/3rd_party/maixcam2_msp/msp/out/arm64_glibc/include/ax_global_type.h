/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_GLOBAL_TYPE_H_
#define _AX_GLOBAL_TYPE_H_
#include "ax_base_type.h"

#define DEF_ALL_MOD_GRP_MAX         (32)
#define DEF_ALL_MOD_CHN_MAX         (16)
#define AX_LINK_DEST_MAXNUM         (6)
#define AX_MAX_COLOR_COMPONENT      (3)         /* VENC support Y/U/V three planes come from external input */
#define AX_MAX_COMPRESS_LOSSY_LEVEL (10)
#define AX_INVALID_ID               (-1U)

#ifndef AX_SUCCESS
#define AX_SUCCESS                  (0)
#endif

#define AX_FRAME_RATE(fps)                      ((fps) * 1000)

#define AX_INVALID_FRMRATE                      (0.0f)

typedef struct axFRAME_RATE_CTRL_T {
    AX_F32  fSrcFrameRate;
    AX_F32  fDstFrameRate;
} AX_FRAME_RATE_CTRL_T;

/* don't change this */
typedef enum {
    SYS_LOG_MIN         = -1,
    SYS_LOG_EMERGENCY   = 0,
    SYS_LOG_ALERT       = 1,
    SYS_LOG_CRITICAL    = 2,
    SYS_LOG_ERROR       = 3,
    SYS_LOG_WARN        = 4,
    SYS_LOG_NOTICE      = 5,
    SYS_LOG_INFO        = 6,
    SYS_LOG_DEBUG       = 7,
    SYS_LOG_MAX
} AX_LOG_LEVEL_E;

typedef enum {
    SYS_LOG_TARGET_MIN = 0,
    SYS_LOG_TARGET_STDERR = 1,
    SYS_LOG_TARGET_SYSLOG = 2,
    SYS_LOG_TARGET_NULL   = 3,
    SYS_LOG_TARGET_MAX
} AX_LOG_TARGET_E;

typedef enum {
    NONE_CHIP_TYPE = 0x0,
    AX620Q_CHIP = 0x1,
    AX620QX_CHIP = 0x2,
    AX630C_CHIP = 0x4,
    AX631_CHIP = 0x5,
    AX620QZ_CHIP = 0x6,
    AX620QP_CHIP = 0x7,
    AX620E_CHIP_MAX
} AX_CHIP_TYPE_E;

typedef enum
{
    PT_PCMU             = 0,
    PT_1016             = 1,
    PT_G721             = 2,
    PT_GSM              = 3,
    PT_G723             = 4,
    PT_DVI4_8K          = 5,
    PT_DVI4_16K         = 6,
    PT_LPC              = 7,
    PT_PCMA             = 8,
    PT_G722             = 9,
    PT_S16BE_STEREO     = 10,
    PT_S16BE_MONO       = 11,
    PT_QCELP            = 12,
    PT_CN               = 13,
    PT_MPEGAUDIO        = 14,
    PT_G728             = 15,
    PT_DVI4_3           = 16,
    PT_DVI4_4           = 17,
    PT_G729             = 18,
    PT_G711A            = 19,
    PT_G711U            = 20,
    PT_G726             = 21,
    PT_G729A            = 22,
    PT_LPCM             = 23,
    PT_CelB             = 25,
    PT_JPEG             = 26,
    PT_CUSM             = 27,
    PT_NV               = 28,
    PT_PICW             = 29,
    PT_CPV              = 30,
    PT_H261             = 31,
    PT_MPEGVIDEO        = 32,
    PT_MPEG2TS          = 33,
    PT_H263             = 34,
    PT_SPEG             = 35,
    PT_MPEG2VIDEO       = 36,
    PT_AAC              = 37,
    PT_WMA9STD          = 38,
    PT_HEAAC            = 39,
    PT_PCM_VOICE        = 40,
    PT_PCM_AUDIO        = 41,
    PT_AACLC            = 42,
    PT_MP3              = 43,
    PT_ADPCMA           = 49,
    PT_AEC              = 50,
    PT_X_LD             = 95,
    PT_H264             = 96,
    PT_D_GSM_HR         = 200,
    PT_D_GSM_EFR        = 201,
    PT_D_L8             = 202,
    PT_D_RED            = 203,
    PT_D_VDVI           = 204,
    PT_D_BT656          = 220,
    PT_D_H263_1998      = 221,
    PT_D_MP1S           = 222,
    PT_D_MP2P           = 223,
    PT_D_BMPEG          = 224,
    PT_MP4VIDEO         = 230,
    PT_MP4AUDIO         = 237,
    PT_VC1              = 238,
    PT_JVC_ASF          = 255,
    PT_D_AVI            = 256,
    PT_DIVX3            = 257,
    PT_AVS              = 258,
    PT_REAL8            = 259,
    PT_REAL9            = 260,
    PT_VP6              = 261,
    PT_VP6F             = 262,
    PT_VP6A             = 263,
    PT_SORENSON         = 264,
    PT_H265             = 265,
    PT_VP8              = 266,
    PT_MVC              = 267,
    PT_PNG              = 268,
    PT_AVS2             = 269,
    PT_VP7              = 270,
    PT_VP9              = 271,
    PT_AMR              = 1001,
    PT_MJPEG            = 1002,
    PT_AMRWB            = 1003,
    PT_PRORES           = 1006,
    PT_OPUS             = 1007,
    PT_BUTT
} AX_PAYLOAD_TYPE_E;

typedef enum {
    AX_VSCAN_FORMAT_RASTER = 0,       /* video raster scan mode */
    AX_VSCAN_FORMAT_BUTT
} AX_VSCAN_FORMAT_E;

typedef enum
{
    AX_COMPRESS_MODE_NONE = 0,   /* no compress */
    AX_COMPRESS_MODE_LOSSLESS,
    AX_COMPRESS_MODE_LOSSY,
    AX_COMPRESS_MODE_BUTT
} AX_COMPRESS_MODE_E;

typedef struct axFRAME_COMPRESS_INFO_T {
    AX_COMPRESS_MODE_E enCompressMode;
    AX_U32    u32CompressLevel;
} AX_FRAME_COMPRESS_INFO_T;

typedef enum axDYNAMIC_RANGE_E
{
    AX_DYNAMIC_RANGE_SDR8 = 0,
    AX_DYNAMIC_RANGE_SDR10,
    AX_DYNAMIC_RANGE_HDR10,
    AX_DYNAMIC_RANGE_HLG,
    AX_DYNAMIC_RANGE_SLF,
    AX_DYNAMIC_RANGE_XDR,
    AX_DYNAMIC_RANGE_BUTT
} AX_DYNAMIC_RANGE_E;

typedef enum axCOLOR_GAMUT_E
{
    AX_COLOR_GAMUT_BT601 = 0,
    AX_COLOR_GAMUT_BT709,
    AX_COLOR_GAMUT_BT2020,
    AX_COLOR_GAMUT_USER,
    AX_COLOR_GAMUT_BUTT
} AX_COLOR_GAMUT_E;

typedef enum
{
    AX_ROTATION_0   = 0,
    AX_ROTATION_90  = 1,
    AX_ROTATION_180 = 2,
    AX_ROTATION_270 = 3,
    AX_ROTATION_BUTT
} AX_ROTATION_E;

typedef enum
{
    AX_FORMAT_INVALID                               = -1,

    /* YUV400 8 bit */
    AX_FORMAT_YUV400                                = 0x0,      /* Y...          */

    /* YUV420 8 bit */
    AX_FORMAT_YUV420_PLANAR                         = 0x1,      /* YYYY... UUUU... VVVV...   I420/YU12  */
    AX_FORMAT_YUV420_PLANAR_VU                      = 0x2,      /* YYYY... VVVV... UUUU...  YV12 */
    AX_FORMAT_YUV420_SEMIPLANAR                     = 0x3,      /* YYYY... UVUVUV...       NV12  */
    AX_FORMAT_YUV420_SEMIPLANAR_VU                  = 0x4,      /* YYYY... VUVUVU...      NV21  */

    /* YUV422 8 bit */
    AX_FORMAT_YUV422_PLANAR                         = 0x8,      /* YYYY... UUUU... VVVV...   I422  */
    AX_FORMAT_YUV422_PLANAR_VU                      = 0x9,      /* YYYY... VVVV... UUUU...  YV16 */
    AX_FORMAT_YUV422_SEMIPLANAR                     = 0xA,      /* YYYY... UVUVUV...       NV61  */
    AX_FORMAT_YUV422_SEMIPLANAR_VU                  = 0xB,      /* YYYY... VUVUVU...      NV16  */
    AX_FORMAT_YUV422_INTERLEAVED_YUVY               = 0xC,      /* YUVYYUVY...           YUVY   */
    AX_FORMAT_YUV422_INTERLEAVED_YUYV               = 0xD,      /* YUYVYUYV...           YUYV   */
    AX_FORMAT_YUV422_INTERLEAVED_UYVY               = 0xE,      /* UYVYUYVY...           UYVY   */
    AX_FORMAT_YUV422_INTERLEAVED_VYUY               = 0xF,      /* VYUYVYUY...           VYUY   */
    AX_FORMAT_YUV422_INTERLEAVED_YVYU               = 0x10,     /* VYUYVYUY...           YVYU   */

    /* YUV444 8 bit */
    AX_FORMAT_YUV444_PLANAR                         = 0x14,     /* YYYY... UUUU... VVVV...   I444  */
    AX_FORMAT_YUV444_PLANAR_VU                      = 0x15,     /* YYYY... VVVV... UUUU...  YV24 */
    AX_FORMAT_YUV444_SEMIPLANAR                     = 0x16,     /* YYYY... UVUVUV...       NV24  */
    AX_FORMAT_YUV444_SEMIPLANAR_VU                  = 0x17,     /* YYYY... VUVUVU...      NV42  */
    AX_FORMAT_YUV444_PACKED                         = 0x18,          /* YUV YUV YUV ...          */

    /* YUV 10 bit */
    AX_FORMAT_YUV400_10BIT                          = 0x20,
    AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B    = 0x24,     /* YYYY... UUUU... VVVV... , 4 Y pixels in 5 bytes, UV packed  */
    AX_FORMAT_YUV420_PLANAR_10BIT_I010              = 0x25,     /*  16 bit pixel, low 10bits valid, high 6 bits invalid */
    AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010       = 0x28,     /* YYYY... UVUVUV... ,  Y/U/V 4 pixels in 5 bytes  */
    AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010          = 0x2A,     /* 16 bit pixel, high 10bits valid, low 6 bits invalid */
    AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016          = 0x2C,     /* 16 bit pixel, low 10bits valid, high 6 bits invalid */
    AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016          = 0x2E,     /* 16 bit pixel, high 10bits valid, low 6 bits invalid */
    AX_FORMAT_YUV420_SEMIPLANAR_10BIT_12P16B        = 0x2F,     /* 12 pixels in 16bytes, low 120bits valid, high 8 bits invalid */
    AX_FORMAT_YUV444_PACKED_10BIT_P010              = 0x30,     /* YUV YUV YUV ... , 16 bit pixel, high 10bits valid, low 6 bits invalid  */
    AX_FORMAT_YUV444_PACKED_10BIT_P101010           = 0x32,     /* YUV YUV YUV ... , 4 pixels storage in 5 bytes */
    AX_FORMAT_YUV422_SEMIPLANAR_10BIT_P101010       = 0x33,     /* YYYY... UVUVUV... ,  Y/U/V 4 pixels in 5 bytes */
    AX_FORMAT_YUV422_SEMIPLANAR_10BIT_P010          = 0x34,     /* 16 bit pixel, high 10bits valid, low 6 bits invalid */

    /* BAYER RAW */
    AX_FORMAT_BAYER_RAW_8BPP                        = 0x80,
    AX_FORMAT_BAYER_RAW_10BPP                       = 0x81,
    AX_FORMAT_BAYER_RAW_12BPP                       = 0x82,
    AX_FORMAT_BAYER_RAW_14BPP                       = 0x83,
    AX_FORMAT_BAYER_RAW_16BPP                       = 0x84,
    AX_FORMAT_BAYER_RAW_10BPP_PACKED                = 0x85,
    AX_FORMAT_BAYER_RAW_12BPP_PACKED                = 0x86,
    AX_FORMAT_BAYER_RAW_14BPP_PACKED                = 0x87,

    /* RGB Format */
    AX_FORMAT_RGB565                                = 0xA0,      /* BBRBGR..., RGB565 16bpp */
    AX_FORMAT_RGB888                                = 0xA1,      /* BBRBGR..., RGB888 24bpp */
    AX_FORMAT_KRGB444                               = 0xA2,
    AX_FORMAT_KRGB555                               = 0xA3,
    AX_FORMAT_KRGB888                               = 0xA4,
    AX_FORMAT_BGR888                                = 0xA5,      /* RGBRGRB..., BGR888 32bpp */
    AX_FORMAT_BGR565                                = 0xA6,      /* RGBRGRB..., BGR565 16bpp */

    AX_FORMAT_ARGB4444                              = 0xC5,      /* BGRABGRA..., ARGB4444 16bpp */
    AX_FORMAT_ARGB1555                              = 0xC6,      /* BGRABGRA..., ARGB1555 16bpp */
    AX_FORMAT_ARGB8888                              = 0xC7,      /* BGRABGRA..., ARGB8888 32bpp */
    AX_FORMAT_ARGB8565                              = 0xC8,      /* BGRABGRA..., ARGB8565 24bpp */
    AX_FORMAT_RGBA8888                              = 0xC9,      /* ABGRABGR..., RGBA8888 32bpp */
    AX_FORMAT_RGBA5551                              = 0xCA,      /* ABGRABGR..., RGBA5551 16bpp */
    AX_FORMAT_RGBA4444                              = 0xCB,      /* ABGRABGR..., RGBA4444 16bpp */
    AX_FORMAT_RGBA5658                              = 0xCC,      /* ABGRABGR..., RGBA5658 24bpp */
    AX_FORMAT_ABGR4444                              = 0xCD,      /* RGBARGBA..., ABGR4444 16bpp */
    AX_FORMAT_ABGR1555                              = 0xCE,      /* RGBARGBA..., ABGR1555 16bpp */
    AX_FORMAT_ABGR8888                              = 0xCF,      /* RGBARGBA..., ABGR8888 32bpp */
    AX_FORMAT_ABGR8565                              = 0xD0,      /* RGBARGBA..., ABGR8565 24bpp */
    AX_FORMAT_BGRA8888                              = 0xD1,      /* ARGBARGB..., BGRA8888 32bpp */
    AX_FORMAT_BGRA5551                              = 0xD2,      /* ARGBARGB..., BGRA5551 16bpp */
    AX_FORMAT_BGRA4444                              = 0xD3,      /* ARGBARGB..., BGRA4444 16bpp */
    AX_FORMAT_BGRA5658                              = 0xD4,      /* ARGBARGB..., BGRA5658 24bpp */

    AX_FORMAT_BITMAP                                = 0xE0,

    AX_FORMAT_MAX
} AX_IMG_FORMAT_E;

typedef enum {
    AX_FRM_FLG_NONE  = 0x0,
    AX_FRM_FLG_USR_PIC  = (0x1 << 0), /* for vdec user picture */
    AX_FRM_FLG_FR_CTRL  = (0x1 << 1), /* for vo frame ctrl */
    AX_FRM_FLG_BUTT
} AX_FRAME_FLAG_E;

typedef enum
{
    AX_ITP_OFFLINE_VPP = 0,
    AX_GDC_ONLINE_VPP = 1,
    AX_ITP_ONLINE_VPP = 2,
    AX_VIN_IVPS_MODE_BUTT
} AX_VIN_IVPS_MODE_E;

typedef struct axVIDEO_FRAME_T {
    AX_U32              u32Width;
    AX_U32              u32Height;
    AX_IMG_FORMAT_E     enImgFormat;
    AX_VSCAN_FORMAT_E   enVscanFormat;
    AX_FRAME_COMPRESS_INFO_T  stCompressInfo;
    AX_DYNAMIC_RANGE_E  stDynamicRange;
    AX_COLOR_GAMUT_E    stColorGamut;

    AX_U32              u32PicStride[AX_MAX_COLOR_COMPONENT];
    AX_U32              u32ExtStride[AX_MAX_COLOR_COMPONENT];

    AX_U64              u64PhyAddr[AX_MAX_COLOR_COMPONENT];          /* frame physics address*/
    AX_U64              u64VirAddr[AX_MAX_COLOR_COMPONENT];          /* frame virtual address*/
    AX_U64              u64ExtPhyAddr[AX_MAX_COLOR_COMPONENT];
    AX_U64              u64ExtVirAddr[AX_MAX_COLOR_COMPONENT];
    AX_U32              u32HeaderSize[AX_MAX_COLOR_COMPONENT];    /* frame header size*/
    AX_U32              u32BlkId[AX_MAX_COLOR_COMPONENT];

    AX_S16              s16CropX;
    AX_S16              s16CropY;
    AX_S16              s16CropWidth;
    AX_S16              s16CropHeight;

    AX_U32              u32TimeRef;
    AX_U64              u64PTS;                 /* Payload TimeStamp */
    AX_U64              u64SeqNum;              /* input frame sequence number */
    AX_U64              u64UserData;            /* Reserved for user, sdk do not use */

    AX_U64              u64PrivateData;         /* SDK reserved, user do not use */
    /* FRAME_FLAG_E, can be OR operation.
     * Can only be assigned as a member of the AX_FRAME_FLAG_E enum.
     */
    AX_U32              u32FrameFlag;

    AX_U32              u32FrameSize;           /* FRAME Size, for isp raw and yuv. */
} AX_VIDEO_FRAME_T;

typedef enum
{
    AX_ID_MIN      = 0x00,
    AX_ID_ISP      = 0x01,
    AX_ID_CE       = 0x02,
    AX_ID_VO       = 0x03,
    AX_ID_VDSP     = 0x04,
    AX_ID_EFUSE    = 0x05,
    AX_ID_NPU      = 0x06,
    AX_ID_VENC     = 0x07,
    AX_ID_VDEC     = 0x08,
    AX_ID_JENC     = 0x09,
    AX_ID_JDEC     = 0x0a,
    AX_ID_SYS      = 0x0b,
    AX_ID_AENC     = 0x0c,
    AX_ID_IVPS     = 0x0d,
    AX_ID_MIPI     = 0x0e,
    AX_ID_ADEC     = 0x0f,
    AX_ID_DMA      = 0x10,
    AX_ID_VIN      = 0x11,
    AX_ID_USER     = 0x12,
    AX_ID_IVES     = 0x13,
    AX_ID_SKEL     = 0x14,
    AX_ID_IVE      = 0x15,
    AX_ID_AVS      = 0x16,
    AX_ID_AVSCALI  = 0x17,
    AX_ID_AUDIO    = 0x1a,
    AX_ID_ALGO     = 0x1b,
    AX_ID_ENGINE   = 0x1c,
    AX_ID_ACODEC   = 0x1f,
    AX_ID_AI       = 0x20,
    AX_ID_AO       = 0x21,
    AX_ID_SENSOR   = 0x22,
    AX_ID_NT       = 0x23,
    AX_ID_TDP      = 0X24,
    AX_ID_VPP      = 0X25,
    AX_ID_GDC      = 0x27,
    AX_ID_BASE     = 0x28,
    AX_ID_THERMAL  = 0x29,
    AX_ID_3A_AE    = 0x2a,
    AX_ID_3A_AWB   = 0x2b,
    AX_ID_3A_AF    = 0x2c,
    AX_ID_AXGZIPD  = 0x2d,
    /* reserve */
    AX_ID_RESERVE  = 0x2e,
    AX_ID_BUTT,
    /* for customer*/
    AX_ID_CUST_MIN = 0x80, /* 128 */
    AX_ID_MAX      = 0xFF  /* 255 */
} AX_MOD_ID_E;

typedef enum
{
    AX_UNLINK_MODE = 0,
    AX_LINK_MODE = 1,
} AX_LINK_MODE_E;

typedef enum axAUDIO_BIT_WIDTH_E {
    AX_AUDIO_BIT_WIDTH_8   = 0,             /* 8bit width */
    AX_AUDIO_BIT_WIDTH_16  = 1,             /* 16bit width*/
    AX_AUDIO_BIT_WIDTH_24  = 2,             /* 24bit width*/
    AX_AUDIO_BIT_WIDTH_32  = 3,   /* 32bit width*/
    AX_AUDIO_BIT_WIDTH_BUTT,
} AX_AUDIO_BIT_WIDTH_E;

typedef enum axAUDIO_SOUND_MODE_E {
    AX_AUDIO_SOUND_MODE_MONO   = 0,         /*mono*/
    AX_AUDIO_SOUND_MODE_STEREO = 1,         /*stereo*/
    AX_AUDIO_SOUND_MODE_BUTT
} AX_AUDIO_SOUND_MODE_E;

typedef struct axAUDIO_FRAME_T {
    AX_AUDIO_BIT_WIDTH_E   enBitwidth;      /*audio frame bitwidth*/
    AX_AUDIO_SOUND_MODE_E  enSoundmode;     /*audio frame momo or stereo mode*/
    AX_U8  *u64VirAddr;
    AX_U64  u64PhyAddr;
    AX_U64  u64TimeStamp;                   /*audio frame timestamp*/
    AX_U32  u32Seq;                         /*audio frame seq*/
    AX_U32  u32Len;                         /*data lenth in frame*/
    AX_U32  u32PoolId[2];
    AX_BOOL bEof;
    AX_U32 u32BlkId;
} AX_AUDIO_FRAME_T;

typedef struct axAUDIO_FRAME_INFO_T {
    AX_AUDIO_FRAME_T  stAFrame;
    AX_MOD_ID_E         enModId;
    AX_BOOL             bEndOfStream;
} AX_AUDIO_FRAME_INFO_T;

typedef struct axVIDEO_FRAME_INFO_T {
    AX_VIDEO_FRAME_T    stVFrame;
    AX_MOD_ID_E         enModId;
    AX_BOOL             bEndOfStream;
} AX_VIDEO_FRAME_INFO_T;


typedef enum {
    AX_NOTIFY_EVENT_SLEEP   = 0,
    AX_NOTIFY_EVENT_WAKEUP  = 1,
    AX_NOTIFY_EVENT_MAX
} AX_NOTIFY_EVENT_E;

typedef enum {
    AX_SYS_CLK_HIGH_MODE             = 0,
    AX_SYS_CLK_HIGH_HOTBALANCE_MODE  = 1,
    AX_SYS_CLK_MID_MODE              = 2,
    AX_SYS_CLK_MID_HOTBALANCE_MODE   = 3,
    AX_SYS_CLK_MAX_MODE              = 4,
} AX_SYS_CLK_LEVEL_E;

typedef enum {
    AX_CPU_CLK_ID       = 0,
    AX_BUS_CLK_ID       = 1,
    AX_NPU_CLK_ID       = 2,
    AX_ISP_CLK_ID       = 3,
    AX_MM_CLK_ID        = 4,
    AX_VPU_CLK_ID       = 5,
    AX_SYS_CLK_MAX_ID   = 6,
} AX_SYS_CLK_ID_E;

typedef struct axMOD_INFO_T {
    AX_MOD_ID_E enModId;
    AX_S32 s32GrpId;
    AX_S32 s32ChnId;
} AX_MOD_INFO_T;

typedef struct axLINK_DEST_S{
    AX_U32 u32DestNum;
    AX_MOD_INFO_T astDestMod[AX_LINK_DEST_MAXNUM];
} AX_LINK_DEST_T;


typedef enum {
    AX_MEMORY_SOURCE_CMM  = 0,
    AX_MEMORY_SOURCE_POOL = 1,
    AX_MEMORY_SOURCE_OS   = 2,
    AX_MEMORY_SOURCE_BUTT,
} AX_MEMORY_SOURCE_E;

typedef struct {
    AX_U64 u64PhyAddr;
    AX_VOID *pVirAddr;
} AX_MEMORY_ADDR_T;

/* OSD attribute extend */
typedef struct axOSD_BMP_ATTR_T {
    AX_U16 u16Alpha;
    AX_IMG_FORMAT_E enRgbFormat;

    AX_U8 *pBitmap; /* pointer to OSD template */
    AX_U64 u64PhyAddr; /* physical address of OSD template */
    AX_U32 u32BmpWidth; /* template width */
    AX_U32 u32BmpHeight; /* template height */

    AX_U32 u32DstXoffset; /* where to overlay, x0 */
    AX_U32 u32DstYoffset; /* where to overlay, y0 */

    /* the below variables are only for bitmap-1 format */
    AX_U32 u32Color; /* RW; range: [0, 0xffffff]; color RGB888; 0xRRGGBB */
    AX_BOOL bColorInv; /* RW; range: [0, 1]; whether use background color or not */
    AX_U32 u32ColorInv; /* RW; range: [0, 0xffffff]; inverse color RGB888; 0xRRGGBB */
    AX_U32 u32ColorInvThr; /* RW; range: [0, 0xffffff]; threshold of color difference with background; 0xRRGGBB */
} AX_OSD_BMP_ATTR_T;

typedef enum {
    AX_ERR_INVALID_MODID        = 0x01, /* invalid module id */
    AX_ERR_INVALID_DEVID        = 0x02, /* invalid device id */
    AX_ERR_INVALID_GRPID        = 0x03, /* invalid group id */
    AX_ERR_INVALID_CHNID        = 0x04, /* invalid channel id */
    AX_ERR_INVALID_PIPEID       = 0x05, /* invalid pipe id */
    AX_ERR_INVALID_STITCHGRPID  = 0x06, /* invalid stitch group id */
    /*reserved*/
    AX_ERR_ILLEGAL_PARAM        = 0x0A, /* at lease one input value is out of range */
    AX_ERR_NULL_PTR             = 0x0B, /* at lease one input pointer is null */
    AX_ERR_BAD_ADDR             = 0x0C, /* at lease one input address is invalid */
    /*reserved*/
    AX_ERR_SYS_NOTREADY         = 0x10, /* a driver is required but not loaded */
    AX_ERR_BUSY                 = 0x11, /* a resource is busy, probably locked by other users */
    AX_ERR_NOT_INIT             = 0x12, /* module is not initialized */
    AX_ERR_NOT_CONFIG           = 0x13, /* module is not configured */
    AX_ERR_NOT_SUPPORT          = 0x14, /* requested function is not supported on this platform */
    AX_ERR_NOT_PERM             = 0x15, /* requested operation is not permitted in this state */
    AX_ERR_EXIST                = 0x16, /* target object already exists */
    AX_ERR_UNEXIST              = 0x17, /* target object does not exist */
    AX_ERR_NOMEM                = 0x18, /* failed to allocate memory from heap */
    AX_ERR_NOBUF                = 0x19, /* failed to borrow buffer from pool */
    AX_ERR_NOT_MATCH            = 0x1A, /* inconsistent parameter configuration between interfaces */
    /*reserved*/
    AX_ERR_BUF_EMPTY            = 0x20, /* buffer contains no data */
    AX_ERR_BUF_FULL             = 0x21, /* buffer contains fresh data */
    AX_ERR_QUEUE_EMPTY          = 0x22, /* failed to read as queue is empty */
    AX_ERR_QUEUE_FULL           = 0x23, /* failed to write as queue is full */
    /*reserved*/
    AX_ERR_TIMED_OUT            = 0x27, /* operation timeout */
    AX_ERR_FLOW_END             = 0x28, /* END signal detected in data stream, processing terminated */
    AX_ERR_UNKNOWN              = 0x29, /* unexpected failure, please contact manufacturer support */
    AX_ERR_OS_FAIL              = 0x30, /* os failure, please contact manufacturer support */

    /*reserved*/
    AX_ERR_BUTT                 = 0x7F, /* maxium code, private error code of all modules
                                        ** must be greater than it */
} AX_ERR_CODE_E;

/******************************************************************************
|----------------------------------------------------------------|
||   FIXED   |   MOD_ID    | SUB_MODULE_ID |   ERR_ID            |
|----------------------------------------------------------------|
|<--8bits----><----8bits---><-----8bits---><------8bits------->|
******************************************************************************/
#define AX_DEF_ERR( module, sub_module, errid) \
    ((AX_S32)( (0x80000000L) | ((module) << 16 ) | ((sub_module)<<8) | (errid) ))


typedef struct {
    AX_S16 nX;
    AX_S16 nY;
} AX_POINT_T;

typedef struct {
    AX_S16 nX;
    AX_S16 nY;
    AX_U16 nW;
    AX_U16 nH;
} AX_BOX_T;

typedef enum {
    AX_COORD_ABS = 0,
    AX_COORD_RATIO,           /* ratio mode: nX,nY:[0, 999]; nW,nH:[1, 1000] */
    AX_COORD_BUTT
} AX_COORD_E;

typedef struct {
    AX_BOOL    bEnable;      /* RW; range: [0, 1]; whether use background color or not; */
    AX_U32     nBgColor;     /* RW; range: [0, 0xffffff]; background color RGB888; */
} AX_BGCOLOR_T;

typedef struct {
    AX_BOOL bEnable;
    AX_BOOL bInv;      /* RW; 0: winin threshold, 1: outside threshold */
    AX_U32  nKeyLow;   /* RW; min value of color key 0xRRGGBB */
    AX_U32  nKeyHigh;  /* RW; max value of color key; 0xRRGGBB */
} AX_COLORKEY_T;

typedef struct {
    AX_U32   nColor;        /* RW; range: [0, 0xffffff]; color RGB888; 0xRRGGBB */
    AX_BOOL  bColorInv;     /* RW; range: [0, 1]; whether use background color inv or not */
    AX_U32   nColorInv;     /* RW; range: [0, 0xffffff]; inverse color RGB888; 0xRRGGBB */
    AX_U32   nColorInvThr;  /* RW; range: [0, 0xffffff]; threshold of color difference with background; 0xRRGGBB */
} AX_BITCOLOR_T;

typedef struct {
    AX_BOOL             bEnable;
    AX_U16              nWidth;
    AX_U16              nHeight;
    AX_U32              nStride;
    AX_IMG_FORMAT_E     eFormat;
    AX_U64              u64PhyAddr[2];

    AX_U8               nAlpha;
    AX_POINT_T          tOffset;
    AX_COLORKEY_T       tColorKey;
    AX_BITCOLOR_T       tBitColor;
} AX_OVERLAY_T;

typedef enum {
    AX_CSC_MATRIX_BT601 = 0,    /* full range */
    AX_CSC_MATRIX_BT709,    /* full range */
    AX_CSC_MATRIX_BT2020,   /* full range */
    AX_CSC_MATRIX_BT601_TV, /* limit range */
    AX_CSC_MATRIX_BT709_TV, /* limit range */
    AX_CSC_MATRIX_USER,
    AX_CSC_MATRIX_TYPE_BUTT
} AX_CSC_MATRIX_TYPE_E;

typedef struct {
    AX_CSC_MATRIX_TYPE_E nType;
    AX_U16 nMatrix[9];
    AX_U16 nInOffset[3];
    AX_U16 nOutOffset[3];
} AX_CSC_MATRIX_T;

typedef enum _AX_GDC_MODE_E_ {
    AX_GDC_MODE_CORE0     = 0,   /* core 0 */
    AX_GDC_MODE_MAX
} AX_GDC_MODE_E;

typedef enum _AX_VDPS_MODE_E_ {
    AX_VDSP_MODE_CORE0     = 0,   /* core 0 */
    AX_VDSP_MODE_CORE1     = 1,   /* core 1 */
    AX_VDSP_MODE_AUTO      = 2,   /* core0/1 balance */
    AX_VDSP_MODE_MAX
} AX_VDSP_MODE_E;

typedef enum _AX_WARP_ENGINE_E_ {
    AX_WARP_ENGINE_GDC     = 0,
    AX_WARP_ENGINE_VDSP    = 1,
    AX_WARP_ENGINE_MAX
} AX_WARP_ENGINE_E;

typedef union _AX_WARP_MODE_U_ {
    AX_GDC_MODE_E  eGdcMode;
    AX_VDSP_MODE_E eVdspMode;
} AX_WARP_MODE_U;

#endif //_AX_GLOBAL_TYPE_H_

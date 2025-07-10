/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __COMMON_VDEC_UTILS_H__
#define __COMMON_VDEC_UTILS_H__


#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#ifdef __linux
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#endif

#include "ax_vdec_type.h"
#include "ax_sys_log.h"

#ifdef AX_VDEC_FFMPEG_ENABLE
#include "libavcodec/codec_id.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_VDEC_MAX_ARGS            (128)
#define SAMPLE_VDEC_OPTION_MAX_COUNT    (300)


#define AX_VDEC_FILE_DIR_LEN            (256)
#define AX_VDEC_FILE_NAME_LEN           (64)
#define AX_VDEC_FILE_PATH_LEN           (AX_VDEC_FILE_DIR_LEN + AX_VDEC_FILE_NAME_LEN)


#define USR_PIC_WIDTH                   (1920)
#define USR_PIC_HEIGHT                  (1080)

#define SEEK_NALU_MAX_LEN               (1024)
#define NAL_CODED_SLICE_CRA             (21)
#define NAL_CODED_SLICE_IDR             (5)
#define STREAM_BUFFER_MAX_SIZE           (3 * 1024 * 1024)
#define STREAM_BUFFER_MIN_SIZE           (1 * 1024 * 1024)
#define STREAM_BUFFER_MAX_SIZE_HIGH_RES  (32 * 1024 * 1024)


#define VDEC_BS_PARSER_BUF_SIZE         0x80000


#define SIZE_ALIGN(x,align) ((((x)+(align)-1)/(align))*(align))
#ifndef ALIGN_UP
#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#endif


#define AX_VDEC_WIDTH_ALIGN     (16)
#define AX_VDEC_HEIGHT_ALIGN     (16)

#define SAMPLE_VDEC_REF_BLK_CNT             (AX_VDEC_MAX_FRAME_BUF_CNT)
#define SAMPLE_VDEC_FRAME_CNT               (1)
#define SAMPLE_VDEC_MAX_STREAM_CNT          AX_VDEC_MAX_GRP_NUM

typedef struct {
    int num;
    int den;
} MPEG_RATIONAL_T;

typedef struct axSAMPLE_H264_SPS_DATA_T {
    uint16_t        width;
    uint16_t        height;
    MPEG_RATIONAL_T pixel_aspect;
    uint8_t         profile;
    uint8_t         level;
} SAMPLE_H264_SPS_DATA_T;

typedef enum axSAMPLE_BSBOUNDARY_TYPE_E {
    BSPARSER_NO_BOUNDARY = 0,
    BSPARSER_BOUNDARY = 1,
    BSPARSER_BOUNDARY_NON_SLICE_NAL = 2
} SAMPLE_BSBOUNDARY_TYPE_E;

typedef struct axSAMPLE_VDEC_USERPIC_T {
    AX_VDEC_USRPIC_T stUserPic;
    AX_BOOL recvStmAfUsrPic;
    AX_S32 s32RecvPicNumBak;
    AX_BOOL usrPicGet;
    AX_CHAR *pUsrPicFilePath;
    FILE *fpUsrPic;
    AX_POOL PoolId;
    AX_POOL BlkId;
} SAMPLE_VDEC_USERPIC_T;

typedef struct axSAMPLE_INPUT_FILE_INFO_T {
    FILE *fInput;
    off_t curPos;
    off_t sFileSize;
    AX_PAYLOAD_TYPE_E enDecType;
} SAMPLE_INPUT_FILE_INFO_T;

typedef struct axSAMPLE_BITSTREAM_INFO_T {
    AX_PAYLOAD_TYPE_E eVideoType;
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_U32 nFps;
    AX_VDEC_GRP VdGrp;
    SAMPLE_INPUT_FILE_INFO_T stBsInfo;
} SAMPLE_BITSTREAM_INFO_T;

typedef struct axSAMPLE_STREAM_BUF_T {
    AX_MEMORY_ADDR_T tBufAddr;
    AX_U32 uBufSize;
    AX_U8 *pBufBeforeFill;
    AX_U8 *pBufAfterFill;
    AX_BOOL bRingbuf;
} SAMPLE_STREAM_BUF_T;

#ifdef AX_VDEC_FFMPEG_ENABLE
typedef struct axSAMPLE_FFMPEG_T {
    AX_S32 s32VideoIndex;
    AVFormatContext *pstAvFmtCtx;
    AVBSFContext *pstAvBSFCtx;
    AVPacket *pstAvPkt;
} SAMPLE_FFMPEG_T;
#endif

typedef enum {
    AX_VDEC_MULTI_MODE_DISABLE = 0,
    AX_VDEC_MULTI_MODE_THREAD,
    AX_VDEC_MULTI_MODE_PROCESS,
    AX_VDEC_MULTI_MODE_BUTT
} AX_VDEC_MULTI_MODE_E;

typedef enum {
    AX_VDEC_GRP_UNEXIST = 0,
    AX_VDEC_GRP_CREATED,
    AX_VDEC_GRP_START_RECV,
    AX_VDEC_GRP_STOP_RECV,
    AX_VDEC_GRP_RESET,
    AX_VDEC_GRP_DESTROYED,
    AX_VDEC_GRP_BUTT
} AX_VDEC_GRP_STATUS_E;

typedef struct axSAMPLE_VDEC_CMD_PARAM_T {
    AX_U32 uGrpCount;
    AX_U32 uStreamCount;
    AX_S32 sLoopDecNum;
    AX_S32 sRecvPicNum;

    AX_U32 u32OutputFifoDepth;
    AX_S32 sWriteFrames;
    AX_U32 uStartGrpId;
    AX_BOOL bJpegDecOneFrm;

    AX_VDEC_MULTI_MODE_E enMultimode;

    AX_CHAR *pInputFilePath;
    AX_CHAR *pTbCfgFilePath;
    AX_CHAR *pOutputFilePath;

    AX_CHAR *pGrpCmdlFile[AX_VDEC_MAX_GRP_NUM];
    AX_BOOL bQuitWait;
    AX_BOOL highRes;
    AX_BOOL bPerfTest;
    AX_BOOL bFfmpegEnable;
    AX_BOOL bDynRes;
    AX_CHAR *pNewInputFilePath;

    /* grp attr */
    AX_U32 u32PicWidth;
    AX_U32 u32PicHeight;
    AX_U32 u32FrameBufCnt;
    AX_PAYLOAD_TYPE_E enDecType;
    AX_VDEC_INPUT_MODE_E enInputMode;
    AX_VDEC_OUTPUT_ORDER_E enOutOrder;

    /* grp prm */
    AX_VDEC_MODE_E enVideoMode;

    /* feature test */
    AX_BOOL bUserPts;
    AX_BOOL bMultiLck;
    AX_U32 u32FrameHeight;
    AX_U32 u32StreamFps; /* dec frame rate, used for calu pts */
    AX_S32 sMilliSec; /* send stream timeout */
    AX_S32 sGetMilliSec; /* get frm timeout */
    AX_POOL_SOURCE_E enFrameBufSrc; /* vb pool type: user,priv,com */
    /* used in stream mod */
    AX_S32 sStreamSize;

    /* function test */
    AX_BOOL bCheckFrmParam;
    AX_BOOL bGetUserData;
    AX_BOOL bGetRbInfo;
    AX_BOOL bGetGrpAtrr;
    AX_BOOL bQueryStatus;
    AX_BOOL bGetVuiParam;
    AX_BOOL bResetGrp;
    AX_BOOL bSleep;
    AX_BOOL bGetDispMode;
    AX_BOOL bGetGrpPrm;
    AX_BOOL bRepeatTest; /* create and destroy vdgrp 30 times */
    AX_BOOL bSelectMode;
    AX_BOOL bSkipRelease;
    AX_BOOL bSkipFrms;
    AX_VDEC_DISPLAY_MODE_E enDisplayMode;
    AX_BOOL bCreatGrpEx;

    /* user pic */
    AX_BOOL bUserPicEnable; /* true when pUsrPicFilePath seted */
    AX_U32  usrPicIdx;
    AX_BOOL recvStmAfUsrPic;
    AX_CHAR *pUsrPicFilePath;
    AX_BOOL bUsrInstant;

    /* for sample vdec_ivps_venc */
    AX_BOOL bOpenIvps;
    AX_U32 waitTime;

    /* for vdec link vo */
    AX_U32 voType;
    AX_U32 voDev;
    AX_U32 voRes;
    AX_U32 voGraphicOpen;

    /* For sample_vdec_ivps_vo. */
    AX_BOOL pollingEna;
    AX_S32 pollingCnt;
    AX_U32 pollingTime;

    AX_IMG_FORMAT_E enImgFormat;
} SAMPLE_VDEC_CMD_PARAM_T;

typedef struct axSAMPLE_VDEC_TBCFG_PARAM_T {
    AX_U32 uStreamCount;
    AX_U32 sLoopDecNum;
    AX_S32 sRecvPicNum;
    AX_S32 sMilliSec;
    AX_S32 sGetMilliSec;
    AX_U32 u32PicWidth;
    AX_U32 u32PicHeight;
    AX_S32 sWriteFrames;
    AX_U32 uStartGrpId;
    AX_BOOL bJpegDecOneFrm;
    AX_VDEC_DISPLAY_MODE_E enDisplayMode;
    AX_VDEC_MULTI_MODE_E enMultimode;
    AX_POOL_SOURCE_E enFrameBufSrc;
    AX_BOOL bUserPicEnable;
    AX_PAYLOAD_TYPE_E enDecType;
    AX_VDEC_INPUT_MODE_E enInputMode;
    AX_CHAR *pInputFilePath;
} SAMPLE_VDEC_TBCFG_PARAM_T;

typedef struct axSAMPLE_VDEC_OUTPUT_INFO_T {
    AX_VDEC_GRP         VdGrp;
    AX_U32              u32Width;
    AX_U32              u32Height;
    AX_U32              u32PicStride;
    AX_IMG_FORMAT_E     enImgFormat;
    AX_BOOL             bOneShot;
} SAMPLE_VDEC_OUTPUT_INFO_T;

typedef struct axSAMPLE_VDEC_USRPIC_CHN_PARAM_T {
    AX_U32 u32PicWidth;                 /* Width of scaler or crop target image */
    AX_U32 u32PicHeight;                /* Height of scaler or crop target image */
    AX_IMG_FORMAT_E enImgFormat;        /* Pixel format of target image */
    AX_CHAR *pUsrPicFilePath;
    AX_BOOL bUserPicEnable;
} SAMPLE_VDEC_USRPIC_PARAM_T;

typedef struct axSAMPLE_VDEC_USRPIC_ARGS_T {
    AX_VDEC_GRP VdGrp;
    AX_BOOL bUsrInstant;
    AX_PAYLOAD_TYPE_E enDecType;
    SAMPLE_VDEC_USERPIC_T *pstVdecUserPic;
    SAMPLE_VDEC_USRPIC_PARAM_T tPicParam;
} SAMPLE_VDEC_USRPIC_ARGS_T;

typedef struct axSAMPLE_VDEC_ARGS_T {
    AX_VDEC_GRP VdGrp;
    AX_VDEC_GRP_ATTR_T tVdGrpAttr;
    AX_POOL_CONFIG_T tPoolConfig;
    AX_POOL PoolId;
    void *pstCtx;
    SAMPLE_VDEC_USRPIC_ARGS_T tUsrPicArgs;
} SAMPLE_VDEC_ARGS_T;

typedef struct axSAMPLE_VDEC_CONTEXT_T {
    SAMPLE_VDEC_CMD_PARAM_T tCmdParam;
    AX_BOOL bTbCfgEnable;
    struct timeval Timebegin;
    struct timeval Timeend;
    AX_U64 u64SelectFrameCnt;
    int argc;
    char **argv;

#ifdef AX_VDEC_FFMPEG_ENABLE
    SAMPLE_FFMPEG_T stFfmpeg[AX_VDEC_MAX_GRP_NUM];
    SAMPLE_BITSTREAM_INFO_T stBitStreamInfo[AX_VDEC_MAX_GRP_NUM];
#endif


    FILE *pInputFd[AX_VDEC_MAX_GRP_NUM];
    FILE *pNewInputFd[AX_VDEC_MAX_GRP_NUM];
    off_t oInputFileSize[AX_VDEC_MAX_GRP_NUM];
    off_t oNewInputFileSize[AX_VDEC_MAX_GRP_NUM];

    AX_BOOL bGrpQuitWait[AX_VDEC_MAX_GRP_NUM];
    AX_BOOL bRecvFlowEnd[AX_VDEC_MAX_GRP_NUM];
    FILE *pOutputFd[AX_VDEC_MAX_GRP_NUM];
    AX_CHAR *pOutputFilePath[AX_VDEC_MAX_GRP_NUM];
    AX_CHAR *pOutputFilePath1[AX_VDEC_MAX_GRP_NUM];
    SAMPLE_VDEC_OUTPUT_INFO_T outInfo;

    AX_VDEC_GRP_STATUS_E GrpStatus[AX_VDEC_MAX_GRP_NUM];
    AX_S32 GrpPID[AX_VDEC_MAX_GRP_NUM];
    pthread_t GrpTid[AX_VDEC_MAX_GRP_NUM];
    pthread_t GrpChnRecvTid[AX_VDEC_MAX_GRP_NUM];
    pthread_t RecvTid;
    SAMPLE_VDEC_USERPIC_T stVdecUserPic;

    AX_BLK blkRef[AX_VDEC_MAX_GRP_NUM][SAMPLE_VDEC_REF_BLK_CNT];
    SAMPLE_VDEC_ARGS_T stVdecGrpArgs[AX_VDEC_MAX_GRP_NUM];
    AX_U64 recvFrmCnt[AX_VDEC_MAX_GRP_NUM];
    AX_U64 releaseFrmCnt[AX_VDEC_MAX_GRP_NUM];
} SAMPLE_VDEC_CONTEXT_T;

const char *SampleVdecRetStr(AX_S32 rv);



#ifndef gettid
#define gettid() syscall(__NR_gettid)
#endif


#define AX_SAMPLE_VDEC_LOG_TAG "sample_vdec"

#if 1
#define SAMPLE_LOG(str, arg...)  do { \
        AX_SYS_LogPrint(SYS_LOG_INFO, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_WHITE"[SAMPLE][AX_VDEC][tid:%ld][INFO][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_NOTICE_LOG(str, arg...)  do { \
        AX_SYS_LogPrint(SYS_LOG_NOTICE, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_BLUE"[SAMPLE][AX_VDEC][tid:%ld][NOTICE][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_WARN_LOG(str, arg...)  do { \
        AX_SYS_LogPrint(SYS_LOG_WARN, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_PURPLE"[SAMPLE][AX_VDEC][tid:%ld][WARN][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_ERR_LOG(str, arg...)  do { \
        printf("\n"MACRO_YELLOW"[SAMPLE][AX_VDEC][tid:%ld][ERROR][%s][line:%d]"str"\n", \
                gettid(), __func__, __LINE__, ##arg); \
        AX_SYS_LogPrint(SYS_LOG_ERROR, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_YELLOW"[SAMPLE][AX_VDEC][tid:%ld][ERROR][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_CRIT_LOG(str, arg...)        do{ \
        printf("\n"MACRO_RED"[SAMPLE][AX_VDEC][tid:%ld][CR_ERROR][%s][line:%d]"str"\n", \
                gettid(), __func__, __LINE__, ##arg); \
        AX_SYS_LogPrint(SYS_LOG_CRITICAL, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_RED"[SAMPLE][AX_VDEC][tid:%ld][CR_ERROR][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_LOG_HIT(str, arg...)  do { \
        AX_SYS_LogPrint(SYS_LOG_ERROR, AX_SAMPLE_VDEC_LOG_TAG, AX_ID_USER, \
                           MACRO_PURPLE"[SAMPLE][AX_VDEC][tid:%ld][T][%s][line:%d]: "str"\n", \
                           gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#else
#define SAMPLE_LOG(str, arg...)  do { \
        printf("[SAMPLE][AX_VDEC][tid:%ld][%s][%d] "str"\n", \
               gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_NOTICE_LOG(str, arg...)  do { \
        printf("[SAMPLE][AX_VDEC][tid:%ld][%s][%d] "str"\n", \
               gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_WARN_LOG(str, arg...)  do { \
        printf("[SAMPLE][AX_VDEC][tid:%ld][%s][%d] "str"\n", \
               gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_ERR_LOG(str, arg...)  do { \
        printf("\n"MACRO_YELLOW"[SAMPLE][AX_VDEC][tid:%ld][ERROR][%s][line:%d]"str"\n", \
                gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_CRIT_LOG(str, arg...)        do{ \
        printf("\n"MACRO_RED"[SAMPLE][AX_VDEC][tid:%ld][CR_ERROR][%s][line:%d]"str"\n", \
                gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_LOG_HIT(str, arg...)  do { \
        printf("\n" "[SAMPLE][AX_VDEC][tid:%ld][DEBUG][%s][line:%d]: "str"\n", \
               gettid(), __func__, __LINE__, ##arg); \
    } while (0)
#endif

#define SAMPLE_LOG_TMP(str, arg...)  do { \
        printf("\n" "[SAMPLE][AX_VDEC][tid:%ld][DEBUG][%s][line:%d]: "str"\n", \
               gettid(), __func__, __LINE__, ##arg); \
    } while (0)

#define SAMPLE_LOG_N(str, arg...)           do{}while(0)
#define SAMPLE_ERR_LOG_N(str, arg...)       do{}while(0)
#define SAMPLE_CRIT_LOG_N(str, arg...)      do{}while(0)


#define AX_VIDEO_FRAME_INFO_T_Print_all(VdGrp, pstFrameInfo) \
do { \
    const AX_VIDEO_FRAME_T *pstVFrame = &(((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->stVFrame); \
    if (pstVFrame->u64PhyAddr[0] != 0) { \
        SAMPLE_LOG("VdGrp=%d, ->enModId:%d, ->bEndOfStream:%d", \
                    VdGrp, \
                    ((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->enModId, ((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->bEndOfStream); \
        SAMPLE_LOG("pstVFrame->u32Width:0x%x", pstVFrame->u32Width); \
        SAMPLE_LOG("pstVFrame->u32Height:0x%x", pstVFrame->u32Height); \
        SAMPLE_LOG("pstVFrame->enImgFormat:0x%x", pstVFrame->enImgFormat); \
        SAMPLE_LOG("pstVFrame->enVscanFormat:0x%x", pstVFrame->enVscanFormat); \
        SAMPLE_LOG("pstVFrame->stDynamicRange:0x%x", pstVFrame->stDynamicRange); \
        SAMPLE_LOG("pstVFrame->stColorGamut:0x%x", pstVFrame->stColorGamut); \
        for (int i = 0; i < AX_MAX_COLOR_COMPONENT; i++) { \
            SAMPLE_LOG("pstVFrame->u32PicStride[%d]:0x%x", i, pstVFrame->u32PicStride[i]); \
            SAMPLE_LOG("pstVFrame->u32ExtStride[%d]:0x%x", i, pstVFrame->u32ExtStride[i]); \
            SAMPLE_LOG("pstVFrame->u64PhyAddr[%d]:0x%llx", i, pstVFrame->u64PhyAddr[i]);     \
            SAMPLE_LOG("pstVFrame->u64VirAddr[%d]:0x%llx", i, pstVFrame->u64VirAddr[i]);     \
            SAMPLE_LOG("pstVFrame->u64ExtPhyAddr[%d]:0x%llx", i, pstVFrame->u64ExtPhyAddr[i]); \
            SAMPLE_LOG("pstVFrame->u64ExtVirAddr[%d]:0x%llx", i, pstVFrame->u64ExtVirAddr[i]); \
            SAMPLE_LOG("pstVFrame->u32BlkId[%d]:0x%x", i, pstVFrame->u32BlkId[i]); \
        } \
        SAMPLE_LOG("pstVFrame->s16CropX:0x%x", pstVFrame->s16CropX); \
        SAMPLE_LOG("pstVFrame->s16CropY:0x%x", pstVFrame->s16CropY); \
        SAMPLE_LOG("pstVFrame->s16CropWidth:0x%x", pstVFrame->s16CropWidth); \
        SAMPLE_LOG("pstVFrame->s16CropHeight:0x%x", pstVFrame->s16CropHeight); \
        SAMPLE_LOG("pstVFrame->u32TimeRef:0x%x", pstVFrame->u32TimeRef); \
        SAMPLE_LOG("pstVFrame->u64PTS:%lld", pstVFrame->u64PTS);           \
        SAMPLE_LOG("pstVFrame->u64SeqNum:0x%llx", pstVFrame->u64SeqNum);     \
        SAMPLE_LOG("pstVFrame->u64UserData:0x%llx", pstVFrame->u64UserData); \
        SAMPLE_LOG("pstVFrame->u64PrivateData:0x%llx", pstVFrame->u64PrivateData);  \
        SAMPLE_LOG("pstVFrame->u32FrameFlag:0x%x", pstVFrame->u32FrameFlag);      \
        SAMPLE_LOG("pstVFrame->u32FrameSize:0x%x", pstVFrame->u32FrameSize);      \
    } else { \
        /* SAMPLE_LOG("pstVFrame->u64PhyAddr[%d]:0x%lx", 0, pstVFrame->u64PhyAddr[0]);  */  \
    } \
} while(0)


#define AX_VIDEO_FRAME_INFO_T_Print_brief(VdGrp, pstFrameInfo) \
do { \
    const AX_VIDEO_FRAME_T *pstVFrame = &(((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->stVFrame); \
    if (pstVFrame->u64PhyAddr[0] != 0) { \
        SAMPLE_LOG("VdGrp=%d, ->enModId:%d, ->bEndOfStream:%d", \
                    VdGrp, \
                    ((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->enModId, ((AX_VIDEO_FRAME_INFO_T *)pstFrameInfo)->bEndOfStream); \
        SAMPLE_LOG("pstVFrame->u32Width:0x%x", pstVFrame->u32Width); \
        SAMPLE_LOG("pstVFrame->u32Height:0x%x", pstVFrame->u32Height); \
        SAMPLE_LOG("pstVFrame->enImgFormat:0x%x", pstVFrame->enImgFormat); \
        for (int i = 0; i < AX_MAX_COLOR_COMPONENT; i++) { \
            SAMPLE_LOG("pstVFrame->u32PicStride[%d]:0x%x", i, pstVFrame->u32PicStride[i]); \
            SAMPLE_LOG("pstVFrame->u32ExtStride[%d]:0x%x", i, pstVFrame->u32ExtStride[i]); \
            SAMPLE_LOG("pstVFrame->u64PhyAddr[%d]:0x%llx", i, pstVFrame->u64PhyAddr[i]);     \
            SAMPLE_LOG("pstVFrame->u64VirAddr[%d]:0x%llx", i, pstVFrame->u64VirAddr[i]);     \
            SAMPLE_LOG("pstVFrame->u64ExtPhyAddr[%d]:0x%llx", i, pstVFrame->u64ExtPhyAddr[i]); \
            SAMPLE_LOG("pstVFrame->u64ExtVirAddr[%d]:0x%llx", i, pstVFrame->u64ExtVirAddr[i]); \
            SAMPLE_LOG("pstVFrame->u32BlkId[%d]:0x%x", i, pstVFrame->u32BlkId[i]); \
        } \
        SAMPLE_LOG("pstVFrame->u64PTS:%lld", pstVFrame->u64PTS);           \
    } else { \
        /* SAMPLE_LOG("pstVFrame->u64PhyAddr[%d]:0x%lx", 0, pstVFrame->u64PhyAddr[0]);  */  \
    } \
} while(0)




#ifdef __cplusplus
}
#endif
#endif

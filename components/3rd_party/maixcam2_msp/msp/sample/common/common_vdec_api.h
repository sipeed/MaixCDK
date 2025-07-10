/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __SAMPLE_VDEC_API_H__
#define __SAMPLE_VDEC_API_H__

/* #include "openssl/md5.h" */
#include "ax_vdec_api.h"
#include "ax_base_type.h"
#include "ax_sys_api.h"
#include "ax_buffer_tool.h"


#include "common_vdec_cmd.h"
#include "common_vdec_utils.h"
#include "common_type.h"

/*
    * input: start of NAL SPS ( 00 00 01 07 or 00 00 00 01 67 0r 67)
    */
int h264_parse_sps(const uint8_t *buf, int len, SAMPLE_H264_SPS_DATA_T * sps);
AX_PAYLOAD_TYPE_E DistinguishVideoType(const AX_U8 *stream, AX_U64 len);


int OutputFileCheckMD5(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_T *frameInfo, char *md5_str);
AX_S32 LoadOneFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, size_t *pLen);

FILE *OutputFileOpen(AX_CHAR **pOutputFilePath, const SAMPLE_VDEC_OUTPUT_INFO_T *pInfo);
AX_S32 OutputFileSaveYUV(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_T *frameInfo, FILE *fp_out, AX_CHAR *pOutputFilePath);


AX_S32 StreamFileParserReadFrame(const SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen);
AX_S32 StreamParserReadFrameJpeg(SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen);

AX_S32 VdecUserPicInit(SAMPLE_VDEC_ARGS_T *pstFuncArgs);
AX_S32 VdecUserPicDeinit(SAMPLE_VDEC_USERPIC_T *pstVdecUserPic);

int VdecCommonPoolPrintf(AX_VOID);

AX_S32 SampleVdecDestroyUserPool(AX_VDEC_GRP VdGrp, AX_POOL PoolId);

int Sample_VdecTestBenchMain(SAMPLE_VDEC_CONTEXT_T *pstSampleCtx);
int Sample_VdecJpegDecodeOneFrame(SAMPLE_VDEC_CMD_PARAM_T *pstCmd);
AX_S32 Sample_VdecTestBenchInit(int argc, char *argv[], SAMPLE_VDEC_CONTEXT_T **ppstVdecCtx);

AX_S32 __VdecUsrPicCreat(SAMPLE_VDEC_USRPIC_ARGS_T *pstUsrPicArgs, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic);
AX_S32 VdecUserPicEnable(AX_VDEC_GRP VdGrp, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic,
                         AX_BOOL *pUsrPicEnable, SAMPLE_VDEC_CONTEXT_T *pstCtx);

AX_VOID VdecUserPicDestroy(AX_VDEC_GRP VdGrp, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic);

#ifdef AX_VDEC_FFMPEG_ENABLE
AX_S32 SampleVdecFfmpegInit(SAMPLE_FFMPEG_T *pstFfmpeg, const AX_CHAR *pcInputFilePath,
                            SAMPLE_BITSTREAM_INFO_T *pstBitStreamInfo);
AX_S32 SampleVdecFfmpegDeinit(SAMPLE_FFMPEG_T *pstFfmpeg, AX_VDEC_GRP VdGrp);
AX_S32 SampleVdecFfmpegExtractOnePic(SAMPLE_FFMPEG_T *pstFfmpeg, SAMPLE_BITSTREAM_INFO_T *pstBitStreamInfo,
                                     SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen);
#endif

AX_BOOL SampleVdecSearchStartCode(AX_U8 *uStrAddr, AX_U32 uLen, AX_U32 *puReadBytes, AX_U32 *puZeroCount);
AX_VOID SampelVdecSetThreadName(const char *nameFmt, ...);

AX_U64 SampleGetFileSize(char * pFileName);
#endif // __SAMPLE_VDEC_API_H__
/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_COMM_AIO_H_
#define _AX_COMM_AIO_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_MAX_AUDIO_FILE_PATH_LEN  256
#define AX_MAX_AUDIO_FILE_NAME_LEN  256

typedef struct axAUDIO_STREAM_T {
    AX_U8 *pStream;         /* the virtual address of stream */
    AX_U64 u64PhyAddr;      /* the physics address of stream */
    AX_U32 u32Len;          /* stream lenth, by bytes */
    AX_U64 u64TimeStamp;    /* frame time stamp*/
    AX_U32 u32Seq;          /* frame seq,if stream is not a valid frame,u32Seq is 0*/
    AX_BOOL bEof;
} AX_AUDIO_STREAM_T;

typedef enum {
    AX_AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8K samplerate*/
    AX_AUDIO_SAMPLE_RATE_12000  = 12000,   /* 12K samplerate*/
    AX_AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025K samplerate*/
    AX_AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16K samplerate*/
    AX_AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050K samplerate*/
    AX_AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24K samplerate*/
    AX_AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32K samplerate*/
    AX_AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1K samplerate*/
    AX_AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48K samplerate*/
    AX_AUDIO_SAMPLE_RATE_64000  = 64000,   /* 64K samplerate*/
    AX_AUDIO_SAMPLE_RATE_96000  = 96000,   /* 96K samplerate*/
} AX_AUDIO_SAMPLE_RATE_E;

/*Defines the configure parameters of AI saving file.*/
typedef struct axAUDIO_SAVE_FILE_INFO_T {
    AX_BOOL     bCfg;
    AX_CHAR     aFilePath[AX_MAX_AUDIO_FILE_PATH_LEN];
    AX_CHAR     aFileName[AX_MAX_AUDIO_FILE_NAME_LEN];
    AX_U32      u32FileSize;  /*in KB*/
} AX_AUDIO_SAVE_FILE_INFO_T;

/*Defines whether the file is saving or not .*/
typedef struct axAUDIO_FILE_STATUS_T {
    AX_BOOL     bSaving;
} AX_AUDIO_FILE_STATUS_T;

typedef enum axAUDIO_FADE_RATE_E
{
    AX_AUDIO_FADE_RATE_32   = 32,
    AX_AUDIO_FADE_RATE_64   = 64,
    AX_AUDIO_FADE_RATE_128  = 128,
    AX_AUDIO_FADE_RATE_256  = 256,
    AX_AUDIO_FADE_RATE_512  = 512,
    AX_AUDIO_FADE_RATE_1024 = 1024,

    AX_AUDIO_FADE_RATE_BUTT
} AX_AUDIO_FADE_RATE_E;

typedef struct axAUDIO_FADE_T
{
    AX_BOOL         bFade;
    AX_AUDIO_FADE_RATE_E enFadeInRate;
    AX_AUDIO_FADE_RATE_E enFadeOutRate;
} AX_AUDIO_FADE_T;

#ifdef __cplusplus
}
#endif

#endif

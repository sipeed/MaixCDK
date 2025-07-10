/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/prctl.h>

#include "common_vdec_utils.h"
#include "ax_vdec_api.h"
#include "common_arg_parse.h"
#include "common_vdec_api.h"

#define AX_DEC_RET_STR_CASE(s32Ret) case (s32Ret): return(#s32Ret)

const char *SampleVdecRetStr(AX_S32 s32Ret)
{
    switch (s32Ret) {
    AX_DEC_RET_STR_CASE(AX_SUCCESS);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_INVALID_GRPID);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_INVALID_CHNID);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_ILLEGAL_PARAM);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NULL_PTR);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_BAD_ADDR);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_SYS_NOTREADY);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_BUSY);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOT_INIT);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOT_CONFIG);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOT_SUPPORT);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOT_PERM);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_EXIST);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_UNEXIST);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOMEM);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NOBUF);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_BUF_EMPTY);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_BUF_FULL);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_QUEUE_EMPTY);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_QUEUE_FULL);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_TIMED_OUT);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_FLOW_END);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_RUN_ERROR);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_STRM_ERROR);
    AX_DEC_RET_STR_CASE(AX_ERR_POOL_UNEXIST);
    AX_DEC_RET_STR_CASE(AX_ERR_POOL_ILLEGAL_PARAM);
    AX_DEC_RET_STR_CASE(AX_ERR_POOL_NOT_SUPPORT);
    AX_DEC_RET_STR_CASE(AX_ERR_POOL_NOT_PERM);
    AX_DEC_RET_STR_CASE(AX_ERR_VDEC_NEED_REALLOC_BUF);
    default:
        SAMPLE_CRIT_LOG("Unknown return code. 0x%x", s32Ret);
        return("unknown code.");
    }
}

typedef struct
{
    const uint8_t * data; // sps
    int            count; /* in parse */
    int            index; /* in parse */
} PARSE_STATE_T;



#define PARSE_INIT(data, bytes) {(data), 8*(bytes), 0}

#define PARSE_EOF(parse) ((parse)->index >= (parse)->count)

static inline void __parse_init(PARSE_STATE_T *parse, const uint8_t *data, int bytes)
{
    parse->data = data;
    parse->count = 8 * bytes;
    parse->index = 0;
}

static inline int __parse_get_bit(PARSE_STATE_T *parse)
{
    if (parse->index >= parse->count)
        return 1; /* -> no infinite colomb's ... */

    int bit = (parse->data[parse->index >> 3] >> (7 - (parse->index & 7))) & 1;
    parse->index++;
    return bit;
}

static inline uint32_t __parse_get_bits(PARSE_STATE_T * parse, uint32_t cnt)
{
    uint32_t bit = 0;
    while (cnt--)
        bit = bit | (__parse_get_bit(parse) << cnt);

    return bit;
}

#define PARSE_SKIP_BIT(parse) __parse_skip_bits(parse, 1)

static inline void __parse_skip_bits(PARSE_STATE_T *parse, int cnt)
{
    parse->index += cnt;
}

#define PARSE_GET_U8(parse)         __parse_get_bits(parse, 8)
#define PARSE_GET_U16(parse)        ((__parse_get_bits(parse, 8)<<8) | __parse_get_bits(parse, 8))

static inline uint32_t __parse_get_ue_golomb(PARSE_STATE_T * parse)
{
    int cnt = 0;
    while (!__parse_get_bit(parse) && cnt < 32)
        cnt++;
    return cnt ? ((1 << cnt) - 1) + __parse_get_bits(parse, cnt) : 0;
}

static inline int32_t __parse_get_se_golomb(PARSE_STATE_T * parse)
{
    uint32_t r = __parse_get_ue_golomb(parse) + 1;
    return (r & 1) ? -(r >> 1) : (r >> 1);
}

static inline void __parse_skip_golomb(PARSE_STATE_T * parse)
{
    int cnt = 0;
    while (!__parse_get_bit(parse) && cnt < 32)
        cnt++;
    __parse_skip_bits(parse, cnt);
}

#define PARSE_SKIP_UE_GOLOMB(parse) __parse_skip_golomb(parse)
#define PARSE_SKIP_SE_GOLOMB(parse) __parse_skip_golomb(parse)

static const MPEG_RATIONAL_T aspect_ratios[] =
{
/* page 213: */
/* 0: unknown */
    {0, 1},
/* 1...16: */
    {1,  1}, {12, 11 }, {10, 11}, {16, 11},
    {40, 33}, {24, 11}, {20, 11}, {32, 11},
    {80, 33}, {18, 11}, {15, 11}, {64, 33},
    {160, 99}, {4,  3}, {3,  2}, {2,  1}
};

int h264_parse_sps(const uint8_t *buf, int len, SAMPLE_H264_SPS_DATA_T *sps)
{
    int findSPS = -1;
    int pos = 0;

    if (buf == NULL) {
        SAMPLE_CRIT_LOG("buf == NULL");
        return -1;
    }

    if (sps == NULL) {
        SAMPLE_CRIT_LOG("sps == NULL");
        return -1;
    }

    if (len < 16) {
        SAMPLE_CRIT_LOG("len:%d < 16", len);
        return -1;
    }

    for (pos = 0; pos < len - 6; pos++) {
        if ((buf[pos] == 0) && (buf[pos + 1] == 0)
                && (buf[pos + 2] == 0x1) && ((buf[pos + 3] & 0x1f) == 0x7)) {
            len -= (pos + 4);
            buf += (pos + 4);
            findSPS = AX_SUCCESS;
            break;
        }
    }

    if (findSPS != AX_SUCCESS) {
        SAMPLE_LOG_TMP("findSPS failed! Failed to parse width and height.\n"
                      "You can configure the '--res' parameter in case the program runs out of memory and cannot decode");
        return findSPS;
    }

    PARSE_STATE_T parse = PARSE_INIT(buf, len);
    int profile_idc, pic_order_cnt_type;
    int frame_mbs_only;
    int i, j;

    profile_idc = PARSE_GET_U8(&parse);
    sps->profile = profile_idc;
    SAMPLE_LOG_N("H.264 SPS: profile_idc %d", profile_idc);
    /* constraint_set0_flag = __parse_get_bit(parse);    */
    /* constraint_set1_flag = __parse_get_bit(parse);    */
    /* constraint_set2_flag = __parse_get_bit(parse);    */
    /* constraint_set3_flag = __parse_get_bit(parse);    */
    /* reserved             = __parse_get_bits(parse,4); */
    sps->level = PARSE_GET_U8(&parse);
    __parse_skip_bits(&parse, 8);
    PARSE_SKIP_UE_GOLOMB(&parse);   /* seq_parameter_set_id */
    if (profile_idc >= 100) {
        if (__parse_get_ue_golomb(&parse) == 3) {/* chroma_format_idc      */
            PARSE_SKIP_BIT(&parse);     /* residual_colour_transform_flag */
        }
        PARSE_SKIP_UE_GOLOMB(&parse); /* bit_depth_luma - 8             */
        PARSE_SKIP_UE_GOLOMB(&parse); /* bit_depth_chroma - 8           */
        PARSE_SKIP_BIT(&parse);       /* transform_bypass               */
        if (__parse_get_bit(&parse)) {   /* seq_scaling_matrix_present     */
            for (i = 0; i < 8; i++) {
                if (__parse_get_bit(&parse)) {
                    /* seq_scaling_list_present    */
                    int last = 8, next = 8, size = (i < 6) ? 16 : 64;
                    for (j = 0; j < size; j++) {
                        if (next) {
                            next = (last + __parse_get_se_golomb(&parse)) & 0xff;
                        }
                        last = next ? next : last;
                    }
                }
            }
        }
    }

    PARSE_SKIP_UE_GOLOMB(&parse);      /* log2_max_frame_num - 4 */
    pic_order_cnt_type = __parse_get_ue_golomb(&parse);
    if (pic_order_cnt_type == 0) {
        PARSE_SKIP_UE_GOLOMB(&parse);    /* log2_max_poc_lsb - 4 */
    }
    else if (pic_order_cnt_type == 1) {
        PARSE_SKIP_BIT(&parse);          /* delta_pic_order_always_zero     */
        PARSE_SKIP_SE_GOLOMB(&parse);    /* offset_for_non_ref_pic          */
        PARSE_SKIP_SE_GOLOMB(&parse);    /* offset_for_top_to_bottom_field  */
        j = __parse_get_ue_golomb(&parse); /* num_ref_frames_in_pic_order_cnt_cycle */
        for (i = 0; i < j; i++) {
            PARSE_SKIP_SE_GOLOMB(&parse);  /* offset_for_ref_frame[i]         */
        }

    }
    PARSE_SKIP_UE_GOLOMB(&parse);      /* ref_frames                      */
    PARSE_SKIP_BIT(&parse);            /* gaps_in_frame_num_allowed       */
    sps->width  /* mbs */ = __parse_get_ue_golomb(&parse) + 1;
    sps->height /* mbs */ = __parse_get_ue_golomb(&parse) + 1;
    frame_mbs_only = __parse_get_bit(&parse);
    SAMPLE_LOG_N("H.264 SPS: pic_width:  %u mbs", (unsigned)sps->width);
    SAMPLE_LOG_N("H.264 SPS: pic_height: %u mbs", (unsigned)sps->height);
    SAMPLE_LOG_N("H.264 SPS: frame only flag: %d", frame_mbs_only);

    sps->width *= 16;
    sps->height *= 16 * (2 - frame_mbs_only);

    if (!frame_mbs_only)
        if (__parse_get_bit(&parse)) /* mb_adaptive_frame_field_flag */
            SAMPLE_LOG_N("H.264 SPS: MBAFF");
    PARSE_SKIP_BIT(&parse);      /* direct_8x8_inference_flag    */
    if (__parse_get_bit(&parse)) {
        /* frame_cropping_flag */
        uint32_t crop_left = __parse_get_ue_golomb(&parse);
        uint32_t crop_right = __parse_get_ue_golomb(&parse);
        uint32_t crop_top = __parse_get_ue_golomb(&parse);
        uint32_t crop_bottom = __parse_get_ue_golomb(&parse);
        SAMPLE_LOG_N("H.264 SPS: cropping %d %d %d %d",
                  crop_left, crop_top, crop_right, crop_bottom);

        sps->width -= 2 * (crop_left + crop_right);
        if (frame_mbs_only)
            sps->height -= 2 * (crop_top + crop_bottom);
        else
            sps->height -= 4 * (crop_top + crop_bottom);
    }

    /* VUI parameters */
    sps->pixel_aspect.num = 0;
    if (__parse_get_bit(&parse)) {
        /* vui_parameters_present flag */
        if (__parse_get_bit(&parse)) {
            /* aspect_ratio_info_present */
            uint32_t aspect_ratio_idc = PARSE_GET_U8(&parse);
            SAMPLE_LOG_N("H.264 SPS: aspect_ratio_idc %d", aspect_ratio_idc);

            if (aspect_ratio_idc == 255 /* Extended_SAR */) {
                sps->pixel_aspect.num = PARSE_GET_U16(&parse); /* sar_width */
                sps->pixel_aspect.den = PARSE_GET_U16(&parse); /* sar_height */
                SAMPLE_LOG_N("H.264 SPS: -> sar %dx%d", sps->pixel_aspect.num, sps->pixel_aspect.den);
            } else {
                if (aspect_ratio_idc < sizeof(aspect_ratios) / sizeof(aspect_ratios[0])) {
                    sps->pixel_aspect = aspect_ratios[aspect_ratio_idc];
                    SAMPLE_LOG_N("H.264 SPS: -> aspect ratio %d / %d", sps->pixel_aspect.num, sps->pixel_aspect.den);
                } else {
                    SAMPLE_LOG_N("H.264 SPS: aspect_ratio_idc out of range !");
                }
            }
        }
    }

    SAMPLE_LOG_N("H.264 SPS: -> video size %dx%d, aspect %d:%d",
              sps->width, sps->height, sps->pixel_aspect.num, sps->pixel_aspect.den);

    return findSPS;
}



AX_PAYLOAD_TYPE_E DistinguishVideoType(const AX_U8 *stream, AX_U64 len)
{
    AX_U64 ReadLen = 0;
    AX_U64 i = 0;
    const AX_U8 *pu8Buf = stream;

    if (stream == NULL) {
        SAMPLE_CRIT_LOG(" stream == NULL");
        return PT_BUTT;
    }

    ReadLen = len;

    if (ReadLen <= 6) {
        SAMPLE_CRIT_LOG(" len:%lld ReadLen:%lld", len, ReadLen);
        return PT_BUTT;
    }

    for (i = 0; i < ReadLen - 6; i++)
    {
        if ((pu8Buf[i+0] == 0) && (pu8Buf[i+1] == 0) && (pu8Buf[i+2] == 1)) {
            if ((pu8Buf[i+3] & 0x1f) == 0x07) {
                return PT_H264;
            }
        }

        if ((pu8Buf[i+0] == 0) && (pu8Buf[i+1] == 0) && (pu8Buf[i+2] == 0) && (pu8Buf[i+3] == 1)) {
            if ((pu8Buf[i+4] == 0x40) && (pu8Buf[i+5] == 0x01)) {
                return PT_H265;
            }
        }
    }

    return PT_BUTT;
}

static AX_S32 __MD5SumValidOnly(AX_U8 *p_lu, AX_U8 *p_ch,
                       AX_U32 coded_width, AX_U32 coded_height,
                       AX_U32 coded_width_ch, AX_U32 coded_height_ch,
                       AX_U32 pic_stride, AX_U32 pic_stride_ch,
                       AX_U32 planar, AX_U32 frame_number, char *md5_str)
{
#if 0

    unsigned char digest[16];
    MD5_CTX ctx;
    int i = 0;
    MD5_Init(&ctx);
    AX_U8 *p_yuv = p_lu;
    if (p_yuv) {
        for (i = 0; i < coded_height; i++) {
            MD5_Update(&ctx, p_yuv, coded_width);
            p_yuv += pic_stride;
        }
    }
    p_yuv = p_ch;
    if (p_yuv) {
        if (!planar) {
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch);
                p_yuv += pic_stride;
            }
        } else {
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch / 2);
                p_yuv += pic_stride_ch;
            }
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch / 2);
                p_yuv += pic_stride_ch;
            }
        }
    }
    MD5_Final(digest, &ctx);
    /*    fprintf(f_out, "FRAME %d: ", frame_number);*/
    for (i = 0; i < 16; i++) {
        snprintf(md5_str + i * 2, 2 + 1, "%02x", digest[i]);
    }
#endif
    return 0;
}

int OutputFileCheckMD5(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_T *frameInfo, char *md5_str)
{
    AX_S32 sRet;
    AX_S32 s32Ret = 0;
    AX_VOID *pLumaVirAddr;
    AX_VOID *pChromaVirAddr;
    AX_U32 lumaMapSize;
    AX_U32 chromaMapSize;

    if (frameInfo == NULL) {
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        SAMPLE_CRIT_LOG("frameInfo == NULL\n");
        goto ERR_RET;
    }

    if (md5_str == NULL) {
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        SAMPLE_CRIT_LOG("md5_str == NULL\n");
        goto ERR_RET;
    }

    if ((frameInfo->stVFrame.u64PhyAddr[0] == 0) || (frameInfo->stVFrame.u32PicStride[0] == 0)) {
        SAMPLE_CRIT_LOG("VdGrp=%d, stVFrame.u64PhyAddr[0]:0x%llx stVFrame.u32PicStride[0]:%d\n",
                       VdGrp, frameInfo->stVFrame.u64PhyAddr[0], frameInfo->stVFrame.u32PicStride[0]);
        s32Ret = 0;
        goto ERR_RET;
    }

    lumaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16);

    pLumaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[0], lumaMapSize);
    if (!pLumaVirAddr) {
        s32Ret = AX_ERR_VDEC_BAD_ADDR;
        SAMPLE_CRIT_LOG("AX_SYS_Mmap luma FAILED\n");
        goto ERR_RET;
    }

    SAMPLE_LOG("AX_SYS_Mmap luma success, pLumaVirAddr=%p,lumaMapSize=%d\n",
               pLumaVirAddr, lumaMapSize);

    chromaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16) / 2;
    pChromaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[1], chromaMapSize);
    if (!pChromaVirAddr) {
        s32Ret = AX_ERR_VDEC_BAD_ADDR;
        SAMPLE_CRIT_LOG("AX_SYS_Mmap chroma FAILED\n");
        goto ERR_RET_CHROMA;
    }

    SAMPLE_LOG("AX_SYS_Mmap chroma success, pChromaVirAddr=%p,chromaMapSize=%d\n",
               pChromaVirAddr, chromaMapSize);

    AX_VOID *p_lu = pLumaVirAddr;
    AX_VOID *p_ch = pChromaVirAddr;
    AX_U32 coded_width = frameInfo->stVFrame.u32Width;
    AX_U32 coded_height = frameInfo->stVFrame.u32Height;
    AX_U32 pic_stride = frameInfo->stVFrame.u32PicStride[0];
    AX_U32 coded_width_ch = frameInfo->stVFrame.u32Width;
    AX_U32 coded_h_ch = frameInfo->stVFrame.u32Height / 2;
    AX_U32 pic_stride_ch = frameInfo->stVFrame.u32PicStride[1];
    s32Ret = __MD5SumValidOnly(p_lu, p_ch, coded_width, coded_height, coded_width_ch, coded_h_ch,
                             pic_stride, pic_stride_ch, 0, 0, md5_str);
    if (s32Ret) {
        SAMPLE_CRIT_LOG("__MD5SumValidOnly FAILED! s32Ret:0x%x", s32Ret);
        goto ERR_RET_LUMA;
    }

ERR_RET_LUMA:
    if (pLumaVirAddr) {
        sRet = AX_SYS_Munmap(pLumaVirAddr, lumaMapSize);
        if (sRet) {
            s32Ret = sRet;
            SAMPLE_CRIT_LOG("AX_SYS_Munmap luma FAILED,sRet=0x%x\n", sRet);
        } else {
            SAMPLE_LOG("AX_SYS_Munmap luma success,pLumaVirAddr=%p,lumaMapSize=%d\n",
                      pLumaVirAddr, lumaMapSize);
        }
    }
ERR_RET_CHROMA:
    if (pChromaVirAddr) {
        sRet = AX_SYS_Munmap(pChromaVirAddr, chromaMapSize);
        if (sRet) {
            s32Ret = sRet;
            SAMPLE_CRIT_LOG("AX_SYS_Munmap chroma FAILED,sRet=0x%x\n", sRet);
        } else {
            SAMPLE_LOG("AX_SYS_Munmap chroma success,pChromaVirAddr=%p,chromaMapSize=%d\n",
                      pChromaVirAddr, chromaMapSize);
        }
    }
ERR_RET:
    return s32Ret;
}


AX_S32 LoadOneFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, size_t *pLen)
{
    AX_S32 res = 0;
    AX_U64 tmp_size = 0;
    AX_U64 read_size = 0;
    FILE *fInput = NULL;

    if (ps8File == NULL) {
        SAMPLE_CRIT_LOG("ps8File == NULL\n");
        goto ERR_RET;
    }

    if (ppu8Mem == NULL) {
        SAMPLE_CRIT_LOG("ppu8Mem == NULL\n");
        goto ERR_RET;
    }

    if (pLen == NULL) {
        SAMPLE_CRIT_LOG("pLen == NULL\n");
        goto ERR_RET;
    }

    fInput = fopen(ps8File, "rb");
    if (fInput == NULL) {
        SAMPLE_CRIT_LOG("Unable to open input file:%s\n", ps8File);
        res = AX_ERR_VDEC_RUN_ERROR;
        goto ERR_RET;
    }

    /* file i/o pointer to full */
    res = fseek(fInput, 0L, SEEK_END);
    if (res) {
        SAMPLE_CRIT_LOG("fseek FAILED! ret:%d\n", res);
        goto ERR_RET;
    }

    *pLen = ftello(fInput);
    rewind(fInput);

    tmp_size = sizeof(AX_U8) * (*pLen);
    *ppu8Mem = (AX_U8 *)malloc(tmp_size);
    if (*ppu8Mem == NULL) {
        SAMPLE_CRIT_LOG("malloc tmp_size:0x%llx FAILED!", tmp_size);
        res = AX_ERR_VDEC_NOMEM;
        goto ERR_RET_CLOSE;
    }

    read_size = *pLen;
    tmp_size = fread(*ppu8Mem, sizeof(AX_U8), read_size, fInput);
    /* read input stream from file to buffer and close input file */
    if (tmp_size != read_size) {
        SAMPLE_CRIT_LOG("fread FAILED! tmp_size:0x%llx != read_size:0x%llx\n", tmp_size, read_size);
        res = AX_ERR_VDEC_STRM_ERROR;
        goto ERR_RET_FREE;
    }

ERR_RET_FREE:
    free((AX_VOID *)*ppu8Mem);
    *ppu8Mem = NULL;
ERR_RET_CLOSE:
    res = fclose(fInput);
    if (res) {
        SAMPLE_CRIT_LOG("fclose FAILED! ret:%d\n", res);
    }
ERR_RET:
    return res;
}

FILE *OutputFileOpen(AX_CHAR **ppOutputFile, const SAMPLE_VDEC_OUTPUT_INFO_T *pInfo)
{
    FILE *fp_out = NULL;
    int ret = 0;
    AX_CHAR *file_path = NULL;
    AX_CHAR *file_name = NULL;
    AX_U32 slen = 0;
    AX_CHAR *pFileOut = NULL;

    if (ppOutputFile == NULL) {
        SAMPLE_CRIT_LOG("ppOutputFile == NULL");
        return NULL;
    }

    if (*ppOutputFile == NULL) {
        pFileOut = calloc(1, AX_VDEC_FILE_PATH_LEN);
        if (pFileOut == NULL) {
            SAMPLE_CRIT_LOG("malloc %d Bytes FAILED!\n", AX_VDEC_FILE_PATH_LEN);
            return NULL;
        }
        *ppOutputFile = pFileOut;

        if (pInfo == NULL) {
            ret = snprintf(pFileOut, AX_VDEC_FILE_NAME_LEN, "%s", "./out.yuv");
        } else {
            if (pInfo->bOneShot) {
                ret = snprintf(pFileOut, AX_VDEC_FILE_NAME_LEN, "oneShot_format%d_w_%d_h_%d.yuv",
                               pInfo->enImgFormat, pInfo->u32Width, pInfo->u32Height);
            } else {
                ret = snprintf(pFileOut, AX_VDEC_FILE_NAME_LEN, "group%d_format%d_w_%d_h_%d.yuv",
                               pInfo->VdGrp, pInfo->enImgFormat, pInfo->u32Width, pInfo->u32Height);
            }
        }

        if (ret < 0) {
            SAMPLE_CRIT_LOG("snprintf FAILED! ret:0x%x pFileOut:%s AX_VDEC_FILE_NAME_LEN:%d\n",
                           ret, pFileOut, AX_VDEC_FILE_NAME_LEN);
            goto ERR_RET_OUTPUTFILE;
        }

        *ppOutputFile = pFileOut;
    } else {
        pFileOut = *ppOutputFile;
    }

    SAMPLE_LOG("output_file_path >>>> pFileOut:%s", pFileOut);
    slen = strlen(pFileOut);
    ret = strncmp (pFileOut + slen - 1, "/", 1);
    if (ret == 0) {
        file_path = malloc(AX_VDEC_FILE_PATH_LEN);
        if (NULL == file_path) {
            SAMPLE_CRIT_LOG("malloc size:%d FAILED!", AX_VDEC_FILE_PATH_LEN);
            goto ERR_RET_OUTPUTFILE;
        }

        file_name = malloc(AX_VDEC_FILE_NAME_LEN);
        if (NULL == file_name) {
            SAMPLE_CRIT_LOG("malloc size:%d FAILED!", AX_VDEC_FILE_NAME_LEN);
            goto ERR_RET_FILEPATH;
        }

        if (pInfo == NULL) {
            ret = snprintf(file_name, AX_VDEC_FILE_NAME_LEN, "%s", "out.yuv");
        } else {
            if (pInfo->bOneShot) {
                ret = snprintf(pFileOut, AX_VDEC_FILE_NAME_LEN, "oneShot_format%d_w_%d_h_%d.yuv",
                               pInfo->enImgFormat, pInfo->u32Width, pInfo->u32Height);
            } else {
                ret = snprintf(pFileOut, AX_VDEC_FILE_NAME_LEN, "group%d_format%d_w_%d_h_%d.yuv",
                               pInfo->VdGrp, pInfo->enImgFormat, pInfo->u32Width, pInfo->u32Height);
            }
        }

        if (ret < 0) {
            SAMPLE_CRIT_LOG("snprintf FAILED! ret:0x%x file_name:%s AX_VDEC_FILE_NAME_LEN:%d\n",
                           ret, file_name, AX_VDEC_FILE_NAME_LEN);
            goto ERR_RET_FILENAME;
        }

        slen = strlen(file_name);
        if (slen >= AX_VDEC_FILE_NAME_LEN) {
            SAMPLE_CRIT_LOG("slen:%d >= AX_VDEC_FILE_NAME_LEN:%d\n",
                           slen, AX_VDEC_FILE_NAME_LEN);
            goto ERR_RET_FILENAME;
        }

        ret = snprintf(file_path, AX_VDEC_FILE_PATH_LEN, "%s%s",
                       pFileOut, file_name);
        if (ret < 0) {
            SAMPLE_CRIT_LOG("snprintf FAILED! ret:0x%x, pFileOut:%s, file_name:%s, AX_VDEC_FILE_PATH_LEN:%d\n",
                           ret, pFileOut, file_name, AX_VDEC_FILE_PATH_LEN);
            goto ERR_RET_FILENAME;
        }

        free(file_name);
        free(pFileOut);
        *ppOutputFile = file_path;
        pFileOut = *ppOutputFile;
    }

    slen = strlen(pFileOut);
    ret = strncmp (pFileOut + slen - 1, "/", 1);
    if (ret == 0) {
        SAMPLE_CRIT_LOG("Please check pFileOut:%s, the last character cannot be '/' \n", pFileOut);
        goto ERR_RET_FILENAME;
    }

    if (access(pFileOut, F_OK) == 0) {
        char new_name[AX_VDEC_FILE_PATH_LEN + AX_VDEC_FILE_NAME_LEN];
        memset(new_name, 0, sizeof(new_name));

        struct timeval current_tv;
        gettimeofday(&current_tv, NULL);

        ret = sprintf(new_name, "%s_%lds_%ldus.bak", pFileOut, current_tv.tv_sec, current_tv.tv_usec);
        if (ret < 0) {
            SAMPLE_CRIT_LOG("snprintf FAILED! ret:0x%x, pFileOut:%s, AX_VDEC_FILE_PATH_LEN + AX_VDEC_FILE_NAME_LEN:%d\n",
                           ret, pFileOut, AX_VDEC_FILE_PATH_LEN + AX_VDEC_FILE_NAME_LEN);
            goto ERR_RET_FILENAME;
        }
        // SAMPLE_LOG("new_name:%s ", new_name);
        if (chmod(pFileOut, 0x777) != 0) {
            SAMPLE_CRIT_LOG("chmod:%s FAILED", pFileOut);
        }

        if (rename(pFileOut, new_name) == 0) {
            SAMPLE_LOG("rename(pFileOut:%s, new_name:%s) success",
                       pFileOut, new_name);
        } else {
            SAMPLE_CRIT_LOG("rename(pFileOut:%s, new_name:%s) FAILED)", pFileOut, new_name);
        }
    }

    if (access(pFileOut, F_OK) == 0) {
        if (chmod(pFileOut, 0x777) != 0) {
            SAMPLE_CRIT_LOG("chmod:%s ", pFileOut);
        }

        if (remove(pFileOut) == 0) {
            if (access(pFileOut, F_OK) == 0) {
                SAMPLE_CRIT_LOG("remove(pFileOut:%s FAIL)", pFileOut);
            }
        } else {
            SAMPLE_CRIT_LOG("remove(pFileOut:%s FAIL)", pFileOut);
        }
    }

    fp_out = fopen(pFileOut, "w");
    if (fp_out == NULL) {
        SAMPLE_CRIT_LOG("VdGrp=%d, fopen pFileOut:%s FAILED!",
                       pInfo->VdGrp, pFileOut);
    }

    SAMPLE_LOG("output file name:%s, open fp_out:%p\n", pFileOut, fp_out);
    return fp_out;

ERR_RET_FILENAME:
    if (file_name != NULL) {
        free(file_name);
    }
ERR_RET_FILEPATH:
    if (file_path != NULL) {
        free(file_path);
    }
ERR_RET_OUTPUTFILE:
    if (*ppOutputFile != NULL) {
        free(*ppOutputFile);
        *ppOutputFile = NULL;
    }

    return NULL;
}

static AX_S32 OutputSaveYUVFile(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_T *frameInfo,
                                FILE *fp_out, AX_CHAR *pOutputFilePath)
{
    int ret = 0;
    AX_S32 sRet = 0;
    AX_U32 i;
    AX_VOID *p_lu = NULL;
    AX_VOID *p_ch = NULL;

    AX_S32 s32Ret = 0;
    AX_VOID *pLumaVirAddr = NULL;
    AX_VOID *pChromaVirAddr = NULL;
    AX_U32 lumaMapSize = 0;
    AX_U32 chromaMapSize = 0;
    int tmp_size = 0;

    if (NULL == frameInfo || NULL == fp_out) {
        SAMPLE_CRIT_LOG("VdGrp=%d, NULL == frameInfo || NULL == fp_out\n", VdGrp);
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if ((frameInfo->stVFrame.u64PhyAddr[0] == 0) || (frameInfo->stVFrame.u32PicStride[0] == 0)) {
        SAMPLE_LOG("VdGrp=%d, stVFrame.u64PhyAddr[0]:0x%llx stVFrame.u32PicStride[0]:%d\n",
                   VdGrp, frameInfo->stVFrame.u64PhyAddr[0], frameInfo->stVFrame.u32PicStride[0]);
        s32Ret = 0;
        goto ERR_RET;
    }

    lumaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16);
    pLumaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[0], lumaMapSize);
    if (NULL == pLumaVirAddr) {
        SAMPLE_CRIT_LOG("VdGrp=%d, AX_SYS_Mmap luma FAILED! .u64PhyAddr[0]:0x%llx\n",
                       VdGrp, frameInfo->stVFrame.u64PhyAddr[0]);
        s32Ret = AX_ERR_VDEC_BAD_ADDR;
        goto ERR_RET;
    }

    SAMPLE_LOG_N("AX_SYS_Mmap luma success, .u64PhyAddr[0]:0x%llx, pLumaVirAddr=%p, lumaMapSize=%d\n",
               frameInfo->stVFrame.u64PhyAddr[0], pLumaVirAddr, lumaMapSize);

    p_lu = pLumaVirAddr;

    SAMPLE_LOG_N("p_lu: %p\n", p_lu);
    SAMPLE_LOG_N("lu_buss: 0x%llx\n", frameInfo->stVFrame.u64PhyAddr[0]);
    SAMPLE_LOG_N("ch_buss: 0x%llx\n", frameInfo->stVFrame.u64PhyAddr[1]);

    AX_U32 coded_width = frameInfo->stVFrame.u32Width;
    AX_U32 coded_height = frameInfo->stVFrame.u32Height;
    AX_U32 pic_stride = frameInfo->stVFrame.u32PicStride[0];
    AX_U32 coded_width_ch = frameInfo->stVFrame.u32Width;
    AX_U32 coded_h_ch = frameInfo->stVFrame.u32Height / 2;
    AX_U32 pic_stride_ch = frameInfo->stVFrame.u32PicStride[1];
    AX_U32 pic_format = frameInfo->stVFrame.enImgFormat;

    if (AX_FORMAT_YUV400 != pic_format) {
        if (AX_FORMAT_YUV444_SEMIPLANAR == pic_format) {
            chromaMapSize = lumaMapSize * 2;
            coded_h_ch = coded_height * 2;
        } else {
            chromaMapSize = lumaMapSize;
        }
        pChromaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[1], chromaMapSize);
        if (NULL == pChromaVirAddr) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_SYS_Mmap luma FAILED! .u64PhyAddr[1]:0x%llx\n",
                       VdGrp, frameInfo->stVFrame.u64PhyAddr[1]);
            s32Ret = AX_ERR_VDEC_BAD_ADDR;
            goto ERR_RET_MUNMAP_LUMA;
        }

        p_ch = pChromaVirAddr;

        SAMPLE_LOG_N("AX_SYS_Mmap chroma success, .u64PhyAddr[1]:0x%llx, pChromaVirAddr=%p, chromaMapSize=%d\n",
               frameInfo->stVFrame.u64PhyAddr[1], pChromaVirAddr, chromaMapSize);
        SAMPLE_LOG_N("p_ch: %p\n", p_ch);
    }

    SAMPLE_LOG_N("write Y\n");
    if (AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010 == pic_format) {
        coded_width = coded_width * 2;
        coded_width_ch = coded_width_ch * 2;
    } else if (AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010 == pic_format) {
        coded_width = coded_width * 10 / 8;
        coded_width_ch = coded_width_ch * 10 / 8;
    }

    SAMPLE_LOG_N("p_lu: %p, p_ch: %p, \n", p_lu, p_ch);
    SAMPLE_LOG_N("coded_width: %u, coded_height: %u, pic_stride: %u, \n"
               "coded_width_ch: %u, coded_h_ch: %u, pic_stride_ch: %u, pixel_bytes: %u, pic_format:%d\n",
               coded_width, coded_height, pic_stride,
               coded_width_ch, coded_h_ch, pic_stride_ch, 1, pic_format);

    for (i = 0; i < coded_height; i++) {
        tmp_size = fwrite(p_lu, 1, coded_width, fp_out);
        if (tmp_size != coded_width) {
            SAMPLE_CRIT_LOG("VdGrp=%d, fwrite FAILED! tmp_size:0x%x != coded_width:0x%x",
                           VdGrp, tmp_size, coded_width);
            s32Ret = AX_ERR_VDEC_NOMEM;
        }
        p_lu += pic_stride;
    }

    if (AX_FORMAT_YUV400 != pic_format) {
        SAMPLE_LOG_N("write UV\n");
        for (i = 0; i < coded_h_ch; i++) {
            tmp_size = fwrite(p_ch, 1, coded_width_ch, fp_out);
            if (tmp_size != coded_width_ch) {
                SAMPLE_CRIT_LOG("VdGrp=%d, fwrite FAILED! tmp_size:0x%x != coded_width_ch:0x%x",
                               VdGrp, tmp_size, coded_width_ch);
                s32Ret = AX_ERR_VDEC_NOMEM;
            }
            p_ch += pic_stride_ch;
        }
    }

    ret = fflush(fp_out);
    if (ret) {
        SAMPLE_CRIT_LOG("VdGrp=%d, fflush FAILED! fp_out:%p",
                       VdGrp, fp_out);
        s32Ret = AX_ERR_VDEC_RUN_ERROR;
    }

    SAMPLE_LOG_N("VdGrp=%d, write YUV done! %s\n", VdGrp, pOutputFilePath);

    if (pChromaVirAddr) {
        sRet = AX_SYS_Munmap(pChromaVirAddr, chromaMapSize);
        if (sRet) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_SYS_Munmap chroma FAILED, sRet=0x%x\n",
                           VdGrp, sRet);
        }
    }

ERR_RET_MUNMAP_LUMA:
    if (pLumaVirAddr) {
        sRet = AX_SYS_Munmap(pLumaVirAddr, lumaMapSize);
        if (sRet) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_SYS_Munmap luma FAILED, sRet=0x%x\n",
                           VdGrp, sRet);
        }
    }
ERR_RET:
    return s32Ret || sRet;
}

AX_S32 OutputFileSaveYUV(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_T *frameInfo, FILE *fp_out, AX_CHAR *pOutputFilePath)
{
    AX_S32 s32Ret = 0;

    if (NULL == frameInfo) {
        SAMPLE_CRIT_LOG("VdGrp=%d, NULL == frameInfo\n", VdGrp);
        return -1;
    }

    if (AX_COMPRESS_MODE_NONE != frameInfo->stVFrame.stCompressInfo.enCompressMode) {
        SAMPLE_CRIT_LOG("VdGrp=%d, invalid enCompressMode: %d\n", VdGrp,
                        frameInfo->stVFrame.stCompressInfo.enCompressMode);
        return -1;
    }

    s32Ret = OutputSaveYUVFile(VdGrp, frameInfo, fp_out, pOutputFilePath);

    return s32Ret;
}

static off_t __FindFileNextStartCode(const SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, AX_U32 *puZeroCount, AX_U32 *pNalType)
{
    AX_S32 i;
    off_t sStart;
    off_t oFileStart;
    off_t oFileOffset = 0;
    AX_U32 uLeftFileSize, uFileReadLen = 0;
    AX_CHAR tmp_buf[VDEC_BS_PARSER_BUF_SIZE] = {0};
    AX_U8 byte;
    int ret_val;
    int ret;

    oFileStart = ftello(pstBsInfo->fInput);
    sStart = oFileStart;
    *puZeroCount = 0;

    while (1) {
        assert(sStart <= pstBsInfo->sFileSize);

        uLeftFileSize = pstBsInfo->sFileSize - sStart;
        if (uLeftFileSize == 0) {
            oFileOffset = pstBsInfo->sFileSize - 1;
            break;
        }

        uFileReadLen = VDEC_BS_PARSER_BUF_SIZE < uLeftFileSize ? VDEC_BS_PARSER_BUF_SIZE : uLeftFileSize;
        fread(tmp_buf, 1, uFileReadLen, pstBsInfo->fInput);
        /* Scan for the beginning of the packet. */
        for (i = 0; i < uFileReadLen; i++) {
            ret_val = tmp_buf[i];
            oFileOffset = sStart + i;
            if (ret_val == EOF) {
                sStart = oFileOffset - 1;
                SAMPLE_LOG_N("sStart:%ld, i:%d, oFileOffset:%d, *puZeroCount:%d",
                            sStart, i, oFileOffset, *puZeroCount);

                ret = fseeko(pstBsInfo->fInput, 0, SEEK_END);
                if (ret) {
                    SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
                    sStart = AX_ERR_VDEC_RUN_ERROR;
                }

                return sStart;
            }

            byte = (unsigned char)ret_val;
            switch (byte) {
            case 0:
                *puZeroCount = *puZeroCount + 1;
                break;
            case 1:
                /* If there's more than three leading zeros, consider only three
                * of them to be part of this packet and the rest to be part of
                * the previous packet. */
                if (*puZeroCount > 3) {
                    *puZeroCount = 3;
                }

                if (*puZeroCount >= 2) {
                    SAMPLE_LOG_N("sStart:%ld, i:%d, oFileOffset:%d, *puZeroCount:%d",
                                sStart, i, oFileOffset, *puZeroCount);
                    if (i < uFileReadLen - 1) {
                        ret_val = (tmp_buf[i + 1] & 0x1f);
                        if (ret_val == 0x1 || ret_val == 0x7  || ret_val == 0x5) {
                            sStart = oFileOffset - *puZeroCount;
                            *pNalType = ret_val;
                            goto FUNC_RET;
                        }
                    }
                }
                *puZeroCount = 0;
                break;
            default:
                *puZeroCount = 0;
                break;
            }
        }

        ret = fseeko(pstBsInfo->fInput, oFileOffset, SEEK_SET);
        if (ret) {
            SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
            sStart = AX_ERR_VDEC_RUN_ERROR;
        }
        sStart = oFileOffset + 1;
    }

FUNC_RET:

    ret = fseeko(pstBsInfo->fInput, oFileOffset + 1, SEEK_SET);
    if (ret) {
        SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
        sStart = AX_ERR_VDEC_RUN_ERROR;
    }
    SAMPLE_LOG_N("func end, ftello(pstBsInfo->fInput):%d uFileReadLen:%d",
                ftello(pstBsInfo->fInput), uFileReadLen);
    return sStart;
}

static int __CheckFileAccessUnitBoundary(const SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, off_t oNalBegin, SAMPLE_BSBOUNDARY_TYPE_E *penBoundary)
{
    int ret = 0;
    int iNalType, iVal;
    SAMPLE_BSBOUNDARY_TYPE_E enBoundary = BSPARSER_NO_BOUNDARY;

    FILE *fInput = pstBsInfo->fInput;
    off_t sStart = ftello(fInput);

    off_t tmp_offset = oNalBegin + 1;
    ret = fseeko(fInput, tmp_offset, SEEK_SET);
    if (ret) {
        SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
        return ret;
    }

    AX_PAYLOAD_TYPE_E enDecType = pstBsInfo->enDecType;

    if (enDecType == PT_H264) {
        iNalType = (getc(fInput) & 0x1F);

        if (iNalType > NAL_CODED_SLICE_IDR) {
            enBoundary = BSPARSER_BOUNDARY_NON_SLICE_NAL;
        } else {
            iVal = getc(fInput);
            /* Check if first mb in slice is 0(ue(v)). */
            if (iVal & 0x80) {
                enBoundary = BSPARSER_BOUNDARY;
            }
        }
    } else if (enDecType == PT_H265) {
        iNalType = (getc(fInput) & 0x7E) >> 1;

        if (iNalType > NAL_CODED_SLICE_CRA) {
            enBoundary = BSPARSER_BOUNDARY_NON_SLICE_NAL;
        } else {
            iVal = getc(fInput);  // nothing interesting here...
            iVal = getc(fInput);
            /* Check if first slice segment in picture */
            if (iVal & 0x80) {
                enBoundary = BSPARSER_BOUNDARY;
            }
        }
    }

    ret = fseeko(fInput, sStart, SEEK_SET);
    if (ret) {
        SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
        return ret;
    }

    *penBoundary = enBoundary;
    return 0;
}


static AX_S32 __StreamReadFrameInRingBuf(const SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, SAMPLE_STREAM_BUF_T *pstStreamBuf,
                                         size_t oStreamLen, size_t *pReadLen)
{
    off_t oOffset = 0;
    AX_U8 *pBufRd = NULL;
    AX_U8 *pBufStart = NULL;
    AX_U32 uBufSize = 0;
    AX_S32 sRet = 0;
    AX_U32 sReadLen = 0;
    AX_U32 tmp_len = 0, right_len = 0, left_len = 0;

    if (pstBsInfo == NULL) {
        SAMPLE_CRIT_LOG("pstBsInfo == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstStreamBuf == NULL) {
        SAMPLE_CRIT_LOG("pstStreamBuf == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    pBufRd = pstStreamBuf->pBufAfterFill;
    pBufStart = pstStreamBuf->tBufAddr.pVirAddr;
    uBufSize = pstStreamBuf->uBufSize;
    if (pBufStart == NULL) {
        SAMPLE_CRIT_LOG("pBufStart == NULL\n");
        sRet = AX_ERR_VDEC_BAD_ADDR;
        goto ERR_RET;
    }

    if (uBufSize < oStreamLen) {
        SAMPLE_CRIT_LOG("uBufSize:0x%x < oStreamLen:0x%x", uBufSize, (AX_U32)oStreamLen);
        sRet = AX_ERR_VDEC_NOMEM;
        goto ERR_RET;
    }

    *pReadLen = 0;
    oOffset = (off_t)(pBufRd - pBufStart);
    pstStreamBuf->pBufBeforeFill = pstStreamBuf->pBufAfterFill;
    if ((oOffset + oStreamLen) < uBufSize) {
        sReadLen = fread(pBufRd, 1, oStreamLen, pstBsInfo->fInput);
        if (sReadLen != oStreamLen) {
            SAMPLE_CRIT_LOG("fread FAILED! sReadLen:0x%x != oStreamLen:0x%x", sReadLen, (AX_U32)oStreamLen);
            sRet = AX_ERR_VDEC_RUN_ERROR;
            goto ERR_RET;
        }

        pstStreamBuf->pBufAfterFill = pBufRd + sReadLen;
    } else {
        /* turnaround */
        right_len = uBufSize - oOffset;
        sReadLen = fread(pBufRd, 1, right_len, pstBsInfo->fInput);
        if (sReadLen != uBufSize - oOffset) {
            SAMPLE_CRIT_LOG("fread FAILED! sReadLen:0x%x != (uBufSize:0x%x - oOffset:0x%llx):0x%x",
                           sReadLen, uBufSize, (AX_U64)oOffset, right_len);
            sRet = AX_ERR_VDEC_RUN_ERROR;
            goto ERR_RET;
        }

        left_len = oStreamLen - (uBufSize - oOffset);
        tmp_len = fread(pBufStart, 1, left_len, pstBsInfo->fInput);
        if (tmp_len != left_len) {
            SAMPLE_CRIT_LOG("fread FAILED! tmp_len:0x%x != left_len:0x%x, oStreamLen:0x%x uBufSize:0x%x oOffset:0x%llx",
                               tmp_len, left_len, (AX_U32)oStreamLen, uBufSize, (AX_U64)oOffset);
            sRet = AX_ERR_VDEC_RUN_ERROR;
            goto ERR_RET;
        }

        sReadLen += tmp_len;
        pstStreamBuf->pBufAfterFill = pBufStart + tmp_len;
    }

    *pReadLen = sReadLen;
ERR_RET:
    return sRet;
}

AX_S32 StreamFileParserReadFrame(const SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen)
{
    int ret = 0;
    AX_S32 sRet = 0;
    AX_U32 uZeroCount = 0;
    SAMPLE_BSBOUNDARY_TYPE_E enBoundary = BSPARSER_NO_BOUNDARY;
    off_t oNalBegin, oStreamLen;
    off_t oBegin, oEnd, oTmpEnd;
    AX_U32 sReadLen = 0;
    AX_U32 uBufSize;
    AX_U8 *pBufStart = NULL;
    AX_U32 u32NalType = 0;

    if (pstBsInfo == NULL) {
        SAMPLE_CRIT_LOG("pstBsInfo == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstStreamBuf == NULL) {
        SAMPLE_CRIT_LOG("pstStreamBuf == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstStreamBuf->tBufAddr.pVirAddr == NULL) {
        SAMPLE_CRIT_LOG("pstStreamBuf->tBufAddr.pVirAddr == NULL\n");
        sRet = AX_ERR_VDEC_BAD_ADDR;
        goto ERR_RET;
    }

    if (pReadLen == NULL) {
        SAMPLE_CRIT_LOG("pReadLen == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    oBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);

    /* Check for non-slice type in current NAL. non slice NALs are
     * decoded one-by-one */
    oNalBegin = oBegin + uZeroCount;
    ret = __CheckFileAccessUnitBoundary(pstBsInfo, oNalBegin, &enBoundary);
    if (ret) {
        SAMPLE_CRIT_LOG("__CheckFileAccessUnitBoundary FAILED! ret:0x%x", ret);
        sRet = AX_ERR_VDEC_STRM_ERROR;
        goto ERR_RET;
    }

    if (u32NalType == 0x7) { /* get i frm video nal */
        oNalBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);

        /* Check for non-slice type in current NAL. non slice NALs are
         * decoded one-by-one */
        oNalBegin = oNalBegin + uZeroCount;
        ret = __CheckFileAccessUnitBoundary(pstBsInfo, oNalBegin, &enBoundary);
        if (ret) {
            SAMPLE_CRIT_LOG("__CheckFileAccessUnitBoundary FAILED! ret:0x%x", ret);
            sRet = AX_ERR_VDEC_STRM_ERROR;
            goto ERR_RET;
        }
    }

    oEnd = oNalBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);

    if (oEnd == oBegin) {
        *pReadLen = 0;
        return 0; /* End of stream */
    }

    /* if there is more stream and a slice type NAL */
    if (enBoundary != BSPARSER_BOUNDARY_NON_SLICE_NAL) {
        while (1) {
            oEnd = oNalBegin;
            oNalBegin += uZeroCount;

            /* Check access unit boundary for next NAL */
            ret = __CheckFileAccessUnitBoundary(pstBsInfo, oNalBegin, &enBoundary);
            if (ret) {
                SAMPLE_CRIT_LOG("__CheckFileAccessUnitBoundary FAILED! ret:0x%x", ret);
                sRet = AX_ERR_VDEC_STRM_ERROR;
                goto ERR_RET;
            }

            if (enBoundary == BSPARSER_NO_BOUNDARY) {
                oNalBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);
            }
            else if (enBoundary == BSPARSER_BOUNDARY_NON_SLICE_NAL) {
                while (1) {
                    oNalBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);
                    oTmpEnd = oNalBegin;
                    oNalBegin += uZeroCount;
                    ret = __CheckFileAccessUnitBoundary(pstBsInfo, oNalBegin, &enBoundary);
                    if (ret) {
                        SAMPLE_CRIT_LOG("__CheckFileAccessUnitBoundary FAILED! ret:0x%x", ret);
                        sRet = AX_ERR_VDEC_STRM_ERROR;
                        goto ERR_RET;
                    }

                    if (enBoundary != BSPARSER_BOUNDARY_NON_SLICE_NAL) {
                        break;
                    }

                    if (oTmpEnd == oNalBegin) {
                        break;
                    }
                }

                if (oTmpEnd == oNalBegin) {
                    break;
                }

                if (enBoundary == BSPARSER_NO_BOUNDARY) {
                    oNalBegin = __FindFileNextStartCode(pstBsInfo, &uZeroCount, &u32NalType);
                }
            }

            if (enBoundary == BSPARSER_BOUNDARY) {
                break;
            }

            if (oEnd == oNalBegin) {
                break;
            }
        }
    }

    ret = fseeko(pstBsInfo->fInput, oBegin, SEEK_SET);
    if (ret) {
        SAMPLE_CRIT_LOG("fseeko FAILED! ret:0x%x", ret);
        sRet = AX_ERR_VDEC_RUN_ERROR;
        goto ERR_RET;
    }

    oStreamLen = oEnd - oBegin;
    uBufSize = pstStreamBuf->uBufSize;
    if (uBufSize < oStreamLen) {
        SAMPLE_CRIT_LOG("uBufSize:0x%x < oStreamLen:0x%x. bufSize is not enough, please increase STREAM_BUFFER_MAX_SIZE",
                        uBufSize, (AX_U32)oStreamLen);
        sRet = AX_ERR_VDEC_NOMEM;
        goto ERR_RET;
    }

    pBufStart = pstStreamBuf->tBufAddr.pVirAddr;

    sReadLen = fread(pBufStart, 1, oStreamLen, pstBsInfo->fInput);
    if (sReadLen != oStreamLen) {
        SAMPLE_CRIT_LOG("fread FAILED! sReadLen:0x%x != oStreamLen:0x%x", sReadLen, (AX_U32)oStreamLen);
        sRet = AX_ERR_VDEC_RUN_ERROR;
        goto ERR_RET;
    }

    pstStreamBuf->pBufBeforeFill = pBufStart;
    pstStreamBuf->pBufAfterFill = pBufStart + sReadLen;

    SAMPLE_LOG("sReadLen:0x%lx, uBufSize:0x%x", sReadLen, uBufSize);
    *pReadLen = sReadLen;
    return 0;
ERR_RET:
    return sRet;
}

static AX_U8 imgGetBytes(FILE *fInput, size_t pos)
{
    AX_U8 data = 0;
    AX_U8 readLen = 0;

    fseeko(fInput, pos, SEEK_SET);
    readLen = fread(&data, 1, 1, fInput);
    if(1 != readLen)
        SAMPLE_CRIT_LOG(" read file failed\n");

    return data;
}

AX_S32 StreamParserReadFrameJpeg(SAMPLE_INPUT_FILE_INFO_T *pstBsInfo, SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen)
{
    size_t i,j;
    AX_U32 jpeg_thumb_in_stream = 0;
    AX_U64 tmp, tmp1, tmp_total = 0;
    size_t curPos = 0;
    AX_U32 imgLen = 0;
    AX_S32 s32Ret = 0;
    size_t stream_length = 0;
    AX_U8 *pBufStart = NULL;
    size_t sReadLen = 0;

    pBufStart = pstStreamBuf->tBufAddr.pVirAddr;
    if (pBufStart == NULL) {
        SAMPLE_CRIT_LOG("pBufStart == NULL\n");
        return -1;
    }

    stream_length = pstBsInfo->sFileSize;
    for (i = pstBsInfo->curPos; i < stream_length; ++i) {
        if (0xFF == imgGetBytes(pstBsInfo->fInput, i)) {
            /* if 0xFFE1 to 0xFFFD ==> skip  */
            if ((((i + 1) < stream_length) &&
                0xE1 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE2 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE3 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE4 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE5 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE6 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE7 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE8 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xE9 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xEA == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xEB == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xEC == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xED == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xEE == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xEF == imgGetBytes(pstBsInfo->fInput, i + 1))) {
                /* increase counter */
                i += 2;

                /* check length vs. data */
                if ((i + 1) > (stream_length)) {
                    s32Ret = AX_ERR_VDEC_STRM_ERROR;
                    goto ret;
                }

                /* get length */
                tmp = imgGetBytes(pstBsInfo->fInput, i);
                tmp1 = imgGetBytes(pstBsInfo->fInput, i + 1);
                tmp_total = (tmp << 8) | tmp1;

                /* check length vs. data */
                if ((tmp_total + i) > (stream_length)) {
                    s32Ret = AX_ERR_VDEC_STRM_ERROR;
                    goto ret;
                }
                /* update */
                i += tmp_total-1;
                continue;
            }

            /* if 0xFFC2 to 0xFFCB ==> skip  */
            if ((((i + 1) < stream_length) &&
                0xC1 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC2 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC3 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC5 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC6 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC7 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC8 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xC9 == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xCA == imgGetBytes(pstBsInfo->fInput, i + 1)) ||
                (((i + 1) < stream_length) &&
                0xCB == imgGetBytes(pstBsInfo->fInput, i + 1)) ) {
                /* increase counter */
                i += 2;

                /* check length vs. data */
                if ((i + 1) > (stream_length)) {
                    s32Ret = AX_ERR_VDEC_STRM_ERROR;
                    goto ret;
                }

                /* get length */
                tmp = imgGetBytes(pstBsInfo->fInput, i);
                tmp1 = imgGetBytes(pstBsInfo->fInput, i + 1);
                tmp_total = (tmp << 8) | tmp1;

                /* check length vs. data */
                if ((tmp_total + i) > (stream_length)) {
                    s32Ret = AX_ERR_VDEC_STRM_ERROR;
                    goto ret;
                }
                /* update */
                i += tmp_total-1;

                /* look for EOI */
                for(j = i; j < stream_length; ++j) {
                    if (0xFF == imgGetBytes(pstBsInfo->fInput, j)) {
                        /* EOI */
                        if (((j + 1) < stream_length) &&
                            0xD9 == imgGetBytes(pstBsInfo->fInput, j + 1)) {
                            /* check length vs. data */
                            if ((j + 2) >= (stream_length)) {
                                curPos = j + 2;
                                s32Ret = 0;
                                goto ret;
                            }
                            /* update */
                            i = j;
                            /* stil data left ==> continue */
                            continue;
                        }
                    }
                }
            }

            /* check if thumbnails in stream */
            if (((i + 1) < stream_length) &&
                0xE0 == imgGetBytes(pstBsInfo->fInput, i + 1)) {
                if (((i + 9) < stream_length) &&
                    0x4A == imgGetBytes(pstBsInfo->fInput, i + 4) &&
                    0x46 == imgGetBytes(pstBsInfo->fInput, i + 5) &&
                    0x58 == imgGetBytes(pstBsInfo->fInput, i + 6) &&
                    0x58 == imgGetBytes(pstBsInfo->fInput, i + 7) &&
                    0x00 == imgGetBytes(pstBsInfo->fInput, i + 8) &&
                    0x10 == imgGetBytes(pstBsInfo->fInput, i + 9)) {
                    jpeg_thumb_in_stream = 1;
                }
            }

            /* EOI */
            if (((i + 1) < stream_length) &&
                0xD9 == imgGetBytes(pstBsInfo->fInput, i + 1)) {
                curPos = i + 2;
                /* update amount of thumbnail or full resolution image */
                if (jpeg_thumb_in_stream) {
                    jpeg_thumb_in_stream = 0;
                } else {
                    s32Ret = 0;
                    goto ret;
                }
            }
        }
    }

ret:
    imgLen = curPos > pstBsInfo->curPos ? curPos - pstBsInfo->curPos : 0;
    if(0 == s32Ret) {
        fseeko(pstBsInfo->fInput, pstBsInfo->curPos, SEEK_SET);
        if (pstStreamBuf->bRingbuf == AX_TRUE) {
            s32Ret = __StreamReadFrameInRingBuf(pstBsInfo, pstStreamBuf, imgLen, &sReadLen);
            if (s32Ret) {
                SAMPLE_CRIT_LOG("__StreamReadFrameInRingBuf FAILED! ret:0x%x", s32Ret);
                return -1;
            }
        } else {
            if (imgLen > pstStreamBuf->uBufSize) {
                SAMPLE_CRIT_LOG("bufSize is not enough(imgLen %d > bufSize %d), please increase STREAM_BUFFER_MAX_SIZE",
                                imgLen, pstStreamBuf->uBufSize);
                return -1;
            }
            sReadLen = fread(pBufStart, 1, imgLen, pstBsInfo->fInput);
            if (sReadLen != imgLen) {
                SAMPLE_CRIT_LOG("fread FAILED! sReadLen:0x%x != imgLen:0x%x", (AX_U32)sReadLen, imgLen);
                return -1;
            }

            pstStreamBuf->pBufBeforeFill = pBufStart;
            pstStreamBuf->pBufAfterFill = pBufStart + sReadLen;
        }
        pstBsInfo->curPos += sReadLen;
    }

    *pReadLen = imgLen;
    return s32Ret;
}

static AX_S32 __VdecUsrPicRead(FILE *pFileIn, AX_VIDEO_FRAME_T *pstFrame)
{
    AX_S32 sRet = 0;
    AX_U32 i = 0, rows = 0, realRead = 0;
    AX_S32 widthSrc = 0, strideSrc = 0, heightSrc = 0;
    AX_IMG_FORMAT_E eFmt;
    AX_VOID *pVaddr = NULL;

    if (pFileIn == NULL) {
        SAMPLE_CRIT_LOG("pFileIn == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstFrame == NULL) {
        SAMPLE_CRIT_LOG("pstFrame == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    widthSrc = pstFrame->u32Width;
    heightSrc = pstFrame->u32Height;
    strideSrc = pstFrame->u32PicStride[0];
    eFmt = pstFrame->enImgFormat;
    pVaddr = (AX_VOID *)(AX_ULONG)pstFrame->u64VirAddr[0];
    switch (eFmt) {
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
        rows = heightSrc * 3 / 2;
        for (i = 0; i < rows; i++) {
            realRead = fread(pVaddr, 1, widthSrc, pFileIn);
            if (realRead < widthSrc) {
                SAMPLE_CRIT_LOG("fread failed! line %d realRead=%d <  widthSrc=%d\n", i, realRead, widthSrc);
                sRet = AX_ERR_VDEC_STRM_ERROR;
                goto ERR_RET;
            }
            pVaddr += strideSrc;
        }
        break;
    default:
        SAMPLE_CRIT_LOG("Invalid format, eFmt = %d\n", eFmt);
    }

ERR_RET:
    return sRet;
}

static AX_S32 __VdecUsrPicInfoFill(SAMPLE_VDEC_USRPIC_ARGS_T *pstUsrPicArgs, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic)
{
    AX_POOL_CONFIG_T stPoolConfig;
    AX_U32 FrameSize = 0;
    AX_BLK blkId = AX_INVALID_BLOCKID;
    AX_VDEC_USRPIC_T *pstUserPic = NULL;
    FILE *fpUserYUV = NULL;
    AX_S32 sRet = 0;
    AX_S32 ret = 0;
    AX_VDEC_GRP VdGrp = 0;
    AX_U32 uWidth = 0;
    AX_U32 u32FrameStride = 0;

    if (pstUsrPicArgs == NULL) {
        SAMPLE_CRIT_LOG("pstUsrPicArgs == NULL\n");
        sRet = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    VdGrp = pstUsrPicArgs->VdGrp;
    SAMPLE_LOG("VdGrp=%d begin\n", VdGrp);

    memset(&stPoolConfig, 0x0, sizeof(AX_POOL_CONFIG_T));

    if (pstUsrPicArgs->tPicParam.bUserPicEnable) {
        if ((AX_FORMAT_YUV420_SEMIPLANAR != pstUsrPicArgs->tPicParam.enImgFormat) &&
            (AX_FORMAT_YUV420_SEMIPLANAR_VU != pstUsrPicArgs->tPicParam.enImgFormat)) {
            SAMPLE_CRIT_LOG("VdGrp=%d, unsupport enImgFormat:%d\n",
                                VdGrp, pstUsrPicArgs->tPicParam.enImgFormat);
            sRet = AX_ERR_VDEC_NOT_SUPPORT;
            goto ERR_RET;
        }
        uWidth = pstUsrPicArgs->tPicParam.u32PicWidth;
        u32FrameStride = AX_COMM_ALIGN(uWidth, AX_VDEC_WIDTH_ALIGN);
        pstVdecUserPic->pUsrPicFilePath = pstUsrPicArgs->tPicParam.pUsrPicFilePath;

        if ((pstUsrPicArgs->enDecType == PT_JPEG) || (pstUsrPicArgs->enDecType == PT_MJPEG)) {
            FrameSize = AX_JDEC_GetYuvBufferSize(u32FrameStride, pstUsrPicArgs->tPicParam.u32PicHeight, AX_FORMAT_YUV444_SEMIPLANAR);
        } else {
            FrameSize = AX_VDEC_GetPicBufferSize(u32FrameStride, pstUsrPicArgs->tPicParam.u32PicHeight, pstUsrPicArgs->enDecType);
        }

        stPoolConfig.MetaSize = 512;
        stPoolConfig.BlkCnt = 1;

        stPoolConfig.BlkSize = FrameSize;
        stPoolConfig.CacheMode = AX_POOL_CACHE_MODE_NONCACHE;
        snprintf((AX_CHAR *)stPoolConfig.PartitionName, AX_MAX_PARTITION_NAME_LEN, "anonymous");
        pstVdecUserPic->PoolId = AX_POOL_CreatePool(&stPoolConfig);
        if (AX_INVALID_POOLID == pstVdecUserPic->PoolId) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_POOL_CreatePool FAILED! BlkCnt:%d, BlkSize:0x%llx\n",
                            VdGrp, stPoolConfig.BlkCnt, stPoolConfig.BlkSize);
            sRet = AX_ERR_VDEC_NOBUF;
            goto ERR_RET_DESTROY;
        }

        blkId = AX_POOL_GetBlock(pstVdecUserPic->PoolId, FrameSize, NULL);
        if (AX_INVALID_BLOCKID == blkId) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_POOL_GetBlock FAILED! PoolId:%d, BlkSize:0x%llx\n",
                                VdGrp, pstVdecUserPic->PoolId, stPoolConfig.BlkSize);
            sRet = AX_ERR_VDEC_NOBUF;
            goto ERR_RET_DESTROY;
        }

        pstVdecUserPic->BlkId = blkId;
        pstUserPic = &pstVdecUserPic->stUserPic;
        pstUserPic->bEnable = AX_TRUE;
        pstUserPic->bInstant = pstUsrPicArgs->bUsrInstant;
        pstUserPic->stFrmInfo.bEndOfStream = AX_TRUE;
        pstUserPic->stFrmInfo.enModId = AX_ID_VDEC;
        pstUserPic->stFrmInfo.stVFrame.u32BlkId[0] = blkId;
        pstUserPic->stFrmInfo.stVFrame.u32FrameSize = FrameSize;
        pstUserPic->stFrmInfo.stVFrame.u32Width = pstUsrPicArgs->tPicParam.u32PicWidth;
        pstUserPic->stFrmInfo.stVFrame.u32Height = pstUsrPicArgs->tPicParam.u32PicHeight;
        pstUserPic->stFrmInfo.stVFrame.enImgFormat = pstUsrPicArgs->tPicParam.enImgFormat;
        pstUserPic->stFrmInfo.stVFrame.enVscanFormat = AX_VSCAN_FORMAT_RASTER;
        pstUserPic->stFrmInfo.stVFrame.stCompressInfo.enCompressMode = AX_COMPRESS_MODE_NONE;
        pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(blkId);
        pstUserPic->stFrmInfo.stVFrame.u64VirAddr[0] = (AX_ULONG)AX_POOL_GetBlockVirAddr(blkId);
        pstUserPic->stFrmInfo.stVFrame.u32PicStride[0] = u32FrameStride;
        pstUserPic->stFrmInfo.stVFrame.u32PicStride[1] = u32FrameStride;
        pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[1] = pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[0] +
                                                            pstUserPic->stFrmInfo.stVFrame.u32PicStride[0] *
                                                            pstUserPic->stFrmInfo.stVFrame.u32Height;
        pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[2] = 0;
        pstUserPic->stFrmInfo.stVFrame.u64VirAddr[1] = pstUserPic->stFrmInfo.stVFrame.u64VirAddr[0] +
                                                            pstUserPic->stFrmInfo.stVFrame.u32PicStride[0] *
                                                            pstUserPic->stFrmInfo.stVFrame.u32Height;
        pstUserPic->stFrmInfo.stVFrame.u64VirAddr[2] = 0;
        pstUserPic->stFrmInfo.stVFrame.u64PTS = 0;
    }

    if (pstUsrPicArgs->tPicParam.enImgFormat) {
        uWidth = pstUserPic->stFrmInfo.stVFrame.u32Width;
        pstVdecUserPic->fpUsrPic = fopen(pstVdecUserPic->pUsrPicFilePath,"rb");
        if (pstVdecUserPic->fpUsrPic == NULL) {
            SAMPLE_CRIT_LOG("VdGrp=%d, can't open file %s in VDEC_PREPARE_USERPIC.\n",
                            VdGrp, pstVdecUserPic->pUsrPicFilePath);
            sRet = AX_ERR_VDEC_RUN_ERROR;
            goto ERR_RET_FREE;
        }

        fpUserYUV = pstVdecUserPic->fpUsrPic;

        sRet = __VdecUsrPicRead(fpUserYUV, &pstUserPic->stFrmInfo.stVFrame);
        if (sRet) {
            SAMPLE_CRIT_LOG("__VdecUsrPicRead FAILED! ret:%d\n", sRet);
            goto ERR_RET_FREE;
        }

        sRet = fclose(fpUserYUV);
        if (sRet) {
            SAMPLE_CRIT_LOG("fclose FAILED! ret:%d\n", sRet);
            sRet = AX_ERR_VDEC_RUN_ERROR;
            goto ERR_RET_FREE;
        }

        fpUserYUV = NULL;
        SAMPLE_LOG("VdGrp=%d, .u64PhyAddr[0]:%llx .u64VirAddr[0]:%llx .u64PhyAddr[1]:%llx .u64VirAddr[1]:%llx\n",
                    VdGrp,
                    pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[0],
                    pstUserPic->stFrmInfo.stVFrame.u64VirAddr[0],
                    pstUserPic->stFrmInfo.stVFrame.u64PhyAddr[1],
                    pstUserPic->stFrmInfo.stVFrame.u64VirAddr[1]);
    }

    return AX_SUCCESS;

ERR_RET_FREE:
    if (fpUserYUV) {
        fclose(fpUserYUV);
        fpUserYUV = NULL;
    }

ERR_RET_DESTROY:
    if (pstUsrPicArgs->tPicParam.bUserPicEnable) {
        if (pstVdecUserPic->BlkId != AX_INVALID_BLOCKID) {
            ret = AX_POOL_ReleaseBlock(pstVdecUserPic->BlkId);
            if (ret)
                SAMPLE_CRIT_LOG("VdGrp=%d, PoolI:%d,  AX_POOL_ReleaseBlock FAILED! ret:0x%x %s",
                                VdGrp, pstVdecUserPic->BlkId, ret, SampleVdecRetStr(ret));
        }
        if (pstVdecUserPic->PoolId != AX_INVALID_POOLID) {
            ret = AX_POOL_DestroyPool(pstVdecUserPic->PoolId);
            if (ret)
                SAMPLE_CRIT_LOG("VdGrp=%d, PoolId:%d, AX_POOL_DestroyPool FAILED! ret:0x%x %s",
                            VdGrp, pstVdecUserPic->PoolId, ret, SampleVdecRetStr(ret));
        }
    }

ERR_RET:
    return sRet;
}

AX_S32 __VdecUsrPicCreat(SAMPLE_VDEC_USRPIC_ARGS_T *pstUsrPicArgs, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic)
{
    AX_S32 s32Ret = AX_SUCCESS;
    AX_VDEC_GRP VdGrp = 0;

    if (pstVdecUserPic == NULL) {
        SAMPLE_CRIT_LOG("null pointer\n");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstUsrPicArgs == NULL) {
        SAMPLE_CRIT_LOG("pstUsrPicArgs == NULL\n");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    VdGrp = pstUsrPicArgs->VdGrp;
    SAMPLE_LOG("VdGrp=%d begin\n", VdGrp);

    if (pstUsrPicArgs->tPicParam.bUserPicEnable == AX_FALSE) {
        pstVdecUserPic->PoolId = AX_INVALID_POOLID;
        goto ERR_RET;
    }

    s32Ret = __VdecUsrPicInfoFill(pstUsrPicArgs, pstVdecUserPic);
    if (s32Ret != AX_SUCCESS) {
        SAMPLE_CRIT_LOG("VdGrp=%d, __VdecUsrPicInfoFill FAILED!\n",VdGrp);
        goto ERR_RET;
    }

    s32Ret = AX_VDEC_DisableUserPic(VdGrp);
    if (s32Ret != AX_SUCCESS) {
        SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_DisableUserPic FAILED! ret:0x%x %s\n",
                        VdGrp, s32Ret, SampleVdecRetStr(s32Ret));
        goto ERR_RET;
    }

    s32Ret = AX_VDEC_SetUserPic(VdGrp, &pstVdecUserPic->stUserPic);
    if (s32Ret != AX_SUCCESS) {
        SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_SetUserPic FAILED! ret:0x%x %s\n",
                        VdGrp, s32Ret, SampleVdecRetStr(s32Ret));
        goto ERR_RET;
    }

ERR_RET:
    return s32Ret;
}

AX_S32 VdecUserPicEnable(AX_VDEC_GRP VdGrp, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic,
                         AX_BOOL *pContSendStm, SAMPLE_VDEC_CONTEXT_T *pstCtx)
{
    AX_S32 sRet = AX_SUCCESS;
    AX_VDEC_RECV_PIC_PARAM_T tRecvParam;

    if (pstVdecUserPic->stUserPic.bEnable) {
        sRet = AX_VDEC_StopRecvStream(VdGrp);
        if (sRet) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_StopRecvStream FAILED! ret:0x%x %s\n",
                            VdGrp, sRet, SampleVdecRetStr(sRet));
            goto ERR_RET;
        } else {
            pstCtx->GrpStatus[VdGrp] = AX_VDEC_GRP_STOP_RECV;
        }

        if (!pstVdecUserPic->stUserPic.bInstant)
            sleep(1);

        sRet = AX_VDEC_EnableUserPic(VdGrp);
        if (sRet) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_EnableUserPic FAILED! ret:0x%x %s\n",
                             VdGrp, sRet, SampleVdecRetStr(sRet));
            goto ERR_RET;
        }

        if (pstVdecUserPic->recvStmAfUsrPic) {
            *pContSendStm = AX_TRUE;
        } else {
            *pContSendStm = AX_FALSE;
        }

        SAMPLE_LOG("VdGrp=%d, AX_VDEC_EnableUserPic finish!\n", VdGrp);

        memset(&tRecvParam, 0, sizeof(tRecvParam));
        tRecvParam.s32RecvPicNum = pstVdecUserPic->s32RecvPicNumBak;
        sRet = AX_VDEC_StartRecvStream(VdGrp, &tRecvParam);
        if (sRet) {
            SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_StartRecvStream FAILED! ret:0x%x %s\n",
                            VdGrp, sRet, SampleVdecRetStr(sRet));
            goto ERR_RET;
        } else {
            pstCtx->GrpStatus[VdGrp] = AX_VDEC_GRP_START_RECV;
        }
    }

ERR_RET:
    return sRet;
}

AX_VOID VdecUserPicDestroy(AX_VDEC_GRP VdGrp, SAMPLE_VDEC_USERPIC_T *pstVdecUserPic)
{
    AX_S32 ret = AX_SUCCESS;

    if (pstVdecUserPic->stUserPic.bEnable) {
        if (pstVdecUserPic->BlkId != AX_INVALID_BLOCKID) {
            ret = AX_POOL_ReleaseBlock(pstVdecUserPic->BlkId);
            if (ret)
                SAMPLE_CRIT_LOG("VdGrp=%d, BlkId:%d,  AX_POOL_ReleaseBlock FAILED! ret:0x%x %s",
                                VdGrp, pstVdecUserPic->BlkId, ret, SampleVdecRetStr(ret));
        }
        if (pstVdecUserPic->PoolId != AX_INVALID_POOLID) {
            ret = AX_POOL_DestroyPool(pstVdecUserPic->PoolId);
            if (ret)
                SAMPLE_CRIT_LOG("VdGrp=%d, PoolId:%d, AX_POOL_DestroyPool FAILED! ret:0x%x %s",
                                VdGrp, pstVdecUserPic->PoolId, ret, SampleVdecRetStr(ret));
        }
    }

    if (pstVdecUserPic->stUserPic.bEnable) {
        ret = AX_VDEC_DisableUserPic(VdGrp);
        if (ret)
                SAMPLE_CRIT_LOG("VdGrp=%d, AX_VDEC_DisableUserPic FAILED! ret:0x%x %s",
                                VdGrp, ret, SampleVdecRetStr(ret));
    }
}


int VdecCommonPoolPrintf(AX_VOID)
{
    return 0;
    AX_S32 s32Ret = 0;
    AX_POOL_FLOORPLAN_T PoolFloorPlan;

    memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));

    s32Ret = AX_POOL_GetConfig(&PoolFloorPlan);
    if (s32Ret) {
        SAMPLE_CRIT_LOG("AX_POOL_SetConfig FAILED! 0x%x\n", s32Ret);
        goto ERR_RET;
    }

    for (int pi = 0; pi < AX_MAX_COMM_POOLS; pi++) {
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].MetaSize:0x%llx", pi, PoolFloorPlan.CommPool[pi].MetaSize);
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].BlkSize:0x%llx", pi, PoolFloorPlan.CommPool[pi].BlkSize);
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].BlkCnt:0x%x", pi, PoolFloorPlan.CommPool[pi].BlkCnt);
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].IsMergeMode:0x%x", pi, PoolFloorPlan.CommPool[pi].IsMergeMode);
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].CacheMode:0x%x", pi, PoolFloorPlan.CommPool[pi].CacheMode);
        SAMPLE_LOG("PoolFloorPlan.CommPool[%d].PartitionName:%s", pi, PoolFloorPlan.CommPool[pi].PartitionName);
    }

ERR_RET:
    return s32Ret;
}

#ifdef AX_VDEC_FFMPEG_ENABLE
AX_S32 SampleVdecFfmpegDeinit(SAMPLE_FFMPEG_T *pstFfmpeg, AX_VDEC_GRP VdGrp)
{
    AX_S32 s32Ret = AX_SUCCESS;

    SAMPLE_LOG("stream %d +++", VdGrp);

    if (pstFfmpeg == NULL) {
        SAMPLE_CRIT_LOG("pstFfmpeg == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstFfmpeg->pstAvPkt) {
        // av_packet_unref(pstFfmpeg->pstAvPkt);
        av_packet_free(&pstFfmpeg->pstAvPkt);
        pstFfmpeg->pstAvPkt = NULL;
    }

    if (pstFfmpeg->pstAvBSFCtx) {
        av_bsf_free(&pstFfmpeg->pstAvBSFCtx);
        pstFfmpeg->pstAvBSFCtx = NULL;
    }

    if (pstFfmpeg->pstAvFmtCtx) {
        avformat_close_input(&pstFfmpeg->pstAvFmtCtx);
        pstFfmpeg->pstAvFmtCtx = NULL;
    }

    SAMPLE_LOG("stream %d ---", VdGrp);

ERR_RET:
    return s32Ret;
}

AX_S32 SampleVdecFfmpegInit(SAMPLE_FFMPEG_T *pstFfmpeg, const AX_CHAR *pcInputFilePath,
                            SAMPLE_BITSTREAM_INFO_T *pstBitStreamInfo)
{
    AX_S32 s32Ret = AX_SUCCESS;
    int ret;
    enum AVCodecID eCodecID = AV_CODEC_ID_H264;
    AX_VDEC_GRP VdGrp = 0;

    if (pstFfmpeg == NULL) {
        SAMPLE_CRIT_LOG("pstFfmpeg == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pcInputFilePath == NULL) {
        SAMPLE_CRIT_LOG("pcInputFilePath == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstBitStreamInfo == NULL) {
        SAMPLE_CRIT_LOG("pstBitStreamInfo == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    VdGrp = pstBitStreamInfo->VdGrp;

    pstFfmpeg->s32VideoIndex = -1;
    pstFfmpeg->pstAvFmtCtx = avformat_alloc_context();
    if (pstFfmpeg->pstAvFmtCtx == NULL) {
        SAMPLE_CRIT_LOG("avformat_alloc_context() failed!");
        s32Ret = AX_ERR_VDEC_RUN_ERROR;
        goto ERR_RET;
    }

    ret = avformat_open_input(&pstFfmpeg->pstAvFmtCtx, pcInputFilePath, NULL, NULL);
    if (ret < 0) {
        AX_CHAR szError[64] = {0};
        av_strerror(ret, szError, 64);
        SAMPLE_CRIT_LOG("open %s fail, error: %d, %s", pcInputFilePath, ret, szError);
        goto ERR_RET;
    }

    ret = avformat_find_stream_info(pstFfmpeg->pstAvFmtCtx, NULL);
    if (ret < 0) {
        SAMPLE_CRIT_LOG("avformat_find_stream_info fail, error = %d", ret);
        goto ERR_RET;
    }

    for (int i = 0; i < pstFfmpeg->pstAvFmtCtx->nb_streams; i++) {
        if (AVMEDIA_TYPE_VIDEO == pstFfmpeg->pstAvFmtCtx->streams[i]->codecpar->codec_type) {
            pstFfmpeg->s32VideoIndex = i;
            break;
        }
    }

    if (-1 == pstFfmpeg->s32VideoIndex) {
        SAMPLE_CRIT_LOG("%s has no video stream!", pcInputFilePath);
        goto ERR_RET;
    } else {
        AVStream *pAvs = pstFfmpeg->pstAvFmtCtx->streams[pstFfmpeg->s32VideoIndex];
        eCodecID = pAvs->codecpar->codec_id;
        switch (eCodecID) {
            case AV_CODEC_ID_H264:
                pstBitStreamInfo->eVideoType = PT_H264;
                break;
            case AV_CODEC_ID_HEVC:
                pstBitStreamInfo->eVideoType = PT_H265;
                break;
            default:
                SAMPLE_CRIT_LOG("Current Only support H264 or HEVC stream %d!", VdGrp);
                goto ERR_RET;
        }

        pstBitStreamInfo->nWidth = pAvs->codecpar->width;
        pstBitStreamInfo->nHeight = pAvs->codecpar->height;
        pstBitStreamInfo->nFps = av_q2d(pAvs->r_frame_rate);
        if (0 == pstBitStreamInfo->nFps) {
            pstBitStreamInfo->nFps = 30;
            SAMPLE_LOG("stream %d fps is 0, set to %d fps", VdGrp, pstBitStreamInfo->nFps);
        }

        SAMPLE_LOG("stream %d: vcodec %d, %dx%d, fps %d",
                    VdGrp, pstBitStreamInfo->eVideoType, pstBitStreamInfo->nWidth, pstBitStreamInfo->nHeight,
                    pstBitStreamInfo->nFps);
    }

    pstFfmpeg->pstAvPkt = av_packet_alloc();
    if (!pstFfmpeg->pstAvPkt) {
        SAMPLE_CRIT_LOG("Create packet(stream %d) fail!", VdGrp);
        goto ERR_RET;
    }

    if ((AV_CODEC_ID_H264 == eCodecID) || (AV_CODEC_ID_HEVC == eCodecID)) {
        const AVBitStreamFilter *pstBSFilter = av_bsf_get_by_name((AV_CODEC_ID_H264 == eCodecID) ?
                                                    "h264_mp4toannexb" : "hevc_mp4toannexb");
        if (!pstBSFilter) {
            SAMPLE_CRIT_LOG("av_bsf_get_by_name(stream %d) fail!", VdGrp);
            goto ERR_RET;
        }

        ret = av_bsf_alloc(pstBSFilter, &pstFfmpeg->pstAvBSFCtx);
        if (ret < 0) {
            SAMPLE_CRIT_LOG("av_bsf_alloc(stream %d) fail, error:%d", VdGrp, ret);
            goto ERR_RET;
        }

        ret = avcodec_parameters_copy(pstFfmpeg->pstAvBSFCtx->par_in,
                                      pstFfmpeg->pstAvFmtCtx->streams[pstFfmpeg->s32VideoIndex]->codecpar);
        if (ret < 0) {
            SAMPLE_CRIT_LOG("avcodec_parameters_copy(stream %d) fail, error:%d", VdGrp, ret);
            goto ERR_RET;
        } else {
            pstFfmpeg->pstAvBSFCtx->time_base_in = pstFfmpeg->pstAvFmtCtx->streams[pstFfmpeg->s32VideoIndex]->time_base;
        }

        ret = av_bsf_init(pstFfmpeg->pstAvBSFCtx);
        if (ret < 0) {
            SAMPLE_CRIT_LOG("av_bsf_init(stream %d) fail, error:%d", VdGrp, ret);
            goto ERR_RET;
        }
    }

    return s32Ret;

ERR_RET:
    SampleVdecFfmpegDeinit(pstFfmpeg, VdGrp);
    return s32Ret;
}

AX_S32 SampleVdecFfmpegExtractOnePic(SAMPLE_FFMPEG_T *pstFfmpeg, SAMPLE_BITSTREAM_INFO_T *pstBitStreamInfo,
                                     SAMPLE_STREAM_BUF_T *pstStreamBuf, size_t *pReadLen)
{
    AX_S32 s32Ret = AX_SUCCESS;
    int ret;
    AVFormatContext *pstAvFmtCtx = NULL;
    AVBSFContext *pstAvBSFCtx = NULL;
    AVPacket *pstAvPkt = NULL;
    static AX_U64 u64FrameCnt = 0;
    AX_VDEC_GRP VdGrp = 0;

    if (pstFfmpeg == NULL) {
        SAMPLE_CRIT_LOG("pstFfmpeg == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    pstAvFmtCtx = pstFfmpeg->pstAvFmtCtx;
    if (pstAvFmtCtx == NULL) {
        SAMPLE_CRIT_LOG("pstAvFmtCtx == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    pstAvBSFCtx = pstFfmpeg->pstAvBSFCtx;
    if (pstAvBSFCtx == NULL) {
        SAMPLE_CRIT_LOG("pstAvBSFCtx == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    pstAvPkt = pstFfmpeg->pstAvPkt;
    if (pstAvPkt == NULL) {
        SAMPLE_CRIT_LOG("pstAvPkt == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstBitStreamInfo == NULL) {
        SAMPLE_CRIT_LOG("pstBitStreamInfo == NULL");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstStreamBuf == NULL) {
        SAMPLE_CRIT_LOG("pstStreamBuf == NULL\n");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    if (pstStreamBuf->tBufAddr.pVirAddr == NULL) {
        SAMPLE_CRIT_LOG("pstStreamBuf->tBufAddr.pVirAddr == NULL\n");
        s32Ret = AX_ERR_VDEC_BAD_ADDR;
        goto ERR_RET;
    }

    if (pReadLen == NULL) {
        SAMPLE_CRIT_LOG("pReadLen == NULL\n");
        s32Ret = AX_ERR_VDEC_NULL_PTR;
        goto ERR_RET;
    }

    VdGrp = pstBitStreamInfo->VdGrp;

    while (1) {
        ret = av_read_frame(pstAvFmtCtx, pstAvPkt);
        if (ret < 0) {
            if (AVERROR_EOF == ret) {
                SAMPLE_LOG("reach eof of stream %d ", VdGrp);
                *pReadLen = 0;
                break;
            } else {
                SAMPLE_CRIT_LOG("av_read_frame(stream %d) fail, error: %d", VdGrp, ret);
                s32Ret = AX_ERR_VDEC_STRM_ERROR;
                break;
            }
        } else {
            if (pstAvPkt->stream_index == pstFfmpeg->s32VideoIndex) {
                ret = av_bsf_send_packet(pstAvBSFCtx, pstAvPkt);
                if (ret < 0) {
                    av_packet_unref(pstAvPkt);
                    SAMPLE_CRIT_LOG("av_bsf_send_packet(stream %d) fail, error: %d", VdGrp, ret);
                    s32Ret = AX_ERR_VDEC_RUN_ERROR;
                    break;
                }

                while (ret >= 0) {
                    ret = av_bsf_receive_packet(pstAvBSFCtx, pstAvPkt);
                    if (ret < 0) {
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        }

                        av_packet_unref(pstAvPkt);
                        SAMPLE_CRIT_LOG("av_bsf_receive_packet(stream %d) fail, error: %d", VdGrp, ret);
                        s32Ret = AX_ERR_VDEC_RUN_ERROR;
                        goto ERR_RET;
                    }

                    if (pstStreamBuf->uBufSize < pstAvPkt->size) {
                        SAMPLE_CRIT_LOG("uBufSize:0x%x < pstAvPkt->size:0x%x. "
                                        "bufSize is not enough, please increase STREAM_BUFFER_MAX_SIZE",
                                        pstStreamBuf->uBufSize, pstAvPkt->size);
                        s32Ret = AX_ERR_VDEC_STRM_ERROR;
                        goto ERR_RET;
                    }

                    u64FrameCnt++;
                    *pReadLen = pstAvPkt->size;
                    memcpy(pstStreamBuf->tBufAddr.pVirAddr, pstAvPkt->data, pstAvPkt->size);

                    SAMPLE_LOG_N("u64FrameCnt:%lld, pstAvPkt->size:%d", u64FrameCnt, pstAvPkt->size);
                }
            }

            av_packet_unref(pstAvPkt);
        }

        break;
    }

ERR_RET:
    return s32Ret;
}
#endif

AX_U32 SampleVdecSearchStartCode(AX_U8 *uStrAddr, AX_U32 uLen, AX_U32 *puReadBytes, AX_U32 *puZeroCount)
{
    AX_U32 i = 0;

    if (AX_NULL == uStrAddr) {
        return AX_FALSE;
    }

    if (uLen < 4) {
        return AX_FALSE;
    }

    if (AX_NULL == puReadBytes) {
        return AX_FALSE;
    }

    if (AX_NULL == puZeroCount) {
        return AX_FALSE;
    }

    *puReadBytes = 0;
    *puZeroCount = 0;

    for (i = 0; i < uLen - 4; i++) {
        if ((uStrAddr[i] == 0)
            && (uStrAddr[i + 1] == 0)
            && (uStrAddr[i + 2] == 0)
            && (uStrAddr[i + 3] == 1)) {
            *puReadBytes = i;
            *puZeroCount = 3;
            return AX_TRUE;
        } else if ((uStrAddr[i] == 0)
            && (uStrAddr[i + 1] == 0)
            && (uStrAddr[i + 2] == 1)) {
            *puReadBytes = i;
            *puZeroCount = 2;
            return AX_TRUE;
        }
    }

    return AX_FALSE;
}

AX_VOID SampelVdecSetThreadName(const char *nameFmt, ...)
{
    AX_CHAR name[16];
    va_list args;

    va_start(args, nameFmt);
    vsnprintf(name, sizeof(name), nameFmt, args);
    va_end(args);

    prctl(PR_SET_NAME, name, NULL, NULL, NULL);
}

AX_U64 SampleGetFileSize(char * pFileName)
{
    AX_CHAR cmd[256] = {0};
    FILE *pInfo = NULL;
    AX_U64 fileSize = 0;
    AX_S32 ret = 0;
    AX_U32 i = 0;

    SAMPLE_LOG_TMP("calu file: %s\n", pFileName);
    sprintf(cmd, "stat %s > fileInfo.txt", pFileName);

    ret = system(cmd);
    if (ret)
        return -1;

    pInfo = fopen("fileInfo.txt", "rb");
    if (NULL == pInfo)
        return -1;

    fgets(cmd, 256, pInfo);
    SAMPLE_LOG_TMP("file: %s\n", cmd);
    fgets(cmd, 256, pInfo);
    SAMPLE_LOG_TMP("info: %s\n", cmd);
    fclose(pInfo);

    if (strlen(cmd) < 10) {
        return -1;
    }

    while(cmd[i]) {
        if (('0' <= cmd[i]) && ('9' >= cmd[i]))
            break;

        SAMPLE_LOG_TMP("pos %d char %c\n", i, cmd[i]);
        i++;
    }

    while(cmd[i]) {
        SAMPLE_LOG_TMP("pos %d char %c\n", i, cmd[i]);
        if (('0' > cmd[i]) || ('9' < cmd[i]))
            break;

        fileSize = fileSize * 10 + (cmd[i] - '0');
        i++;
    }

    return fileSize;
}

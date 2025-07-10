/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_BUFFER_TOOL_H_
#define _AX_BUFFER_TOOL_H_

#include "ax_base_type.h"
#include "ax_global_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define AX_COMM_ALIGN(value, n) (((value) + (n) - 1) & ~((n) - 1))

/*
 * uHeight: Image height in pixels
 * uWidthStride: Image width in pixels
 * eImageFormat: Image Format, only support raw
 * pstCompressInfo: Compression mode and compression level
 * uAlignSize: Aligned size in bytes, If input is 0, align by 128 bytes
 */
static __inline AX_U32 AX_VIN_GetImgBufferSize(AX_U32 uHeight, AX_U32 uWidthStride, AX_IMG_FORMAT_E eImageFormat,
        AX_FRAME_COMPRESS_INFO_T *pstCompressInfo, AX_U32 uAlignSize)
{
    AX_U32 uWidthBeat = 0;
    AX_U32 uBufSize = 0;
    AX_U32 uPixBits = 0;
    AX_U32 uNum = 0, uBaseWidth = 0;
    AX_BOOL bFormatYuv = AX_TRUE;

    switch (eImageFormat) {
    case AX_FORMAT_BAYER_RAW_8BPP:
        uPixBits = 8;
        break;
    case AX_FORMAT_BAYER_RAW_10BPP_PACKED:
        uPixBits = 10;
        break;
    case AX_FORMAT_BAYER_RAW_12BPP_PACKED:
        uPixBits = 12;
        break;
    case AX_FORMAT_BAYER_RAW_14BPP_PACKED:
        uPixBits = 14;
        break;
    case AX_FORMAT_BAYER_RAW_10BPP:
    case AX_FORMAT_BAYER_RAW_12BPP:
    case AX_FORMAT_BAYER_RAW_14BPP:
    case AX_FORMAT_BAYER_RAW_16BPP:
        uPixBits = 16;
        break;
    case AX_FORMAT_YUV400:
    case AX_FORMAT_YUV420_PLANAR:
    case AX_FORMAT_YUV420_PLANAR_VU:
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_PLANAR:
    case AX_FORMAT_YUV422_PLANAR_VU:
    case AX_FORMAT_YUV422_SEMIPLANAR:
    case AX_FORMAT_YUV422_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_INTERLEAVED_YUVY:
    case AX_FORMAT_YUV422_INTERLEAVED_YUYV:
    case AX_FORMAT_YUV422_INTERLEAVED_UYVY:
    case AX_FORMAT_YUV422_INTERLEAVED_VYUY:
    case AX_FORMAT_YUV444_PLANAR:
    case AX_FORMAT_YUV444_PLANAR_VU:
    case AX_FORMAT_YUV444_SEMIPLANAR:
    case AX_FORMAT_YUV444_SEMIPLANAR_VU:
    case AX_FORMAT_YUV444_PACKED:
        uPixBits = 8;
        break;
    case AX_FORMAT_YUV400_10BIT:
    case AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010:
    case AX_FORMAT_YUV444_PACKED_10BIT_P101010:
        uPixBits = 10;
        break;
    case AX_FORMAT_YUV420_PLANAR_10BIT_I010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016:
    case AX_FORMAT_YUV444_PACKED_10BIT_P010:
        uPixBits = 16;
        break;
    default:
        uPixBits = 8;
        break;
    }

    if (pstCompressInfo != AX_NULL) {
        if (pstCompressInfo->enCompressMode != AX_COMPRESS_MODE_NONE) {
            switch (eImageFormat) {
            case AX_FORMAT_BAYER_RAW_8BPP:
                uPixBits = 8;
                break;
            case AX_FORMAT_BAYER_RAW_10BPP:
            case AX_FORMAT_BAYER_RAW_10BPP_PACKED:
                uPixBits = 10;
                break;
            case AX_FORMAT_BAYER_RAW_12BPP:
            case AX_FORMAT_BAYER_RAW_12BPP_PACKED:
                uPixBits = 12;
                break;

            case AX_FORMAT_BAYER_RAW_14BPP:
            case AX_FORMAT_BAYER_RAW_14BPP_PACKED:
                uPixBits = 14;
                break;

            case AX_FORMAT_BAYER_RAW_16BPP:
                uPixBits = 16;
                break;
            default:
                break;
            }
        }
    }

    if (((uWidthStride * uPixBits) % 64) != 0) {
        uWidthBeat = ((uWidthStride * uPixBits) / 64) + 1;
    } else {
        uWidthBeat = (uWidthStride * uPixBits) / 64 ;
    }

    if (uAlignSize != 0) {
        uWidthBeat = (((uWidthBeat * 8 + uAlignSize - 1) / uAlignSize) * uAlignSize);
        uWidthBeat = uWidthBeat / 8;
    } else {
        uWidthBeat = (((uWidthBeat * 8  + 63) / 64) * 64);
        uWidthBeat = uWidthBeat / 8;
    }

    // calc dma buffer size
    switch (eImageFormat) {
    case AX_FORMAT_BAYER_RAW_8BPP:
    case AX_FORMAT_BAYER_RAW_10BPP:
    case AX_FORMAT_BAYER_RAW_12BPP:
    case AX_FORMAT_BAYER_RAW_14BPP:
    case AX_FORMAT_BAYER_RAW_16BPP:
    case AX_FORMAT_BAYER_RAW_10BPP_PACKED:
    case AX_FORMAT_BAYER_RAW_12BPP_PACKED:
    case AX_FORMAT_BAYER_RAW_14BPP_PACKED:
        uHeight = (uHeight + 7) / 8 * 8;
        uBufSize = uWidthBeat * 8 * uHeight;
        bFormatYuv = AX_FALSE;
        break;
    case AX_FORMAT_YUV400:
    case AX_FORMAT_YUV400_10BIT:
        uHeight = (uHeight + 7) / 8 * 8;
        uBufSize = uWidthBeat * 8 * uHeight;
        break;
    case AX_FORMAT_YUV420_PLANAR:
    case AX_FORMAT_YUV420_PLANAR_VU:
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
    case AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B:
    case AX_FORMAT_YUV420_PLANAR_10BIT_I010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016:
        uHeight = (uHeight + 7) / 8 * 8;
        uBufSize = uWidthBeat * 8 * uHeight * 3 / 2;
        break;
    case AX_FORMAT_YUV422_PLANAR:
    case AX_FORMAT_YUV422_PLANAR_VU:
    case AX_FORMAT_YUV422_SEMIPLANAR:
    case AX_FORMAT_YUV422_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_INTERLEAVED_YUVY:
    case AX_FORMAT_YUV422_INTERLEAVED_YUYV:
    case AX_FORMAT_YUV422_INTERLEAVED_UYVY:
    case AX_FORMAT_YUV422_INTERLEAVED_VYUY:
        uHeight = (uHeight + 7) / 8 * 8;
        uBufSize = uWidthBeat * 8 * uHeight * 2;
        break;
    case AX_FORMAT_YUV444_PLANAR:
    case AX_FORMAT_YUV444_PLANAR_VU:
    case AX_FORMAT_YUV444_SEMIPLANAR:
    case AX_FORMAT_YUV444_SEMIPLANAR_VU:
    case AX_FORMAT_YUV444_PACKED:
    case AX_FORMAT_YUV444_PACKED_10BIT_P010:
    case AX_FORMAT_YUV444_PACKED_10BIT_P101010:
        uHeight = (uHeight + 7) / 8 * 8;
        uBufSize = uWidthBeat * 8 * uHeight * 3;
        break;
    default:
        uBufSize = uWidthBeat * 8 * uHeight * 3 / 2;
        break;
    }

    if (pstCompressInfo != AX_NULL) {
        if (pstCompressInfo->enCompressMode == AX_COMPRESS_MODE_LOSSLESS) {
            if (bFormatYuv == AX_TRUE) {
                uBufSize += uHeight * 96; /* add the header buf for compress data */
            } else {
                uBufSize += uHeight * 32; /* add the header buf for compress data */
            }
        } else if (pstCompressInfo->enCompressMode == AX_COMPRESS_MODE_LOSSY) {
            if (bFormatYuv == AX_TRUE) {
                if ((0 != pstCompressInfo->u32CompressLevel) && (10 >= pstCompressInfo->u32CompressLevel)) {
                    if (8 == uPixBits) {
                        if (pstCompressInfo->u32CompressLevel <= 8) {
                            uBufSize = uBufSize * pstCompressInfo->u32CompressLevel * 0.125;
                        }
                    } else if (10 == uPixBits) {
                        uBufSize = uBufSize * pstCompressInfo->u32CompressLevel * 0.1;
                    } else {
                        //do nothing
                    }
                }
            } else {
                if (uHeight > 2160) {
                    //raw lossy
                    uBaseWidth = 896;
                } else {
                    //raw lossy
                    uBaseWidth = 768;
                }
                uNum = (uWidthStride + uBaseWidth - 1) / uBaseWidth;

                if (uNum <= 1) {
                    uBufSize = (((uNum * 2 - 2) * 32 + uWidthStride) * uHeight * uPixBits / 2 + 63) / 64 * 8;
                } else if ((uNum > 1) && (uNum < 3)) {
                    uBufSize = (((32 + uBaseWidth) * uHeight * uPixBits / 2 + 63) / 64 * 8 + 63) / 64 * 64;
                    uBufSize += ((32 + uWidthStride - uBaseWidth * (uNum - 1)) * uHeight * uPixBits / 2 + 63) / 64 * 8;
                } else {
                    uBufSize = (((32 + uBaseWidth) * uHeight * uPixBits / 2 + 63) / 64 * 8 + 63) / 64 * 64;
                    uBufSize += (((64 +  uBaseWidth) * uHeight * uPixBits / 2 + 63) / 64 * 8 + 63) / 64 * 64 * (uNum - 2);
                    uBufSize += ((32 + uWidthStride - uBaseWidth * (uNum - 1)) * uHeight * uPixBits / 2 + 63) / 64 * 8;
                }
            }
        } else {
            //without fbc
        }
    }

    return uBufSize;
}

static __inline AX_U32 AX_VDEC_GetPicBufferSize(AX_U32 uWidth, AX_U32 uHeight, AX_PAYLOAD_TYPE_E enType)
{
    AX_U32 picSizeInMbs = 0;
    AX_U32 picSize = 0;
    AX_U32 dmvMemSize = 0;
    AX_U32 refBuffSize = 0;

    if ((PT_H264 == enType)) {
        picSizeInMbs = (AX_COMM_ALIGN(uHeight, 16) >> 4) * (AX_COMM_ALIGN(uWidth, 16) >> 4);

        /* luma */
        picSize = (AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3) >> 1;

        /* buffer size of dpb pic = picSize + dir_mv_size + tbl_size */
        dmvMemSize = picSizeInMbs * 64;
        refBuffSize = picSize  + dmvMemSize + 32;
    } else if ((PT_JPEG == enType) || (PT_MJPEG == enType)) {
        picSize = (AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3) >> 1;
        refBuffSize = picSize;
    } else {
        refBuffSize = -1;
    }

    return refBuffSize;
}

static __inline AX_U32 AX_JDEC_GetYuvBufferSize(AX_U32 uWidth, AX_U32 uHeight, AX_IMG_FORMAT_E eOutputFormat)
{
    AX_U32 picSize = 0;

    if (AX_FORMAT_YUV400 == eOutputFormat) {
        picSize = AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16);
    } else if ((AX_FORMAT_YUV420_PLANAR <= eOutputFormat ) && (eOutputFormat <= AX_FORMAT_YUV420_SEMIPLANAR_VU)) {
        picSize = (AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3) >> 1;
    } else if ((AX_FORMAT_YUV422_PLANAR <= eOutputFormat ) && (eOutputFormat <= AX_FORMAT_YUV422_INTERLEAVED_YVYU)) {
        picSize = AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 2;
    } else if ((AX_FORMAT_YUV444_PLANAR <= eOutputFormat ) && (eOutputFormat <= AX_FORMAT_YUV444_PACKED)) {
        picSize = AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3;
    } else {
        picSize = AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3;
    }

    return picSize;
}

/*
 * uHeight: Image height in pixels
 * uWidthStride: Image width in bytes
 * eImageFormat: Image Format, only support Yuv
 * pstCompressInfo: Compression mode and compression level
 * uAlignSize: Aligned size in bytes
 */
static __inline AX_U32 AX_VIN_GetYuvImgBufferSize(AX_U32 uHeight, AX_U32 uWidthStride, AX_IMG_FORMAT_E eImageFormat,
        AX_FRAME_COMPRESS_INFO_T *pstCompressInfo, AX_U32 uAlignSize)
{
    AX_U32 uBufSize = 0;
    AX_U32 uPixBits = 0;

    switch (eImageFormat) {
    case AX_FORMAT_YUV400:
    case AX_FORMAT_YUV420_PLANAR:
    case AX_FORMAT_YUV420_PLANAR_VU:
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_PLANAR:
    case AX_FORMAT_YUV422_PLANAR_VU:
    case AX_FORMAT_YUV422_SEMIPLANAR:
    case AX_FORMAT_YUV422_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_INTERLEAVED_YUVY:
    case AX_FORMAT_YUV422_INTERLEAVED_YUYV:
    case AX_FORMAT_YUV422_INTERLEAVED_UYVY:
    case AX_FORMAT_YUV422_INTERLEAVED_VYUY:
    case AX_FORMAT_YUV444_PLANAR:
    case AX_FORMAT_YUV444_PLANAR_VU:
    case AX_FORMAT_YUV444_SEMIPLANAR:
    case AX_FORMAT_YUV444_SEMIPLANAR_VU:
    case AX_FORMAT_YUV444_PACKED:
        uPixBits = 8;
        break;
    case AX_FORMAT_YUV400_10BIT:
    case AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010:
    case AX_FORMAT_YUV444_PACKED_10BIT_P101010:
        uPixBits = 10;
        break;
    case AX_FORMAT_YUV420_PLANAR_10BIT_I010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016:
    case AX_FORMAT_YUV444_PACKED_10BIT_P010:
        uPixBits = 16;
        break;
    default:
        uPixBits = 8;
        break;
    }
    uHeight = (uHeight + 7) / 8 * 8;
    // calc dma buffer size
    switch (eImageFormat) {
    case AX_FORMAT_YUV400:
    case AX_FORMAT_YUV400_10BIT:
        uBufSize = uWidthStride * uHeight;
        break;
    case AX_FORMAT_YUV420_PLANAR:
    case AX_FORMAT_YUV420_PLANAR_VU:
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
    case AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B:
    case AX_FORMAT_YUV420_PLANAR_10BIT_I010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016:
        uBufSize = uWidthStride * uHeight * 3 / 2;
        break;
    case AX_FORMAT_YUV422_PLANAR:
    case AX_FORMAT_YUV422_PLANAR_VU:
    case AX_FORMAT_YUV422_SEMIPLANAR:
    case AX_FORMAT_YUV422_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_INTERLEAVED_YUVY:
    case AX_FORMAT_YUV422_INTERLEAVED_YUYV:
    case AX_FORMAT_YUV422_INTERLEAVED_UYVY:
    case AX_FORMAT_YUV422_INTERLEAVED_VYUY:
        uBufSize = uWidthStride * uHeight * 2;
        break;
    case AX_FORMAT_YUV444_PLANAR:
    case AX_FORMAT_YUV444_PLANAR_VU:
    case AX_FORMAT_YUV444_SEMIPLANAR:
    case AX_FORMAT_YUV444_SEMIPLANAR_VU:
    case AX_FORMAT_YUV444_PACKED:
    case AX_FORMAT_YUV444_PACKED_10BIT_P010:
    case AX_FORMAT_YUV444_PACKED_10BIT_P101010:
        uBufSize = uWidthStride * uHeight * 3;
        break;
    default:
        uBufSize = uWidthStride * uHeight * 3 / 2;
        break;
    }

    if (pstCompressInfo != AX_NULL) {
        if (pstCompressInfo->enCompressMode == AX_COMPRESS_MODE_LOSSLESS) {
            uBufSize += uHeight * 96; /* add the header buf for compress data */
        } else if (pstCompressInfo->enCompressMode == AX_COMPRESS_MODE_LOSSY) {
            if ((0 != pstCompressInfo->u32CompressLevel) && (10 >= pstCompressInfo->u32CompressLevel)) {
                if (8 == uPixBits) {
                    if (pstCompressInfo->u32CompressLevel <= 8) {
                        uBufSize = uBufSize * pstCompressInfo->u32CompressLevel * 0.125;
                    }
                } else if (10 == uPixBits) {
                    uBufSize = uBufSize * pstCompressInfo->u32CompressLevel * 0.1;
                } else {
                    //do nothing
                }
            }
        } else {
            //without fbc
        }
    }

    return uBufSize;
}

/*
 * uWidthPixel: Image width in pixels
 * eImageFormat: Image Format
 * uAlignSizeByte: Aligned size in bytes
 * return uStrideByte: Image width in bytes
 */
static __inline AX_U32 AX_VIN_GetImgStrideByte(AX_U32 uWidthPixel, AX_IMG_FORMAT_E eImageFormat,
        AX_U32 uAlignSizeByte)
{
    AX_U32 uStrideByte = 0;
    AX_U32 uWidthBeat = 0;
    AX_U32 uPixBits = 0;

    switch (eImageFormat) {
    case AX_FORMAT_BAYER_RAW_8BPP:
        uPixBits = 8;
        break;
    case AX_FORMAT_BAYER_RAW_10BPP_PACKED:
        uPixBits = 10;
        break;
    case AX_FORMAT_BAYER_RAW_12BPP_PACKED:
        uPixBits = 12;
        break;
    case AX_FORMAT_BAYER_RAW_14BPP_PACKED:
        uPixBits = 14;
        break;
    case AX_FORMAT_BAYER_RAW_10BPP:
    case AX_FORMAT_BAYER_RAW_12BPP:
    case AX_FORMAT_BAYER_RAW_14BPP:
    case AX_FORMAT_BAYER_RAW_16BPP:
        uPixBits = 16;
        break;
    case AX_FORMAT_YUV400:
    case AX_FORMAT_YUV420_PLANAR:
    case AX_FORMAT_YUV420_PLANAR_VU:
    case AX_FORMAT_YUV420_SEMIPLANAR:
    case AX_FORMAT_YUV420_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_PLANAR:
    case AX_FORMAT_YUV422_PLANAR_VU:
    case AX_FORMAT_YUV422_SEMIPLANAR:
    case AX_FORMAT_YUV422_SEMIPLANAR_VU:
    case AX_FORMAT_YUV422_INTERLEAVED_YUVY:
    case AX_FORMAT_YUV422_INTERLEAVED_YUYV:
    case AX_FORMAT_YUV422_INTERLEAVED_UYVY:
    case AX_FORMAT_YUV422_INTERLEAVED_VYUY:
    case AX_FORMAT_YUV444_PLANAR:
    case AX_FORMAT_YUV444_PLANAR_VU:
    case AX_FORMAT_YUV444_SEMIPLANAR:
    case AX_FORMAT_YUV444_SEMIPLANAR_VU:
    case AX_FORMAT_YUV444_PACKED:
        uPixBits = 8;
        break;
    case AX_FORMAT_YUV400_10BIT:
    case AX_FORMAT_YUV420_PLANAR_10BIT_UV_PACKED_4Y5B:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P101010:
    case AX_FORMAT_YUV444_PACKED_10BIT_P101010:
    case AX_FORMAT_YUV422_SEMIPLANAR_10BIT_P101010:
        uPixBits = 10;
        break;
    case AX_FORMAT_YUV420_PLANAR_10BIT_I010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P010:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_P016:
    case AX_FORMAT_YUV420_SEMIPLANAR_10BIT_I016:
    case AX_FORMAT_YUV444_PACKED_10BIT_P010:
        uPixBits = 16;
        break;
    default:
        uPixBits = 8;
        break;
    }

    if (((uWidthPixel * uPixBits) % 64) != 0) {
        uWidthBeat = ((uWidthPixel * uPixBits) / 64) + 1;
    } else {
        uWidthBeat = (uWidthPixel * uPixBits) / 64 ;
    }

    if (uAlignSizeByte != 0) {
        uStrideByte = ((uWidthBeat * 8 + uAlignSizeByte - 1) / uAlignSizeByte) * uAlignSizeByte;
    } else {
        uStrideByte = ((uWidthBeat * 8 + 63) / 64 * 64) ;
    }

    return uStrideByte;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif //_AX_BUFFER_TOOL_H_

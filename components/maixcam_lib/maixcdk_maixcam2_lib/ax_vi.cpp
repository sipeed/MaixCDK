
#include "ax_middleware.hpp"
#include "ax_venc_api.h"
#include "ax_ivps_api.h"
#include "ax_isp_api.h"
#include "ax_global_type.h"
#include "ax_isp_api.h"
#include "common_sys.h"
#include "common_venc.h"
#include "common_vin.h"
#include "common_cam.h"
#include "common_nt.h"
#include "common_isp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#define ALIGN_UP_16(value) ((value + 0xF) & (~0xF))
#define ALIGN_UP_64(value) ((value + 0x3F) & (~0x3F))
#define SAMPLE_VENC_CHN_NUM_MAX   (3)
#ifdef __SAMPLE_LOG_EN__
#define ALOGI(fmt, ...) printf("\033[1;30;32mINFO   :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // green
#else
#define ALOGI(fmt, ...) \
    do                  \
    {                   \
    } while (0)
#endif
#define ALOGI2(fmt, ...) printf("\033[1;30;33mINFO   :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // yellow
#define ALOGE(fmt, ...) printf("\033[1;30;31mERROR  :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__)  // red

namespace maix::middleware::maixcam2
{
    typedef struct _stVencGetStreamParam
    {
        AX_S32 VeChn;
        AX_BOOL bThreadStart;
        AX_PAYLOAD_TYPE_E ePayloadType;
    } VENC_GETSTREAM_PARAM_T;

    typedef struct _stRCInfo
    {
        SAMPLE_VENC_RC_E eRCType;
        AX_U32 nMinQp;
        AX_U32 nMaxQp;
        AX_U32 nMinIQp;
        AX_U32 nMaxIQp;
        AX_S32 nIntraQpDelta;
    } RC_INFO_T;

    typedef struct _stVideoConfig
    {
        AX_PAYLOAD_TYPE_E ePayloadType;
        AX_U32 nGOP;
        AX_U32 nSrcFrameRate;
        AX_U32 nDstFrameRate;
        AX_U32 nStride;
        AX_S32 nInWidth;
        AX_S32 nInHeight;
        AX_S32 nOutWidth;
        AX_S32 nOutHeight;
        AX_IMG_FORMAT_E eImgFormat;
        RC_INFO_T stRCInfo;
        AX_S32 nBitrate;
    } VIDEO_CONFIG_T;

    typedef enum {
        SAMPLE_VIN_NONE  = -1,
        SAMPLE_VIN_SINGLE_DUMMY  = 0,
        SAMPLE_VIN_SINGLE_OS04A10 = 1,
        SAMPLE_VIN_SINGLE_SC450AI  = 2,
        SAMPLE_VIN_BUTT
    } SAMPLE_VIN_CASE_E;

    typedef struct {
        AX_U32 nStride;
        AX_S32 nWidth;
        AX_S32 nHeight;
        AX_IMG_FORMAT_E eImgFormat;
        AX_COMPRESS_MODE_E eFbcMode;
    } SAMPLE_CHN_ATTR_T;

    typedef struct {
        SAMPLE_VIN_CASE_E eSysCase;
        COMMON_VIN_MODE_E eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode;
        SAMPLE_LOAD_RAW_NODE_E eLoadRawNode;
        AX_BOOL bAiispEnable;
        AX_S32 nDumpFrameNum;
        AX_S32 nPipeId; /* For VIN */
        AX_S32 nGrpId;  /* For IVPS */
        AX_S32 nOutChnNum;
        char *pFrameInfo;
        AX_VIN_IVPS_MODE_E eMode;
        AX_IVPS_ROTATION_E eRotAngle;
        AX_U32 statDeltaPtsFrmNum;
    } SAMPLE_VIN_PARAM_T;

    // typedef struct {
    //     AX_BOOL bEnable;
    //     AX_RTSP_HANDLE pRtspHandle;
    // } SAMPLE_RTSP_PARAM_T;


    /* comm pool */
    COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleDummySdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 10},      /* vin raw16 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 10},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
    };

    COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 4},    /* vin nv21/nv21 use */
        {1920, 1080, 1920, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
        {720, 576, 720, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
    };

    /* private pool */
    COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleDummySdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 10},
    };

    COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleOs04a10Sdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP_PACKED, 12, AX_COMPRESS_MODE_LOSSY, 4},      /* vin raw16 use */
    };

    // SC450AI
    COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs450aiSdr[] = {
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 4},    /* vin nv21/nv21 use */
        {1920, 1080, 1920, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
        {720, 576, 720, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
    };

    COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleOs450aiSdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP_PACKED, 8, AX_COMPRESS_MODE_LOSSY, 4},      /* vin raw10 use */
    };

    static AX_CAMERA_T gCams[MAX_CAMERAS] = {0};
    // static SAMPLE_RTSP_PARAM_T gRtspParam;
    static SAMPLE_CHN_ATTR_T gOutChnAttr[] = {
        {
            .nStride = 2688,
            .nWidth =  2688,
            .nHeight = 1520,
            .eFbcMode = AX_COMPRESS_MODE_NONE,
        },
        {
            .nStride = 1920,
            .nWidth =  1920,
            .nHeight = 1080,
            .eFbcMode = AX_COMPRESS_MODE_NONE,
        },
        {
            .nStride = 640,
            .nWidth = 640,
            .nHeight = 360,
            .eFbcMode = AX_COMPRESS_MODE_NONE,
        }
    };

    static AX_ISP_IQ_LDC_PARAM_T ldc_param = {
        /* nLdcEnable */
        1,
        /* nType */
        0,
        /* tLdcV1Param */
        {
            /* bAspect */
            AX_FALSE,
            /* nXRatio */
            0,
            /* nYRatio */
            0,
            /* nXYRatio */
            0,
            /* nCenterXOffset */
            0,
            /* nCenterYOffset */
            0,
            /* nDistortionRatio */
            1234,
            /* nSpreadCoef */
            0,
        },
        /* tLdcV2Param */
        {
            /* nMatrix[3][3] */
            {
                {0, 0, 0, /*0 - 2*/},
                {0, 0, 0, /*0 - 2*/},
                {0, 0, 1, /*0 - 2*/},
            },
            /* nDistortionCoeff[8] */
            {0, 0, 0, 0, 0, 0, 0, 0, /*0 - 7*/},
        },
    };


    static AX_VOID __cal_dump_pool(COMMON_SYS_POOL_CFG_T pool[], AX_SNS_HDR_MODE_E eHdrMode, AX_S32 nFrameNum)
    {
        if (NULL == pool) {
            return;
        }
        if (nFrameNum > 0) {
            switch (eHdrMode) {
            case AX_SNS_LINEAR_MODE:
                pool[0].nBlkCnt += nFrameNum;
                break;

            case AX_SNS_HDR_2X_MODE:
                pool[0].nBlkCnt += nFrameNum * 2;
                break;

            case AX_SNS_HDR_3X_MODE:
                pool[0].nBlkCnt += nFrameNum * 3;
                break;

            case AX_SNS_HDR_4X_MODE:
                pool[0].nBlkCnt += nFrameNum * 4;
                break;

            default:
                pool[0].nBlkCnt += nFrameNum;
                break;
            }
        }
    }

    static AX_VOID __set_pipe_hdr_mode(AX_U32 *pHdrSel, AX_SNS_HDR_MODE_E eHdrMode)
    {
        if (NULL == pHdrSel) {
            return;
        }

        switch (eHdrMode) {
        case AX_SNS_LINEAR_MODE:
            *pHdrSel = 0x1;
            break;

        case AX_SNS_HDR_2X_MODE:
            *pHdrSel = 0x1 | 0x2;
            break;

        case AX_SNS_HDR_3X_MODE:
            *pHdrSel = 0x1 | 0x2 | 0x4;
            break;

        case AX_SNS_HDR_4X_MODE:
            *pHdrSel = 0x1 | 0x2 | 0x4 | 0x8;
            break;

        default:
            *pHdrSel = 0x1;
            break;
        }
    }

    static AX_VOID __set_vin_attr(AX_CAMERA_T *pCam, SAMPLE_SNS_TYPE_E eSnsType, AX_SNS_HDR_MODE_E eHdrMode,
                                  COMMON_VIN_MODE_E eSysMode, AX_BOOL bAiispEnable)
    {
        pCam->eSnsType = eSnsType;
        pCam->tSnsAttr.eSnsMode = eHdrMode;
        pCam->tDevAttr.eSnsMode = eHdrMode;
        pCam->eHdrMode = eHdrMode;
        pCam->eSysMode = eSysMode;
        pCam->tPipeAttr[pCam->nPipeId].eSnsMode = eHdrMode;
        pCam->tPipeAttr[pCam->nPipeId].bAiIspEnable = bAiispEnable;
        if (eHdrMode > AX_SNS_LINEAR_MODE) {
            pCam->tDevAttr.eSnsOutputMode = AX_SNS_DOL_HDR;
        }

        if (COMMON_VIN_TPG == eSysMode) {
            pCam->tDevAttr.eSnsIntfType = AX_SNS_INTF_TYPE_TPG;
        }

        if (COMMON_VIN_LOADRAW == eSysMode) {
            pCam->bEnableDev = AX_FALSE;
        } else {
            pCam->bEnableDev = AX_TRUE;
        }
        pCam->bChnEn[0] = AX_TRUE;
        pCam->bRegisterSns = AX_TRUE;

        return;
    }

    static AX_U32 __sample_case_single_dummy(AX_CAMERA_T *pCamList, SAMPLE_SNS_TYPE_E eSnsType,
            SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs)
    {
        AX_S32 i = 0;
        AX_CAMERA_T *pCam = NULL;
        COMMON_VIN_MODE_E eSysMode = pVinParam->eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode = pVinParam->eHdrMode;
        SAMPLE_LOAD_RAW_NODE_E eLoadRawNode = pVinParam->eLoadRawNode;
        pCam = &pCamList[0];
        pCommonArgs->nCamCnt = 1;

        for (i = 0; i < pCommonArgs->nCamCnt; i++) {
            pCam = &pCamList[i];
            pCam->nPipeId = 0;
            COMMON_VIN_GetSnsConfig(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
                                    &pCam->tSnsClkAttr, &pCam->tDevAttr,
                                    &pCam->tPipeAttr[pCam->nPipeId], pCam->tChnAttr);

            pCam->nDevId = 0;
            pCam->nRxDev = 0;
            pCam->tSnsClkAttr.nSnsClkIdx = 0;
            pCam->tDevBindPipe.nNum =  1;
            pCam->tDevBindPipe.nPipeId[0] = pCam->nPipeId;
            pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj(eSnsType);
            pCam->eBusType = COMMON_ISP_GetSnsBusType(eSnsType);
            pCam->eLoadRawNode = eLoadRawNode;
            __set_pipe_hdr_mode(&pCam->tDevBindPipe.nHDRSel[0], eHdrMode);
            __set_vin_attr(pCam, eSnsType, eHdrMode, eSysMode, pVinParam->bAiispEnable);
            for (AX_S32 j = 0; j < AX_VIN_MAX_PIPE_NUM; j++) {
                pCam->tPipeInfo[j].ePipeMode = SAMPLE_PIPE_MODE_VIDEO;
                pCam->tPipeInfo[j].bAiispEnable = pVinParam->bAiispEnable;
                strncpy(pCam->tPipeInfo[j].szBinPath, "null.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
            }
        }

        return 0;
    }
    #if 0
    static AX_U32 __sample_case_single_os08a20(AX_CAMERA_T *pCamList, SAMPLE_SNS_TYPE_E eSnsType,
            SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs)
    {
        AX_CAMERA_T *pCam = NULL;
        COMMON_VIN_MODE_E eSysMode = pVinParam->eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode = pVinParam->eHdrMode;
        AX_S32 j = 0;
        pCommonArgs->nCamCnt = 1;
        pCam = &pCamList[0];
        COMMON_VIN_GetSnsConfig(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
                                &pCam->tSnsClkAttr, &pCam->tDevAttr,
                                &pCam->tPipeAttr, pCam->tChnAttr);

        pCam->nDevId = 0;
        pCam->nRxDev = 0;
        pCam->nPipeId = 0;
        pCam->tSnsClkAttr.nSnsClkIdx = 0;
        pCam->tDevBindPipe.nNum =  1;
        pCam->tDevBindPipe.nPipeId[0] = pCam->nPipeId;
        pCam->ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
        pCam->eBusType = COMMON_ISP_GetSnsBusType(eSnsType);
        __set_pipe_hdr_mode(&pCam->tDevBindPipe.nHDRSel[0], eHdrMode);
        __set_vin_attr(pCam, eSnsType, eHdrMode, eSysMode, pVinParam->bAiispEnable);
        for (j = 0; j < pCam->tDevBindPipe.nNum; j++) {
            pCam->tPipeInfo[j].ePipeMode = SAMPLE_PIPE_MODE_VIDEO;
            pCam->tPipeInfo[j].bAiispEnable = pVinParam->bAiispEnable;
            if (pCam->tPipeInfo[j].bAiispEnable) {
                if (eHdrMode <= AX_SNS_LINEAR_MODE) {
                    strncpy(pCam->tPipeInfo[j].szBinPath, "/opt/etc/os08a20_sdr_dual3dnr.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
                } else {
                    strncpy(pCam->tPipeInfo[j].szBinPath, "/opt/etc/os08a20_hdr_2x_ainr.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
                }
            } else {
                strncpy(pCam->tPipeInfo[j].szBinPath, "null.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
            }
        }
        return 0;
    }
    #endif
    static AX_U32 __sample_case_single_os04a10(AX_CAMERA_T *pCamList, SAMPLE_SNS_TYPE_E eSnsType,
            SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs)
    {
        AX_CAMERA_T *pCam = NULL;
        COMMON_VIN_MODE_E eSysMode = pVinParam->eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode = pVinParam->eHdrMode;
        AX_U32 j = 0;
        pCommonArgs->nCamCnt = 1;
        pCam = &pCamList[0];
        COMMON_VIN_GetSnsConfig(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
                                &pCam->tSnsClkAttr, &pCam->tDevAttr,
                                &pCam->tPipeAttr[pCam->nPipeId], pCam->tChnAttr);
        pCam->nDevId = 0;
        pCam->nRxDev = 0;
        pCam->nPipeId = 0;
        pCam->tSnsClkAttr.nSnsClkIdx = 0;
        pCam->tDevBindPipe.nNum =  1;
        pCam->tDevBindPipe.nPipeId[0] = pCam->nPipeId;
        pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj(eSnsType);
        pCam->eBusType = COMMON_ISP_GetSnsBusType(eSnsType);
        __set_pipe_hdr_mode(&pCam->tDevBindPipe.nHDRSel[0], eHdrMode);
        __set_vin_attr(pCam, eSnsType, eHdrMode, eSysMode, pVinParam->bAiispEnable);
        for (j = 0; j < pCam->tDevBindPipe.nNum; j++) {
            pCam->tPipeInfo[j].ePipeMode = SAMPLE_PIPE_MODE_VIDEO;
            pCam->tPipeInfo[j].bAiispEnable = pVinParam->bAiispEnable;
            strncpy(pCam->tPipeInfo[j].szBinPath, "null.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
        }
        return 0;
    }

    static AX_U32 __sample_case_single_sc450ai(AX_CAMERA_T *pCamList, SAMPLE_SNS_TYPE_E eSnsType,
            SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs)
    {
        AX_CAMERA_T *pCam = NULL;
        COMMON_VIN_MODE_E eSysMode = pVinParam->eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode = pVinParam->eHdrMode;
        AX_U32 j = 0;
        SAMPLE_LOAD_RAW_NODE_E eLoadRawNode = pVinParam->eLoadRawNode;
        pCommonArgs->nCamCnt = 1;

        pCam = &pCamList[0];
        pCam->nPipeId = 0;
        COMMON_VIN_GetSnsConfig(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
                                    &pCam->tSnsClkAttr, &pCam->tDevAttr,
                                    &pCam->tPipeAttr[pCam->nPipeId], pCam->tChnAttr);
        pCam->nDevId = 0;
        pCam->nRxDev = 0;
        pCam->tSnsClkAttr.nSnsClkIdx = 0;
        pCam->tDevBindPipe.nNum =  1;
        pCam->eLoadRawNode = eLoadRawNode;
        pCam->tDevBindPipe.nPipeId[0] = pCam->nPipeId;
        pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj(eSnsType);
        pCam->eBusType = COMMON_ISP_GetSnsBusType(eSnsType);
        pCam->eInputMode = AX_INPUT_MODE_MIPI;
        __set_pipe_hdr_mode(&pCam->tDevBindPipe.nHDRSel[0], eHdrMode);
        __set_vin_attr(pCam, eSnsType, eHdrMode, eSysMode, pVinParam->bAiispEnable);
        for (j = 0; j < pCam->tDevBindPipe.nNum; j++) {
            pCam->tPipeInfo[j].ePipeMode = SAMPLE_PIPE_MODE_VIDEO;
            pCam->tPipeInfo[j].bAiispEnable = pVinParam->bAiispEnable;
            if (pCam->tPipeInfo[j].bAiispEnable) {
                if (eHdrMode <= AX_SNS_LINEAR_MODE) {
                    strncpy(pCam->tPipeInfo[j].szBinPath, "/opt/etc/sc450ai_sdr_dual3dnr.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
                } else {
                    strncpy(pCam->tPipeInfo[j].szBinPath, "/opt/etc/sc450ai_hdr_2x_ainr.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
                }
            } else {
                strncpy(pCam->tPipeInfo[j].szBinPath, "null.bin", sizeof(pCam->tPipeInfo[j].szBinPath));
            }
        }
        return 0;
    }

    static AX_U32 __sample_case_config(SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs,
                                       COMMON_SYS_ARGS_T *pPrivArgs)
    {
        AX_CAMERA_T         *pCamList = &gCams[0];
        SAMPLE_SNS_TYPE_E   eSnsType = OMNIVISION_OS04A10;

        ALOGI2("eSysCase %d, eSysMode %d, eLoadRawNode %d, eHdrMode %d, bAiispEnable %d", pVinParam->eSysCase,
                     pVinParam->eSysMode,
                     pVinParam->eLoadRawNode, pVinParam->eHdrMode, pVinParam->bAiispEnable);

        switch (pVinParam->eSysCase) {
        case SAMPLE_VIN_SINGLE_OS04A10:
            eSnsType = OMNIVISION_OS04A10;
            /* comm pool config */
            __cal_dump_pool(gtSysCommPoolSingleOs04a10Sdr, pVinParam->eHdrMode, pVinParam->nDumpFrameNum);
            pCommonArgs->nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Sdr) / sizeof(gtSysCommPoolSingleOs04a10Sdr[0]);
            pCommonArgs->pPoolCfg = gtSysCommPoolSingleOs04a10Sdr;

            /* private pool config */
            __cal_dump_pool(gtPrivatePoolSingleOs04a10Sdr, pVinParam->eHdrMode, pVinParam->nDumpFrameNum);
            pPrivArgs->nPoolCfgCnt = sizeof(gtPrivatePoolSingleOs04a10Sdr) / sizeof(gtPrivatePoolSingleOs04a10Sdr[0]);
            pPrivArgs->pPoolCfg = gtPrivatePoolSingleOs04a10Sdr;

            /* cams config */
            __sample_case_single_os04a10(pCamList, eSnsType, pVinParam, pCommonArgs);
            break;
        case SAMPLE_VIN_SINGLE_SC450AI:
            eSnsType = SMARTSENS_SC450AI;
            /* comm pool config */
            __cal_dump_pool(gtSysCommPoolSingleOs450aiSdr, pVinParam->eHdrMode, pVinParam->nDumpFrameNum);
            pCommonArgs->nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs450aiSdr) / sizeof(gtSysCommPoolSingleOs450aiSdr[0]);
            pCommonArgs->pPoolCfg = gtSysCommPoolSingleOs450aiSdr;

            /* private pool config */
            __cal_dump_pool(gtPrivatePoolSingleOs450aiSdr, pVinParam->eHdrMode, pVinParam->nDumpFrameNum);
            pPrivArgs->nPoolCfgCnt = sizeof(gtPrivatePoolSingleOs450aiSdr) / sizeof(gtPrivatePoolSingleOs450aiSdr[0]);
            pPrivArgs->pPoolCfg = gtPrivatePoolSingleOs450aiSdr;

            /* cams config */
            __sample_case_single_sc450ai(pCamList, eSnsType, pVinParam, pCommonArgs);
            break;
        case SAMPLE_VIN_SINGLE_DUMMY:
        default:
            eSnsType = SAMPLE_SNS_DUMMY;
            /* pool config */
            pCommonArgs->nPoolCfgCnt = sizeof(gtSysCommPoolSingleDummySdr) / sizeof(gtSysCommPoolSingleDummySdr[0]);
            pCommonArgs->pPoolCfg = gtSysCommPoolSingleDummySdr;

            /* private pool config */
            pPrivArgs->nPoolCfgCnt = sizeof(gtPrivatePoolSingleDummySdr) / sizeof(gtPrivatePoolSingleDummySdr[0]);
            pPrivArgs->pPoolCfg = gtPrivatePoolSingleDummySdr;

            /* cams config */
            __sample_case_single_dummy(pCamList, eSnsType, pVinParam, pCommonArgs);
            break;
        }

        return 0;
    }

    typedef struct {
        COMMON_SYS_ARGS_T tCommonArgs;
    } ax_vin_param_t;

    static SAMPLE_VIN_CASE_E get_vi_case_from_sensor_name(char *sensor_name) {
        if (strcmp(sensor_name, "os04a10") == 0) {
            return SAMPLE_VIN_SINGLE_OS04A10;
        } else if (strcmp(sensor_name, "sc450ai") == 0) {
            return SAMPLE_VIN_SINGLE_SC450AI;
        } else {
            log::error("Can't find sensor %s", sensor_name);
            err::check_raise(err::ERR_RUNTIME, "Can't find sensor");
        }
    }

    VI::VI(char *sensor_name, bool raw)
    {
        COMMON_SYS_ARGS_T tCommonArgs = {0};
        COMMON_SYS_ARGS_T tPrivArgs = {0};

        SAMPLE_VIN_PARAM_T tVinParam = {
            .eSysCase = get_vi_case_from_sensor_name(sensor_name),
            .eSysMode = COMMON_VIN_SENSOR,
            .eHdrMode = AX_SNS_LINEAR_MODE,
            .eLoadRawNode = raw ? LOAD_RAW_IFE : LOAD_RAW_NONE,
            .bAiispEnable = AX_FALSE,
            .statDeltaPtsFrmNum = 0,
        };

        /* Step1: cam config & pool Config */
        __sample_case_config(&tVinParam, &tCommonArgs, &tPrivArgs);


    }

    Frame::Frame(void *frame) {
        this->frame = frame;
        this->w = 0;
        this->h = 0;
        this->fmt = 0;
        this->data = nullptr;
        this->len = 0;
    }

    Frame::~Frame() {

    }

    VI::~VI()
    {
        auto ax_vin_param = (ax_vin_param_t *)_param;
        if (ax_vin_param) {
            ::free(ax_vin_param);
        }
    }

    err::Err VI::init()
    {
        return err::ERR_NONE;
    }

    err::Err VI::deinit()
    {
        return err::ERR_NONE;
    }

    err::Err VI::add_channel(int ch, int width, int height, int format, int fps, int depth, bool mirror, bool vflip, int fit)
    {
        return err::ERR_NONE;
    }

    err::Err VI::del_channel(int ch)
    {
        return err::ERR_NONE;
    }

    err::Err VI::del_channel_all()
    {
        return err::ERR_NONE;
    }

    maixcam2::Frame *VI::pop(int ch)
    {
        return nullptr;
    }
};

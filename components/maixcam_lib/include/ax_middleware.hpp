#ifndef __AX_MIDDLEWARE_HPP__
#define __AX_MIDDLEWARE_HPP__

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include "maix_basic.hpp"
#include "maix_image.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "ax_venc_api.h"
#include "ax_ivps_api.h"
#include "ax_isp_api.h"
#include "ax_global_type.h"
#include "ax_isp_api.h"
#include "ax_buffer_tool.h"
#include "ax_vin_error_code.h"
#include "common_sys.h"
#include "common_venc.h"
#include "common_vin.h"
#include "common_cam.h"
#include "common_nt.h"
#include "common_isp.h"
#ifdef __cplusplus
}
#endif

using namespace maix;

typedef struct {
    int init_count;
    COMMON_SYS_ARGS_T tCommonArgs;
    COMMON_SYS_ARGS_T tPrivArgs;
} ax_sys_param_t;

typedef enum {
    AX_VENC_TYPE_JPG = 0,
    AX_VENC_TYPE_H264,
    AX_VENC_TYPE_H265,
    AX_VENC_TYPE_MJPG,
} ax_venc_type_e;

typedef struct {
    bool en;
    ax_venc_type_e type;
    int w;
    int h;
    AX_IMG_FORMAT_E fmt;
    union {
        struct {
            int first_frame_start_qp;
            int stat_time;
            int bitrate;
            int qp_min;
            int qp_max;
        } mjpg;
        struct {
            int first_frame_start_qp;
            int gop;
            int input_fps;
            int output_fps;
            int bitrate;
            int min_qp;
            int max_qp;
            int min_iqp;
            int max_iqp;
            int intra_qp_delta;
            int de_breath_qp_delta;
            int min_iprop;
            int max_iprop;
        } h264;
        struct {
            int first_frame_start_qp;
            int gop;
            int input_fps;
            int output_fps;
            int bitrate;
            int min_qp;
            int max_qp;
            int min_iqp;
            int max_iqp;
            int intra_qp_delta;
            int de_breath_qp_delta;
            int min_iprop;
            int max_iprop;
            int qp_delta_rgn;
            AX_VENC_QPMAP_QP_TYPE_E qp_map_type;
            AX_VENC_QPMAP_BLOCK_TYPE_E qp_map_blk_type;
            AX_VENC_QPMAP_BLOCK_UNIT_E qp_map_block_unit;
            AX_VENC_RC_CTBRC_MODE_E ctb_rc_mode;
        } h265;
    };
} ax_venc_param_t;

typedef struct {
    AX_CAMERA_T cams[MAX_CAMERAS];
    ax_venc_param_t venc[AX_MAX_VENC_CHN_NUM];
    int venc_init_count;
    COMMON_SYS_ARGS_T tCommonArgs;
    COMMON_SYS_ARGS_T tPrivArgs;
    ax_sys_param_t *sys_param;
    void *vi_param;
    void *vo_param;
} ax_global_param_t;

static void __ax_signal_handler(int signal)
{
    (void)signal;
    log::info("signal %d received, exit now", signal);
    app::set_exit_flag(1);
    log::info("wait 1000ms..");
    time::sleep_ms(1000);
    util::do_exit_function();
    ::exit(signal);
}

static __attribute__((constructor)) void __register_handler(void)
{
    int signal_arr[] = {
        SIGINT,
        SIGILL,
        SIGABRT,
        SIGFPE,
        SIGSEGV,
        SIGTERM,
        SIGHUP,
        SIGTRAP,
        SIGPIPE,
        SIGKILL,
        SIGALRM,
        SIGBUS,
    };

    for (size_t i = 0; i < sizeof(signal_arr) / sizeof(signal_arr[0]); i++) {
        signal(signal_arr[i], __ax_signal_handler);
    }
}

namespace maix::middleware::maixcam2 {
    ax_global_param_t *get_ax_global_param();
    void ax_global_param_lock();
    void ax_global_param_unlock();


    inline image::Format get_maix_fmt_from_ax(AX_IMG_FORMAT_E format) {
        switch (format) {
            case AX_FORMAT_YUV400:
                return image::FMT_GRAYSCALE;
            case AX_FORMAT_RGB888:
                return image::FMT_BGR888;   // actualy is rgb888
            case AX_FORMAT_BGR888:
                return image::FMT_RGB888;   // actualy is bgr888
            case AX_FORMAT_ARGB8888:
                return image::FMT_BGRA8888;     // actualy is rgba8888
            case AX_FORMAT_ABGR8888:
                return image::FMT_RGBA8888;     // actualy is bgra8888
            case AX_FORMAT_YUV420_SEMIPLANAR:
                return image::FMT_YUV420SP;
            case AX_FORMAT_YUV420_SEMIPLANAR_VU:
                return image::FMT_YVU420SP;
            default:
                return image::FMT_INVALID;
        }
    }

    inline AX_IMG_FORMAT_E get_ax_fmt_from_maix(image::Format format) {
        switch (format)
        {
        case image::FMT_GRAYSCALE:
            return AX_FORMAT_YUV400;
        case image::FMT_RGB888:
            return AX_FORMAT_BGR888;       // actualy is rgb888
        case image::FMT_BGR888:
            return AX_FORMAT_RGB888;       // actualy is bgr888
        case image::FMT_RGBA8888:
            return AX_FORMAT_ABGR8888;     // actualy is rgba8888
        case image::FMT_BGRA8888:
            return AX_FORMAT_ARGB8888;      // actualy is bgra8888
        case image::FMT_YUV420SP:
            return AX_FORMAT_YUV420_SEMIPLANAR;
        case image::FMT_YVU420SP:
            return AX_FORMAT_YUV420_SEMIPLANAR_VU;
        default:
            return AX_FORMAT_INVALID;
        }
    }

    typedef enum {
        SAMPLE_VIN_NONE  = -1,
        SAMPLE_VIN_SINGLE_DUMMY  = 0,
        SAMPLE_VIN_SINGLE_OS04A10 = 1,
        SAMPLE_VIN_SINGLE_SC450AI  = 2,
        SAMPLE_VIN_BUTT
    } SAMPLE_VIN_CASE_E;

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
    static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleDummySdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 10},      /* vin raw16 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 10},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
    };

    static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 4},    /* vin nv21/nv21 use */
        {1920, 1080, 1920, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
        {720, 576, 720, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
    };

    /* private pool */
    static COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleDummySdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 10},
    };

    static COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleOs04a10Sdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP_PACKED, 12, AX_COMPRESS_MODE_LOSSY, 4},      /* vin raw16 use */
    };

    // SC450AI
    static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs450aiSdr[] = {
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 3, AX_COMPRESS_MODE_LOSSY, 4},    /* vin nv21/nv21 use */
        {2688, 1520, 2688, AX_FORMAT_YUV420_SEMIPLANAR, 4},    /* vin nv21/nv21 use */
        {1920, 1080, 1920, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
        {720, 576, 720, AX_FORMAT_YUV420_SEMIPLANAR, 3},    /* vin nv21/nv21 use */
    };

    static COMMON_SYS_POOL_CFG_T gtPrivatePoolSingleOs450aiSdr[] = {
        {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP_PACKED, 8, AX_COMPRESS_MODE_LOSSY, 4},      /* vin raw10 use */
    };

    // static AX_CAMERA_T gCams[MAX_CAMERAS] = {0};

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

    static void __vi_get_sns_config(SAMPLE_SNS_TYPE_E eSnsType,
        AX_MIPI_RX_ATTR_T *ptMipiAttr, AX_SNS_ATTR_T *ptSnsAttr,
        AX_SNS_CLK_ATTR_T *ptSnsClkAttr, AX_VIN_DEV_ATTR_T *pDevAttr,
        AX_VIN_PIPE_ATTR_T *pPipeAttr, AX_VIN_CHN_ATTR_T *pChnAttr) {
        COMMON_VIN_GetSnsConfig(eSnsType, ptMipiAttr, ptSnsAttr, ptSnsClkAttr, pDevAttr, pPipeAttr, pChnAttr);
        switch (eSnsType) {
        case SMARTSENS_SC450AI:
        {
            // auto chn0_attr = &pChnAttr[0];
            // chn0_attr->eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR_VU;
            // chn0_attr->tCompressInfo = {AX_COMPRESS_MODE_NONE, 0};
        }
        break;
        default:
            err::check_raise(err::ERR_NOT_IMPL, "not implemented");
            break;
        }
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
            __vi_get_sns_config(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
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

    static AX_U32 __sample_case_single_os04a10(AX_CAMERA_T *pCamList, SAMPLE_SNS_TYPE_E eSnsType,
            SAMPLE_VIN_PARAM_T *pVinParam, COMMON_SYS_ARGS_T *pCommonArgs)
    {
        AX_CAMERA_T *pCam = NULL;
        COMMON_VIN_MODE_E eSysMode = pVinParam->eSysMode;
        AX_SNS_HDR_MODE_E eHdrMode = pVinParam->eHdrMode;
        AX_U32 j = 0;
        pCommonArgs->nCamCnt = 1;
        pCam = &pCamList[0];
        __vi_get_sns_config(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
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
        __vi_get_sns_config(eSnsType, &pCam->tMipiAttr, &pCam->tSnsAttr,
                                    &pCam->tSnsClkAttr, &pCam->tDevAttr,
                                    &pCam->tPipeAttr[pCam->nPipeId], pCam->tChnAttr);
        pCam->nDevId = 0;
        pCam->nRxDev = 0;
        pCam->nI2cAddr = 0x30;
        pCam->nI2cNode = 0;
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
        ax_global_param_lock();
        auto g_param = get_ax_global_param();
        AX_CAMERA_T         *pCamList = &g_param->cams[0];
        SAMPLE_SNS_TYPE_E   eSnsType = OMNIVISION_OS04A10;

        // printf("eSysCase %d, eSysMode %d, eLoadRawNode %d, eHdrMode %d, bAiispEnable %d\r\n", pVinParam->eSysCase,
        //              pVinParam->eSysMode,
        //              pVinParam->eLoadRawNode, pVinParam->eHdrMode, pVinParam->bAiispEnable);

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
        ax_global_param_unlock();
        return 0;
    }

    static SAMPLE_VIN_CASE_E __get_vi_case_from_sensor_name(char *sensor_name) {
        if (strcmp(sensor_name, "os04a10") == 0) {
            return SAMPLE_VIN_SINGLE_OS04A10;
        } else if (strcmp(sensor_name, "sc450ai") == 0) {
            return SAMPLE_VIN_SINGLE_SC450AI;
        } else {
            log::error("Can't find sensor %s", sensor_name);
            return SAMPLE_VIN_NONE;
        }
    }

    // static bool __check_board_config_path()
    // {
    //     if (fs::exists("/boot/board")) {
    //         return true;
    //     }
    //     return false;
    // }

    // static int _get_mclk_id(void) {
    //     int mclk_id = 0;        // default mclk id

    //     if (__check_board_config_path()) {
    //         std::string mclk_id_str;
    //         auto device_configs = sys::device_configs();
    //         auto it = device_configs.find("cam_mclk");
    //         if (it != device_configs.end()) {
    //             mclk_id_str = it->second;
    //         }
    //         if (!mclk_id_str.empty()) {
    //             mclk_id = atoi(mclk_id_str.c_str());
    //         } else {
    //             std::string board_id = sys::device_id();
    //             if (board_id == "maixcam2") {
    //                 mclk_id = 0;
    //             } else {
    //                 mclk_id = 1;
    //             }
    //         }
    //     }

    //     return mclk_id;
    // }

    // static std::vector<int> __get_lane_id_from_board_file() {
    //     std::vector<int> lane_id = {2, 5, 0, 1, 3, 4};  // clk0, clk1, data0, data1, data2, data3

    //     if (__check_board_config_path()) {
    //         auto device_configs = sys::device_configs();
    //         auto it = device_configs.find("lane_id");
    //         if (it != device_configs.end()) {
    //             auto lane_id_str = it->second;
    //             std::string item;
    //             std::stringstream ss(lane_id_str);
    //             while (std::getline(ss, item, ',')) {
    //                 lane_id.push_back(std::stoi(item));
    //             }
    //         }
    //     }

    //     return lane_id;
    // }

    // static std::vector<int> _get_pn_swap_from_board_file() {
    //     std::vector<int> pn_swap;

    //     if (__check_board_config_path()) {
    //         auto device_configs = sys::device_configs();
    //         auto it = device_configs.find("pn_swap");
    //         if (it != device_configs.end()) {
    //             auto pn_swap_str = it->second;
    //             std::string item;
    //             std::stringstream ss(pn_swap_str);
    //             while (std::getline(ss, item, ',')) {
    //                 pn_swap.push_back(std::stoi(item));
    //             }
    //         }
    //     }

    //     return pn_swap;
    // }

    // static AX_S32 __hw_reset(unsigned int gpio_num, unsigned int gpio_out_val)
    // {
    //     FILE *fp = AX_NULL;
    //     char file_name[50];
    //     char buf[10];

    //     sprintf(file_name, "/sys/class/gpio/gpio%d", gpio_num);
    //     if (0 != access(file_name, F_OK)) {
    //         sprintf(file_name, "/sys/class/gpio/export");
    //         fp = fopen(file_name, "w");
    //         if (fp == AX_NULL) {
    //             log::error("Cannot open %s.\n", file_name);
    //             return -1;
    //         }
    //         fprintf(fp, "%d", gpio_num);
    //         fclose(fp);

    //         sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
    //         fp = fopen(file_name, "w");
    //         if (fp == AX_NULL) {
    //             log::error("Cannot open %s.\n", file_name);
    //             return -1;
    //         }
    //         fprintf(fp, "out");
    //         fclose(fp);
    //     }

    //     sprintf(file_name, "/sys/class/gpio/gpio%d/value", gpio_num);
    //     fp = fopen(file_name, "w");
    //     if (fp == AX_NULL) {
    //         log::error("Cannot open %s.\n", file_name);
    //         return -1;
    //     }
    //     if (gpio_out_val) {
    //         strcpy(buf, "1");
    //     } else {
    //         strcpy(buf, "0");
    //     }
    //     fprintf(fp, "%s", buf);
    //     fclose(fp);

    //     return 0;
    // }

    static std::vector<int> __scan_i2c_addr(int id)
    {
        char buf[32];
        std::vector<int> data;
        int addr_start = 0x08;
        int addr_end = 0x77;

        snprintf(buf, sizeof(buf), "/dev/i2c-%d", id);
        int fd = ::open(buf, O_RDWR);
        if (fd < 0)
        {
            ::close(fd);
            throw err::Exception(err::Err::ERR_IO, "open " + std::string(buf) + " failed");
        }

        for (int address = addr_start; address <= addr_end; ++address)
        {
            if (::ioctl(fd, I2C_SLAVE, address) < 0)
            {
                continue;
            }

            unsigned char buffer[1];
            if (::read(fd, buffer, sizeof(buffer)) >= 0)
            {
                data.push_back(address);
            }
        }

        ::close(fd);
        return data;
    }

    static std::pair<bool, std::string> __get_sensor_name(void) {
        char name[30];
        // AX_S32 axRet = 0;
        // axRet = AX_MIPI_RX_Init();
        // if (0 != axRet) {
        //     COMM_CAM_PRT("AX_MIPI_RX_Init failed, ret=0x%x.\n", axRet);
        //     return {false, ""};
        // }

        // axRet = AX_ISP_OpenSnsClk(0, AX_SNS_CLK_24M);
        // if (0 != axRet) {
        //     COMM_ISP_PRT("AX_ISP_OpenSnsClk failed, nRet=0x%x.\n", axRet);
        //     return {false, ""};
        // }
        // sensor reset
        // __hw_reset(97, 0);      // 0,97 1,49
        // time::sleep_us(5);
        // __hw_reset(97, 1);
        // time::sleep_ms(5);

        // scan iic list
        std::vector<int> addr_list = __scan_i2c_addr(0);
        for (size_t i = 0; i < addr_list.size(); i++) {
            // log::info("i2c4 addr: 0x%02x", addr_list[i]);
            switch (addr_list[i]) {
                case 0x30:
                    // log::info("find sc450ai, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "sc450ai");
                    return {true, name};
                default: break;
            }
        }

        // AX_ISP_CloseSnsClk(0);
        // return {false, ""};
        return {true, "sc450ai"};
    }

    class SYS {
    public:
        SYS(bool raw = false) {
            COMMON_SYS_ARGS_T tCommonArgs = {0};
            COMMON_SYS_ARGS_T tPrivArgs = {0};
            ax_sys_param_t *p_sys_param = nullptr;

            ax_global_param_lock();
            auto g_param = get_ax_global_param();
            if (g_param->sys_param == nullptr) {    // first init
                ax_global_param_unlock();

                auto get_sensor_res = __get_sensor_name();
                if (!get_sensor_res.first) {
                    err::check_raise(err::ERR_RUNTIME, "get sensor name failed");
                }
                auto sensor_name = get_sensor_res.second;
                SAMPLE_VIN_PARAM_T tVinParam = {
                    .eSysCase = __get_vi_case_from_sensor_name((char *)sensor_name.c_str()),
                    .eSysMode = COMMON_VIN_SENSOR,
                    .eHdrMode = AX_SNS_LINEAR_MODE,
                    .eLoadRawNode = raw ? LOAD_RAW_IFE : LOAD_RAW_NONE,
                    .bAiispEnable = AX_FALSE,
                    .statDeltaPtsFrmNum = 0,
                };

                p_sys_param = (ax_sys_param_t *)::malloc(sizeof(ax_sys_param_t));
                if (p_sys_param == nullptr) {
                    ax_global_param_unlock();
                    err::check_raise(err::ERR_RUNTIME, "malloc failed");
                }
                memset(p_sys_param, 0, sizeof(ax_sys_param_t));

                __sample_case_config(&tVinParam, &tCommonArgs, &tPrivArgs);

                ::memcpy(&p_sys_param->tCommonArgs, &tCommonArgs, sizeof(COMMON_SYS_ARGS_T));
                ::memcpy(&p_sys_param->tPrivArgs, &tPrivArgs, sizeof(COMMON_SYS_ARGS_T));
                AX_S32 axRet = COMMON_SYS_Init(&p_sys_param->tCommonArgs);
                if (axRet) {
                    ax_global_param_unlock();
                    COMM_ISP_PRT("COMMON_SYS_Init fail, ret:0x%x", axRet);
                    err::check_raise(err::ERR_RUNTIME, "COMMON_SYS_Init failed");
                }
                p_sys_param->init_count = 1;

                ax_global_param_lock();
                g_param->sys_param = p_sys_param;
                ax_global_param_unlock();
            } else {
                p_sys_param = (ax_sys_param_t *)g_param->sys_param;
                p_sys_param->init_count ++;
                ax_global_param_unlock();
            }

            log::info("sys init success, count:%d", p_sys_param->init_count);
        }
        ~SYS() {
            ax_global_param_lock();
            auto g_param = get_ax_global_param();
            auto p_sys_param = (ax_sys_param_t *)g_param->sys_param;
            if (p_sys_param->init_count > 1) {
                p_sys_param->init_count --;
                log::info("sys deinit success, count:%d", p_sys_param->init_count);
            } else {
                p_sys_param->init_count = 0;
                COMMON_SYS_DeInit();

                log::info("sys deinit success, count:%d", p_sys_param->init_count);
                ::free(p_sys_param);
                g_param->sys_param = nullptr;
                printf("maix multi-media driver released.\r\n");
            }
            ax_global_param_unlock();

        }
    };

    typedef enum {
        FRAME_FROM_IVPS_CHN = 0,
        FRAME_FROM_SYS_MEM_ALLOC,
        FRAME_FROM_VENC_GET_STREAM,
        FRAME_FROM_GET_BLOCK,
    } frame_from_e;

    class Frame {
    public:
        void *data;
        int len;
        int w;
        int h;
        AX_IMG_FORMAT_E fmt;
        void *frame;
        void *__param;
        Frame(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn, AX_VIDEO_FRAME_T *ptFrame, frame_from_e from = FRAME_FROM_IVPS_CHN, AX_IMG_FORMAT_E invert_fmt = AX_FORMAT_INVALID);
        Frame(int venc_ch, AX_VENC_STREAM_T *frame, frame_from_e from = FRAME_FROM_VENC_GET_STREAM);
        Frame(int w, int h, void *data, int data_size, AX_IMG_FORMAT_E fmt);
        Frame(int pool_id, int w, int h, void *data, int data_size, AX_IMG_FORMAT_E fmt);
        ~Frame();
        frame_from_e from();
        err::Err get_video_frame(AX_VIDEO_FRAME_T * frame);
        err::Err get_venc_stream(AX_VENC_STREAM_T * stream);
    };

    class VI {
    public:
        VI();
        ~VI();
        err::Err init();
        err::Err deinit();
        int get_unused_channel();
        err::Err add_channel(int ch, int width, int height, AX_IMG_FORMAT_E format, int fps, int depth, bool mirror, bool vflip, int fit);
        err::Err del_channel(int ch);
        err::Err del_channel_all();
        err::Err set_windowing(int ch, int x, int y, int w, int h);
        maixcam2::Frame *pop(int ch, int32_t timeout_ms = 1000);
        maixcam2::Frame *pop_raw(int ch, int32_t timeout_ms = 1000);
        int set_and_get_exposure(int value);        // input -1: get only
        int set_and_get_gain(int value);            // input -1: get only
        int set_and_get_luma(int value);            // input -1: get only
        int set_and_get_saturation(int value);      // input -1: get only
        int set_and_get_constrast(int value);       // input -1: get only
        int set_and_get_hue(int value);             // input -1: get only
        int set_and_get_mirror(int ch, int value);  // input -1: get only
        int set_and_get_flip(int ch, int value);    // input -1: get only
        int set_and_get_awb_mode(int value);        // input -1: get only
        int set_and_get_exp_mode(int value);        // input -1: get only
    private:
        // void *_param = nullptr;
    };

    class VENC {
    public:
        VENC(ax_venc_param_t *cfg);
        ~VENC();
        err::Err push(maixcam2::Frame *frame, int32_t timeout_ms);
        maixcam2::Frame * pop(int32_t timeout_ms);
        err::Err get_config(ax_venc_param_t *cfg);
    private:
        int _ch;
        int _fd;
    };
};

#endif

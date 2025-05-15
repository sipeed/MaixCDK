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
#include "ax_vdec_api.h"
#include "ax_ivps_api.h"
#include "ax_isp_api.h"
#include "ax_global_type.h"
#include "ax_isp_api.h"
#include "ax_buffer_tool.h"
#include "ax_vin_error_code.h"
#include "ax_vo_api.h"
#include "ax_ai_api.h"
#include "ax_ao_api.h"
#include "ax_aenc_api.h"
#include "ax_adec_api.h"
#include "ax_acodec_api.h"
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

namespace maix::middleware::maixcam2 {
    class Frame;
    class SYS;
    class VENC;
    class ENGINE;
    err::Err ax_jpg_enc_init(void);
    err::Err ax_jpg_enc_deinit(void);
    maixcam2::Frame *ax_jpg_enc_once(maixcam2::Frame *frame, int quality);

    typedef enum {
        AX_VENC_TYPE_JPG = 0,
        AX_VENC_TYPE_H264,
        AX_VENC_TYPE_H265,
        AX_VENC_TYPE_MJPG,
    } ax_venc_type_e;

    typedef enum {
        AX_VDEC_TYPE_JPG = 0,
        AX_VDEC_TYPE_H264,
        AX_VDEC_TYPE_H265,
        AX_VDEC_TYPE_MJPG,
    } ax_vdec_type_e;

    typedef struct {
        bool en;
        ax_venc_type_e type;
        int w;
        int h;
        AX_IMG_FORMAT_E fmt;
        union {
            struct {
                int input_fps;
                int output_fps;
            } jpg;

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
        bool en;
        ax_vdec_type_e type;
        int w;
        int h;
        AX_IMG_FORMAT_E fmt;
        int blk_cnt;
        int blk_size;
        AX_POOL pool_id;
    } ax_vdec_param_t;

    typedef struct {
        int channels;
        int rate;
        int encode_rate;
        AX_AUDIO_BIT_WIDTH_E bits;
        AX_AI_LAYOUT_MODE_E layout;
        AX_PAYLOAD_TYPE_E payload_type;
        unsigned int period_size;
        unsigned int period_count;
        bool cfg_pool_en;
        AX_POOL_CONFIG_T pool_cfg;
        bool cfg_pub_attr;
        AX_AI_ATTR_T pub_attr;
        bool vqe_en;
        AX_AP_UPTALKVQE_ATTR_T vqe_attr;
        bool ns_en;
        AX_NS_CONFIG_T ns_attr;
        bool hpf_en;
        AX_ACODEC_FREQ_ATTR_T hpf_attr;
        bool lpf_en;
        AX_ACODEC_FREQ_ATTR_T lpf_attr;
        bool eq_en;
        AX_ACODEC_EQ_ATTR_T eq_attr;
        bool aed_en;
        AX_AED_ATTR_T aed_attr;
    } ax_audio_in_param_t;

    typedef struct {
        int channels;
        int rate;
        int encode_rate;
        AX_AUDIO_BIT_WIDTH_E bits;
        AX_PAYLOAD_TYPE_E payload_type;
        unsigned int period_size;
        unsigned int period_count;
        bool insert_silence;
        bool cfg_pool_en;
        AX_POOL_CONFIG_T pool_cfg;
        bool cfg_pub_attr;
        AX_AO_ATTR_T pub_attr;
        bool vqe_en;
        AX_AP_DNVQE_ATTR_T vqe_attr;
        bool hpf_en;
        AX_ACODEC_FREQ_ATTR_T hpf_attr;
        bool lpf_en;
        AX_ACODEC_FREQ_ATTR_T lpf_attr;
        bool eq_en;
        AX_ACODEC_EQ_ATTR_T eq_attr;
    } ax_audio_out_param_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        COMMON_SYS_ARGS_T tCommonArgs;
        COMMON_SYS_ARGS_T tPrivArgs;
    } ax_sys_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        AX_ENGINE_NPU_MODE_T mode;
    } ax_engine_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        ax_vdec_param_t vdec[AX_VDEC_MAX_GRP_NUM];
    } ax_vdec_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        ax_venc_param_t venc[AX_MAX_VENC_CHN_NUM];
    } ax_venc_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        SYS *sys;
        VENC *venc;
    } ax_jpg_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        AX_CAMERA_T cams[MAX_CAMERAS];
        int init_count2;
        AX_S32 nGrpId;
        AX_S32 nChnNum;
        AX_S32 nGroupInputWidth;
        AX_S32 nGroupInputHeight;
        AX_IMG_FORMAT_E nGroupInputFormat;
        AX_IVPS_GRP_ATTR_T stGrpAttr;
        AX_IVPS_ROTATION_E eRotAngle;
        AX_IVPS_PIPELINE_ATTR_T stPipelineAttr;
        COMMON_SYS_ARGS_T tCommonArgs;
        COMMON_SYS_ARGS_T tPrivArgs;
        AX_U32 VinId;
        AX_U32 IvpsId;
        ENGINE *engine;
        struct {
            int w;
            int h;
            AX_IMG_FORMAT_E fmt;
            int fps;
            int depth;
            AX_BOOL mirror;
            AX_BOOL flip;
            int fit;    // fit = 0, width to new width, height to new height, may be stretch
                        // fit = 1, keep aspect ratio, fill blank area with black color
                        // fit = 2, keep aspect ratio, crop image to fit new size
        } chn_out[AX_VIN_CHN_ID_MAX];
    } ax_vi_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
    } ax_vo_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        int card;
        int device;
        SYS *sys;
        AX_POOL pool_id;
        ax_audio_in_param_t param;
        bool eq_en;
        bool hpf_en;
        bool lpf_en;
        bool aed_en;
        AX_AI_ATTR_T attr;
    } ax_ai_mod_t;

    typedef struct {
        pthread_mutex_t lock;
        int init_count;
        int card;
        int device;
        SYS *sys;
        AX_POOL pool_id;
        ax_audio_out_param_t param;
        bool eq_en;
        bool hpf_en;
        bool lpf_en;
        AX_AO_ATTR_T attr;
    } ax_ao_mod_t;

    typedef enum {
        AX_MOD_SYS,
        AX_MOD_ENGINE,
        AX_MOD_VI,
        AX_MOD_VO,
        AX_MOD_VENC,
        AX_MOD_VDEC,
        AX_MOD_JPG,
        AX_MOD_AI,
        AX_MOD_AO,
        AX_MOD_MAX,
    } ax_mod_e;

    class AxModuleParam {
    public:
        static AxModuleParam& getInstance() {
            static AxModuleParam instance; // **C++11 线程安全**
            return instance;
        }

        void lock(ax_mod_e mod) {
            switch (mod) {
            case AX_MOD_SYS:
                pthread_mutex_lock(&__sys_mod.lock);
            break;
            case AX_MOD_ENGINE:
                pthread_mutex_lock(&__engine_mod.lock);
            break;
            case AX_MOD_VI:
                pthread_mutex_lock(&__vi_mod.lock);
            break;
            case AX_MOD_VO:
                pthread_mutex_lock(&__vo_mod.lock);
            break;
            case AX_MOD_VENC:
                pthread_mutex_lock(&__venc_mod.lock);
            break;
            case AX_MOD_VDEC:
                pthread_mutex_lock(&__vdec_mod.lock);
            break;
            case AX_MOD_JPG:
                pthread_mutex_lock(&__jpg_mod.lock);
            break;
            case AX_MOD_AI:
                pthread_mutex_lock(&__ai_mod.lock);
            break;
            case AX_MOD_AO:
                pthread_mutex_lock(&__ao_mod.lock);
            break;
            default:
                err::check_raise(err::ERR_RUNTIME, "unknown ax module");
            break;
            }
        }

        void *get_param(ax_mod_e mod) {
            switch (mod) {
            case AX_MOD_SYS:
                return &__sys_mod;
            break;
            case AX_MOD_ENGINE:
                return &__engine_mod;
            break;
            case AX_MOD_VI:
                return &__vi_mod;
            break;
            case AX_MOD_VO:
                return &__vo_mod;
            break;
            case AX_MOD_VENC:
                return &__venc_mod;
            break;
            case AX_MOD_VDEC:
                return &__vdec_mod;
            break;
            case AX_MOD_JPG:
                return &__jpg_mod;
            break;
            case AX_MOD_AI:
                return &__ai_mod;
            break;
            case AX_MOD_AO:
                return &__ao_mod;
            break;
            default:
                return nullptr;
            }
        }

        void unlock(ax_mod_e mod) {
            switch (mod) {
            case AX_MOD_SYS:
                pthread_mutex_unlock(&__sys_mod.lock);
            break;
            case AX_MOD_ENGINE:
                pthread_mutex_unlock(&__engine_mod.lock);
            break;
            case AX_MOD_VI:
                pthread_mutex_unlock(&__vi_mod.lock);
            break;
            case AX_MOD_VO:
                pthread_mutex_unlock(&__vo_mod.lock);
            break;
            case AX_MOD_VENC:
                pthread_mutex_unlock(&__venc_mod.lock);
            break;
            case AX_MOD_VDEC:
                pthread_mutex_unlock(&__vdec_mod.lock);
            break;
            case AX_MOD_JPG:
                pthread_mutex_unlock(&__jpg_mod.lock);
            break;
            case AX_MOD_AI:
                pthread_mutex_unlock(&__ai_mod.lock);
            break;
            case AX_MOD_AO:
                pthread_mutex_unlock(&__ao_mod.lock);
            break;
            default:
                err::check_raise(err::ERR_RUNTIME, "unknown ax module");
            break;
            }
        }
    private:
        ax_sys_mod_t __sys_mod;
        ax_engine_mod_t __engine_mod;
        ax_vi_mod_t __vi_mod;
        ax_vo_mod_t __vo_mod;
        ax_venc_mod_t __venc_mod;
        ax_vdec_mod_t __vdec_mod;
        ax_jpg_mod_t __jpg_mod;
        ax_ai_mod_t __ai_mod;
        ax_ao_mod_t __ao_mod;
        AxModuleParam() {
            __sys_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __engine_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __vi_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __vo_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __venc_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __vdec_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __jpg_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __ai_mod.lock = PTHREAD_MUTEX_INITIALIZER;
            __ao_mod.lock = PTHREAD_MUTEX_INITIALIZER;
        }
        ~AxModuleParam() = default;
        AxModuleParam(const AxModuleParam &) = delete;
        AxModuleParam& operator=(const AxModuleParam &) = delete;
    };

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

#ifdef __cplusplus
    extern "C" {
#endif
    static AX_SENSOR_REGISTER_FUNC_T *COMMON_ISP_GetSnsObj_User(SAMPLE_SNS_TYPE_E eSnsType)
    {
#if CONFIG_MAIXCAM_LOAD_SENSOR_LIBRARY_WITHOUT_DLOPEN
        AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;
        switch (eSnsType) {
#if CONFIG_AX620E_MSP_ENABLE_SENSOR_LIB
        case OMNIVISION_OS04A10:
        extern AX_SENSOR_REGISTER_FUNC_T gSnsos04a10Obj;
            ptSnsHdl = &gSnsos04a10Obj;
            break;
        case SMARTSENS_SC450AI:
        extern AX_SENSOR_REGISTER_FUNC_T gSnssc450aiObj;
            ptSnsHdl = &gSnssc450aiObj;
            break;
#endif
        default:
            err::check_raise(err::ERR_NOT_IMPL, "No supported sensor library found");
            break;
        }

        return ptSnsHdl;
#else
        return COMMON_ISP_GetSnsObj(eSnsType);
#endif
    }
#ifdef __cplusplus
};
#endif
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
            pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj_User(eSnsType);
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
        pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj_User(eSnsType);
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
        pCam->ptSnsHdl[pCam->nPipeId] = COMMON_ISP_GetSnsObj_User(eSnsType);
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
        auto &mod_param = AxModuleParam::getInstance();
        mod_param.lock(AX_MOD_VI);
        auto vi_param = (ax_vi_mod_t *)mod_param.get_param(AX_MOD_VI);
        AX_CAMERA_T         *pCamList = &vi_param->cams[0];
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
        mod_param.unlock(AX_MOD_VI);
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
        bool __is_inited;
        bool __raw;
    public:
        SYS(bool raw = false) {
            __raw = raw;
            __is_inited = false;
        }
        ~SYS() {
            if (__is_inited) {
                deinit();
            }

            // release jpg
            ax_sys_mod_t *sys_param = nullptr;
            auto &mod_param = AxModuleParam::getInstance();

            mod_param.lock(AX_MOD_SYS);
            sys_param = (ax_sys_mod_t *)mod_param.get_param(AX_MOD_SYS);
            auto sys_init_count = sys_param->init_count;
            mod_param.unlock(AX_MOD_SYS);

            mod_param.lock(AX_MOD_JPG);
            auto jpg_param = (ax_jpg_mod_t *)mod_param.get_param(AX_MOD_JPG);
            auto jpg_init_count = jpg_param->init_count;
            mod_param.unlock(AX_MOD_JPG);

            if (sys_init_count == 1 && jpg_init_count > 0) {
                auto err = ax_jpg_enc_deinit();
                if (err != err::ERR_NONE) {
                    log::error("ax_jpg_enc_deinit failed, err: %d", err);
                }
            }
        }

        err::Err init() {
            if (__is_inited) return err::ERR_NONE;
            ax_sys_mod_t *sys_param = nullptr;
            auto &mod_param = AxModuleParam::getInstance();
            mod_param.lock(AX_MOD_SYS);
            sys_param = (ax_sys_mod_t *)mod_param.get_param(AX_MOD_SYS);
            if (sys_param->init_count > 0) {
                sys_param->init_count ++;
            } else {
                COMMON_SYS_ARGS_T tCommonArgs = {0};
                COMMON_SYS_ARGS_T tPrivArgs = {0};
                SAMPLE_VIN_PARAM_T tVinParam = {
                    .eSysCase = SAMPLE_VIN_SINGLE_SC450AI,
                    .eSysMode = COMMON_VIN_SENSOR,
                    .eHdrMode = AX_SNS_LINEAR_MODE,
                    .eLoadRawNode = __raw ? LOAD_RAW_IFE : LOAD_RAW_NONE,
                    .bAiispEnable = AX_FALSE,
                    .statDeltaPtsFrmNum = 0,
                };

                auto get_sensor_res = __get_sensor_name();
                if (!get_sensor_res.first) {
                    log::error("get sensor name failed");
                    return err::ERR_RUNTIME;
                }
                tVinParam.eSysCase = __get_vi_case_from_sensor_name((char *)get_sensor_res.second.c_str());

                // check if enable ai-isp
                AX_BOOL ai_isp_on = app::get_sys_config_kv("npu", "ai_isp", "1") == "1" ? AX_TRUE : AX_FALSE;
                tVinParam.bAiispEnable = ai_isp_on;

                __sample_case_config(&tVinParam, &tCommonArgs, &tPrivArgs);
                ::memcpy(&sys_param->tCommonArgs, &tCommonArgs, sizeof(COMMON_SYS_ARGS_T));
                ::memcpy(&sys_param->tPrivArgs, &tPrivArgs, sizeof(COMMON_SYS_ARGS_T));
                AX_S32 axRet = COMMON_SYS_Init(&sys_param->tCommonArgs);
                if (axRet) {
                    mod_param.unlock(AX_MOD_SYS);
                    COMM_ISP_PRT("COMMON_SYS_Init fail, ret:0x%x", axRet);
                    return err::ERR_RUNTIME;
                }
                sys_param->init_count = 1;
            }
            mod_param.unlock(AX_MOD_SYS);

            log::info("sys init success, count:%d", sys_param->init_count);
            __is_inited = true;
            return err::ERR_NONE;
        }

        void deinit() {
            if (!__is_inited) return;
            ax_sys_mod_t *sys_param = nullptr;
            auto &mod_param = AxModuleParam::getInstance();
            mod_param.lock(AX_MOD_SYS);
            sys_param = (ax_sys_mod_t *)mod_param.get_param(AX_MOD_SYS);
            err::check_null_raise(sys_param, "Get ax sys parameter failed!");
            if (sys_param->init_count > 1) {
                sys_param->init_count --;
                log::info("sys deinit success, count:%d", sys_param->init_count);
            } else {
                sys_param->init_count = 0;
                COMMON_SYS_DeInit();
                printf("maix multi-media driver released.\r\n");
            }
            mod_param.unlock(AX_MOD_SYS);
            __is_inited = false;
        }
    };

    typedef enum {
        FRAME_FROM_IVPS_CHN = 0,
        FRAME_FROM_SYS_MEM_ALLOC,
        FRAME_FROM_VENC_GET_STREAM,
        FRAME_FROM_GET_BLOCK,
        FRAME_FROM_MALLOC,
        FRAME_FROM_VDEC_GET_STREAM,
        FRAME_FROM_AX_MALLOC,
        FRAME_FROM_AUDIO_GET_FRAME,
        FRAME_FROM_AUDIO_FRAME,
    } frame_from_e;

    class Frame {
    public:
        void *data;
        uint64_t phy_addr;
        int len;
        int w;
        int h;
        AX_IMG_FORMAT_E fmt;
        void *frame;
        void *__param;
        Frame(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn, AX_VIDEO_FRAME_T *ptFrame, frame_from_e from = FRAME_FROM_IVPS_CHN, AX_IMG_FORMAT_E invert_fmt = AX_FORMAT_INVALID);
        Frame(int venc_ch, AX_VENC_STREAM_T *frame, frame_from_e from = FRAME_FROM_VENC_GET_STREAM);
        Frame(int vdec_ch, AX_VIDEO_FRAME_INFO_T *frame, frame_from_e from = FRAME_FROM_VDEC_GET_STREAM);
        Frame(int w, int h, void *data, int data_size, AX_IMG_FORMAT_E fmt);
        Frame(int pool_id, int w, int h, void *data, int data_size, AX_IMG_FORMAT_E fmt);
        Frame(void *data, int data_size, frame_from_e from = FRAME_FROM_MALLOC);
        Frame(int card, int device, AX_AUDIO_FRAME_T *frame, frame_from_e from = FRAME_FROM_AUDIO_GET_FRAME);
        Frame(int card, int device, void *data, int data_size, AX_AUDIO_BIT_WIDTH_E bit_width, AX_AUDIO_SOUND_MODE_E sound_mode, frame_from_e from = FRAME_FROM_AUDIO_FRAME);
        ~Frame();
        frame_from_e from();
        err::Err get_video_frame(AX_VIDEO_FRAME_T * frame);
        err::Err get_audio_frame(AX_AUDIO_FRAME_T * frame);
        err::Err get_venc_stream(AX_VENC_STREAM_T * stream);
        err::Err get_video_frame_info(AX_VIDEO_FRAME_INFO_T * stream);
    };

    class ENGINE {
        bool __is_inited;
        AX_ENGINE_NPU_MODE_T __mode;
    public:
        ENGINE(AX_ENGINE_NPU_MODE_T mode);
        ~ENGINE();
        err::Err init();
        void deinit();
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

    class VO {
        int __layer;
        int __ch;
        bool __is_inited;
    public:
        VO();
        ~VO();
        err::Err init();
        void deinit();
        err::Err add_channel(int layer, int ch, int width, int height, AX_IMG_FORMAT_E format, int fit);
        err::Err del_channel(int layer, int ch);
        err::Err del_channel_all();
        err::Err push(int layer, int ch, maixcam2::Frame *frame);
        int set_and_get_mirror(int ch, int value);
        int set_and_get_flip(int ch, int value);
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
    };

    class VDEC {
    public:
        VDEC(ax_vdec_param_t *cfg);
        ~VDEC();
        err::Err push(maixcam2::Frame *frame, int32_t timeout_ms);
        maixcam2::Frame * pop(int32_t timeout_ms);
        err::Err get_config(ax_vdec_param_t *cfg);
    private:
        int _ch;
        void *param;
    };

    class AudioIn {
    public:
        AudioIn(ax_audio_in_param_t *cfg);
        ~AudioIn();
        err::Err init();
        err::Err deinit();
        maixcam2::Frame *read(int32_t timeout_ms = -1);
        float volume(float volume);
        err::Err reset();
        int period_size(int size);
        int period_count(int count);
    private:
        void *param;
    };

    class AudioOut {
    public:
        AudioOut(ax_audio_out_param_t *cfg);
        ~AudioOut();
        err::Err init();
        err::Err deinit();
        bool mute(int mute); // mute = -1, get mute, mute = 0, mute, mute = 1, unmute
        float volume(float volume); // volume = -1, get volume, volume = 0~1, set volume
        err::Err pause();
        err::Err resume();
        err::Err write(maixcam2::Frame *frame, int32_t timeout_ms = -1);
        err::Err clear(void);
        err::Err wait(int32_t timeout_ms = -1);
        err::Err state(int &total_num, int &free_num, int &busy_num, int &pcm_delay);
        int period_size(int size = -1);
        int period_count(int count = -1);
    private:
        void *param;
    };
};

#endif

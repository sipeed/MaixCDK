#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ive.h"
#include "sample_comm.h"
#include "sophgo_middleware.h"

#define MMF_VI_MAX_CHN 2		// manually limit the max channel number of vi
#define MMF_VO_MAX_CHN 1		// manually limit the max channel number of vo
#define MMF_VO_RGB_LAYER    0
#define MMF_VO_YUV_LAYER    1

#if VPSS_MAX_PHY_CHN_NUM < MMF_VI_MAX_CHN
#error "VPSS_MAX_PHY_CHN_NUM < MMF_VI_MAX_CHN"
#endif
typedef struct {
	int mmf_used_cnt;
	int vo_rotate;	// 90, 180, 270
	bool vi_chn_is_inited[MMF_VI_MAX_CHN];
	bool vo_chn_is_inited[MMF_VO_MAX_CHN];
	SIZE_S vi_size;
	VIDEO_FRAME_INFO_S vi_frame[MMF_VI_MAX_CHN];
	SAMPLE_VO_CONFIG_S vo_cfg[MMF_VO_MAX_CHN];
	VB_CONFIG_S vb_conf;
} priv_t;

priv_t priv;

static void priv_param_init(void)
{
	memset(&priv, 0, sizeof(priv));
	priv.vo_rotate = 90;
	priv.vb_conf.astCommPool[1].u32BlkSize = 1280 * 720 * 4;
	priv.vb_conf.astCommPool[1].u32BlkCnt = 3;
	priv.vb_conf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
}

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;

void mmf_dump_frame(VIDEO_FRAME_INFO_S *frame) {
	if (frame == NULL) {
		printf("frame is null\n");
		return;
	}

	VIDEO_FRAME_S *vframe = &frame->stVFrame;
	printf("u32Width:\t\t%d\n", vframe->u32Width);
	printf("u32Height:\t\t%d\n", vframe->u32Height);
	printf("u32Stride[0]:\t\t%d\n", vframe->u32Stride[0]);
	printf("u32Stride[1]:\t\t%d\n", vframe->u32Stride[1]);
	printf("u32Stride[2]:\t\t%d\n", vframe->u32Stride[2]);
	printf("u32Length[0]:\t\t%d\n", vframe->u32Length[0]);
	printf("u32Length[1]:\t\t%d\n", vframe->u32Length[1]);
	printf("u32Length[2]:\t\t%d\n", vframe->u32Length[2]);
	printf("u64PhyAddr[0]:\t\t%#lx\n", vframe->u64PhyAddr[0]);
	printf("u64PhyAddr[1]:\t\t%#lx\n", vframe->u64PhyAddr[1]);
	printf("u64PhyAddr[2]:\t\t%#lx\n", vframe->u64PhyAddr[2]);
	printf("pu8VirAddr[0]:\t\t%p\n", vframe->pu8VirAddr[0]);
	printf("pu8VirAddr[1]:\t\t%p\n", vframe->pu8VirAddr[1]);
	printf("pu8VirAddr[2]:\t\t%p\n", vframe->pu8VirAddr[2]);

	printf("enPixelFormat:\t\t%d\n", vframe->enPixelFormat);
	printf("enBayerFormat:\t\t%d\n", vframe->enBayerFormat);
	printf("enVideoFormat:\t\t%d\n", vframe->enVideoFormat);
	printf("enCompressMode:\t\t%d\n", vframe->enCompressMode);
	printf("enDynamicRange:\t\t%d\n", vframe->enDynamicRange);
	printf("enColorGamut:\t\t%d\n", vframe->enColorGamut);

	printf("s16OffsetTop:\t\t%d\n", vframe->s16OffsetTop);
	printf("s16OffsetBottom:\t\t%d\n", vframe->s16OffsetBottom);
	printf("s16OffsetLeft:\t\t%d\n", vframe->s16OffsetLeft);
	printf("s16OffsetRight:\t\t%d\n", vframe->s16OffsetRight);
}

static int _try_release_sys(void)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	SAMPLE_INI_CFG_S	   	stIniCfg;
	SAMPLE_VI_CONFIG_S 		stViConfig;
	if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
		SAMPLE_PRT("Parse complete\n");
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevNum(stIniCfg.devNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_SetDevNum failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_IniToViCfg failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_DestroyIsp failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_DestroyVi(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_DestroyVi failed with %#x\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

int _try_release_vio_all(void)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	s32Ret = mmf_del_vi_channel_all();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("mmf_del_vi_channel_all failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = mmf_del_vo_channel_all(0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("mmf_del_vo_channel_all failed with %#x\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static void _mmf_sys_exit(void)
{
	if (g_stViConfig.s32WorkingViNum != 0) {
		SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	}
	SAMPLE_COMM_SYS_Exit();
}

static CVI_S32 _mmf_sys_init(SIZE_S stSize)
{
	VB_CONFIG_S	   stVbConf;
	CVI_U32        u32BlkSize, u32BlkRotSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	COMPRESS_MODE_E    enCompressMode   = COMPRESS_MODE_NONE;

	memcpy(&stVbConf, &priv.vb_conf, sizeof(VB_CONFIG_S));
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt		= 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, enCompressMode, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, enCompressMode, DEFAULT_ALIGN);
	u32BlkSize = MAX(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	memcpy(&priv.vb_conf.astCommPool[0], &stVbConf.astCommPool[0], sizeof(VB_POOL_CONFIG_S));
	SAMPLE_PRT("common pool[0] BlkSize %d cnt:%d\n", u32BlkSize, stVbConf.astCommPool[0].u32BlkCnt);

	memcpy(&stVbConf.astCommPool[1], &priv.vb_conf.astCommPool[1], sizeof(VB_POOL_CONFIG_S));
	SAMPLE_PRT("common pool[1] BlkSize %d cnt:%d\n", stVbConf.astCommPool[1].u32BlkSize, stVbConf.astCommPool[1].u32BlkCnt);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		goto error;
	}

	// FIXME: VB config maybe not correct
	system("sync");
	return s32Ret;
error:
	_mmf_sys_exit();
	return s32Ret;
}

static CVI_S32 _mmf_vpss_deinit(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	/*start vpss*/
	abChnEnable[VpssChn] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

static CVI_S32 _mmf_vpss_init(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, SIZE_S stSizeIn, SIZE_S stSizeOut, PIXEL_FORMAT_E formatOut, int fps)
{
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CROP_INFO_S   stGrpCropInfo;
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat                  = SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW                        = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev                      = 0;

	astVpssChnAttr[VpssChn].u32Width                    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height                   = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = formatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = fps;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = fps;
	astVpssChnAttr[VpssChn].u32Depth                    = 0;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X       = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y       = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width   = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height  = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x ! retry!!!\n", s32Ret);
		s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		}
		s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("retry to init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		} else {
			SAMPLE_PRT("retry to init vpss group ok!\n");
		}
	}

	CVI_S32 corp_scale_w = (CVI_S32)stSizeIn.u32Width / stSizeOut.u32Width;
	CVI_S32 corp_scale_h = (CVI_S32)stSizeIn.u32Height / stSizeOut.u32Height;
	CVI_U32 crop_w = corp_scale_w < corp_scale_h ? stSizeOut.u32Width * corp_scale_w: stSizeOut.u32Width * corp_scale_h;
	CVI_U32 crop_h = corp_scale_w < corp_scale_h ? stSizeOut.u32Height * corp_scale_w: stSizeOut.u32Height * corp_scale_h;
	if (corp_scale_h < 0 || corp_scale_w < 0) {
		SAMPLE_PRT("crop scale error. corp_scale_w: %d, corp_scale_h: %d\n", corp_scale_w, corp_scale_h);
		goto error;
	}

	stGrpCropInfo.bEnable = CVI_TRUE;
	stGrpCropInfo.stCropRect.s32X = (stSizeIn.u32Width - crop_w) / 2;
	stGrpCropInfo.stCropRect.s32Y = (stSizeIn.u32Height - crop_h) / 2;
	stGrpCropInfo.stCropRect.u32Width = crop_w;
	stGrpCropInfo.stCropRect.u32Height = crop_h;
	s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, &stGrpCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("set vpss group crop failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	return s32Ret;
error:
	_mmf_vpss_deinit(VpssGrp, VpssChn);
	return s32Ret;
}

static CVI_S32 _mmf_init(void)
{
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S	   stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;

	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_DEBUG;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	s32Ret = _mmf_sys_init(stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sys init failed. s32Ret: 0x%x !\n", s32Ret);
		goto _need_exit_sys_and_deinit_vi;
	}

	s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi init failed. s32Ret: 0x%x !\n", s32Ret);
		goto _need_exit_sys_and_deinit_vi;
	}

	priv.vi_size.u32Width = stSize.u32Width;
	priv.vi_size.u32Height = stSize.u32Height;

	return s32Ret;

_need_exit_sys_and_deinit_vi:
	_mmf_sys_exit();

	return s32Ret;
}

static void _mmf_deinit(void)
{
	_mmf_sys_exit();
}

static int _vi_get_unused_ch() {
	for (int i = 0; i < MMF_VI_MAX_CHN; i++) {
		if (priv.vi_chn_is_inited[i] == false) {
			return i;
		}
	}
	return -1;
}

int mmf_init(void)
{
    if (priv.mmf_used_cnt) {
		priv.mmf_used_cnt ++;
        printf("mmf already inited(cnt:%d)\n", priv.mmf_used_cnt);
        return 0;
    }

	priv_param_init();

	if (_try_release_sys() != CVI_SUCCESS) {
		printf("try release sys failed\n");
		return -1;
	} else {
		printf("try release sys ok\n");
	}

    if (_mmf_init() != CVI_SUCCESS) {
        printf("mmf init failed\n");
        return -1;
    } else {
		printf("mmf init ok\n");
	}

	priv.mmf_used_cnt = 1;

	if (_try_release_vio_all() != CVI_SUCCESS) {
		printf("try release vio failed\n");
		return -1;
	} else {
		printf("try release vio ok\n");
	}
    return 0;
}

bool mmf_is_init(void)
{
    return priv.mmf_used_cnt > 0 ? true : false;
}

int mmf_deinit(void) {
    if (!priv.mmf_used_cnt) {
        printf("%s: mmf not inited\n", __func__);
        return 0;
    }

	priv.mmf_used_cnt --;

	if (priv.mmf_used_cnt) {
		printf("mmf still in use(cnt:%d)\n", priv.mmf_used_cnt);
		return 0;
	} else {
		printf("mmf deinit\n");
		_mmf_deinit();
	}
    return 0;
}

int mmf_get_vi_unused_channel(void) {
	return _vi_get_unused_ch();
}

int mmf_add_vi_channel(int ch, int width, int height, int format) {
	if (!priv.mmf_used_cnt) {
		printf("%s: mmf not inited\n", __func__);
		return -1;
	}

	if (width <= 0 || height <= 0) {
		printf("invalid width or height\n");
		return -1;
	}

	if (format != PIXEL_FORMAT_NV21
		&& format != PIXEL_FORMAT_RGB_888
		&& format != PIXEL_FORMAT_BGR_888) {
		printf("invalid format\n");
		return -1;
	}

	if (mmf_vi_chn_is_open(ch)) {
		printf("vi ch %d already open\n", ch);
		return -1;
	}

	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSizeIn, stSizeOut;
	int fps = 30;
	PIXEL_FORMAT_E formatOut = (PIXEL_FORMAT_E)format;
	stSizeIn.u32Width   = priv.vi_size.u32Width;
	stSizeIn.u32Height  = priv.vi_size.u32Height;
	stSizeOut.u32Width  = ALIGN(width, DEFAULT_ALIGN);
	stSizeOut.u32Height = height;
	s32Ret = _mmf_vpss_init(0, ch, stSizeIn, stSizeOut, formatOut, fps);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x. try again..\r\n", s32Ret);
		s32Ret = _mmf_vpss_deinit(0, ch);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vpss deinit failed. s32Ret: 0x%x !\n", s32Ret);
			return -1;
		}

		s32Ret = _mmf_vpss_init(0, ch, stSizeIn, stSizeOut, formatOut, fps);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
			return -1;
		}
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, ch, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		goto _need_deinit_vpss;
	}

	priv.vi_chn_is_inited[ch] = true;
	return 0;
_need_deinit_vpss:
	_mmf_vpss_deinit(0, ch);
	return -1;
}

int mmf_del_vi_channel(int ch) {
	if (!priv.mmf_used_cnt) {
		printf("%s: mmf not inited\n", __func__);
		return -1;
	}

	if (ch < 0 || ch >= MMF_VI_MAX_CHN) {
		printf("invalid ch %d\n", ch);
		return -1;
	}

	if (priv.vi_chn_is_inited[ch] == false) {
		printf("vi ch %d not open\n", ch);
		return -1;
	}

	CVI_S32 s32Ret = CVI_SUCCESS;
	s32Ret = SAMPLE_COMM_VI_UnBind_VPSS(0, ch, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi unbind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		// return -1; // continue to deinit vpss
	}

	_mmf_vpss_deinit(0, ch);

	priv.vi_chn_is_inited[ch] = false;
	return s32Ret;
}

int mmf_del_vi_channel_all() {
	if (!priv.mmf_used_cnt) {
		printf("%s: mmf not inited\n", __func__);
		return -1;
	}

	for (int i = 0; i < MMF_VI_MAX_CHN; i++) {
		if (priv.vi_chn_is_inited[i] == true) {
			mmf_del_vi_channel(i);
		}
	}
	return 0;
}

bool mmf_vi_chn_is_open(int ch) {
	if (ch < 0 || ch >= MMF_VI_MAX_CHN) {
		printf("invalid ch %d\n", ch);
		return false;
	}

	return priv.vi_chn_is_inited[ch];
}

int mmf_vi_aligned_width(int ch) {
	UNUSED(ch);
	return DEFAULT_ALIGN;
}

int mmf_vi_frame_pop(int ch, void **data, int *len, int *width, int *height, int *format) {
	if (!priv.vi_chn_is_inited[ch]) {
        printf("vi ch %d not open\n", ch);
        return -1;
    }
    if (ch < 0 || ch >= MMF_VI_MAX_CHN) {
        printf("invalid ch %d\n", ch);
        return -1;
    }
    if (data == NULL || len == NULL || width == NULL || height == NULL || format == NULL) {
        printf("invalid param\n");
        return -1;
    }

	int ret = -1;
	VIDEO_FRAME_INFO_S *frame = &priv.vi_frame[ch];
	if (CVI_VPSS_GetChnFrame(0, ch, frame, 3000) == 0) {
        int image_size = frame->stVFrame.u32Length[0]
                        + frame->stVFrame.u32Length[1]
				        + frame->stVFrame.u32Length[2];
        CVI_VOID *vir_addr;
        vir_addr = CVI_SYS_Mmap(frame->stVFrame.u64PhyAddr[0], image_size);
        CVI_SYS_IonInvalidateCache(frame->stVFrame.u64PhyAddr[0], vir_addr, image_size);

		frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)vir_addr;		// save virtual address for munmap
		// printf("width: %d, height: %d, total_buf_length: %d, phy:%#lx  vir:%p\n",
		// 	   frame->stVFrame.u32Width,
		// 	   frame->stVFrame.u32Height, image_size,
        //        frame->stVFrame.u64PhyAddr[0], vir_addr);

		*data = vir_addr;
        *len = image_size;
        *width = frame->stVFrame.u32Width;
        *height = frame->stVFrame.u32Height;
        *format = frame->stVFrame.enPixelFormat;
		return 0;
    }
	return ret;
}

void mmf_vi_frame_free(int ch) {
	VIDEO_FRAME_INFO_S *frame = &priv.vi_frame[ch];
	int image_size = frame->stVFrame.u32Length[0]
                        + frame->stVFrame.u32Length[1]
				        + frame->stVFrame.u32Length[2];
	CVI_SYS_Munmap(frame->stVFrame.pu8VirAddr[0], image_size);
	if (CVI_VI_ReleaseChnFrame(0, ch, frame) != 0)
			SAMPLE_PRT("CVI_VI_ReleaseChnFrame NG\n");
}

// manage vo channels
int mmf_get_vo_unused_channel(int layer) {
	UNUSED(layer);
	for (int i = 0; i < MMF_VO_MAX_CHN; i++) {
		if (priv.vo_chn_is_inited[i] == false) {
			return i;
		}
	}
	return -1;
}
int mmf_add_vo_channel(int layer, int ch, int width, int height, int format) {
	UNUSED(layer);
	if (ch < 0 || ch >= MMF_VO_MAX_CHN) {
		printf("invalid ch %d\n", ch);
		return false;
	}

	SAMPLE_VO_CONFIG_S stVoConfig;
	RECT_S stDefDispRect  = {0, 0, (CVI_U32)width, (CVI_U32)height};
	SIZE_S stDefImageSize = {(CVI_U32)width, (CVI_U32)height};
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 panel_init = false;
	VO_PUB_ATTR_S stVoPubAttr;

	if (priv.vo_rotate == 90 || priv.vo_rotate == 270) {
		stDefDispRect.u32Width = (CVI_U32)height;
		stDefDispRect.u32Height = (CVI_U32)width;
		stDefImageSize.u32Width = (CVI_U32)height;
		stDefImageSize.u32Height = (CVI_U32)width;
	}

	CVI_VO_Get_Panel_Status(0, ch, &panel_init);
	if (panel_init) {
		CVI_VO_GetPubAttr(0, &stVoPubAttr);
		SAMPLE_PRT("Panel w=%d, h=%d.\n",\
				stVoPubAttr.stSyncInfo.u16Hact, stVoPubAttr.stSyncInfo.u16Vact);
		stDefDispRect.u32Width = stVoPubAttr.stSyncInfo.u16Hact;
		stDefDispRect.u32Height = stVoPubAttr.stSyncInfo.u16Vact;
		stDefImageSize.u32Width = stVoPubAttr.stSyncInfo.u16Hact;
		stDefImageSize.u32Height = stVoPubAttr.stSyncInfo.u16Vact;
	}
	s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		goto error;
	}

	stVoConfig.VoDev	 = 0;
	stVoConfig.stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync  = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat	 = (PIXEL_FORMAT_E)format;
	stVoConfig.enVoMode	 = VO_MODE_1MUX;
	s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		goto error;
	}

	memcpy(&priv.vo_cfg[ch], &stVoConfig, sizeof(SAMPLE_VO_CONFIG_S));

	switch (priv.vo_rotate) {
		case 0:break;
		case 90:
			CVI_VO_SetChnRotation(layer, ch, ROTATION_90);
			break;
		case 180:
			CVI_VO_SetChnRotation(layer, ch, ROTATION_180);
			break;
		case 270:
			CVI_VO_SetChnRotation(layer, ch, ROTATION_270);
			break;
		default:
			break;
	}
	priv.vo_chn_is_inited[ch] = true;
error:
	return s32Ret;
}

int mmf_del_vo_channel(int layer, int ch) {
	UNUSED(layer);

	if (ch < 0 || ch >= MMF_VO_MAX_CHN) {
		printf("invalid ch %d\n", ch);
		return CVI_FALSE;
	}

	if (priv.vo_chn_is_inited[ch] == false) {
		return CVI_SUCCESS;
	}

	if (CVI_SUCCESS != SAMPLE_COMM_VO_StopVO(&priv.vo_cfg[ch])) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StopVO failed with %#x\n", CVI_FAILURE);
		return CVI_FAILURE;
	}

	priv.vo_chn_is_inited[ch] = false;
	return CVI_SUCCESS;
}

int mmf_del_vo_channel_all(int layer) {
	CVI_S32 s32Ret = CVI_SUCCESS;
	for (int i = 0; i < MMF_VO_MAX_CHN; i++) {
		if (priv.vo_chn_is_inited[i] == true) {
			s32Ret = mmf_del_vo_channel(layer, i);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("mmf_del_vo_channel failed with %#x\n", s32Ret);
				// return s32Ret; // continue to del other chn
			}
		}
	}
	return CVI_SUCCESS;
}

bool mmf_vo_channel_is_open(int layer, int ch) {
	UNUSED(layer);
	if (ch < 0 || ch >= MMF_VO_MAX_CHN) {
		printf("invalid ch %d\n", ch);
		return false;
	}

	return priv.vo_chn_is_inited[ch];
}

// flush vo
int mmf_vo_frame_push(int layer, int ch, void *data, int len, int width, int height, int format) {
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;
	UNUSED(len);
	COMMON_GetPicBufferConfig(width, height, (PIXEL_FORMAT_E)format, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (priv.vo_cfg[ch].enPixFormat != (PIXEL_FORMAT_E)format) {
		printf("vo ch %d format not match. input:%d need:%d\n", ch, format, priv.vo_cfg[ch].enPixFormat);
		return CVI_FAILURE;
	}

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)format;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = width;
	stVideoFrame.stVFrame.u32Height = height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_SendFrame: Can't acquire vb block\n");
		return CVI_FAILURE;
	}
	// printf("u32PoolId:%d, u32Length:%d, u64PhyAddr:%#lx\r\n", CVI_VB_Handle2PoolId(blk), stVbCalConfig.u32VBSize, CVI_VB_Handle2PhysAddr(blk));

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	CVI_U32 total_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1] + stVideoFrame.stVFrame.u32Length[2];
	stVideoFrame.stVFrame.pu8VirAddr[0]
			= (CVI_U8*)CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], total_size);
	stVideoFrame.stVFrame.pu8VirAddr[1] = stVideoFrame.stVFrame.pu8VirAddr[0] + stVideoFrame.stVFrame.u32Length[0];
	stVideoFrame.stVFrame.pu8VirAddr[2] = stVideoFrame.stVFrame.pu8VirAddr[1] + stVideoFrame.stVFrame.u32Length[1];

	switch (format) {
	case PIXEL_FORMAT_RGB_888:
		if (stVideoFrame.stVFrame.u32Stride[0] != (CVI_U32)width) {
			for (int h = 0; h < height; h++) {
				memcpy((uint8_t *)stVideoFrame.stVFrame.pu8VirAddr[0] + width * h * 3, ((uint8_t *)data) + width * h * 3, width * 3);
			}
		} else {
			memcpy(stVideoFrame.stVFrame.pu8VirAddr[0], ((uint8_t *)data), width * height * 3);
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0],
							stVideoFrame.stVFrame.pu8VirAddr[0],
							width * height * 3);
	break;
	case PIXEL_FORMAT_NV21:
		if (stVideoFrame.stVFrame.u32Stride[0] != (CVI_U32)width) {
			for (int h = 0; h < height * 3 / 2; h ++) {
				memcpy((uint8_t *)stVideoFrame.stVFrame.pu8VirAddr[0] + stVideoFrame.stVFrame.u32Stride[0] * h,
						((uint8_t *)data) + width * h, width);
			}
		} else {
			memcpy(stVideoFrame.stVFrame.pu8VirAddr[0], ((uint8_t *)data), width * height * 3 / 2);
		}

		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0],
							stVideoFrame.stVFrame.pu8VirAddr[0],
							width * height * 3 / 2);
	break;
	default:
		printf("format not support\n");
		return CVI_FAILURE;
	}

	// mmf_dump_frame(&stVideoFrame);
	s32Ret = CVI_VO_SendFrame(layer, ch, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VO_SendFrame failed >< with %#x\n", s32Ret);
		return s32Ret;
	}
	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return CVI_SUCCESS;
}

int mmf_invert_format_to_maix(int mmf_format) {
	switch (mmf_format) {
		case PIXEL_FORMAT_RGB_888:
			return 0;
		case PIXEL_FORMAT_BGR_888:
			return 1;
		case PIXEL_FORMAT_NV21:
			return 8;
		default:
			return 0xFF;
	}
}

int mmf_invert_format_to_mmf(int maix_format) {
	switch (maix_format) {
		case 0:
			return PIXEL_FORMAT_RGB_888;
		case 1:
			return PIXEL_FORMAT_BGR_888;
		case 8:
			return PIXEL_FORMAT_NV21;
		default:
			return -1;
	}
}
/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __ISP_SENSOR_OPS_H__
#define __ISP_SENSOR_OPS_H__


#define AX_SENSOR_SET_DEFAULT_LINEAR_PARAM()                                                    \
do {                                                                                            \
        ptDftParam->ptDpc           = (typeof(ptDftParam->ptDpc))&dpc_param_sdr;                \
        ptDftParam->ptBlc           = (typeof(ptDftParam->ptBlc))&blc_param_sdr;                \
        ptDftParam->ptCsc           = (typeof(ptDftParam->ptCsc))&csc_param_sdr;                \
        ptDftParam->ptCa            = (typeof(ptDftParam->ptCa))&ca_param_sdr;                  \
        ptDftParam->ptDemosaic      = (typeof(ptDftParam->ptDemosaic))&demosaic_param_sdr;      \
        ptDftParam->ptGic           = (typeof(ptDftParam->ptGic))&gic_param_sdr;                \
        ptDftParam->ptFcc           = (typeof(ptDftParam->ptFcc))&fcc_param_sdr;                \
        ptDftParam->ptGamma         = (typeof(ptDftParam->ptGamma))&gamma_param_sdr;            \
        ptDftParam->ptCc            = (typeof(ptDftParam->ptCc))&cc_param_sdr;                  \
        ptDftParam->ptRltm          = (typeof(ptDftParam->ptRltm))&rltm_param_sdr;              \
        ptDftParam->ptRaw2dnr       = (typeof(ptDftParam->ptRaw2dnr))&raw2dnr_param_sdr;        \
        ptDftParam->ptDehaze        = (typeof(ptDftParam->ptDehaze))&dehaze_param_sdr;          \
        ptDftParam->ptDepurple      = (typeof(ptDftParam->ptDepurple))&depurple_param_sdr;      \
        ptDftParam->ptLsc           = (typeof(ptDftParam->ptLsc))&lsc_param_sdr;                \
        ptDftParam->ptSharpen       = (typeof(ptDftParam->ptSharpen))&sharpen_param_sdr;        \
        ptDftParam->ptScm           = (typeof(ptDftParam->ptScm))&scm_param_sdr;                \
        ptDftParam->ptYnr           = (typeof(ptDftParam->ptYnr))&ynr_param_sdr;                \
        ptDftParam->ptCnr           = (typeof(ptDftParam->ptCnr))&cnr_param_sdr;                \
        ptDftParam->ptCcmp          = (typeof(ptDftParam->ptCcmp))&ccmp_param_sdr;              \
        ptDftParam->ptYcproc        = (typeof(ptDftParam->ptYcproc))&ycproc_param_sdr;          \
        ptDftParam->ptHs2dlut       = (typeof(ptDftParam->ptHs2dlut))&hs2dlut_param_sdr;        \
        ptDftParam->ptYcrt          = (typeof(ptDftParam->ptYcrt))&ycrt_param_sdr;              \
        ptDftParam->ptAinr          = (typeof(ptDftParam->ptAinr))&ainr_param_sdr;              \
        ptDftParam->ptYuv3dnr       = (typeof(ptDftParam->ptYuv3dnr))&yuv3dnr_param_sdr;        \
        ptDftParam->ptLdc           = (typeof(ptDftParam->ptLdc))&ldc_param_sdr;                \
        ptDftParam->ptClp           = (typeof(ptDftParam->ptClp))&clp_param_sdr;                \
        ptDftParam->ptDis           = (typeof(ptDftParam->ptDis))&dis_param_sdr;                \
        ptDftParam->pIspMeParam     = (typeof(ptDftParam->pIspMeParam))&me_param_sdr;           \
        ptDftParam->ptNuc           = (typeof(ptDftParam->ptNuc))&nuc_param_sdr;                \
        ptDftParam->ptScene         = (typeof(ptDftParam->ptScene))&scene_param_sdr;            \
} while(0)

#define AX_SENSOR_SET_3A_DEFAULT_LINEAR_PARAM()                                                    \
do {                                                                                            \
        ptDftParam->ptAwbDftParam   = (typeof(ptDftParam->ptAwbDftParam))&awb_param_sdr;        \
        ptDftParam->ptAeDftParam    = (typeof(ptDftParam->ptAeDftParam))&ae_param_sdr;          \
} while(0)

#define AX_SENSOR_SET_DEFAULT_HDR_2X_PARAM()                                                    \
do {                                                                                            \
        ptDftParam->ptDpc           = (typeof(ptDftParam->ptDpc))&dpc_param_hdr_2x;             \
        ptDftParam->ptBlc           = (typeof(ptDftParam->ptBlc))&blc_param_hdr_2x;             \
        ptDftParam->ptCsc           = (typeof(ptDftParam->ptCsc))&csc_param_hdr_2x;             \
        ptDftParam->ptCa            = (typeof(ptDftParam->ptCa))&ca_param_hdr_2x;               \
        ptDftParam->ptDemosaic      = (typeof(ptDftParam->ptDemosaic))&demosaic_param_hdr_2x;   \
        ptDftParam->ptGic           = (typeof(ptDftParam->ptGic))&gic_param_hdr_2x;             \
        ptDftParam->ptFcc           = (typeof(ptDftParam->ptFcc))&fcc_param_hdr_2x;             \
        ptDftParam->ptDepurple      = (typeof(ptDftParam->ptDepurple))&depurple_param_hdr_2x;   \
        ptDftParam->ptHdr           = (typeof(ptDftParam->ptHdr))&hdr_param_hdr_2x;             \
        ptDftParam->ptGamma         = (typeof(ptDftParam->ptGamma))&gamma_param_hdr_2x;         \
        ptDftParam->ptCc            = (typeof(ptDftParam->ptCc))&cc_param_hdr_2x;               \
        ptDftParam->ptRltm          = (typeof(ptDftParam->ptRltm))&rltm_param_hdr_2x;           \
        ptDftParam->ptRaw2dnr       = (typeof(ptDftParam->ptRaw2dnr))&raw2dnr_param_hdr_2x;     \
        ptDftParam->ptDehaze        = (typeof(ptDftParam->ptDehaze))&dehaze_param_hdr_2x;       \
        ptDftParam->ptLsc           = (typeof(ptDftParam->ptLsc))&lsc_param_hdr_2x;             \
        ptDftParam->ptSharpen       = (typeof(ptDftParam->ptSharpen))&sharpen_param_hdr_2x;     \
        ptDftParam->ptScm           = (typeof(ptDftParam->ptScm))&scm_param_hdr_2x;             \
        ptDftParam->ptYnr           = (typeof(ptDftParam->ptYnr))&ynr_param_hdr_2x;             \
        ptDftParam->ptCnr           = (typeof(ptDftParam->ptCnr))&cnr_param_hdr_2x;             \
        ptDftParam->ptCcmp          = (typeof(ptDftParam->ptCcmp))&ccmp_param_hdr_2x;           \
        ptDftParam->ptYcproc        = (typeof(ptDftParam->ptYcproc))&ycproc_param_hdr_2x;       \
        ptDftParam->ptHs2dlut       = (typeof(ptDftParam->ptHs2dlut))&hs2dlut_param_hdr_2x;     \
        ptDftParam->ptYcrt          = (typeof(ptDftParam->ptYcrt))&ycrt_param_hdr_2x;           \
        ptDftParam->ptAinr          = (typeof(ptDftParam->ptAinr))&ainr_param_hdr_2x;           \
        ptDftParam->ptYuv3dnr       = (typeof(ptDftParam->ptYuv3dnr))&yuv3dnr_param_hdr_2x;     \
        ptDftParam->ptLdc           = (typeof(ptDftParam->ptLdc))&ldc_param_hdr_2x;             \
        ptDftParam->ptClp           = (typeof(ptDftParam->ptClp))&clp_param_hdr_2x;             \
        ptDftParam->ptDis           = (typeof(ptDftParam->ptDis))&dis_param_hdr_2x;             \
        ptDftParam->pIspMeParam     = (typeof(ptDftParam->pIspMeParam))&me_param_hdr_2x;        \
        ptDftParam->ptNuc           = (typeof(ptDftParam->ptNuc))&nuc_param_hdr_2x;             \
        ptDftParam->ptScene         = (typeof(ptDftParam->ptScene))&scene_param_hdr_2x;         \
} while (0)

#define AX_SENSOR_SET_3A_DEFAULT_HDR_2X_PARAM()                                                    \
do {                                                                                            \
        ptDftParam->ptAwbDftParam   = (typeof(ptDftParam->ptAwbDftParam))&awb_param_hdr_2x;     \
        ptDftParam->ptAeDftParam    = (typeof(ptDftParam->ptAeDftParam))&ae_param_hdr_2x;       \
} while (0)

#define AX_SENSOR_SET_DEFAULT_HDR_3X_PARAM()                                                    \
do {} while (0)

#define AX_SENSOR_SET_DEFAULT_HDR_4X_PARAM()                                                    \
do {} while (0)

#define AX_SENSOR_SET_DEFAULT_SPECIAL_PARAM(type)                                               \
do {                                                                                            \
        ptDftParam->ptDpc           = (typeof(ptDftParam->ptDpc))&dpc_param_##type;             \
        ptDftParam->ptBlc           = (typeof(ptDftParam->ptBlc))&blc_param_##type;             \
        ptDftParam->ptCsc           = (typeof(ptDftParam->ptCsc))&csc_param_##type;             \
        ptDftParam->ptCa            = (typeof(ptDftParam->ptCa))&ca_param_##type;               \
        ptDftParam->ptDemosaic      = (typeof(ptDftParam->ptDemosaic))&demosaic_param_##type;   \
        ptDftParam->ptGic           = (typeof(ptDftParam->ptGic))&gic_param_##type;             \
        ptDftParam->ptFcc           = (typeof(ptDftParam->ptFcc))&fcc_param_##type;             \
        ptDftParam->ptGamma         = (typeof(ptDftParam->ptGamma))&gamma_param_##type;         \
        ptDftParam->ptCc            = (typeof(ptDftParam->ptCc))&cc_param_##type;               \
        ptDftParam->ptRltm          = (typeof(ptDftParam->ptRltm))&rltm_param_##type;           \
        ptDftParam->ptRaw2dnr       = (typeof(ptDftParam->ptRaw2dnr))&raw2dnr_param_##type;     \
        ptDftParam->ptDehaze        = (typeof(ptDftParam->ptDehaze))&dehaze_param_##type;       \
        ptDftParam->ptDepurple      = (typeof(ptDftParam->ptDepurple))&depurple_param_##type;   \
        ptDftParam->ptHdr           = (typeof(ptDftParam->ptHdr))&hdr_param_##type;             \
        ptDftParam->ptLsc           = (typeof(ptDftParam->ptLsc))&lsc_param_##type;             \
        ptDftParam->ptSharpen       = (typeof(ptDftParam->ptSharpen))&sharpen_param_##type;     \
        ptDftParam->ptScm           = (typeof(ptDftParam->ptScm))&scm_param_##type;             \
        ptDftParam->ptYnr           = (typeof(ptDftParam->ptYnr))&ynr_param_##type;             \
        ptDftParam->ptCnr           = (typeof(ptDftParam->ptCnr))&cnr_param_##type;             \
        ptDftParam->ptCcmp          = (typeof(ptDftParam->ptCcmp))&ccmp_param_##type;           \
        ptDftParam->ptYcproc        = (typeof(ptDftParam->ptYcproc))&ycproc_param_##type;       \
        ptDftParam->ptHs2dlut       = (typeof(ptDftParam->ptHs2dlut))&hs2dlut_param_##type;     \
        ptDftParam->ptYcrt          = (typeof(ptDftParam->ptYcrt))&ycrt_param_##type;           \
        ptDftParam->ptAinr          = (typeof(ptDftParam->ptAinr))&ainr_param_##type;           \
        ptDftParam->ptYuv3dnr       = (typeof(ptDftParam->ptYuv3dnr))&yuv3dnr_param_##type;     \
        ptDftParam->ptLdc           = (typeof(ptDftParam->ptLdc))&ldc_param_##type;             \
        ptDftParam->ptClp           = (typeof(ptDftParam->ptClp))&clp_param_##type;             \
        ptDftParam->ptDis           = (typeof(ptDftParam->ptDis))&dis_param_##type;             \
        ptDftParam->pIspMeParam     = (typeof(ptDftParam->pIspMeParam))&me_param_##type;        \
        ptDftParam->ptNuc           = (typeof(ptDftParam->ptNuc))&nuc_param_##type;             \
        ptDftParam->ptScene         = (typeof(ptDftParam->ptScene))&scene_param_##type;         \
} while(0)

#define AX_SENSOR_SET_3A_DEFAULT_SPECIAL_PARAM(type)                                               \
do {                                                                                            \
        ptDftParam->ptAwbDftParam   = (typeof(ptDftParam->ptAwbDftParam))&awb_param_##type;     \
        ptDftParam->ptAeDftParam    = (typeof(ptDftParam->ptAeDftParam))&ae_param_##type;       \
} while(0)

#endif  /* __ISP_SENSOR_OPS_H__ */


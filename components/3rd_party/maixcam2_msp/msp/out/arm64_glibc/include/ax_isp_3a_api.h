/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_ISP_3A_API_H_
#define _AX_ISP_3A_API_H_

#include "ax_base_type.h"
#include "ax_isp_api.h"
#include "ax_isp_3a_struct.h"
#include "ax_sensor_struct.h"
#include "ax_lens_iris_struct.h"
#include "ax_lens_af_struct.h"


#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 AX_ISP_IQ_SetAwbMode(AX_U8 pipe, int nAWBMode);
AX_S32 AX_ISP_IQ_SetAwbParam(AX_U8 pipe, AX_ISP_IQ_AWB_PARAM_T *pIspAwbParam);
AX_S32 AX_ISP_IQ_GetAwbParam(AX_U8 pipe, AX_ISP_IQ_AWB_PARAM_T *pIspAwbParam);
AX_S32 AX_ISP_IQ_GetAwbStatus(AX_U8 pipe, AX_ISP_IQ_AWB_STATUS_T *pIspAwbStatus);
AX_S32 AX_ISP_IQ_AwbSyncInit();
AX_S32 AX_ISP_IQ_AwbSyncDeInit();
AX_S32 AX_ISP_IQ_AwbSyncCalibration(AX_ISP_IQ_AWB_SYNC_RATIO_T *pAwbSyncRatio);
AX_S32 AX_ISP_IQ_SetAwbSyncParam(AX_ISP_IQ_AWB_SYNC_RATIO_T *pAwbSyncRatio);
AX_S32 AX_ISP_IQ_GetAwbSyncParam(AX_ISP_IQ_AWB_SYNC_RATIO_T *pAwbSyncRatio);

AX_S32 AX_ISP_IQ_SetAeParam(AX_U8 pipe, AX_ISP_IQ_AE_PARAM_T *pIspAeParam);
AX_S32 AX_ISP_IQ_GetAeParam(AX_U8 pipe, AX_ISP_IQ_AE_PARAM_T *pIspAeParam);
AX_S32 AX_ISP_IQ_GetAeStatus(AX_U8 pipe, AX_ISP_IQ_AE_STATUS_T *pIspAeStatus);
AX_S32 AX_ISP_IQ_GetAeHwLimit(AX_U8 pipe, AX_ISP_IQ_EXP_HW_LIMIT_T *pIspAeHwLimit);
AX_S32 AX_ISP_IQ_CalibrateAeLuxK(AX_U8 pipe, AX_ISP_IQ_LUX_K_CALIB_INPUT_T *pIspAeLuxk);
AX_U32 AX_ISP_IQ_GetAeLuxK(AX_U8 pipe);

AX_S32 AX_ISP_IQ_AeSyncInit(void);
AX_S32 AX_ISP_IQ_AeSyncDeInit(void);
AX_S32 AX_ISP_IQ_AeSyncCalibration(AX_ISP_IQ_AE_SYNC_RATIO_T *pAeSyncRatio);
AX_S32 AX_ISP_IQ_SetAeSyncParam(AX_ISP_IQ_AE_SYNC_RATIO_T *pAeSyncRatio);
AX_S32 AX_ISP_IQ_GetAeSyncParam(AX_ISP_IQ_AE_SYNC_RATIO_T *pAeSyncRatio);

AX_S32 AX_ISP_IQ_CalibrateAeNoiseLevel(AX_U8 pipe,AX_ISP_IQ_NOISE_LEVEL_CALIB_INPUT_T *pIspAeNoiseLevel);
AX_U32 AX_ISP_IQ_GetAeNoiseLevel(AX_U8 pipe);

AX_S32 AX_ISP_IQ_CalibrateAeSleepSeting(AX_U8 pipe, AX_ISP_AE_SLEEP_SETTING_T *pIspAeSleepSetting);
AX_S32 AX_ISP_IQ_GetAeSleepSetting(AX_U8 pipe, AX_ISP_AE_SLEEP_SETTING_T *pIspAeSleepSetting);
AX_S32 AX_ISP_IQ_SetAeDetectObjectROI(AX_U8 pipe, AX_ISP_AE_DETECT_OBJECT_PARAM_T *pDetectObjectParam);
AX_S32 AX_ISP_IQ_SetAeLongFrameMode(AX_U8 pipe, AX_ISP_LFHDR_MODE_E eLFHdrMode);
AX_S32 AX_ISP_IQ_GetAeLongFrameMode(AX_U8 pipe, AX_ISP_LFHDR_MODE_E *pELFHdrMode);

AX_S32 AX_ISP_ALG_AeGetScanStatus(AX_U8 pipe, AX_ISP_AE_SCAN_STATUS *pScanStatusToUI);
AX_S32 AX_ISP_ALG_AeAgainScanStart(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AeDgainScanStart(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AeShutterScanStart(AX_U8 pipe);

AX_S32 AX_ISP_AeAfdStatusCallback(AX_U8 pipe, AX_ISP_AE_AFD_STATUS_FUNC_T *pAfdFuncs);

AX_S32 AX_ISP_ALG_AeRegisterSensor(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *pSensorHandle);
AX_S32 AX_ISP_ALG_AeUnRegisterSensor(AX_U8 pipe);

AX_S32 AX_ISP_ALG_AwbRegisterSensor(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *pSensorHandle);
AX_S32 AX_ISP_ALG_AwbUnRegisterSensor(AX_U8 pipe);

AX_S32 AX_ISP_ALG_AeRegisterLensIris(AX_U8 pipe, AX_LENS_ACTUATOR_IRIS_FUNC_T *ptLensIrisReg);
AX_S32 AX_ISP_ALG_AeUnRegisterLensIris(AX_U8 pipe);

AX_S32 AX_ISP_ALG_CAfRegisterLensAf(AX_U8 pipe, AX_LENS_AF_FUNCS_T *ptLensAfReg);
AX_S32 AX_ISP_ALG_CAfUnRegisterLensAf(AX_U8 pipe);

/* Callback Functions Called by the AX Platform 3A Framework. */
AX_S32 AX_ISP_ALG_AeInit(AX_U8 pipe, AX_ISP_AE_INITATTR_T *pAeInitParam, AX_ISP_AE_RESULT_T *pAeResult);
AX_S32 AX_ISP_ALG_AeDeInit(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AeRun(AX_U8 pipe, AX_ISP_AE_INPUT_INFO_T *pAeInputInfo, AX_ISP_AE_RESULT_T *pAeResult);
AX_S32 AX_ISP_ALG_AeCtrl(AX_U8 pipe, AX_ISP_CTRL_CMD_E nAeCtrlCmd, AX_ISP_AE_RESULT_T *pAeResult, void *pValue);

AX_S32 AX_ISP_ALG_Ae_AFDInit(AX_U8 pipe, AX_U32 nZoneRow, AX_AE_AFD_OUT_T *pAFDOut);
AX_S32 AX_ISP_ALG_Ae_AFDDeInit(AX_U8 pipe);
AX_S32 AX_ISP_ALG_Ae_AFDRun(AX_U8  pipe, AX_ISP_AE_INPUT_INFO_T *pAeInputInfo, AX_F32 refVal, AX_F32 fLineTime,
                            AX_AE_AFD_PARAMS_T *pAFDParams, AX_AE_AFD_OUT_T *pAFDOut);

AX_S32 AX_ISP_ALG_AwbInit(AX_U8 pipe, AX_ISP_AWB_INITATTR_T *pAwbInitParam, AX_ISP_AWB_RESULT_T *pAwbResult);
AX_S32 AX_ISP_ALG_AwbRun(AX_U8 pipe, AX_ISP_AWB_INPUT_INFO_T *pAwbInputInfo, AX_ISP_AWB_RESULT_T *pAwbResult);
AX_S32 AX_ISP_ALG_AwbCtrl(AX_U8 pipe, AX_ISP_CTRL_CMD_E nAwbCtrlCmd, AX_ISP_AWB_RESULT_T *pAwbResult, void *pValue);
AX_S32 AX_ISP_ALG_AwbDeInit(AX_U8 pipe);

AX_S32 AX_ISP_ALG_CAfInit(AX_U8 pipe);
AX_S32 AX_ISP_ALG_CAfRun(AX_U8 pipe);
AX_S32 AX_ISP_ALG_CAfDeInit(AX_U8 pipe);

AX_S32 AX_ISP_IQ_SetCafParam(AX_U8 pipe, AX_ISP_IQ_CAF_PARAM_T *pIspCafParam);
AX_S32 AX_ISP_IQ_GetCafParam(AX_U8 pipe, AX_ISP_IQ_CAF_PARAM_T *pIspCafParam);

AX_S32 AX_ISP_ALG_AfSetZoomPos(AX_U8  pipe, AX_ISP_CAF_ZOOM_POS *pZoomPosFromUI);
AX_S32 AX_ISP_ALG_AfSetFocusPos( AX_U8  pipe,AX_ISP_CAF_FOCUS_POS *pFocusPosFromUI);
AX_S32 AX_ISP_ALG_AfSetLensRatio(AX_U8 pipe,AX_ISP_CAF_ZOOM_RATIO *pLensZoomRatioUI);
AX_S32 AX_ISP_ALG_AfGetCurrentZoomPos(AX_U8 pipe, AX_ISP_CAF_ZOOM_POS *pResultToUI);
AX_S32 AX_ISP_ALG_AfGetCurrentFocusPos(AX_U8 pipe, AX_ISP_CAF_FOCUS_POS *pResultToUI);
AX_S32 AX_ISP_ALG_AfFvScanStart(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AfGetScanStatus(AX_U8 pipe ,AX_ISP_CAF_SCAN_STATUS *pscanstatus);
AX_S32 AX_ISP_ALG_AfGetScanInfo(AX_U8 pipe,AX_ISP_CAF_SCAN_FV_INFO *afParams);
AX_S32 AX_ISP_ALG_AfZoomWideMove(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AfZoomTeleMove(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AfStopMove(AX_U8 pipe);
AX_S32 AX_ISP_ALG_AfFarMove(AX_U8 pipe,AX_U8 speed);
AX_S32 AX_ISP_ALG_AfNearMove(AX_U8 pipe,AX_U8 speed);

AX_S32 AX_ISP_IQ_GetDayNightStatus(AX_U8 pipe, AX_U32 *dayNightMode);
AX_S32 AX_ISP_IQ_SetIrParam(AX_U8 pipe, AX_ISP_IQ_IR_PARAM_T *pIspIrParam);
#ifdef __cplusplus
}
#endif

#endif //_AX_ISP_3A_H_

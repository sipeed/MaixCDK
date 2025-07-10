/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_LENS_AF_STRUCT_H_
#define _AX_LENS_AF_STRUCT_H_

#include "ax_base_type.h"
#include "ax_isp_3a_struct.h"

#define ACTUATOR_MAX_NUM 8
#define TRACK_TABLE_MAX_ROW 128
#define TRACK_TABLE_MAX_COL 32

typedef enum AxAfMotorStatus_s {
    AF_MOTOR_IDLE          = 0,
    AF_MOTOR_MOVE_FINISHED = 1,
    AF_MOTOR_PI_FOUND      = 2,
    AF_MOTOR_PI_FIND_ERR   = 3,
    AF_MOTOR_ERROR      = 4,
} AxAfMotorStatus_t;

typedef enum AxAfMotorsType {
    MOTOR_AF      = 1,
    MOTOR_ZOOM    = 2,
    MOTOR_IRIS    = 3,
}AxAfMotorsType_s;

typedef struct {
    AX_U8   curveTableRows;
    AX_U8   curveTableCols;
    AX_S32  curveTable[TRACK_TABLE_MAX_ROW * TRACK_TABLE_MAX_COL];
    AX_S32  distanceTable[TRACK_TABLE_MAX_COL];
} AX_LENS_AF_TRACK_TABLE_T;


typedef struct {

    /* Focus Motor Params */
    AX_S32  focusHwMaxStep;          // Max Step of Focus Motor Hardware
    AX_S32  focusHwMinPos;           // Min Position of Focus Motor Hardware
    AX_S32  focusHwMaxPos;           // Max Position of Focus Motor Hardware
    AX_S32  focusHwWideFarPos;       // Focus Motor Position of Far End in Wide Angle
    AX_U8   focusRangeExtBase;       // Base-Extended-Range of Scan and Search (Unit:Step)
    AX_U8   focusRangeExtRatio;      // How Many focusRangeExtBase are Added to Theoretical Far & Near End

    /* Zoom Motors Params */
    AX_U8   zoomRatioToPosTableRows;                      //
    AX_F32  zoomRatioToPosTable[TRACK_TABLE_MAX_ROW * 3]; //
    AX_S32  zoom1HwMaxStep;          // Max Step of Zoom1 Motor Hardware
    AX_S32  zoom1HwMinPos;           // Min Position of Zoom1 Motor Hardware
    AX_S32  zoom1HwMaxPos;           // Max Position of Zoom1 Motor Hardware
    AX_S32  zoom2HwMaxStep;          // Max Step of Zoom2 Motor Hardware
    AX_S32  zoom2HwMinPos;           // Min Position of Zoom2 Motor Hardware
    AX_S32  zoom2HwMaxPos;           // Max Position of Zoom2 Motor Hardware
    AX_S32  zoomSecondHwPos;         //The position of the zoom posetable second or third line
    AX_S32  zoomSpiltPos;            //Critical positions in linear and nonlinear regions

    /* Object Distance */
    AX_U8   distLimitNear;           // Nearest Object Distance (Unit:cm)
    AX_LENS_AF_TRACK_TABLE_T  tTrackTable;      // Zoom Track Table


    AX_U8   *pZoomSpeedStepTable;          //Step table of different zoom lengths.
    AX_S32  *pZoomSpeedPosTable;           //zoom pos table of Segmenting Zoom Length.

    AX_S32 *pCaliActTable;
    AX_S32 *pPreCaliTable;
} AX_LENS_AF_HARDWARE_PARAMS_T;


typedef struct {
    AX_S32 focusPiOffset;
    AX_S32 zoom1PiOffset;
    AX_S32 zoom2PiOffset;
} AX_LENS_AF_PI_CALIB_PARAMS_T;


typedef struct {
    AX_LENS_AF_PI_CALIB_PARAMS_T  tPiParams;
} AX_LENS_AF_CALIB_PARAMS_T;


typedef struct {

    /* Optical Params, Mechanical Params, AxAF Algo Tuning Params of A Lens. */
    AX_S32(*pfn_af_lens_get_tuning_params)(AX_U8 nPipeId,
                                           AX_ISP_IQ_CAF_PARAM_T *pAfTuningParams);
    AX_S32(*pfn_af_lens_get_hardware_params)(AX_U8 nPipeId,
            AX_LENS_AF_HARDWARE_PARAMS_T *pAfHwParams);

    AX_S32(*pfn_af_lens_get_calib_params)(AX_U8 nPipeId,
                                          AX_LENS_AF_CALIB_PARAMS_T *pAfHwParams);
    AX_S32(*pfn_lens_init)(AX_U8 nPipeId, AxAfMotorsType_s motorstype);

    /* af focus actuator */
    AX_U8(*pfn_af_lens_get_rstb_status)(AX_U8 nPipeId);
    AX_U8(*pfn_af_focus_get_status)(AX_U8 nPipeId);
    AX_S32(*pfn_af_focus_move)(AX_U8 nPipeId, AX_S32 nStep, AX_S32 nDirection);
    AX_S32(*pfn_af_focus_exit)(AX_U8 nPipeId);

    /* af zoom actuator */
    AX_U8(*pfn_af_zoom_get_status)(AX_U8 nPipeId);
    AX_S32(*pfn_af_zoom_move)(AX_U8 nPipeId, AX_S32 nStep, AX_S32 ndirection, AX_S32 nzoom1Pos);
    AX_S32(*pfn_af_zoom_exit)(AX_U8 nPipeId);

} AX_LENS_AF_FUNCS_T;

#endif


/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __ISP_SENSOR_INTERNAL_H__
#define __ISP_SENSOR_INTERNAL_H__

#include "ax_base_type.h"
#include "ax_sensor_struct.h"
#include "isp_sensor_ops.h"

#define SENSOR_MAX_GAIN_STEP    (2000)

/* nSnsRawType */
#define AX_SNS_RAWTYPE_8BIT     (0x00000001U)
#define AX_SNS_RAWTYPE_10BIT    (0x00000002U)
#define AX_SNS_RAWTYPE_12BIT    (0x00000004U)
#define AX_SNS_RAWTYPE_14BIT    (0x00000008U)
#define AX_SNS_RAWTYPE_16BIT    (0x00000010U)

/* nSnsResolution */
#define AX_SNS_RES_3840_2160    (0x00000001U)       /**<  3840x2160   */
#define AX_SNS_RES_2688_1520    (0x00000002U)       /**<  2688x1520   */
#define AX_SNS_RES_2592_1944    (0x00000004U)       /**<  2592x1944   */
#define AX_SNS_RES_2560_1440    (0x00000008U)       /**<  2560x1440   */
#define AX_SNS_RES_4000_3000    (0x00000010U)       /**<  3840x2160   */
#define AX_SNS_RES_2880_1620    (0x00000020U)       /**<  2880x1620   */
#define AX_SNS_RES_2880_1616    (0x00000040U)       /**<  2880x1620   */
#define AX_SNS_RES_2592_1948    (0x00000080U)       /**<  2592x1948   */
#define AX_SNS_RES_1080P        (0x00001000U)       /**<  1920x1080   */
#define AX_SNS_RES_720P         (0x00002000U)       /**<  1280x720    */
#define AX_SNS_RES_D1           (0x00004000U)       /**<  720x576     */
#define AX_SNS_RES_VGA          (0x00008000U)       /**<  640x480     */

/* nSnsFps */
#define AX_SNS_15FPS            (0x00000010U)
#define AX_SNS_25FPS            (0x00000020U)
#define AX_SNS_30FPS            (0x00000040U)
#define AX_SNS_45FPS            (0x00000080U)
#define AX_SNS_50FPS            (0x00000100U)
#define AX_SNS_60FPS            (0x00000200U)

#define IS_LINEAR_MODE(mode)      (AX_SNS_LINEAR_MODE == (mode))
#define IS_HDR_MODE(mode)         (((mode) < AX_SNS_HDR_MODE_MAX) && ((mode) > AX_SNS_LINEAR_MODE))
#define IS_2DOL_HDR_MODE(mode)    (AX_SNS_HDR_2X_MODE == (mode))
#define IS_3DOL_HDR_MODE(mode)    (AX_SNS_HDR_3X_MODE == (mode))
#define IS_4DOL_HDR_MODE(mode)    (AX_SNS_HDR_4X_MODE == (mode))

#define HDR_LONG_FRAME_IDX              (0)
#define HDR_MEDIUM_FRAME_IDX            (1)
#define HDR_SHORT_FRAME_IDX             (2)
#define HDR_SMART_SHORT_FRAME_IDX       (3)

#define SNS_1_SECOND_UNIT_US            (1000000)

#define AX_SNS_FPS_EPS                  (0.3f)
#define AX_SNS_FPS_MIN                  (SNS_MIN_FRAME_RATE - AX_SNS_FPS_EPS)
#define AX_SNS_FPS_MAX                  (SNS_MAX_FRAME_RATE + AX_SNS_FPS_EPS)


typedef struct s_camera_i2c_reg_array {
    int addr;
    int value;
} camera_i2c_reg_array;

typedef struct {
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_F32 fFrameRate;
    AX_U32 nVts;
    AX_SNS_HDR_MODE_E eHDRMode; /* AX_SNS_WMODE_E */
    AX_SNS_MASTER_SLAVE_E eMasterSlaveSel;
} AX_SNS_MODE_T;


typedef struct {
    AX_U32 sns_i2c_bnum;
    AX_U32 sns_i2c_fd;
    AX_U32 slave_addr;
    AX_U32 address_byte;
    AX_U32 data_byte;
    AX_U32 swap_byte;
} AX_SNS_I2C_T;

typedef struct _AX_SNS_STATE_OBJ_ {
    AX_S32                          sns_id;
    AX_BOOL                         bSyncInit;         /* AX_TRUE: ae ctrl regs init */
    AX_U32                          eImgMode;          /* xxx_settings.h  enum xxx_SETTING_SEL_E  */
    AX_ISP_LFHDR_MODE_E             eLFHdrMode;
    AX_SNS_I2C_T                    sns_i2c_obj;
    AX_SNS_MODE_T                   sns_mode_obj;
    AX_SNS_ATTR_T                   sns_attr_param;
    AX_SNS_EXP_CTRL_PARAM_T         ae_ctrl_param;
    AX_SNS_AWB_PARAM_T              awb_ctrl_param;
    AX_SNS_REGS_CFG_TABLE_T         sztRegsInfo[2];    /* [0]: sensor regs info of cur-frame; [1]: sensor regs info of pre-frame ; */
} SNS_STATE_OBJ;


typedef struct _AX_SNS_DRV_DELAY_TABLE_T_ {
    AX_U32  nIdex;              /* register index */
    AX_U32  nRegAddr;           /* register address */
    AX_U32  nRegValue;          /* Sensor register value */
    AX_U8   nDelayFrmNum;       /* Number of frames for register delay configuration */
    AX_U8   reserve[3];
} AX_SNS_DRV_DELAY_TABLE_T;


#endif

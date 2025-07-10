/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_SENSOR_STRUCT_H_
#define _AX_SENSOR_STRUCT_H_

#include "ax_base_type.h"
#include "ax_isp_common.h"
#include "ax_isp_iq_api.h"
#include "ax_isp_3a_struct.h"

typedef AX_S32 SENSOR_DEV;
typedef AX_U8  ISP_PIPE_ID;

#define HDR_MAX_FRAME_NUM               (4)
#define ISP_MAX_I2C_SNS_REGISTER_NUM        (56)
#define ISP_MAX_SPI_SNS_REGISTER_NUM        (32)


#define SNS_MIN_FRAME_RATE              (1)
#define SNS_MAX_FRAME_RATE              (60)

#define AX_SNS_L_FSOF                   (0x00)
#define AX_SNS_L_FEOF                   (0x01)
#define AX_SNS_S_FSOF                   (0x10)
#define AX_SNS_S_FEOF                   (0x11)
#define AX_SNS_VS_FSOF                  (0x20)
#define AX_SNS_VS_FEOF                  (0x21)

#define SNS_STREAM_REG_NUM              (8)

typedef enum _AX_SNS_ADGAIN_MODE_E_ {
    AX_AGAIN_ONLY           = 0,
    AX_ADGAIN_SEPARATION    = 1,
    AX_ADGAIN_COMBINED      = 2,
} AX_SNS_ADGAIN_MODE_E;

typedef enum _AX_SNS_HCGLCG_MODE_E_ {
    AX_HCG_MODE             = 0,
    AX_LCG_MODE             = 1,
    AX_LCG_NOTSUPPORT_MODE  = 2,
} AX_SNS_HCGLCG_MODE_E;

typedef enum _AX_SNS_MASTER_SLAVE_E_ {
    AX_SNS_MASTER        = 0,
    AX_SNS_SLAVE         = 1,
    AX_SNS_SYNC_MASTER   = 2,
    AX_SNS_SYNC_SLAVE    = 3,
} AX_SNS_MASTER_SLAVE_E;

typedef struct _AX_SNS_CAP_T_ {
    AX_U32 nSnsMode_caps;
    AX_U32 nSnsRawType_caps;
    AX_U32 nSnsFps_caps;
    AX_U32 nSnsResolution_caps;
} AX_SNS_CAP_T;

typedef enum _AX_SNS_CLK_RATE_E_ {
    AX_SNS_CLK_NOT_CFG  = -1,
    AX_SNS_CLK_24M      = 24000000,     /* 24MHz, Default value of chip configuration */
    AX_SNS_CLK_27M      = 27000000,     /* 27MHz */
    AX_SNS_CLK_37_125M  = 37125000,     /* 37.125MHz */
    AX_SNS_CLK_74_25M   = 74250000,     /* 74.25MHz */
} AX_SNS_CLK_RATE_E;

typedef enum _AX_SNS_MIRRORFLIP_TYPE_E_ {
    AX_SNS_MF_NORMAL      = 0,
    AX_SNS_MF_MIRROR      = 1,
    AX_SNS_MF_FLIP        = 2,
    AX_SNS_MF_MIRROR_FLIP = 3,
    AX_SNS_MF_BUTT
} AX_SNS_MIRRORFLIP_TYPE_E;

/*
IIC/SPI connection is used between sensor and ISP
*/
typedef enum _AX_SNS_CONNECT_TYPE_E_ {
    ISP_SNS_CONNECT_I2C_TYPE = 0,
    ISP_SNS_CONNECT_SPI_TYPE,
    ISP_SNS_CONNECT_TYPE_BUTT,
} AX_SNS_CONNECT_TYPE_E;

typedef enum _AX_SNS_SLEEP_WAKEUP_E_ {
    AX_SNS_EVENT_SLEEP          = 0,
    AX_SNS_EVENT_WAKE_UP        = 1,
    AX_SNS_EVENT_BUTT
} AX_SNS_SLEEP_WAKEUP_E;

typedef enum _AX_SNS_STREAM_CTRL_E_ {
    AX_SNS_STREAM_CTRL_NOTINIT  = 0,
    AX_SNS_STREAM_CTRL_REG      = 1,
    AX_SNS_STREAM_CTRL_GPIO     = 2,
    AX_SNS_STREAM_CTRL_BOTH     = 3,
    AX_SNS_STREAM_CTRL_BUTT
} AX_SNS_STREAM_CTRL_E;

typedef struct _AX_SNS_ATTR_T_ {
    AX_U32                  nWidth;
    AX_U32                  nHeight;
    AX_F32                  fFrameRate;
    AX_SNS_HDR_MODE_E       eSnsMode;
    AX_RAW_TYPE_E           eRawType;
    AX_BAYER_PATTERN_E      eBayerPattern;
    AX_BOOL                 bTestPatternEnable;     /* Active Sensor Test Pattern */
    AX_SNS_MASTER_SLAVE_E   eMasterSlaveSel;
    AX_U32                  nSettingIndex;          /* optional, Not Recommended. if nSettingIndex > 0 will take effect */
    AX_SNS_OUTPUT_MODE_E    eSnsOutputMode;
} AX_SNS_ATTR_T;

typedef struct _AX_SNS_AE_LFHDR_ATTR_T_ {
    AX_ISP_LFHDR_MODE_E     eLFHdrMode;
} AX_SNS_AE_LFHDR_ATTR_T;

typedef struct _AX_SNS_AE_GAIN_TABLE_T_ {
    AX_S32  nAgainTableSize;
    AX_F32 *pAgainTable;
    AX_S32  nDgainTableSize;
    AX_F32 *pDgainTable;
} AX_SNS_AE_GAIN_TABLE_T;


typedef struct _AX_SNS_AE_SLOW_SHUTTER_TBL_T_ {
    AX_F32      fMaxIntTime;        /* unit is line */
    AX_F32      fRealFps;
} AX_SNS_AE_SLOW_SHUTTER_TBL_T;

typedef struct _AX_SNS_AE_SLOW_SHUTTER_PARAM_T_ {
    AX_SNS_AE_SLOW_SHUTTER_TBL_T    tSlowShutterTbl[SNS_MAX_FRAME_RATE];
    AX_F32                          fMaxFps;
    AX_F32                          fMinFps;
    AX_S32                          nGroupNum;
} AX_SNS_AE_SLOW_SHUTTER_PARAM_T;

typedef struct _AX_SNS_AE_INT_TIME_RANGE_T_ {
    AX_F32 fMinIntegrationTime[HDR_MAX_FRAME_NUM];     /* minimum:0-long frame IntegrationTime, 1- medium frame IntegrationTime, 2- short frame IntegrationTime, 3- ; unit is line */
    AX_F32 fMaxIntegrationTime[HDR_MAX_FRAME_NUM];     /* max mum:0-long frame IntegrationTime, 1- medium frame IntegrationTime, 2- short frame IntegrationTime, 3- ; unit is line */
} AX_SNS_AE_INT_TIME_RANGE_T;

typedef struct _AX_SNS_AE_LIMIT_T_ {
    AX_F32                      fMinAgain[HDR_MAX_FRAME_NUM];    /* minimum:0-long analog gain, 1- medium analog gain, 2- short analog gain */
    AX_F32                      fMaxAgain[HDR_MAX_FRAME_NUM];    /* max mum:0-long analog gain, 1- medium analog gain, 2- short analog gain */

    AX_F32                      fMinDgain[HDR_MAX_FRAME_NUM];    /* minimum:0-long digital gain, 1- medium digital gain, 2- short digital gain */
    AX_F32                      fMaxDgain[HDR_MAX_FRAME_NUM];    /* max mum:0-long digital gain, 1- medium digital gain, 2- short digital gain */

    AX_F32                      fMinRatio;
    AX_F32                      fMaxRatio;
    AX_SNS_AE_INT_TIME_RANGE_T  tIntTimeRange;
} AX_SNS_AE_LIMIT_T;

typedef struct _AX_SNS_AE_PARAM_T_ {
    /* current ae param */
    AX_F32 fCurAGain[HDR_MAX_FRAME_NUM];                /* current analog gain: 0-long frame, 1- medium frame, 2- short frame  */
    AX_F32 fCurDGain[HDR_MAX_FRAME_NUM];                /* current digital gain: 0-long frame, 1- medium frame, 2- short frame  */
    AX_F32 fCurIspDGain[HDR_MAX_FRAME_NUM];             /* current isp digital gain: 0-long frame, 1- medium frame, 2- short frame  */
    AX_F32 fCurIntegrationTime[HDR_MAX_FRAME_NUM];      /* current Integration time: 0-long frame , 1- medium frame, 2- short frame  ; unit is line */

    /* ae step param */
    AX_F32 fAGainIncrement[HDR_MAX_FRAME_NUM];          /* analog gain step: 0-long frame, 1- medium frame, 2- short frame */
    AX_F32 fDGainIncrement[HDR_MAX_FRAME_NUM];          /* digital gain step: 0-long frame, 1- medium frame, 2- short frame */
    AX_F32 fIspDGainIncrement[HDR_MAX_FRAME_NUM];       /* isp digital gain step: 0-long frame, 1- medium frame, 2- short frame */
    AX_F32 fIntegrationTimeIncrement[HDR_MAX_FRAME_NUM];  /* Integration time step: 0-long frame, 1- medium frame, 2- short frame ; unit is line */

    AX_F32 fCurFps;
    AX_F32 fCurRatio;

    AX_F32 fIntegrationTimeOffset[HDR_MAX_FRAME_NUM];   /* Integration time offset unit:line */
} AX_SNS_AE_PARAM_T;

typedef struct _AX_SNS_AE_GAIN_CFG_T_ {
    /* gain: 0-long frame, 1- medium frame, 2- short frame, 3-very short frame */
    AX_F32 fGain[HDR_MAX_FRAME_NUM];
} AX_SNS_AE_GAIN_CFG_T;

typedef struct _AX_SNS_AE_SHUTTER_CFG_T_ {
    /* integration time: 0-long frame, 1- medium frame, 2- short frame, 3-very short frame ; unit is line */
    AX_F32 fIntTime[HDR_MAX_FRAME_NUM];

    /* integration time ratio: index=0: longFrame/mediumFrame, index=1: mediumFrame/shortFrame, index=2: shortFrame/veryShortFrame, index=3:Not used */
    AX_F32 fHdrRatio[HDR_MAX_FRAME_NUM];
} AX_SNS_AE_SHUTTER_CFG_T;

typedef struct _AX_SNS_EXP_CTRL_PARAM_T_ {
    AX_SNS_ATTR_T           sns_dev_attr;
    AX_SNS_AE_LIMIT_T       sns_ae_limit;
    AX_SNS_AE_PARAM_T       sns_ae_param;

    AX_SNS_HCGLCG_MODE_E    eSnsHcgLcgMode;             /* supported hcg/lcg mode  AX_SNS_HCGLCG_MODE_E */
    AX_F32                  fSnsHcgLcgRatio;

    AX_F32                  fTimePerLine;
    AX_F32                  fLineTime;
    AX_F32                  fInitIntegrationTime;       /* Initial value ; unit is line */
    AX_F32                  fInitAGain;                 /* Initial analog gain */
    AX_F32                  fInitDGain;                 /* Initial digital gain */
} AX_SNS_EXP_CTRL_PARAM_T;

typedef struct _AX_SNS_BLACK_LEVEL_T_ {
    AX_U16  nBlackLevel[AX_ISP_BAYER_CHN_NUM];
} AX_SNS_BLACK_LEVEL_T;

typedef struct _AX_SNS_COMMBUS_T_ {
    union {
    AX_S8   I2cDev;
    struct {
        AX_S8  bit4SpiDev       : 4;
        AX_S8  bit4SpiCs        : 4;
    } SpiDev;
    };
    AX_SNS_CONNECT_TYPE_E  busType;
    AX_U32   nPwdnGpio;
} AX_SNS_COMMBUS_T;

typedef struct _AX_SNS_ISP_I2C_DATA_T_ {
    AX_BOOL bUpdate;            /* AX_TRUE: The sensor registers are written, AX_FALSE: The sensor registers are not written*/
    AX_U8   nDelayFrmNum;       /* Number of frames for register delay configuration */
    AX_U8   nDevAddr;           /* sensor device address */
    AX_U8   nIntPos;            /* Position of the register takes effect, only support AX_SNS_L_FSOF/AX_SNS_S_FSOF */
    AX_U8   reserve;
    AX_U32  nRegAddr;           /* register address */
    AX_U32  nAddrByteNum;       /* Bit width of the register address */
    AX_U32  nData;              /* Sensor register data */
    AX_U32  nDataByteNum;       /* Bit width of sensor register data */
} AX_SNS_ISP_I2C_DATA_T;

typedef struct _AX_SNS_ISP_SPI_DATA_T_ {
    AX_BOOL bUpdate;
    AX_U8   nDelayFrmNum;       /* Number of frames for register delay configuration */
    AX_U8   nIntPos;            /* Position of the register takes effect */
    AX_U8   reserve[2];
    AX_U32  nDevAddr;           /* Sensor device address */
    AX_U32  nDevAddrByteNum;    /* Bit width of the sensor device address */
    AX_U32  nRegAddr;           /* Sensor register address */
    AX_U32  nRegAddrByteNum;    /* Bit width of the sensor register address */
    AX_U32  nData;              /* Sensor register data */
    AX_U32  nDataByteNum;       /* Bit width of sensor register data */
} AX_SNS_ISP_SPI_DATA_T;

typedef struct _AX_SNS_ISP_EXP_INFO_T_ {
    AX_U32      szExpTime[HDR_MAX_FRAME_NUM];   /* unit: line */
    AX_F32      szCurAGain[HDR_MAX_FRAME_NUM];
    AX_F32      szCurDGain[HDR_MAX_FRAME_NUM];
    AX_U32      szLineGap[HDR_MAX_FRAME_NUM];   /* index0: long frame and medium frame line gap, exposure in unit of line */
} AX_SNS_ISP_EXP_INFO_T;

typedef struct _AX_SNS_STREAM_REG_TABLE_T_ {
    AX_U32  nRegAddr;                       /* Stream Control Register address */
    AX_U16  nData;                          /* Stream register data */
    AX_U16  nDelayUs;                       /* delay after register write [us] */
} AX_SNS_STREAM_REG_TABLE_T;

typedef struct _AX_SNS_STREAM_CTRL_T_ {
    AX_U8   bConfig;
    AX_U8   nDevAddr;                       /* Device address */
    AX_U8   nAddrByteNum;                   /* Bit width of the register address */
    AX_U8   nDataByteNum;                   /* Bit width of sensor register data */
    AX_U8   nDataOnNum;                     /* Num of sensor on register data */
    AX_U8   nDataOffNum;                    /* Num of sensor off register data */
    AX_U16  nDelayMclkUs;                   /* delay after MCLK [us] */
    AX_U16  nDelayPwdnUs;                   /* delay after pwdn, before register write [us] */
    AX_U32  nPwdnbGpio;                     /* Pwdnb gpio num */
    AX_SNS_STREAM_REG_TABLE_T tOn[SNS_STREAM_REG_NUM];
    AX_SNS_STREAM_REG_TABLE_T tOff[SNS_STREAM_REG_NUM];
    AX_SNS_STREAM_CTRL_E eCtrl;             /* Which way for stream control */
} AX_SNS_STREAM_CTRL_T;

typedef struct _AX_SNS_REGS_CFG_TABLE_T_ {
    AX_BOOL                 bConfig;
    AX_SNS_CONNECT_TYPE_E   eSnsType;
    AX_BOOL                 bQuickEffectEn;

    /*Number of registers requiring timing control. The member value cannot be dynamically changed*/
    AX_U8                   nRegNum;
    /* Maximum number of delayed frames from the time when all sensor registers are configured to the
       time when configurations take effect. Used when ispgain is synchronized with sensor exp registers */
    AX_U8                   nCfg2ValidDelayMax;
    /* Number of frames that need to be dropped due to sensor abnormalities */
    AX_U8                   nDropFrame;
    AX_U8                   nReserve[1];
    AX_SNS_COMMBUS_T        tComBus;
    union {
        AX_SNS_ISP_I2C_DATA_T   sztI2cData[ISP_MAX_I2C_SNS_REGISTER_NUM];
        AX_SNS_ISP_SPI_DATA_T   sztSpiData[ISP_MAX_SPI_SNS_REGISTER_NUM];
    } sztData;
    AX_SNS_ISP_EXP_INFO_T   tSnsExpInfo;
    AX_SNS_STREAM_CTRL_T    tStreamCtrl;
} AX_SNS_REGS_CFG_TABLE_T;

typedef struct _AX_SNS_AWB_PARAM_T_ {
    AX_U32 nGrGain[HDR_MAX_FRAME_NUM];         /* = gain * 1024.0f, gain:[1.00f, 15.0f) */
    AX_U32 nGbGain[HDR_MAX_FRAME_NUM];         /* = gain * 1024.0f, gain:[1.00f, 15.0f) */
    AX_U32 nRgain[HDR_MAX_FRAME_NUM];          /* = gain * 1024.0f, gain:[1.00f, 15.0f) */
    AX_U32 nBgain[HDR_MAX_FRAME_NUM];          /* = gain * 1024.0f, gain:[1.00f, 15.0f) */
    AX_U32 nColorTemp[HDR_MAX_FRAME_NUM];      /* = Accuracy:U32.0, CCT Range:[1000, 20000]  Color Temperature */
} AX_SNS_AWB_PARAM_T;


typedef union _AX_SNS_FUNC_MASK_T_ {
    struct {
        AX_U32  bitSnsChipidMaskEn              : 1;   /* RW; [0] pfn_sensor_chipid */
        AX_U32  bitSnsResetMaskEn               : 1;   /* RW: [1] pfn_sensor_reset */
        AX_U32  bitSnsInitMaskEn                : 1;   /* RW; [2] pfn_sensor_init */
        AX_U32  bitSnsExitMaskEn                : 1;   /* RW; [3] pfn_sensor_exit */
        AX_U32  bitSnsStreamCtrlMaskEn          : 1;   /* RW; [4] pfn_sensor_streaming_ctrl */
        AX_U32  bitSnsSetModeMaskEn             : 1;   /* RW; [5] pfn_sensor_set_mode */
        AX_U32  bitSnsGetModeMaskEn             : 1;   /* RW; [6] pfn_sensor_get_mode */
        AX_U32  bitSnsTestpatternMaskEn         : 1;   /* RW; [7] pfn_sensor_testpattern */
        AX_U32  bitSnsSleepWakeupMaskEn         : 1;   /* RW; [8] pfn_sensor_sleep_wakeup */

        AX_U32  bitSnsSetFpsMaskEn              : 1;   /* RW; [9] pfn_sensor_set_fps */
        AX_U32  bitSnsGetFpsMaskEn              : 1;   /* RW; [10] pfn_sensor_get_fps */

        AX_U32  bitSnsWriteRegisterMaskEn       : 1;   /* RW; [11] pfn_sensor_write_register */
        AX_U32  bitSnsReadRegisterMaskEn        : 1;   /* RW; [12] pfn_sensor_read_register */

        AX_U32  bitSnsDefaultParamsMaskEn       : 1;   /* RW; [13] pfn_sensor_get_default_params */
        AX_U32  bitSnsBlackLevelMaskEn          : 1;   /* RW; [14] pfn_sensor_get_black_level */
        AX_U32  bitSnsGetRegInfoMaskEn          : 1;   /* RW; [15] pfn_sensor_get_sns_reg_info */

        AX_U32  bitSnsMaskEnRsv                 : 16;  /* RW; [16:31] */
    };
    AX_U32  u32Key;
} AX_SNS_FUNC_MASK_T;

typedef struct _AX_SENSOR_REGISTER_FUNC_T_ {
    /* sensor ctrl */
    AX_S32(*pfn_sensor_chipid)(ISP_PIPE_ID nPipeId, int *pSnsChipId);
    AX_S32(*pfn_sensor_reset)(ISP_PIPE_ID nPipeId, AX_U32 nResetGpio);
    AX_VOID(*pfn_sensor_init)(ISP_PIPE_ID nPipeId);
    AX_VOID(*pfn_sensor_exit)(ISP_PIPE_ID nPipeId);
    AX_S32(*pfn_sensor_streaming_ctrl)(ISP_PIPE_ID nPipeId, AX_U32 on);
    AX_S32(*pfn_sensor_set_slaveaddr)(ISP_PIPE_ID nPipeId, AX_U8 nSlaveAddr);

    AX_S32(*pfn_sensor_set_mode)(ISP_PIPE_ID nPipeId, AX_SNS_ATTR_T *ptSnsMode);
    AX_S32(*pfn_sensor_get_mode)(ISP_PIPE_ID nPipeId, AX_SNS_ATTR_T *ptSnsMode);
    AX_S32(*pfn_sensor_testpattern)(ISP_PIPE_ID nPipeId, AX_U32 nTestpatternEn);
    AX_S32(*pfn_sensor_mirror_flip)(ISP_PIPE_ID nPipeId, AX_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
    AX_S32(*pfn_sensor_sleep_wakeup)(ISP_PIPE_ID nPipeId, AX_SNS_SLEEP_WAKEUP_E eSleepWakeup);

    AX_S32(*pfn_sensor_set_fps)(ISP_PIPE_ID nPipeId, AX_F32 fFps);
    AX_S32(*pfn_sensor_get_fps)(ISP_PIPE_ID nPipeId, AX_F32 *pFps);

    /* sensor read/write */
    AX_S32(*pfn_sensor_set_bus_info)(ISP_PIPE_ID nPipeId, AX_SNS_COMMBUS_T tSnsBusInfo);
    AX_S32(*pfn_sensor_write_register)(ISP_PIPE_ID nPipeId, AX_U32 nAddr, AX_U32 nData);
    AX_S32(*pfn_sensor_read_register)(ISP_PIPE_ID nPipeId, AX_U32 nAddr, AX_U32 *pData);

    /* module default parameters */
    AX_S32(*pfn_sensor_get_isp_default_params)(ISP_PIPE_ID nPipeId, AX_SENSOR_DEFAULT_PARAM_T *ptDftParam);
    AX_S32(*pfn_sensor_get_3a_default_params)(ISP_PIPE_ID nPipeId, AX_SENSOR_3A_DEFAULT_PARAM_T *ptDftParam);
    AX_S32(*pfn_sensor_get_black_level)(ISP_PIPE_ID nPipeId, AX_SNS_BLACK_LEVEL_T *ptBlackLevel);

    /* ae ctrl */
    AX_S32(*pfn_sensor_get_hw_exposure_params)(ISP_PIPE_ID nPipeId, AX_SNS_EXP_CTRL_PARAM_T *ptAeCtrlParam);

    AX_S32(*pfn_sensor_get_gain_table)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_TABLE_T *ptSnsGainTbl);
    AX_S32(*pfn_sensor_set_again)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptAGain);
    AX_S32(*pfn_sensor_set_dgain)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptDGain);
    AX_S32(*pfn_sensor_hcglcg_ctrl)(ISP_PIPE_ID nPipeId, AX_U32 nSnsHcgLcg);

    AX_S32(*pfn_sensor_set_integration_time)(ISP_PIPE_ID nPipeId, AX_SNS_AE_SHUTTER_CFG_T *ptIntTime);
    AX_S32(*pfn_sensor_get_integration_time_range)(ISP_PIPE_ID nPipeId, AX_F32 fHdrRatio, AX_SNS_AE_INT_TIME_RANGE_T *ptIntTimeRange);
    AX_S32(*pfn_sensor_set_slow_fps)(ISP_PIPE_ID nPipeId, AX_F32 fFps);
    AX_S32(*pfn_sensor_get_slow_shutter_param)(ISP_PIPE_ID nPipeId, AX_SNS_AE_SLOW_SHUTTER_PARAM_T *ptSlowShutterParam);

    AX_S32(*pfn_sensor_get_sns_reg_info)(ISP_PIPE_ID nPipeId, AX_SNS_REGS_CFG_TABLE_T *ptSnsRegsInfo);
    AX_S32(*pfn_sensor_set_lfhdr_mode)(ISP_PIPE_ID nPipeId, AX_SNS_AE_LFHDR_ATTR_T *ptLFHdrAttr);

    AX_S32(*pfn_sensor_get_temperature_info)(ISP_PIPE_ID nPipeId, AX_S32 *nTemperature);

    /* awb ctrl */
    AX_S32(*pfn_sensor_set_wbgain)(ISP_PIPE_ID nPipeId, AX_SNS_AWB_PARAM_T *ptAWBresult);
    AX_S32(*pfn_sensor_get_wbgain)(ISP_PIPE_ID nPipeId, AX_SNS_AWB_PARAM_T *ptAWBresult);
} AX_SENSOR_REGISTER_FUNC_T;

#endif //_AX_SENSOR_STRUCT_H_

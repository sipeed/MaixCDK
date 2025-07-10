/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _COMMON_CONFIG_H__
#define _COMMON_CONFIG_H__

#include "ax_global_type.h"
#include "ax_vin_api.h"
#include "ax_mipi_rx_api.h"

AX_MIPI_RX_ATTR_T gDummyMipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_4,
    .nDataRate =  400,
    .nDataLaneMap[0] = 0,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 3,
    .nDataLaneMap[3] = 4,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gDummySnsAttr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .fFrameRate = 30.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,                //12->16
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gDummySnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_NOT_CFG,
};

AX_VIN_DEV_ATTR_T gDummyDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_TPG,
    .tDevImgRgn[0] = {0, 0, 2688, 1520},
    .tDevImgRgn[1] = {0, 0, 2688, 1520},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl = {1.0, 1.0},
};

AX_VIN_PIPE_ATTR_T gDummyPipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 2688, 1520},
    .nWidthStride = 2688,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt =  AX_FORMAT_BAYER_RAW_12BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gDummyChn0Attr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .nWidthStride = 2688,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_SNS_ATTR_T gDVPSnsAttr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .fFrameRate = 15.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gDVPSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gDVPDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_DVP,
    .tDevImgRgn[0] = {0, 0, 2560, 1440},
    .tDevImgRgn[1] = {0, 0, 2560, 1440},
    .tDevImgRgn[2] = {0, 0, 2560, 1440},
    .tDevImgRgn[3] = {0, 0, 2560, 1440},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    /* dvp param cfg */
    .tDvpIntfAttr.eLaneMapMode = AX_VIN_DVP_DATA_PIN_START_0,
    .tDvpIntfAttr.tSyncCfg.eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tDvpIntfAttr.tSyncCfg.eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tDvpIntfAttr.tSyncCfg.eHsyncSignalMode = AX_VIN_SYNC_SIGNAL_MODE_HIGH_LEVEL,
    .tDvpIntfAttr.nDvpHsyncMux = 0x08,
    .tDvpIntfAttr.nDvpVsyncMux = 0x09,
    .tDvpIntfAttr.nDvpDataMux = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                 0x07, 0x10, 0x11, 0x12, 0x13, -1, -1},
};

AX_VIN_PIPE_ATTR_T gDVPPipeAttr = {
    .ePipeWorkMode = 0x1,
    .tPipeImgRgn = {0, 0, 2560, 1440},
    .nWidthStride = 2560,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gDVPChn0Attr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .nWidthStride = 2560,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};


AX_SNS_ATTR_T gDVPIRSnsAttr = {
    .nWidth = 256,
    .nHeight = 192,
    .fFrameRate = 50.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW16,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gDVPIRSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gDVPIRDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_DVP,
    .tDevImgRgn[0] = {0, 0, 256, 192},
    .tDevImgRgn[1] = {0, 0, 256, 192},
    .tDevImgRgn[2] = {0, 0, 256, 192},
    .tDevImgRgn[3] = {0, 0, 256, 192},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_16BPP,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    // .nConvYuv422To420En = 1,
    // .nConvFactor = 2,

    /* dvp special param cfg */
    .tDvpIntfAttr.nShiftBit = 0,
    .tDvpIntfAttr.nMsbShiftBit = 0,
    .tDvpIntfAttr.nLsbShiftBit = 0,
    .tDvpIntfAttr.eLsbMsbCfg = AX_VIN_DVP_MSB_MODE,
    .tDvpIntfAttr.eCycleMode = AX_VIN_DVP_CYCLE_MODE_DUAL,
    .tDvpIntfAttr.bGrayCodeEnable = 0,
    /* dvp param cfg */
    .tDvpIntfAttr.eLaneMapMode = AX_VIN_DVP_DATA_PIN_START_0,
    .tDvpIntfAttr.tSyncCfg.eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tDvpIntfAttr.tSyncCfg.eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tDvpIntfAttr.tSyncCfg.eHsyncSignalMode = AX_VIN_SYNC_SIGNAL_MODE_HIGH_LEVEL,
    .tDvpIntfAttr.nDvpHsyncMux = 0x08,
    .tDvpIntfAttr.nDvpVsyncMux = 0x09,
    .tDvpIntfAttr.nDvpDataMux = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                 0x07, -1, -1, -1, -1, -1, -1},
};

AX_VIN_PIPE_ATTR_T gDVPIRPipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_ISP_BYPASS_MODE,
    .tPipeImgRgn = {0, 0, 256, 192},
    .nWidthStride = 256,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_16BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gDVPIRChn0Attr = {
    .nWidth = 256,
    .nHeight = 192,
    .nWidthStride = 256,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 3,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_SNS_ATTR_T gBT601SnsAttr = {
    .nWidth = 720,
    .nHeight = 480,
    .fFrameRate = 30.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_YUV422,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gBT601SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_27M,
};

AX_VIN_DEV_ATTR_T gBT601DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_BT601,
    .tDevImgRgn[0] = {0, 0, 720, 480},
    .tDevImgRgn[1] = {0, 0, 720, 480},
    .tDevImgRgn[2] = {0, 0, 720, 480},
    .tDevImgRgn[3] = {0, 0, 720, 480},
    .ePixelFmt = AX_FORMAT_YUV422_INTERLEAVED_UYVY,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .nConvYuv422To420En = 1,
    .nConvFactor = 2,

    /* bt param cfg */
    .tBtIntfAttr.eDataChnSel = AX_VIN_INTF_DATA_MODE_SINGLE_CHN,
    .tBtIntfAttr.eSyncCtrlMode = AX_VIN_BT_CTRL_VSYNC_HSYNC, //internal sync
    .tBtIntfAttr.tSyncCfg.eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.tSyncCfg.eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.nDataActiveMode = 1,
    .tBtIntfAttr.eSyncHeaderType = AX_VIN_BT_SYNC_HEADER_8_BYTE,
    .tBtIntfAttr.nYcOrder = 1,

    /* bt delay cfg [only external sync need cfg]*/
    .tBtIntfAttr.tTimingBlank.nHsyncHfb =  0,
    .tBtIntfAttr.tTimingBlank.nHsyncHact =  0,
    .tBtIntfAttr.tTimingBlank.nHsyncHbb =  0xF4,
    .tBtIntfAttr.tTimingBlank.nVsyncVfb =  0x04,
    .tBtIntfAttr.tTimingBlank.nVsyncVact =  0,
    .tBtIntfAttr.tTimingBlank.nVsyncVbb =  0x10,

    /* bt sync code cfg [only internal sync need cfg]*/
    // .tBtIntfAttr.tBtSyncCode[0].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[0].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[0].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[0].nVldSav = 0X80,

    /* bt sync code [others bt chn]cfg [bt tdm mode need cfg]*/
    // .tBtIntfAttr.tBtSyncCode[1].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSav = 0X80,

     /* bt sync mux cfg  [only external sync need cfg]*/
    .tBtIntfAttr.nBtHsyncMux = 0x01,
    .tBtIntfAttr.nBtVsyncMux = 0x00,

    .tBtIntfAttr.nBtDataMux = {0x08, 0x09, 0x02, 0x03, 0x04, 0x05, 0x06,
                                 0x07, 0x0a, 0x0b, -1, -1, -1, -1},
    /* bt tdm mode chn cfg*/
    .tBtIntfAttr.eBtTdmMode = AX_VIN_BT_NORMAL_MODE,
};

AX_VIN_PIPE_ATTR_T gBT601PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_SUB_YUV_MODE,
    .tPipeImgRgn = {0, 0, 720, 480},
    .nWidthStride = 720,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_YUV422_SEMIPLANAR,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gBT601Chn0Attr = {
    .nWidth = 720,
    .nHeight = 480,
    .nWidthStride = 720,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 3,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_SNS_ATTR_T gBT656SnsAttr = {
    .nWidth = 720,
    .nHeight = 480,
    .fFrameRate = 30.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_YUV422,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gBT656SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_27M,
};

AX_VIN_DEV_ATTR_T gBT656DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_BT656,
    .tDevImgRgn[0] = {0, 0, 720, 480},
    .tDevImgRgn[1] = {0, 0, 720, 480},
    .tDevImgRgn[2] = {0, 0, 720, 480},
    .tDevImgRgn[3] = {0, 0, 720, 480},
    .ePixelFmt = AX_FORMAT_YUV422_INTERLEAVED_UYVY,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .nConvYuv422To420En = 1,
    .nConvFactor = 2,

    /* bt param cfg */
    .tBtIntfAttr.eDataChnSel = AX_VIN_INTF_DATA_MODE_SINGLE_CHN,
    .tBtIntfAttr.eSyncCtrlMode = AX_VIN_BT_CTRL_SYNC_CODE, //internal sync
    .tBtIntfAttr.tSyncCfg.eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.tSyncCfg.eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.nDataActiveMode = 1,
    .tBtIntfAttr.eSyncHeaderType = AX_VIN_BT_SYNC_HEADER_8_BYTE,
    .tBtIntfAttr.nYcOrder = 1,

    /* bt delay cfg [only external sync need cfg]*/
    // .tBtIntfAttr.tTimingBlank.nHsyncHfb =  0,
    // .tBtIntfAttr.tTimingBlank.nHsyncHact =  0,
    // .tBtIntfAttr.tTimingBlank.nHsyncHbb =  0xF4,
    // .tBtIntfAttr.tTimingBlank.nVsyncVfb =  0x04,
    // .tBtIntfAttr.tTimingBlank.nVsyncVact =  0,
    // .tBtIntfAttr.tTimingBlank.nVsyncVbb =  0x10,

    /* bt sync code cfg [only internal sync need cfg]*/
    .tBtIntfAttr.tBtSyncCode[0].nInvSavMask = 0xFF,
    .tBtIntfAttr.tBtSyncCode[0].nVldSavMask = 0xFF,
    .tBtIntfAttr.tBtSyncCode[0].nInvSav = 0xAB,
    .tBtIntfAttr.tBtSyncCode[0].nVldSav = 0X80,

    /* bt sync code [others bt chn]cfg [bt tdm mode need cfg]*/
    // .tBtIntfAttr.tBtSyncCode[1].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSav = 0X80,

     /* bt sync mux cfg  [only external sync need cfg]*/
    // .tBtIntfAttr.nBtHsyncMux = 0x01,
    // .tBtIntfAttr.nBtVsyncMux = 0x00,

    .tBtIntfAttr.nBtDataMux = {0x08, 0x09, 0x02, 0x03, 0x04, 0x05, 0x06,
                                 0x07, 0x0a, 0x0b, -1, -1, -1, -1},
    /* bt tdm mode chn cfg*/
    .tBtIntfAttr.eBtTdmMode = AX_VIN_BT_NORMAL_MODE,
};

AX_VIN_PIPE_ATTR_T gBT656PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_SUB_YUV_MODE,
    .tPipeImgRgn = {0, 0, 720, 480},
    .nWidthStride = 720,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_YUV422_SEMIPLANAR,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gBT656Chn0Attr = {
    .nWidth = 720,
    .nHeight = 480,
    .nWidthStride = 720,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 3,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_SNS_ATTR_T gBT1120SnsAttr = {
    .nWidth = 720,
    .nHeight = 480,
    .fFrameRate = 30.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_YUV422,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gBT1120SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_27M,
};

AX_VIN_DEV_ATTR_T gBT1120DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_BT1120,
    .tDevImgRgn[0] = {0, 0, 720, 480},
    .tDevImgRgn[1] = {0, 0, 720, 480},
    .tDevImgRgn[2] = {0, 0, 720, 480},
    .tDevImgRgn[3] = {0, 0, 720, 480},
    .ePixelFmt = AX_FORMAT_YUV422_INTERLEAVED_UYVY,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .nConvYuv422To420En = 1,
    .nConvFactor = 2,

    /* bt param cfg */
    .tBtIntfAttr.eDataChnSel = AX_VIN_INTF_DATA_MODE_DUAL_CHN,
    .tBtIntfAttr.eSyncCtrlMode = AX_VIN_BT_CTRL_SYNC_CODE, //internal sync
    .tBtIntfAttr.tSyncCfg.eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.tSyncCfg.eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
    .tBtIntfAttr.nDataActiveMode = 1,
    .tBtIntfAttr.eSyncHeaderType = AX_VIN_BT_SYNC_HEADER_4_BYTE,
    .tBtIntfAttr.nYcOrder = 0,

    /* bt delay cfg [only external sync need cfg]*/
    // .tBtIntfAttr.tTimingBlank.nHsyncHfb =  0,
    // .tBtIntfAttr.tTimingBlank.nHsyncHact =  0,
    // .tBtIntfAttr.tTimingBlank.nHsyncHbb =  0xF4,
    // .tBtIntfAttr.tTimingBlank.nVsyncVfb =  0x04,
    // .tBtIntfAttr.tTimingBlank.nVsyncVact =  0,
    // .tBtIntfAttr.tTimingBlank.nVsyncVbb =  0x10,

    /* bt sync code cfg [only internal sync need cfg]*/
    .tBtIntfAttr.tBtSyncCode[0].nInvSavMask = 0xFF,
    .tBtIntfAttr.tBtSyncCode[0].nVldSavMask = 0xFF,
    .tBtIntfAttr.tBtSyncCode[0].nInvSav = 0xAB,
    .tBtIntfAttr.tBtSyncCode[0].nVldSav = 0X80,

    /* bt sync code [others bt chn]cfg [bt tdm mode need cfg]*/
    // .tBtIntfAttr.tBtSyncCode[1].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[1].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[1].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[2].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[2].nVldSav = 0X80,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSavMask = 0xFF,
    // .tBtIntfAttr.tBtSyncCode[3].nInvSav = 0xAB,
    // .tBtIntfAttr.tBtSyncCode[3].nVldSav = 0X80,

     /* bt sync mux cfg  [only external sync need cfg]*/
    // .tBtIntfAttr.nBtHsyncMux = 0x01,
    // .tBtIntfAttr.nBtVsyncMux = 0x00,

    .tBtIntfAttr.nBtDataMux = {0x08, 0x09, 0x02, 0x03, 0x04, 0x05, 0x06,
                                 0x07, -1, 0-1, 0x12, 0x13, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11},
    /* bt tdm mode chn cfg*/
    .tBtIntfAttr.eBtTdmMode = AX_VIN_BT_NORMAL_MODE,
};

AX_VIN_PIPE_ATTR_T gBT1120PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_ISP_BYPASS_MODE,
    .tPipeImgRgn = {0, 0, 720, 480},
    .nWidthStride = 720,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_YUV422_SEMIPLANAR,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gBT1120Chn0Attr = {
    .nWidth = 720,
    .nHeight = 480,
    .nWidthStride = 720,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 3,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_LVDS_ATTR_T gLvdsAttr = {
    .eLaneNum = AX_SLVDS_DATA_LANE_4,
    .nDataRate = 900,
    .nDataLaneMap[0] = 0,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 3,
    .nDataLaneMap[3] = 4,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gLvdsSnsAttr = {
    .nWidth = 1600,
    .nHeight = 1200,
    .fFrameRate = 11.0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
};

AX_SNS_CLK_ATTR_T gLvdsSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gLvdsDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 1600, 1200},
    .tDevImgRgn[1] = {0, 0, 1600, 1200},
    .tDevImgRgn[2] = {0, 0, 1600, 1200},
    .tDevImgRgn[3] = {0, 0, 1600, 1200},
    .tMipiIntfAttr.szImgVc[0] = 0,
    .tMipiIntfAttr.szImgDt[0] = 0x2b,
    .tMipiIntfAttr.szInfoVc[0] = 0x1f,
    .tMipiIntfAttr.szInfoDt[0] = 0x3f,
    .tMipiIntfAttr.szImgVc[1] = 1,
    .tMipiIntfAttr.szImgDt[1] = 0x2b,
    .tMipiIntfAttr.szInfoVc[1] = 0x1f,
    .tMipiIntfAttr.szInfoDt[1] = 0x3f,
    .tMipiIntfAttr.szImgVc[2] = 2,
    .tMipiIntfAttr.szImgDt[2] = 0x2b,
    .tMipiIntfAttr.szInfoVc[2] = 0x1f,
    .tMipiIntfAttr.szInfoDt[2] = 0x3f,
    .tMipiIntfAttr.szImgVc[3] = 3,
    .tMipiIntfAttr.szImgDt[3] = 0x2b,
    .tMipiIntfAttr.szInfoVc[3] = 0x1f,
    .tMipiIntfAttr.szInfoDt[3] = 0x3f,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
};

AX_VIN_PIPE_ATTR_T gLvdsPipeAttr = {
    .tPipeImgRgn = {0, 0, 1600, 1200},
    .nWidthStride = 1600,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_16BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

AX_VIN_CHN_ATTR_T gLvdsChn0Attr = {
    .nWidth = 1600,
    .nHeight = 1200,
    .nWidthStride = 1600,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 3,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
};

// ### SIPEED EDIT ###
AX_MIPI_RX_ATTR_T gOs04a10MipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_4,
    .nDataRate =  80,
    .nDataLaneMap[0] = 3,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 4,
    .nDataLaneMap[3] = 0,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};
// ### SIPEED EDIT END ###

AX_SNS_ATTR_T gOs04a10SnsAttr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .fFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    // .nSettingIndex = 12,
};

AX_SNS_CLK_ATTR_T gOs04a10SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gOs04a10DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 2688, 1520},
    .tDevImgRgn[1] = {0, 0, 2688, 1520},
    .tDevImgRgn[2] = {0, 0, 2688, 1520},
    .tDevImgRgn[3] = {0, 0, 2688, 1520},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    //.tMipiIntfAttr.szImgVc[0] = 0,
    //.tMipiIntfAttr.szImgDt[0] = MIPI_CSI_DT_RAW10,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gOs04a10PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 2688, 1520},
    .nWidthStride = 2688,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gOs04a10Chn0Attr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .nWidthStride = 2688,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

// ### SIPEED EDIT ###
AX_MIPI_RX_ATTR_T gOs04d10MipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_2,
    .nDataRate = 720,
    .nDataLaneMap[0] = 2,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = -1,
    .nDataLaneMap[3] = -1,
    .nClkLane[0]     = 0,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gOs04d10SnsAttr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .fFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    // .nSettingIndex = 12,
};

AX_SNS_CLK_ATTR_T gOs04d10SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gOs04d10DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 2560, 1440},
    .tDevImgRgn[1] = {0, 0, 2560, 1440},
    .tDevImgRgn[2] = {0, 0, 2560, 1440},
    .tDevImgRgn[3] = {0, 0, 2560, 1440},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    //.tMipiIntfAttr.szImgVc[0] = 0,
    //.tMipiIntfAttr.szImgDt[0] = MIPI_CSI_DT_RAW10,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gOs04d10PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 2560, 1440},
    .nWidthStride = 2560,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gOs04d10Chn0Attr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .nWidthStride = 2560,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_MIPI_RX_ATTR_T gSc450aiMipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_4,
    .nDataRate =  80,
    .nDataLaneMap[0] = 0,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 3,
    .nDataLaneMap[3] = 4,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gSc450aiSnsAttr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .fFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    // .nSettingIndex = 12,
};

AX_SNS_CLK_ATTR_T gSc450aiSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gSc450aiDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 2688, 1520},
    .tDevImgRgn[1] = {0, 0, 2688, 1520},
    .tDevImgRgn[2] = {0, 0, 2688, 1520},
    .tDevImgRgn[3] = {0, 0, 2688, 1520},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    //.tMipiIntfAttr.szImgVc[0] = 0,
    //.tMipiIntfAttr.szImgDt[0] = MIPI_CSI_DT_RAW10,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gSc450aiPipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 2688, 1520},
    .nWidthStride = 2688,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gSc450aiChn0Attr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .nWidthStride = 2688,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

// ### SIPEED EDIT ###
AX_MIPI_RX_ATTR_T gSc850slMipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_4,
    .nDataRate =  1080,
    .nDataLaneMap[0] = 3,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 4,
    .nDataLaneMap[3] = 0,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gSc850slSnsAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .fFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    // .nSettingIndex = 12,
};

AX_SNS_CLK_ATTR_T gSc850slSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gSc850slDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 3840, 2160},
    .tDevImgRgn[1] = {0, 0, 3840, 2160},
    .tDevImgRgn[2] = {0, 0, 3840, 2160},
    .tDevImgRgn[3] = {0, 0, 3840, 2160},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    //.tMipiIntfAttr.szImgVc[0] = 0,
    //.tMipiIntfAttr.szImgDt[0] = MIPI_CSI_DT_RAW10,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gSc850slPipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 3840, 2160},
    .nWidthStride = 3840,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gSc850slChn0Attr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .nWidthStride = 3840,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};
// ### SIPEED EDIT END ###

AX_MIPI_RX_ATTR_T gs5kjn1sq03MipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_4,
    .nDataRate =  80,
    .nDataLaneMap[0] = 0,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 3,
    .nDataLaneMap[3] = 4,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gs5kjn1sq03SnsAttr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .fFrameRate = 60,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .nSettingIndex = 1,
};

AX_SNS_CLK_ATTR_T gs5kjn1sq03SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gs5kjn1sq03DevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_TRUE,
    .nNonImgDataSize = 321600, /*480 * 536 * 10 / 8*/
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 1280, 1080},
    .tDevImgRgn[1] = {0, 0, 1280, 1080},
    .tDevImgRgn[2] = {0, 0, 1280, 1080},
    .tDevImgRgn[3] = {0, 0, 1280, 1080},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    .tMipiIntfAttr.szInfoVc[0] = 1,
    .tMipiIntfAttr.szInfoDt[0] = 0x30,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gs5kjn1sq03PipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 1280, 1080},
    .nWidthStride = 1280,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gs5kjn1sq03Chn0Attr = {
    .nWidth = 1280,
    .nHeight = 1080,
    .nWidthStride = 1280,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_MIPI_RX_ATTR_T gSc200aiMipiAttr = {
    .ePhyMode = AX_MIPI_PHY_TYPE_DPHY,
    .eLaneNum = AX_MIPI_DATA_LANE_2,
    .nDataRate =  80,
    .nDataLaneMap[0] = 0,
    .nDataLaneMap[1] = 1,
    .nDataLaneMap[2] = 3,
    .nDataLaneMap[3] = 4,
    .nClkLane[0]     = 2,
    .nClkLane[1]     = 5,
};

AX_SNS_ATTR_T gSc200aiSnsAttr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .fFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    // .nSettingIndex = 12,
};

AX_SNS_CLK_ATTR_T gSc200aiSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_VIN_DEV_ATTR_T gSc200aiDevAttr = {
    .bImgDataEnable = AX_TRUE,
    .bNonImgDataEnable = AX_FALSE,
    .eDevMode = AX_VIN_DEV_ONLINE,
    .eSnsIntfType = AX_SNS_INTF_TYPE_MIPI_RAW,
    .tDevImgRgn[0] = {0, 0, 1920, 1080},
    .tDevImgRgn[1] = {0, 0, 1920, 1080},
    .tDevImgRgn[2] = {0, 0, 1920, 1080},
    .tDevImgRgn[3] = {0, 0, 1920, 1080},

    /* When users transfer special data, they need to configure VC&DT for szImgVc/szImgDt/szInfoVc/szInfoDt */
    //.tMipiIntfAttr.szImgVc[0] = 0,
    //.tMipiIntfAttr.szImgDt[0] = MIPI_CSI_DT_RAW10,
    //.tMipiIntfAttr.szImgVc[1] = 1,
    //.tMipiIntfAttr.szImgDt[1] = MIPI_CSI_DT_RAW10,

    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eBayerPattern = AX_BP_RGGB,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .tCompressInfo = {AX_COMPRESS_MODE_NONE, 0},
    .tFrameRateCtrl= {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_PIPE_ATTR_T gSc200aiPipeAttr = {
    .ePipeWorkMode = AX_VIN_PIPE_NORMAL_MODE1,
    .tPipeImgRgn = {0, 0, 1920, 1080},
    .nWidthStride = 1920,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP_PACKED,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 0},
    .tNrAttr = {{0, {AX_COMPRESS_MODE_LOSSLESS, 0}}, {0, {AX_COMPRESS_MODE_NONE, 0}}},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

AX_VIN_CHN_ATTR_T gSc200aiChn0Attr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .nWidthStride = 1920,
    .eImgFormat = AX_FORMAT_YUV420_SEMIPLANAR,
    .nDepth = 1,
    .tCompressInfo = {AX_COMPRESS_MODE_LOSSY, 4},
    .tFrameRateCtrl = {AX_INVALID_FRMRATE, AX_INVALID_FRMRATE},
};

#endif //_COMMON_CONFIG_H__

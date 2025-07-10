/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#include "ax_sensor_struct.h"
#include "ax_buffer_tool.h"
// #include "common_sys.h"
#include "ax_vin_api.h"
#include "common_vin.h"
#include "common_type.h"
#include "common_config.h"



AX_S32 COMMON_VIN_GetSnsConfig(SAMPLE_SNS_TYPE_E eSnsType,
                               AX_MIPI_RX_ATTR_T *ptMipiAttr, AX_SNS_ATTR_T *ptSnsAttr,
                               AX_SNS_CLK_ATTR_T *ptSnsClkAttr, AX_VIN_DEV_ATTR_T *pDevAttr,
                               AX_VIN_PIPE_ATTR_T *pPipeAttr, AX_VIN_CHN_ATTR_T *pChnAttr)
{
    switch (eSnsType) {
    case SAMPLE_SNS_DUMMY:
        memcpy(ptMipiAttr, &gDummyMipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gDummySnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gDummySnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gDummyDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gDummyPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gDummyChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_DVP:
        memcpy(ptSnsAttr, &gDVPSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gDVPSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gDVPDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gDVPPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gDVPChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_DVP_IR:
        memcpy(ptSnsAttr, &gDVPIRSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gDVPIRSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gDVPIRDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gDVPIRPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gDVPIRChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_BT601:
        memcpy(ptSnsAttr, &gBT601SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gBT601SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gBT601DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT601PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gBT601Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_BT656:
        memcpy(ptSnsAttr, &gBT656SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gBT656SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gBT656DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT656PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gBT656Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_BT1120:
        memcpy(ptSnsAttr, &gBT1120SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gBT1120SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gBT1120DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT1120PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gBT1120Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SAMPLE_SNS_LVDS:
        memcpy(ptSnsAttr, &gLvdsSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gLvdsSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gLvdsDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gLvdsPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gLvdsChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case OMNIVISION_OS04A10:
        memcpy(ptMipiAttr, &gOs04a10MipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04a10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04a10DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04a10PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gOs04a10Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
// ### SIPEED EDIT ###
    case OMNIVISION_OS04D10:
        memcpy(ptMipiAttr, &gOs04d10MipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gOs04d10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04d10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04d10DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04d10PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gOs04d10Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
// ### SIPEED EDIT END ###
    case SMARTSENS_SC200AI:
        memcpy(ptMipiAttr, &gSc200aiMipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gSc200aiSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gSc200aiSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gSc200aiDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gSc200aiPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gSc200aiChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SMARTSENS_SC450AI:
        memcpy(ptMipiAttr, &gSc450aiMipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gSc450aiSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gSc450aiSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gSc450aiDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gSc450aiPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gSc450aiChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
// ### SIPEED EDIT ###
    case SMARTSENS_SC850SL:
        memcpy(ptMipiAttr, &gSc850slMipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gSc850slSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gSc850slSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gSc850slDevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gSc850slPipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gSc850slChn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
// ### SIPEED EDIT END ###
    case SAMSUNG_S5KJN1SQ03:
        memcpy(ptMipiAttr, &gs5kjn1sq03MipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gs5kjn1sq03SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gs5kjn1sq03SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gs5kjn1sq03DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gs5kjn1sq03PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gs5kjn1sq03Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case OMNIVISION_OS04A10_DCG:
    case OMNIVISION_OS04A10_DCG_VS:
        memcpy(ptMipiAttr, &gOs04a10MipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04a10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04a10DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04a10PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gOs04a10Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    default:
        memcpy(ptMipiAttr, &gOs04a10MipiAttr, sizeof(AX_MIPI_RX_ATTR_T));
        memcpy(ptSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04a10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04a10DevAttr, sizeof(AX_VIN_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04a10PipeAttr, sizeof(AX_VIN_PIPE_ATTR_T));
        memcpy(&pChnAttr[0], &gOs04a10Chn0Attr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    }

    return 0;
}

AX_S32 COMMON_VIN_StartMipi(AX_U8 nRxDev, AX_INPUT_MODE_E eInputMode, AX_MIPI_RX_ATTR_T *ptMipiAttr, AX_LANE_COMBO_MODE_E eLaneComboMode)
{
    AX_S32 nRet = 0;
    AX_MIPI_RX_DEV_T  tMipiDev = {0};

    tMipiDev.eInputMode = eInputMode;
    tMipiDev.eBtClkMode = AX_BT_CLK_MODE_SDR;
    memcpy(&tMipiDev.tMipiAttr, ptMipiAttr, sizeof(AX_MIPI_RX_ATTR_T));

    if ((tMipiDev.eInputMode == AX_INPUT_MODE_MIPI) || (tMipiDev.eInputMode == AX_INPUT_MODE_LVDS)) {
        if (tMipiDev.tMipiAttr.eLaneNum == AX_MIPI_DATA_LANE_4) {
            AX_MIPI_RX_SetLaneCombo(AX_LANE_COMBO_MODE_0);
        } else {
            AX_MIPI_RX_SetLaneCombo(AX_LANE_COMBO_MODE_1);
        }
    } else {
        if (((gBT656DevAttr.tBtIntfAttr.eBtTdmMode != AX_VIN_BT_NORMAL_MODE) && (eInputMode == AX_INPUT_MODE_BT656)) ||
            ((gBT1120DevAttr.tBtIntfAttr.eBtTdmMode != AX_VIN_BT_NORMAL_MODE) && (eInputMode == AX_INPUT_MODE_BT1120))){
            tMipiDev.eBtClkMode = AX_BT_CLK_MODE_DDR;
        }
        AX_MIPI_RX_SetLaneCombo(eLaneComboMode);
    }

    nRet = AX_MIPI_RX_SetAttr(nRxDev, &tMipiDev);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_MIPI_RX_SetAttr failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_MIPI_RX_Reset(nRxDev);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_MIPI_RX_Reset, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_MIPI_RX_Start(nRxDev);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_MIPI_RX_Start failed, ret=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_VIN_StopMipi(AX_U8 nRxDev)
{
    AX_S32 axRet;

    axRet = AX_MIPI_RX_Stop(nRxDev);
    if (0 != axRet) {
        COMM_VIN_PRT("AX_MIPI_RX_Stop failed, ret=0x%x.\n", axRet);
    }

    return 0;
}


AX_S32 COMMON_VIN_StartDev(AX_U8 devId, AX_BOOL bEnableDev, AX_VIN_DEV_ATTR_T *pDevAttr)
{
    AX_S32 nRet = 0;
    AX_VIN_DUMP_ATTR_T  tDumpAttr = {0};

    if (bEnableDev) {
        if (AX_VIN_DEV_OFFLINE == pDevAttr->eDevMode) {
            tDumpAttr.bEnable = AX_TRUE;
            tDumpAttr.nDepth = 3;
            nRet = AX_VIN_SetDevDumpAttr(devId, AX_VIN_DUMP_QUEUE_TYPE_DEV, &tDumpAttr);
            if (0 != nRet) {
                COMM_VIN_PRT(" AX_VIN_SetDevDumpAttr failed, ret=0x%x.\n", nRet);
                return -1;
            }
        }

        nRet = AX_VIN_EnableDev(devId);
        if (0 != nRet) {
            COMM_VIN_PRT("AX_VIN_EnableDev failed, ret=0x%x.\n", nRet);
            return -1;
        }
    }

    return 0;
}

AX_S32 COMMON_VIN_StopDev(AX_U8 devId, AX_BOOL bEnableDev)
{
    AX_S32 axRet;
    AX_VIN_DEV_ATTR_T tDevAttr = {0};
    AX_VIN_DUMP_ATTR_T tDumpAttr = {0};

    AX_VIN_GetDevAttr(devId, &tDevAttr);

    if (bEnableDev) {
        axRet = AX_VIN_DisableDev(devId);
        if (0 != axRet) {
            COMM_VIN_PRT("AX_VIN_DisableDev failed, devId=%d, ret=0x%x.\n", devId, axRet);
        }

        if (AX_VIN_DEV_OFFLINE == tDevAttr.eDevMode) {
            tDumpAttr.bEnable = AX_FALSE;
            axRet = AX_VIN_SetDevDumpAttr(devId, AX_VIN_DUMP_QUEUE_TYPE_DEV, &tDumpAttr);
            if (0 != axRet) {
                COMM_VIN_PRT(" AX_VIN_SetSnsDumpAttr failed, ret=0x%x.\n", axRet);
            }
        }

    }

    return 0;
}

AX_S32 COMMON_VIN_CreateDev(AX_U8 devId, AX_U32 nRxDev, AX_VIN_DEV_ATTR_T *pDevAttr,
                            AX_VIN_DEV_BIND_PIPE_T *ptDevBindPipe)
{
    AX_S32 nRet = 0;

    nRet = AX_VIN_CreateDev(devId, pDevAttr);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_VIN_CreateDev failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_VIN_SetDevAttr(devId, pDevAttr);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_VIN_CreateDev failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_VIN_SetDevBindPipe(devId, ptDevBindPipe);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_VIN_SetDevBindPipe failed, ret=0x%x\n", nRet);
        return -1;
    }

    nRet = AX_VIN_SetDevBindMipi(devId, nRxDev);
    if (0 != nRet) {
        COMM_VIN_PRT("AX_VIN_SetDevBindMipi failed, ret=0x%x\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_VIN_DestroyDev(AX_U8 devId)
{
    AX_S32 axRet;

    axRet = AX_VIN_DestroyDev(devId);
    if (0 != axRet) {
        COMM_VIN_PRT("AX_VIN_DestroyDev failed, devId=%d, ret=0x%x.\n", devId, axRet);
    }

    return 0;
}

AX_S32 COMMON_VIN_SetPipeAttr(COMMON_VIN_MODE_E eSysMode, SAMPLE_LOAD_RAW_NODE_E eLoadRawNode, AX_U8 nPipeId,
                              AX_VIN_PIPE_ATTR_T *pPipeAttr)
{
    AX_S32 axRet;
    //AX_VIN_DUMP_ATTR_T sPipeDumpAttr = {0};
    //AX_VIN_DUMP_ATTR_T *pPipeDump = &sPipeDumpAttr;
    AX_U8 nPipeWorkMode = 0;

    axRet = AX_VIN_CreatePipe(nPipeId, pPipeAttr);
    if (0 != axRet) {
        COMM_VIN_PRT("AX_VIN_CreatePipe failed, ret=0x%x.\n", axRet);
        return -1;
    }
    axRet = AX_VIN_SetPipeAttr(nPipeId, pPipeAttr);
    if (0 != axRet) {
        COMM_VIN_PRT("AX_VI_SetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    nPipeWorkMode = (AX_U8)pPipeAttr->ePipeWorkMode;
    switch (nPipeWorkMode) {
    case AX_VIN_PIPE_NORMAL_MODE1:
        if (COMMON_VIN_LOADRAW == eSysMode && LOAD_RAW_ITP == eLoadRawNode) {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_ITP,
                                              AX_VIN_FRAME_SOURCE_TYPE_USER);  // IFE + ITP RAW+RGB+YUV DOMAIN
            if (axRet) {
                COMM_PRT("pipe %d src %d  frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_ITP);
            }
            break;
        } else if (COMMON_VIN_LOADRAW == eSysMode) {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_VIN_FRAME_SOURCE_TYPE_USER);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
        } else if (LOAD_RAW_ITP == eLoadRawNode) {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_ITP, AX_VIN_FRAME_SOURCE_TYPE_USER);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_ITP);
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_VIN_FRAME_SOURCE_TYPE_DEV);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
        } else {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_VIN_FRAME_SOURCE_TYPE_DEV);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
        }
        break;
    case AX_VIN_PIPE_SUB_YUV_MODE:
        if (COMMON_VIN_LOADRAW == eSysMode) {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_VIN_FRAME_SOURCE_TYPE_USER);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_YUV, AX_VIN_FRAME_SOURCE_TYPE_USER);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
        } else {
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_VIN_FRAME_SOURCE_TYPE_DEV);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
            axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_VIN_FRAME_SOURCE_ID_YUV, AX_VIN_FRAME_SOURCE_TYPE_DEV);
            if (axRet)
                COMM_PRT("pipe %d src %d frame source set failed....\n", nPipeId, AX_VIN_FRAME_SOURCE_ID_IFE);
        }
        break;
    default:
        COMM_PRT("pipe work mode%x.\n", pPipeAttr->ePipeWorkMode);
        break;
    }

    return 0;
}

AX_S32 COMMON_VIN_StartChn(AX_U8 pipe, AX_VIN_CHN_ATTR_T *ptChnAttr, AX_BOOL *pChnEn)
{
    AX_S32 nRet = 0;
    AX_S32 chn = 0;

    for (chn = 0; chn < AX_VIN_CHN_ID_MAX; chn++) {
        nRet = AX_VIN_SetChnAttr(pipe, chn, &ptChnAttr[chn]);
        if (0 != nRet) {
            COMM_VIN_PRT("AX_VIN_SetChnAttr failed, nRet=0x%x.\n", nRet);
            return -1;
        }
        if(pChnEn[chn] == AX_TRUE) {
            nRet = AX_VIN_EnableChn(pipe, chn);
            if (0 != nRet) {
                COMM_VIN_PRT("AX_VIN_EnableChn failed, nRet=0x%x.\n", nRet);
                return -1;
            }
        }
    }

    return 0;
}

AX_S32 COMMON_VIN_StopChn(AX_U8 pipe)
{
    AX_S32 nRet = 0;
    AX_S32 chn = 0;

    for (chn = 0; chn < AX_VIN_CHN_ID_MAX; chn++) {
        nRet = AX_VIN_DisableChn(pipe, chn);
        if (0 != nRet) {
            COMM_VIN_PRT("AX_VIN_DisableChn failed, nRet=0x%x.\n", nRet);
            return -1;
        }
    }

    return 0;
}



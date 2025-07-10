
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
#include <sys/prctl.h>

#include "ax_vin_api.h"
#include "ax_isp_api.h"
#include "ax_vin_error_code.h"
#include "ax_mipi_rx_api.h"
#include "common_cam.h"
#include "common_sys.h"
#include "common_type.h"
#include "ax_isp_3a_api.h"

static pthread_t gDispatchThread[MAX_CAMERAS] = {0};
static AX_S32 g_dispatcher_loop_exit[MAX_CAMERAS] = {0};
static void *DispatchThread(void *args);
static AX_S32 SysFrameDispatch(AX_U8 nPipeId, AX_CAMERA_T *pCam, AX_SNS_HDR_MODE_E eHdrMode);


static AX_S32 COMMON_CAM_StitchAttrInit()
{
    AX_U8  grpId = 0;
    AX_S32 axRet = 0;
    AX_VIN_STITCH_GRP_ATTR_T tStitchGrpAttr = {0};

    /* 1.read param */
    axRet = AX_VIN_GetStitchGrpAttr(grpId, &tStitchGrpAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_GetStitchGrpAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    /* 2.modify param : To do */

    /* 3.write param */
    axRet = AX_VIN_SetStitchGrpAttr(grpId, &tStitchGrpAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetStitchGrpAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_NPU_Init()
{
    AX_S32 axRet = 0;

    /* NPU Init */
    AX_ENGINE_NPU_ATTR_T attr;
    memset(&attr, 0, sizeof(AX_ENGINE_NPU_ATTR_T));
    attr.eHardMode = AX_ENGINE_VIRTUAL_NPU_ENABLE;
    axRet = AX_ENGINE_Init(&attr);
    if (0 != axRet) {
        COMM_CAM_PRT("AX_ENGINE_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }
    return 0;
}

AX_S32 COMMON_CAM_PrivPoolInit(COMMON_SYS_ARGS_T *pPrivPoolArgs)
{
    AX_S32 axRet = 0;
    AX_POOL_FLOORPLAN_T tPoolFloorPlan = {0};

    if (pPrivPoolArgs == NULL) {
        return -1;
    }

    /* Calc Pool BlkSize/BlkCnt */
    axRet = COMMON_SYS_CalcPool(pPrivPoolArgs->pPoolCfg, pPrivPoolArgs->nPoolCfgCnt, &tPoolFloorPlan);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_SYS_CalcPool failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetPoolAttr(&tPoolFloorPlan);
    if (0 != axRet) {
        COMM_CAM_PRT("AX_VIN_SetPoolAttr fail!Error Code:0x%X\n", axRet);
        return -1;
    } else {
        printf("AX_VIN_SetPoolAttr success!\n");
    }

    return 0;
}

AX_S32 COMMON_CAM_Init(AX_VOID)
{
    AX_S32 axRet = 0;

    /* VIN Init */
    axRet = AX_VIN_Init();
    if (0 != axRet) {
        COMM_CAM_PRT("AX_VIN_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* Stitch Init */
    axRet = COMMON_CAM_StitchAttrInit();
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_CAM_StitchAttrInit failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* MIPI Init */
    axRet = AX_MIPI_RX_Init();
    if (0 != axRet) {
        COMM_CAM_PRT("AX_MIPI_RX_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_CAM_Deinit(AX_VOID)
{
    AX_S32 axRet = 0;

    axRet = AX_MIPI_RX_DeInit();
    if (0 != axRet) {
        COMM_CAM_PRT("AX_MIPI_RX_DeInit failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    axRet = AX_VIN_Deinit();
    if (0 != axRet) {
        COMM_CAM_PRT("AX_VIN_DeInit failed, ret=0x%x.\n", axRet);
        return -1 ;
    }
    axRet = AX_ENGINE_Deinit();
    if (0 != axRet) {
        COMM_CAM_PRT("AX_ENGINE_Deinit failed, ret=0x%x.\n", axRet);
        return -1 ;
    }
    return axRet;
}

static AX_S32 __common_cam_open(AX_CAMERA_T *pCam)
{
    AX_S32 i = 0;
    AX_S32 axRet = 0;
    AX_U8 nPipeId = 0;
    AX_U8 nDevId = pCam->nDevId;
    AX_U32 nRxDev = pCam->nRxDev;
    AX_INPUT_MODE_E eInputMode = pCam->eInputMode;

    /* confige sensor clk */
    axRet = AX_ISP_OpenSnsClk(pCam->tSnsClkAttr.nSnsClkIdx, pCam->tSnsClkAttr.eSnsClkRate);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_ISP_OpenSnsClk failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_ISP_ResetSnsObj(nPipeId, nDevId, pCam->ptSnsHdl[pCam->nPipeId]);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_ISP_ResetSnsObj failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_VIN_StartMipi(nRxDev, eInputMode, &pCam->tMipiAttr, pCam->eLaneComboMode);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_StartMipi failed, r-et=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_VIN_CreateDev(nDevId, nRxDev, &pCam->tDevAttr, &pCam->tDevBindPipe);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_CreateDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    for (i = 0; i < pCam->tDevBindPipe.nNum; i++) {
        nPipeId = pCam->tDevBindPipe.nPipeId[i];
        pCam->tPipeAttr[pCam->nPipeId].bAiIspEnable = pCam->tPipeInfo[i].bAiispEnable;
        axRet = COMMON_VIN_SetPipeAttr(pCam->eSysMode, pCam->eLoadRawNode, nPipeId, &pCam->tPipeAttr[pCam->nPipeId]);
        if (0 != axRet) {
            COMM_CAM_PRT("COMMON_ISP_SetPipeAttr failed, ret=0x%x.\n", axRet);
            return -1;
        }
        if (pCam->bRegisterSns) {
            axRet = COMMON_ISP_RegisterSns(nPipeId, nDevId, pCam->eBusType, pCam->ptSnsHdl[pCam->nPipeId], pCam->nI2cAddr, pCam->nI2cNode);
            if (0 != axRet) {
                COMM_CAM_PRT("COMMON_ISP_RegisterSns failed, ret=0x%x.\n", axRet);
                return -1;
            }
            axRet = COMMON_ISP_SetSnsAttr(nPipeId, &pCam->tSnsAttr, &pCam->tSnsClkAttr);
            if (0 != axRet) {
                COMM_CAM_PRT("COMMON_ISP_SetSnsAttr failed, ret=0x%x.\n", axRet);
                return -1;
            }
        }
        axRet = COMMON_ISP_Init(nPipeId, pCam->ptSnsHdl[pCam->nPipeId], pCam->bRegisterSns, pCam->bUser3a,
                                &pCam->tAeFuncs, &pCam->tAwbFuncs, &pCam->tAfFuncs, &pCam->tLscFuncs,
                                pCam->tPipeInfo[i].szBinPath);
        if (0 != axRet) {
            COMM_CAM_PRT("COMMON_ISP_StartIsp failed, axRet = 0x%x.\n", axRet);
            return -1;
        }
        axRet = COMMON_VIN_StartChn(nPipeId, pCam->tChnAttr, pCam->bChnEn);
        if (0 != axRet) {
            COMM_CAM_PRT("COMMON_ISP_StartChn failed, nRet = 0x%x.\n", axRet);
            return -1;
        }
        axRet = AX_VIN_StartPipe(nPipeId);
        if (0 != axRet) {
            COMM_CAM_PRT("AX_VIN_StartPipe failed, ret=0x%x\n", axRet);
            return -1;
        }

        axRet = AX_ISP_Start(nPipeId);
        if (0 != axRet) {
            COMM_CAM_PRT("AX_ISP_Open failed, ret=0x%x\n", axRet);
            return -1;
        }
        /* When there are multiple pipe, only the first pipe needs AE */
        // if (0 < i) {
        //     axRet = COMMON_ISP_SetAeToManual(nPipeId);
        //     if (0 != axRet) {
        //         COMM_CAM_PRT("COMMON_ISP_SetAeToManual failed, ret=0x%x\n", axRet);
        //         return -1;
        //     }
        // }
    }

    axRet = COMMON_VIN_StartDev(nDevId, pCam->bEnableDev, &pCam->tDevAttr);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_StartDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    if (pCam->bRegisterSns && pCam->bEnableDev) {
        for (i = 0; i < pCam->tDevBindPipe.nNum; i++) {
            axRet = AX_ISP_StreamOn(pCam->tDevBindPipe.nPipeId[i]);
            if (0 != axRet) {
                COMM_CAM_PRT(" failed, ret=0x%x.\n", axRet);
                return -1;
            }
        }
    }

    if (pCam->bEnableDev && pCam->eLoadRawNode == LOAD_RAW_ITP) {
        if (pCam->nNumber >= MAX_CAMERAS) {
            COMM_CAM_PRT("Access g_dispatcher_loop_exit[%d] out of bounds \n", pCam->nNumber);
            return -1;
        }
        g_dispatcher_loop_exit[pCam->nNumber] = 0;
        axRet = pthread_create(&gDispatchThread[pCam->nNumber], NULL, DispatchThread, (AX_VOID *)(pCam));
        if (0 != axRet) {
            COMM_CAM_PRT("pthread_create failed, ret=0x%x.\n", axRet);
            return -1;
        }
    }
    return 0;
}

static AX_S32 __common_cam_close(AX_CAMERA_T *pCam)
{
    AX_U8 i = 0;
    AX_S32 axRet = 0;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;
    AX_U32 nRxDev = pCam->nRxDev;

    if (pCam->nNumber < MAX_CAMERAS) {
        g_dispatcher_loop_exit[pCam->nNumber] = 1;
        if (gDispatchThread[pCam->nNumber] != 0) {
            axRet = pthread_join(gDispatchThread[pCam->nNumber], NULL);
            if (axRet < 0) {
                COMM_CAM_PRT(" dispacher thread exit failed, ret=0x%x.\n", axRet);
            }
            gDispatchThread[pCam->nNumber] = 0;
        }
    } else {
        COMM_CAM_PRT("Access g_dispatcher_loop_exit[%d] out of bounds \n", pCam->nNumber);
    }

    for (i = 0; i < pCam->tDevBindPipe.nNum; i++) {
        nPipeId = pCam->tDevBindPipe.nPipeId[i];
        axRet |= AX_ISP_Stop(nPipeId);
        if (0 != axRet) {
            COMM_ISP_PRT("AX_ISP_Stop failed, ret=0x%x.\n", axRet);
        }
    }

    axRet = COMMON_VIN_StopDev(nDevId, pCam->bEnableDev);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_StopDev failed, ret=0x%x.\n", axRet);
    }

    if (pCam->bRegisterSns && pCam->bEnableDev) {
        for (i = 0; i < pCam->tDevBindPipe.nNum; i++) {
            AX_ISP_StreamOff(pCam->tDevBindPipe.nPipeId[i]);

        }
    }

    axRet = AX_ISP_CloseSnsClk(pCam->tSnsClkAttr.nSnsClkIdx);
    if (0 != axRet) {
        COMM_CAM_PRT("AX_VIN_CloseSnsClk failed, ret=0x%x.\n", axRet);
    }

    for (i = 0; i < pCam->tDevBindPipe.nNum; i++) {
        nPipeId = pCam->tDevBindPipe.nPipeId[i];
        axRet = AX_VIN_StopPipe(nPipeId);
        if (0 != axRet) {
            COMM_CAM_PRT("AX_VIN_StopPipe failed, ret=0x%x.\n", axRet);
        }

        COMMON_VIN_StopChn(nPipeId);

        COMMON_ISP_DeInit(nPipeId, pCam->bRegisterSns);
        if (pCam->tSwitchInfo.mipi_switch_en == AX_TRUE) {
            axRet = AX_ISP_UnRegisterSensorExt(nPipeId, 1);
            if (0 != axRet) {
                COMM_CAM_PRT("AX_ISP_UnRegisterSensorExt failed, ret=0x%x.\n", axRet);
            }
            axRet = AX_ISP_UnRegisterSensorExt(nPipeId, 2);
            if (0 != axRet) {
                COMM_CAM_PRT("AX_ISP_UnRegisterSensorExt failed, ret=0x%x.\n", axRet);
            }
        } else {
            COMMON_ISP_UnRegisterSns(nPipeId);
        }

        AX_VIN_DestroyPipe(nPipeId);
    }

    axRet = COMMON_VIN_StopMipi(nRxDev);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_StopMipi failed, ret=0x%x.\n", axRet);
    }

    axRet = COMMON_VIN_DestroyDev(nDevId);
    if (0 != axRet) {
        COMM_CAM_PRT("COMMON_VIN_DestroyDev failed, ret=0x%x.\n", axRet);
    }

    COMM_CAM_PRT("%s: nDevId %d: exit.\n", __func__, nDevId);

    return AX_SUCCESS;
}

AX_S32 COMMON_CAM_Open(AX_CAMERA_T *pCamList, AX_U8 Num)
{
    AX_U16 i = 0;
    if (pCamList == NULL) {
        return -1;
    }

    for (i = 0; i < Num; i++) {
        if (AX_SUCCESS == __common_cam_open(&pCamList[i])) {
            pCamList[i].bOpen = AX_TRUE;
            COMM_CAM_PRT("camera %d is open\n", i);
        } else {
            goto EXIT;
        }
    }
    return 0;
EXIT:
    for (i = 0; i < Num; i++) {
        if (!pCamList[i].bOpen) {
            continue;
        }
        __common_cam_close(&pCamList[i]);
    }
    return -1;
}

AX_S32 COMMON_CAM_Close(AX_CAMERA_T *pCamList, AX_U8 Num)
{
    AX_U16 i = 0;
    if (pCamList == NULL) {
        return -1;
    }

    for (i = 0; i < Num; i++) {
        if (!pCamList[i].bOpen) {
            continue;
        }
        if (AX_SUCCESS == __common_cam_close(&pCamList[i])) {
            COMM_CAM_PRT("camera %d is close\n", i);
            pCamList[i].bOpen = AX_FALSE;
        } else {
            return -1;
        }
    }

    return 0;
}

static AX_BOOL SeqNumIsMatch(AX_U8 nDevId, AX_IMG_INFO_T *frameBufferArr,  AX_U64 *frameSeqs, AX_U64 maxFrameSeq,
                             AX_SNS_HDR_MODE_E eHdrMode)
{
    AX_BOOL frameSeqNotMatch = AX_FALSE;
    AX_S32 j = 0;

    do {
        frameSeqNotMatch = AX_FALSE;
        for (j = 0; j < eHdrMode; j++) {
            if (frameSeqs[j] < maxFrameSeq) {
                COMM_ISP_PRT("FrameSeq(%lld) doesn't match (max_frame_seq:%lld), drop blk_id: 0x%x\n",
                             frameSeqs[j], maxFrameSeq, frameBufferArr[j].tFrameInfo.stVFrame.u32BlkId[0]);
                AX_VIN_ReleaseRawFrame(nDevId, AX_VIN_PIPE_DUMP_NODE_IFE, j, frameBufferArr + j);
                AX_VIN_GetRawFrame(nDevId, AX_VIN_PIPE_DUMP_NODE_IFE, j, frameBufferArr + j, -1);
                frameSeqNotMatch = AX_TRUE;
            }
        }

        if (frameSeqNotMatch) {
            for (j = 0; j < eHdrMode; j++) {
                frameSeqs[j] = frameBufferArr[j].tFrameInfo.stVFrame.u64SeqNum;
                if (frameSeqs[j] > maxFrameSeq) {
                    maxFrameSeq = frameSeqs[j];
                }
            }
        }
    } while (frameSeqNotMatch);

    return AX_TRUE;
}

static AX_S32 SysFrameDispatch(AX_U8 nPipeId, AX_CAMERA_T *pCam, AX_SNS_HDR_MODE_E eHdrMode)
{
    AX_S32 axRet = 0;
    AX_S32 j = 0;
    AX_S32 timeOutMs = 1000;
    AX_U64 maxFrameSeq = 0;
    AX_IMG_INFO_T frameBufferArr[AX_SNS_HDR_FRAME_MAX] = {0};
    AX_U64 frameSeqs[AX_SNS_HDR_FRAME_MAX] = {0};
    AX_BOOL isMatch = AX_FALSE;

    for (j = 0; j < eHdrMode; j++) {
        axRet = AX_VIN_GetRawFrame(nPipeId, AX_VIN_PIPE_DUMP_NODE_IFE, j, frameBufferArr + j, timeOutMs);
        if (axRet != 0) {
            if (AX_ERR_VIN_RES_EMPTY == axRet) {
                COMM_ISP_PRT("nonblock error, 0x%x\n", axRet);
                return axRet;
            }

            usleep(10 * 1000);
            AX_VIN_ReleaseRawFrame(nPipeId, AX_VIN_PIPE_DUMP_NODE_IFE, j, frameBufferArr + j);
            return axRet;
        }

        frameSeqs[j] = frameBufferArr[j].tFrameInfo.stVFrame.u64SeqNum;
        if (frameSeqs[j] > maxFrameSeq) {
            maxFrameSeq = frameSeqs[j];
        }
    }

    isMatch = SeqNumIsMatch(nPipeId, frameBufferArr, frameSeqs, maxFrameSeq, eHdrMode);
    if (isMatch == AX_TRUE) {
        axRet = AX_VIN_SendRawFrame(nPipeId, AX_VIN_FRAME_SOURCE_ID_ITP, eHdrMode,
                                    (const AX_IMG_INFO_T **)&frameBufferArr, timeOutMs);
        if (axRet != 0) {
            COMM_ISP_PRT("Send Pipe raw frame failed\n");
        }
    }

    for (j = 0; j < eHdrMode; j++) {
        AX_VIN_ReleaseRawFrame(nPipeId, AX_VIN_PIPE_DUMP_NODE_IFE, j, frameBufferArr + j);
    }

    return 0;
}

static void *DispatchThread(void *args)
{
    AX_CAMERA_T *pCam = (AX_CAMERA_T *)args;
    AX_CHAR token[32] = {0};

    AX_U8 nPipeId = pCam->nPipeId;
    AX_SNS_HDR_MODE_E eHdrMode = pCam->eHdrMode;

    snprintf(token, 32, "RAW_DISP_%u", nPipeId);
    prctl(PR_SET_NAME, token);

    while (!g_dispatcher_loop_exit[pCam->nNumber]) {
        SysFrameDispatch(nPipeId, pCam, eHdrMode);
    }

    return NULL;
}

AX_S32 Save_Raw_File(AX_VOID *frame_addr, int frame_size, int chn)
{
    AX_S32 axRet = 0;
    FILE *pstFile = NULL;
    AX_U8 file_name[128] = "";

    sprintf((char *)file_name, "out_chn_%d.raw", chn);
    pstFile = fopen((char *)file_name, "wb");
    if (pstFile == NULL) {
        COMM_ISP_PRT("fail to open video file !\n");
        return -1;
    }
    fwrite((char *)frame_addr, frame_size, 1, pstFile);
    if (pstFile) {
        fclose(pstFile);
    }
    return axRet;
}

AX_S32 Save_YUV_File(AX_VOID *frame_addr, int frame_size, int chn)
{
    AX_S32 axRet = 0;
    FILE *pstFile = NULL;
    AX_U8 file_name[128] = "";

    sprintf((char *)file_name, "out_chn_%d.yuv", chn);
    pstFile = fopen((char *)file_name, "wb");
    if (pstFile == NULL) {
        COMM_ISP_PRT("fail to open video file !\n");
        return -1;
    }

    fwrite((char *)frame_addr, frame_size, 1, pstFile);
    if (pstFile) {
        fclose(pstFile);
    }
    return axRet;
}

AX_S32 COMMON_CAM_CaptureFrameProc(AX_U32 nCapturePipeId, const AX_IMG_INFO_T *pImgInfo[])
{
    AX_S32 axRet = 0;
    AX_IMG_INFO_T capture_img_info = {0};
    AX_U32 nRefPipeId = 0;
    AX_ISP_IQ_AE_PARAM_T tUserCaptureFrameAeParam;
    AX_ISP_IQ_AWB_PARAM_T tUserCaptureFrameAwbParam;
    //AX_ISP_IQ_AINR_PARAM_T  tUserCaptureFrameAinrParam;

    /* use your capture raw frame's ae in manual mode, this is just a sample*/
    AX_ISP_IQ_GetAeParam(nRefPipeId, &tUserCaptureFrameAeParam);
    tUserCaptureFrameAeParam.nEnable = AX_FALSE;
    AX_ISP_IQ_SetAeParam(nCapturePipeId, &tUserCaptureFrameAeParam);

    /* use your capture raw frame's awb in manual mode, this is just a sample*/
    AX_ISP_IQ_GetAwbParam(nRefPipeId, &tUserCaptureFrameAwbParam);
    tUserCaptureFrameAwbParam.nEnable = AX_FALSE;
    AX_ISP_IQ_SetAwbParam(nCapturePipeId, &tUserCaptureFrameAwbParam);

    /* 1. first send raw frame*/
    AX_ISP_RunOnce(nCapturePipeId);

    axRet = AX_VIN_SendRawFrame(nCapturePipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_SNS_LINEAR_MODE,
                                pImgInfo, 3000);
    if (axRet != 0) {
        COMM_ISP_PRT("Send Pipe raw frame failed");
        return axRet;
    }
    /* The first frame data is invalid for the user */
    axRet = AX_VIN_GetYuvFrame(nCapturePipeId, AX_VIN_CHN_ID_MAIN, &capture_img_info, 3000);
    if (axRet != 0) {
        COMM_ISP_PRT("func:%s, return error!.\n", __func__);
        return axRet;
    }
    AX_VIN_ReleaseYuvFrame(nCapturePipeId, AX_VIN_CHN_ID_MAIN, &capture_img_info);

    AX_ISP_RunOnce(nCapturePipeId);

    axRet = AX_VIN_SendRawFrame(nCapturePipeId, AX_VIN_FRAME_SOURCE_ID_IFE, AX_SNS_LINEAR_MODE,
                                pImgInfo, 3000);
    if (axRet != 0) {
        COMM_ISP_PRT("Send Pipe raw frame failed");
        return axRet;
    }

    /* The second frame data is the final result frame */
    axRet = AX_VIN_GetYuvFrame(nCapturePipeId, AX_VIN_CHN_ID_MAIN, &capture_img_info, 3000);
    if (axRet != 0) {
        COMM_ISP_PRT("func:%s, return error!.\n", __func__);
        return axRet;
    }

    /* Users can use second YUV frame for application development */

    /* User Code */
    /* ...... */

    AX_VIN_ReleaseYuvFrame(nCapturePipeId, AX_VIN_CHN_ID_MAIN, &capture_img_info);

    COMM_ISP_PRT("Capture Frame Proc success.\n");
    return AX_SUCCESS;
}

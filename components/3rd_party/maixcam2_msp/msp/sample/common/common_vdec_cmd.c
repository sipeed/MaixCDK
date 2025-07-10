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
#include <ctype.h>

#include "common_vdec_cmd.h"

static int __ParseResParams(char *optarg, AX_U32 *w, AX_U32 *h) {
    char *p = optarg;
    char *q = p;

        while (*p && isdigit(*p)) p++;
        if (!*p || *p != 'x') return 1;
        *p++ = '\0'; *w = atoi(q); q = p;
        while (*p && isdigit(*p)) p++;
        if (*p) return 1;
        *p++ = '\0'; *h = atoi(q); q = p;
    return 0;
}

extern char *__progname;

static void __PrintHelp_Com()
{
    printf("usage: %s -i streamFile <args>\n", __progname);
    printf("args:\n");

    printf("  -c:       group count. (1-16), default: 1\n");
    printf("  -L:       loop decode number. (int), default: 1\n");
    printf("  -N:       receive decode number. (int), default: <= 0, no limit\n");
    printf("  -w:       write YUV frame to file. (0: not write, others: write), default: 0\n");
    printf("  -W:       max output buffer width. (for pool GetPicBufferSize), default: 1920\n");
    printf("  -H:       max outbut buffer height. (for pool GetPicBufferSize), default: 1920\n");
    printf("  -i:       input file. user specified input bitstream file path), \n");
    /* printf("  -o:       output file. (user specified output yuv file path), default: ./out.yuv\n"); */
    printf("  -M:       video mode. (3: stream, 1: frame), default: 1\n");
    printf("  -T:       video type. (96: PT_H264, 265: PT_H265, 26: PT_JPEG), default: 96 (PT_H264)\n");
    printf("  -j:       whether test jpeg single frame decoding function. \n"
            "               (0: not test, 1: test), default: 0\n");
    printf("  -q:       whether to wait input 'q' to quit program when finish stream decoding. \n"
            "               (0: not wait, 1: wait), default: 0\n");
    printf("  -s:       if video mode is stream, parameter is valid, and is send stream size, \n"
            "               Byte. it is less than stream buffer size 3M Byte, default: 10k Byte\n");
    printf("\n");
    printf("  --u32FrameHeight:     set frame height. default: 0\n");
    printf("  --sMilliSec:          send timeout flag.  (-1: block, 0: unblock, > 0: msec), default: -1\n");
    printf("  --sGetMilliSec:          receive timeout flag.  (-1: block, 0: unblock, > 0: msec), default: -1\n");
    printf("  --select:             select mode. (0: disable, else enable, def disable\n");

    printf("  --res:                stream resolution (--res=WidthxHeight, default: 1920x1920.\n");
    printf("  --highRes:            for high resolution test(for only one group of jdec).  \n"
            "                           (0: disable, 1: enable. default: 0\n");
    printf("  --bDynRes:            dynamic resolution test of jdec. \n"
            "                           1: enable. 0: disable. default: 0\n");
    printf("  --newInput:           another input file for jdec dynamic resolution test.\n");

    printf("  --uStartGrpId:        start group id. (0-15), default: 0\n");
    printf("  --enDisplayMode:      display mode. (0: preview mode, 1: playback mode), default: 1\n");
    printf("  --enFrameBufSrc:      output frame buf source  (1: private pool, 2: user pool), default: 2, \n");
    printf("  --u32FrameBufCnt:     frame buffer count. (default: 8, \n");
    printf("  --u32StreamFps:       stream fps, used for calu pts (default: 0, no pts \n");
    /* printf("  --bPerfTest:          whether do performance test \n"
            "                           (now support for jdec). 1: enable. 0: disable. default: 0\n"); */
    printf("  --bCheckFrmParam      whether check frm info (pts, seqnum, timeref) when get frm");
    printf("  --bGetUserData:       whether get user data when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bGetRbInfo:         whether get input ringbuf info when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bGetGrpAtrr:        whether get grp atrr info when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bQueryStatus:       whether query group decode status when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bGetVuiParam:       whether get vui param when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bResetGrp:          whether add reset operation when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bGetDispMode:       whether get display mode. 1: enable. 0: disable. default: 0\n");
    printf("  --bGetGrpPrm:         whether get grp prm. 1: enable. 0: disable. default: 0\n");
    printf("  --bRepeatTest:        create and destroy vdgrp multi times. 1: enable. 0: disable. default: 0\n");
    printf("  --bSleep:             whether add sleep operation when decode. 1: enable. 0: disable. default: 0\n");
    printf("  --bSkipRelease:       skip one frm release, to test reset/destroy. 1: enable. 0: disable. default: 0\n");
    printf("  --bSkipFrms:          skip frms by set pts to -1: enable. 0: disable. default: 0\n");

    printf("  --usrPicFile:         user picture file. (for inserting user picture.\n");
    printf("  --usrPicIdx:          Specifies which frame to insert the user picture after. (default: 7\n");
    /* printf("  --bUsrInstant         whether insert user picture instantly. (1: enable. 0: disable. default: 0\n"); */
    printf("  --recvStmAfUsrPic     whether to start recv stream after inserting user picture. \n"
            "                           (1: enable. 0: disable. default: 0\n");
    printf("  --enVideoMode:        Video Mode. 0: VIDEO_DEC_MODE_IPB. 1: VIDEO_DEC_MODE_IP. 2: VIDEO_DEC_MODE_I. default: 0\n");
    printf("  --enOutOrder:         outputorder. 0: AX_VDEC_OUTPUT_ORDER_DISP. 1: AX_VDEC_OUTPUT_ORDER_DEC. default: 0\n");
    printf("  --bCreatGrpEx:        whether to automatically assign an available group. 1: enable. 0: disable. default: 0\n");

    printf("  --waitTime:           waitTime. sample run time, def 20s,used in vdec_ivps_venc and vdec_ivps_vo sample\n");
    printf("  --bOpenIvps:          bOpenIvps. 1 open ivps in vdec_ivps_venc sample, def 0\n");

    /* vdec_ivps_vo param */
    printf("  --voType:             vo type,used in vdec -> vo, 0 dpi, 1 bt601, 2 bt656, 3 bt1120, 4 dsi, 5 lvds, def 0, see AX_VO_INTF_TYPE_E \n");
    printf("  --voDev:              vo dev,used in vdec -> vo, 0 dev0, 1 dev1, def 0 \n");
    printf("  --voRes:              vo resolution,used in vdec -> vo, see AX_VO_INTF_SYNC_E \n");
    printf("  --voGraphicOpen:      vo GUI (1: enable. 0: disable. default: 0\n");
}

static AX_S32 __SampleInitOptions(SAMPLE_OPTION_T **ppOptions, SAMPLE_OPTION_NAME_T **ppOptName)
{
    int i = 0;
    int j = 0;
    AX_S32 ret = 0;
    SAMPLE_OPTION_T *pOptions = NULL;
    AX_U32 tmp_size = 0;
    SAMPLE_OPTION_NAME_T *pOptName = NULL;

    if (ppOptions == NULL) {
        SAMPLE_CRIT_LOG("ppOptions == NULL");
        return -1;
    }

    tmp_size = sizeof(SAMPLE_OPTION_T) * SAMPLE_VDEC_OPTION_MAX_COUNT;
    pOptions = calloc(1, tmp_size);
    if (pOptions == NULL) {
        SAMPLE_CRIT_LOG("calloc tmp_size:%d FAILED!", tmp_size);
        return -1;
    }

    ret = SampleOptionsFill(pOptions, i++, "help", 'h', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "uGrpCount", 'c', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sLoopDecNum", 'L', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sRecvPicNum", 'N', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sWriteFrames", 'w', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "u32PicWidth", 'W', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "u32PicHeight", 'H', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "u32FrameHeight", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "pInputFilePath", 'i', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "pOutputFilePath", 'o', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enInputMode", 'M', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sStreamSize", 's', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enDecType", 'T', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bJpegDecOneFrm", 'j', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bQuitWait", 'q', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sMilliSec", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "sGetMilliSec", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "res", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bUsrInstant", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "recvStmAfUsrPic", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "usrPicFile", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "usrPicIdx", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bDynRes", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "newInput", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "highRes", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bPerfTest", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bCheckFrmParam", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetUserData", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetRbInfo", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetGrpAtrr", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bQueryStatus", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetVuiParam", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bResetGrp", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bSkipRelease", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bSkipFrms", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bSleep", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetDispMode", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bGetGrpPrm", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bRepeatTest", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "outputfile", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "tb_cfg_file", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enDisplayMode", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "multimode", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "select", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "streamcfg", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "nstream", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "uStartGrpId", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "u32FrameBufCnt", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enFrameBufSrc", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "u32StreamFps", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bFfmpegEnable", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "waitTime", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bOpenIvps", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "voType", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "voDev", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "voRes", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "voGraphicOpen", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "pollingEna", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "pollingCnt", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "pollingTime", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enVideoMode", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "enOutOrder", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bUserPts", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bMultiLck", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;
    ret = SampleOptionsFill(pOptions, i++, "bCreatGrpEx", '0', AX_TRUE);
    if (ret) goto ERR_RET_LOG;

    tmp_size = sizeof(SAMPLE_OPTION_NAME_T) * AX_VDEC_MAX_GRP_NUM;
    pOptName = calloc(1, tmp_size);
    if (pOptName == NULL) {
        SAMPLE_CRIT_LOG("calloc tmp_size:%d FAILED!", tmp_size);
        ret = -1;
        goto ERR_RET;
    }

    for (j = 0; j < AX_VDEC_MAX_GRP_NUM; j++) {
        if ((pOptName + j) == NULL) {
            SAMPLE_CRIT_LOG("SampleOptionsFill FAILED! j:%d\n", j);
            ret = -1;
            goto ERR_RET;
        }

        int len = snprintf((pOptName + j)->name, AX_OPTION_NAME_LEN, "%s%d", "grp_cmd_", j);
        if ((len < 0)) {
            SAMPLE_CRIT_LOG("snprintf FAILED! ret:0x%x \n", len);
            ret = -1;
            goto ERR_RET;
        }

        ret = SampleOptionsFill(pOptions, i++, (pOptName + j)->name, '0', AX_TRUE);
        if (ret) {
            SAMPLE_CRIT_LOG("SampleOptionsFill FAILED! i:%d, ret:0x%x \n", i, ret);
            ret = -1;
            goto ERR_RET;
        }

        // SAMPLE_LOG("(pOptName + j:%d):%s ", j, (pOptName + j)->name);
    }

    *ppOptions = pOptions;
    *ppOptName = pOptName;

    return 0;

ERR_RET_LOG:
    SAMPLE_CRIT_LOG("SampleOptionsFill FAILED! i:%d, ret:0x%x \n", i, ret);
ERR_RET:
    if (pOptions != NULL) {
        free(pOptions);
    }

    if (pOptName != NULL) {
        free(pOptName);
    }
    return ret;
}

static AX_S32 __VdecCmdParaPrint(SAMPLE_VDEC_CMD_PARAM_T *pstPara, AX_VDEC_GRP VdGrp)
{
    SAMPLE_LOG_N("pstPara->uGrpCount:%d", pstPara->uGrpCount);
    SAMPLE_LOG_N("pstPara->uStreamCount:%d", pstPara->uStreamCount);
    SAMPLE_LOG_N("pstPara->sLoopDecNum:%d", pstPara->sLoopDecNum);
    SAMPLE_LOG_N("pstPara->sRecvPicNum:%d", pstPara->sRecvPicNum);
    SAMPLE_LOG_N("pstPara->sMilliSec:%d", pstPara->sMilliSec);
    SAMPLE_LOG_N("pstPara->sGetMilliSec:%d", pstPara->sGetMilliSec);
    SAMPLE_LOG_N("pstPara->u32PicWidth:%d", pstPara->u32PicWidth);
    SAMPLE_LOG_N("pstPara->u32PicHeight:%d", pstPara->u32PicHeight);
    SAMPLE_LOG_N("pstPara->u32FrameHeight:%d", pstPara->u32FrameHeight);
    SAMPLE_LOG_N("pstPara->sWriteFrames:%d", pstPara->sWriteFrames);
    SAMPLE_LOG_N("pstPara->uStartGrpId:%d", pstPara->uStartGrpId);
    SAMPLE_LOG_N("pstPara->bJpegDecOneFrm:%d", pstPara->bJpegDecOneFrm);
    SAMPLE_LOG_N("pstPara->enMultimode:%d", pstPara->enMultimode);
    SAMPLE_LOG_N("pstPara->enDecType:%d", pstPara->enDecType);
    SAMPLE_LOG_N("pstPara->enInputMode:%d", pstPara->enInputMode);
    SAMPLE_LOG_N("pstPara->sStreamSize:%d", pstPara->sStreamSize);
    SAMPLE_LOG_N("pstPara->pInputFilePath:%s", pstPara->pInputFilePath);
    SAMPLE_LOG_N("pstPara->bFfmpegEnable:%d", pstPara->bFfmpegEnable);
    SAMPLE_LOG_N("pstPara->enVideoMode:%d", pstPara->enVideoMode);
    SAMPLE_LOG_N("pstPara->enOutOrder:%d", pstPara->enOutOrder);
    SAMPLE_LOG_N("pstPara->bUserPts:%d", pstPara->bUserPts);
    SAMPLE_LOG_N("pstPara->bMultiLck:%d", pstPara->bMultiLck);
    SAMPLE_LOG_N("pstPara->usrPicIdx:%d", pstPara->usrPicIdx);
    SAMPLE_LOG_N("pstPara->recvStmAfUsrPic:%d", pstPara->recvStmAfUsrPic);
    SAMPLE_LOG_N("pstPara->pUsrPicFilePath:%s", pstPara->pUsrPicFilePath);

    return 0;
}

static AX_VOID __VdecParameterAdjust(SAMPLE_VDEC_CMD_PARAM_T *pstCmd)
{
    if ((PT_JPEG == pstCmd->enDecType) || (PT_MJPEG == pstCmd->enDecType)) {
        if (pstCmd->enInputMode == AX_VDEC_INPUT_MODE_NAL)
            pstCmd->enInputMode = AX_VDEC_INPUT_MODE_FRAME;
    }

    if (pstCmd->highRes) {
        pstCmd->uGrpCount = 1;
    }
}

static AX_S32 __VdecParameterCheck(SAMPLE_VDEC_CMD_PARAM_T *pCml, AX_VDEC_GRP VdGrp)
{
    AX_S32 ret;
    SAMPLE_VDEC_CMD_PARAM_T *pstCmd = pCml;

    if (pstCmd->uGrpCount < 1 || pstCmd->uGrpCount > AX_VDEC_MAX_GRP_NUM) {
        SAMPLE_CRIT_LOG("Invalid group number:%d\n", pstCmd->uGrpCount);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->uStartGrpId < 0 || pstCmd->uStartGrpId >= AX_VDEC_MAX_GRP_NUM) {
        SAMPLE_CRIT_LOG("Invalid group id:%d\n", pstCmd->uStartGrpId);
        ret = -1;
        goto ERR_RET;
    }

    if (NULL == pstCmd->pInputFilePath) {
        if (pstCmd->pGrpCmdlFile[VdGrp] == NULL) {
            SAMPLE_CRIT_LOG("pstCmd->pGrpCmdlFile[%d] == NULL, NULL == pstCmd->pInputFilePath\n",
                           VdGrp);
            ret = -1;
            goto ERR_RET;
        }
    }

    if (pstCmd->uStreamCount > 0) {
        if (pstCmd->pGrpCmdlFile[pstCmd->uStreamCount - 1] == NULL) {
            SAMPLE_CRIT_LOG("VdGrp=%d, pstCmd->pGrpCmdlFile[%d - 1] == NULL\n",
                            VdGrp, pstCmd->uStreamCount);
            ret = -1;
            goto ERR_RET;
        }
    }

    if ((pstCmd->enDecType != PT_H265) && (pstCmd->enDecType != PT_H264) &&
            (pstCmd->enDecType != PT_JPEG) && (pstCmd->enDecType != PT_MJPEG)) {
        SAMPLE_CRIT_LOG("Invalid decode type:%d, AX650 can not supported\n", pstCmd->enDecType);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->enInputMode >= AX_VDEC_INPUT_MODE_BUTT) {
        SAMPLE_CRIT_LOG("Invalid decode mode:%d\n", pstCmd->enInputMode);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->enMultimode >= AX_VDEC_MULTI_MODE_PROCESS) {
        SAMPLE_CRIT_LOG("Invalid multi mode:%d\n", pstCmd->enMultimode);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->enDisplayMode >= AX_VDEC_DISPLAY_MODE_BUTT) {
        SAMPLE_CRIT_LOG("Invalid display mode:%d\n", pstCmd->enDisplayMode);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->sStreamSize > STREAM_BUFFER_MAX_SIZE) {
        SAMPLE_CRIT_LOG("Invalid stream size:%d, it is bigger than stream buffer size\n",
                        pstCmd->sStreamSize);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->sStreamSize <= 0) {
        SAMPLE_CRIT_LOG("Invalid stream size:%d\n", pstCmd->sStreamSize);
        ret = -1;
        goto ERR_RET;
    }

    if ((pstCmd->enFrameBufSrc != AX_POOL_SOURCE_PRIVATE)
            && (pstCmd->enFrameBufSrc != AX_POOL_SOURCE_USER)) {
        SAMPLE_CRIT_LOG("Unsupport frame buf source mode:%d\n", pstCmd->enFrameBufSrc);
        ret = -1;
        goto ERR_RET;
    }

    if (pstCmd->uStreamCount > pstCmd->uGrpCount) {
        pstCmd->uGrpCount = pstCmd->uStreamCount;
    }

    return 0;
ERR_RET:

    return ret;
}


AX_S32 VdecCmdLineParseAndCheck(AX_S32 argc, AX_CHAR **argv, SAMPLE_VDEC_CMD_PARAM_T *pstPara,
                                AX_VDEC_GRP VdGrp, AX_BOOL bLink)
{
    SAMPLE_PARAMETER_T prm;
    AX_S32 sRet = 0;
    AX_S32 ret = 0;
    AX_S32 s32Ret = 0;
    AX_CHAR *p;
    SAMPLE_PARAMETER_T *pPrm = &prm;
    SAMPLE_VDEC_CMD_PARAM_T *pstCmd = pstPara;
    AX_CHAR *optarg;
    AX_CHAR arg[VDEC_CMD_MAX_ARG_LEN];
    AX_S32 argLen = 0;
    AX_U32 slen = 0;
    int len;
    int i;

    SAMPLE_OPTION_T *options = NULL;
    SAMPLE_OPTION_NAME_T *pOptName = NULL;

    static AX_BOOL streamcfg_para = AX_FALSE;

    if (argc < 2) {
        __PrintHelp_Com();
        exit(0);
    }

    if (strcmp(argv[1], "-h") == 0) {
        __PrintHelp_Com();
        exit(0);
    }

    for (i = 0; i < argc; i++) {
        SAMPLE_LOG_N("VdGrp=%d, argc:%d, argv[%d]:%s", VdGrp, argc, i, argv[i]);
    }

    ret = __SampleInitOptions(&options, &pOptName);
    if (ret) {
        SAMPLE_CRIT_LOG("VdGrp=%d, __SampleInitOptions FAILED! ret:0x%x", VdGrp, ret);
        goto ERR_RET;
    }

    prm.cnt = 1;
    while ((s32Ret = SampleGetOption(argc, argv, options, &prm)) != -1) {
        if (s32Ret == -2) {
            // SAMPLE_CRIT_LOG("SampleGetOption ret -2");
            sRet = -1;
            goto ERR_RET;
        }

        p = prm.argument;
        optarg = p;

        SAMPLE_LOG_N("VdGrp=%d, pPrm->short_opt:%c, pPrm->longOpt:%s",
                  VdGrp, pPrm->short_opt, pPrm->longOpt);

        switch (pPrm->short_opt) {
        case 'c':
            pstCmd->uGrpCount = atoi(optarg);
            break;
        case 'L':
            pstCmd->sLoopDecNum = atoi(optarg);
            break;
        case 'N':
            pstCmd->sRecvPicNum = atoi(optarg);
            break;
        case 'w':
            pstCmd->sWriteFrames = atoi(optarg);
            break;
        case 'W':
            pstCmd->u32PicWidth = atoi(optarg);
            break;
        case 'H':
            pstCmd->u32PicHeight = atoi(optarg);
            break;
        case 'i':
            pstCmd->pInputFilePath = malloc(AX_VDEC_FILE_PATH_LEN);
            if (pstCmd->pInputFilePath == NULL) {
                SAMPLE_CRIT_LOG("VdGrp=%d, malloc %d Bytes FAILED!\n",
                                VdGrp, AX_VDEC_FILE_PATH_LEN);
                ret = -3;
                goto ERR_RET;
            }

            slen = strlen(optarg);
            if (slen >= AX_VDEC_FILE_PATH_LEN) {
                SAMPLE_CRIT_LOG("VdGrp=%d, (strlen(optarg):%d >= AX_VDEC_FILE_PATH_LEN:%d\n",
                               VdGrp, slen, AX_VDEC_FILE_PATH_LEN);
                ret = -1;
                goto ERR_RET;
            }

            len = snprintf(pstCmd->pInputFilePath, slen + 1, "%s", optarg);
            if ((len < 0) || (len != slen)) {
                SAMPLE_CRIT_LOG("VdGrp=%d, snprintf FAILED! ret:0x%x optarg:%s slen:%d\n",
                                VdGrp, len, optarg, slen);
                ret = -1;
                goto ERR_RET;
            }
            break;
        case 'o':
            pstCmd->pOutputFilePath = malloc(AX_VDEC_FILE_PATH_LEN);
            if (pstCmd->pOutputFilePath == NULL) {
                SAMPLE_CRIT_LOG("VdGrp=%d, malloc %d Bytes FAILED!\n",
                                VdGrp, AX_VDEC_FILE_PATH_LEN);
                ret = -3;
                goto ERR_RET;
            }

            slen = strlen(optarg);
            if (slen >= AX_VDEC_FILE_DIR_LEN) {
                SAMPLE_CRIT_LOG("VdGrp=%d, (strlen(optarg):%d >= AX_VDEC_FILE_DIR_LEN:%d\n",
                                VdGrp, slen, AX_VDEC_FILE_DIR_LEN);
                ret = -1;
                goto ERR_RET;
            }

            len = snprintf(pstCmd->pOutputFilePath, slen + 1, "%s", optarg);
            if ((len < 0) || (len != slen)) {
                SAMPLE_CRIT_LOG("VdGrp=%d, snprintf FAILED! ret:0x%x optarg:%s slen:%d\n",
                                VdGrp, len, optarg, slen);
                ret = -1;
                goto ERR_RET;
            }
            break;
        case 'M':
            pstCmd->enInputMode = atoi(optarg);
            break;
        case 's':
            pstCmd->sStreamSize = atoi(optarg);
            break;
        case 'T':
            pstCmd->enDecType = atoi(optarg);
            break;
        case 'j':
            pstCmd->bJpegDecOneFrm = atoi(optarg);
            break;
        case 'q':
            pstCmd->bQuitWait = atoi(optarg);
            break;
        case 'h':
            ret = -1;
            break;
        case '0': {
            if (strcmp(pPrm->longOpt, "sMilliSec") == 0) {
                pstCmd->sMilliSec = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "sGetMilliSec") == 0) {
                pstCmd->sGetMilliSec = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "res") == 0) {
                argLen = (strlen(optarg) + 1) < VDEC_CMD_MAX_ARG_LEN ?
                            (strlen(optarg) + 1) : VDEC_CMD_MAX_ARG_LEN;
                memcpy(arg, optarg, argLen);
                if (__ParseResParams(optarg, &pstCmd->u32PicWidth,
                                            &pstCmd->u32PicHeight)) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, Illegal resolution argument: --res=%s\n",
                                    VdGrp, arg);
                    ret = 1;
                    goto ERR_RET;
                }
            } else if (strcmp(pPrm->longOpt, "u32FrameHeight") == 0) {
                pstCmd->u32FrameHeight = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "usrPicFile") == 0) {
                pstCmd->pUsrPicFilePath = malloc(AX_VDEC_FILE_PATH_LEN);
                if (pstCmd->pUsrPicFilePath == NULL) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, malloc %d Bytes FAILED!\n",
                                    VdGrp, AX_VDEC_FILE_PATH_LEN);
                    ret = -3;
                    goto ERR_RET;
                }

                slen = strlen(optarg);
                if (slen >= AX_VDEC_FILE_PATH_LEN) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, (strlen(optarg):%d >= AX_VDEC_FILE_PATH_LEN:%d\n",
                                   VdGrp, slen, AX_VDEC_FILE_PATH_LEN);
                    ret = -1;
                    goto ERR_RET;
                }

                len = snprintf(pstCmd->pUsrPicFilePath, slen + 1, "%s", optarg);
                if ((len < 0) || (len != slen)) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, snprintf FAILED! ret:0x%x optarg:%s slen:%d\n",
                                    VdGrp, len, optarg, slen);
                    ret = -1;
                    goto ERR_RET;
                }

                pstCmd->bUserPicEnable = AX_TRUE;
            } else if (strcmp(pPrm->longOpt, "usrPicIdx") == 0) {
               pstCmd->usrPicIdx = atoi(optarg);
               if (pstCmd->usrPicIdx == 0) {
                   pstCmd->usrPicIdx++;
                   SAMPLE_LOG_TMP("usrPicIdx should > 0, modify usrPicIdx = 1\n");
               }
            } else if (strcmp(pPrm->longOpt, "bDynRes") == 0) {
               pstCmd->bDynRes = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "newInput") == 0) {
                pstCmd->pNewInputFilePath = malloc(AX_VDEC_FILE_PATH_LEN);
                if (pstCmd->pNewInputFilePath == NULL) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, malloc %d Bytes FAILED!\n",
                                    VdGrp, AX_VDEC_FILE_PATH_LEN);
                    ret = -3;
                    goto ERR_RET;
                }

                slen = strlen(optarg);
                if (slen >= AX_VDEC_FILE_PATH_LEN) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, (strlen(optarg):%d >= AX_VDEC_FILE_PATH_LEN:%d\n",
                                   VdGrp, slen, AX_VDEC_FILE_PATH_LEN);
                    ret = -1;
                    goto ERR_RET;
                }

                len = snprintf(pstCmd->pNewInputFilePath, slen + 1, "%s", optarg);
                if ((len < 0) || (len != slen)) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, snprintf FAILED! ret:0x%x optarg:%s slen:%d\n",
                                    VdGrp, len, optarg, slen);
                    ret = -1;
                    goto ERR_RET;
                }
            } else if (strcmp(pPrm->longOpt, "highRes") == 0) {
               pstCmd->highRes = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bUsrInstant") == 0) {
               pstCmd->bUsrInstant = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "recvStmAfUsrPic") == 0) {
               pstCmd->recvStmAfUsrPic = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bPerfTest") == 0) {
               pstCmd->bPerfTest = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bCheckFrmParam") == 0) {
               pstCmd->bCheckFrmParam = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetUserData") == 0) {
               pstCmd->bGetUserData = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetRbInfo") == 0) {
               pstCmd->bGetRbInfo = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetGrpAtrr") == 0) {
               pstCmd->bGetGrpAtrr = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bQueryStatus") == 0) {
               pstCmd->bQueryStatus = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetVuiParam") == 0) {
               pstCmd->bGetVuiParam = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bResetGrp") == 0) {
               pstCmd->bResetGrp = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bSkipRelease") == 0) {
               pstCmd->bSkipRelease = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bSkipFrms") == 0) {
               pstCmd->bSkipFrms = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bSleep") == 0) {
               pstCmd->bSleep = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetDispMode") == 0) {
               pstCmd->bGetDispMode = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bGetGrpPrm") == 0) {
               pstCmd->bGetGrpPrm = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bRepeatTest") == 0) {
               pstCmd->bRepeatTest = atoi(optarg);
            }else if (strcmp(pPrm->longOpt, "outputfile") == 0) {
                pstCmd->pOutputFilePath = malloc(AX_VDEC_FILE_PATH_LEN);
                if (pstCmd->pOutputFilePath == NULL) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, malloc %d Bytes FAILED!\n",
                                    VdGrp, AX_VDEC_FILE_PATH_LEN);
                    ret = -3;
                    goto ERR_RET;
                }

                slen = strlen(optarg);
                if (slen >= AX_VDEC_FILE_DIR_LEN) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, (strlen(optarg):%d >= AX_VDEC_FILE_DIR_LEN:%d\n",
                                VdGrp, slen, AX_VDEC_FILE_DIR_LEN);
                    ret = -1;
                    goto ERR_RET;
                }

                len = snprintf(pstCmd->pOutputFilePath, slen + 1, "%s", optarg);
                if ((len < 0) || (len != slen)) {
                    SAMPLE_CRIT_LOG("VdGrp=%d, snprintf FAILED! ret:0x%x optarg:%s slen:%d\n",
                                    VdGrp, len, optarg, slen);
                    ret = -1;
                    goto ERR_RET;
                }
                break;
            } else if (strcmp(pPrm->longOpt, "tb_cfg_file") == 0) {
                pstCmd->pTbCfgFilePath = optarg;
                // SAMPLE_LOG("pstCmd->tb_cfg_file: %s\n", pstCmd->tb_cfg_file);
            } else if (strcmp(pPrm->longOpt, "enDisplayMode") == 0) {
                pstCmd->enDisplayMode = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "multimode") == 0) {
                pstCmd->enMultimode = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "select") == 0) {
                pstCmd->bSelectMode = atoi(optarg);
                if (pstCmd->bSelectMode) {
                    pstCmd->sGetMilliSec = 0;
                }
            } else if (strcmp(pPrm->longOpt, "streamcfg") == 0) {
                pstCmd->pGrpCmdlFile[pstCmd->uStreamCount] = optarg;
                streamcfg_para = AX_TRUE;
                SAMPLE_LOG("VdGrp=%d, pstCmd->pGrpCmdlFile[pstCmd->uStreamCount:%d]:%s",
                           VdGrp, pstCmd->uStreamCount,
                           pstCmd->pGrpCmdlFile[pstCmd->uStreamCount]);
                pstCmd->uStreamCount++;
            } else if (strcmp(pPrm->longOpt, "nstream") == 0) {
                pstCmd->uStreamCount = atoi(optarg);
                break;
            } else if (strcmp(pPrm->longOpt, "uStartGrpId") == 0) {
                pstCmd->uStartGrpId = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "enDecType") == 0) {
                pstCmd->enDecType = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "uOutputFifoDepth") == 0) {
                pstCmd->u32OutputFifoDepth = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "u32FrameBufCnt") == 0) {
                pstCmd->u32FrameBufCnt = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "enFrameBufSrc") == 0) {
                pstCmd->enFrameBufSrc = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "u32StreamFps") == 0) {
                pstCmd->u32StreamFps = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bFfmpegEnable") == 0) {
                pstCmd->bFfmpegEnable = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "enVideoMode") == 0) {
                pstCmd->enVideoMode = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "enOutOrder") == 0) {
                pstCmd->enOutOrder = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "waitTime") == 0) {
                pstCmd->waitTime = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bOpenIvps") == 0) {
                pstCmd->bOpenIvps = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "voType") == 0) {
                pstCmd->voType = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "voDev") == 0) {
                pstCmd->voDev = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "voRes") == 0) {
                pstCmd->voRes = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "voGraphicOpen") == 0) {
                pstCmd->voGraphicOpen = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "pollingEna") == 0) {
                pstCmd->pollingEna = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "pollingCnt") == 0) {
                pstCmd->pollingCnt = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "pollingTime") == 0) {
                pstCmd->pollingTime = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bUserPts") == 0) {
                pstCmd->bUserPts = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bMultiLck") == 0) {
                pstCmd->bMultiLck = atoi(optarg);
            } else if (strcmp(pPrm->longOpt, "bCreatGrpEx") == 0) {
                pstCmd->bCreatGrpEx = atoi(optarg);
            } else {
                if ((pstCmd->uStreamCount > 0) && (pstCmd->uStreamCount < AX_VDEC_MAX_GRP_NUM)) {
                    AX_CHAR StreamCfgName[AX_VDEC_FILE_NAME_LEN];

                    if (streamcfg_para == AX_FALSE) {
                        for (i = 0; i < pstCmd->uStreamCount; i++) {
                            memset(StreamCfgName, 0, sizeof(StreamCfgName));
                            sprintf(StreamCfgName, "grp_cmd_%d", i);
                            if (strcmp(pPrm->longOpt, StreamCfgName) == 0) {
                                pstCmd->pGrpCmdlFile[i] = optarg;
                                continue;
                            }
                        }
                        break;
                    } else {
                        for (i = 0; i < pstCmd->uStreamCount; i++) {
                            memset(StreamCfgName, 0, sizeof(StreamCfgName));
                            sprintf(StreamCfgName, "grp_cmd_%d", i);

                            if (strcmp(pPrm->longOpt, StreamCfgName) == 0) {
                                SAMPLE_CRIT_LOG("VdGrp=%d, pGrpCmdlFile[0]:%s and %s:%s "
                                                "cannot coexist! uStreamCount:%d",
                                                VdGrp, pstCmd->pGrpCmdlFile[0],
                                                StreamCfgName, optarg, pstCmd->uStreamCount);
                                break;
                            }
                        }

                        if (optopt == 0) {
                            break;
                        }
                    }
                } else {
                    SAMPLE_LOG("pstCmd->uStreamCount:%d", pstCmd->uStreamCount);
                }

                SAMPLE_CRIT_LOG("VdGrp=%d, Unknown option character 0x%x ", VdGrp, optopt);
                ret = 1;
                goto ERR_RET;
            }
            break;
        }
        case '?':
            if (isprint(optopt))
                SAMPLE_CRIT_LOG("Unknown option `-%c'.\n", optopt);
            else
                SAMPLE_CRIT_LOG("Unknown option character `\\x%x'.\n", optopt);
            ret = 1;
            goto ERR_RET;
        default:
            SAMPLE_CRIT_LOG("VdGrp=%d, unknow options:%c.\n", VdGrp, prm.short_opt);
            ret = -1;
            goto ERR_RET;
        }
    }

    __VdecParameterAdjust(pstCmd);
    if (__VdecParameterCheck(pstCmd, VdGrp) != 0) {
        SAMPLE_CRIT_LOG("VdGrp=%d, __VdecParameterCheck FAILED!", VdGrp);
        sRet = -1;
    }

    __VdecCmdParaPrint(pstCmd, VdGrp);

ERR_RET:
    if (sRet || ret) {
        __PrintHelp_Com();
    }

    if (options != NULL) {
        free(options);
    }

    if (pOptName != NULL) {
        free(pOptName);
    }

    return sRet || ret;
}


AX_S32 VdecDefaultParamsSet(SAMPLE_VDEC_CMD_PARAM_T *pstCmdPara)
{
    SAMPLE_VDEC_CMD_PARAM_T *pstCmd = pstCmdPara;
    if (pstCmd == NULL) {
        SAMPLE_CRIT_LOG("pstCmd == NULL");
        return -1;
    }
    memset(pstCmd, 0, sizeof(SAMPLE_VDEC_CMD_PARAM_T));

    pstCmd->uGrpCount = 1;
    pstCmd->uStreamCount = 0;
    pstCmd->sLoopDecNum = 1;
    pstCmd->sRecvPicNum = 0;
    pstCmd->sMilliSec = -1;
    pstCmd->sGetMilliSec = -1;
    pstCmd->u32PicWidth = AX_VDEC_MAX_WIDTH;
    pstCmd->u32PicHeight = AX_VDEC_MAX_HEIGHT;
    pstCmd->u32FrameHeight = 0;
    pstCmd->sWriteFrames = 0; // 0: not write, others: write
    pstCmd->bJpegDecOneFrm = AX_FALSE;
    pstCmd->enDisplayMode = AX_VDEC_DISPLAY_MODE_PLAYBACK;
    pstCmd->enMultimode = AX_VDEC_MULTI_MODE_DISABLE;
    pstCmd->bSelectMode = 0;
    pstCmd->enFrameBufSrc = AX_POOL_SOURCE_USER;
    pstCmd->enDecType = PT_H264;
    pstCmd->enInputMode = AX_VDEC_INPUT_MODE_FRAME;
    pstCmd->pInputFilePath = NULL;
    pstCmd->pTbCfgFilePath = NULL;
    pstCmd->pOutputFilePath = NULL;
    pstCmd->pUsrPicFilePath = NULL;
    pstCmd->pNewInputFilePath = NULL;
    pstCmd->sStreamSize = 10 * 1024;
    pstCmd->pollingTime = 10;
    pstCmd->usrPicIdx = 7;
    pstCmd->u32FrameBufCnt = 8;
    pstCmd->uStartGrpId = 0;
    pstCmd->u32StreamFps = 0;
    pstCmd->bFfmpegEnable = AX_FALSE;
    pstCmd->enVideoMode = VIDEO_DEC_MODE_IPB;
    pstCmd->enOutOrder = AX_VDEC_OUTPUT_ORDER_DISP;
    pstCmd->bUserPts = AX_FALSE;
    pstCmd->bMultiLck = AX_FALSE;
    pstCmd->waitTime = 20; /* second */
    pstCmd->bOpenIvps = AX_FALSE;

    pstCmd->bResetGrp = AX_FALSE;
    pstCmd->bQueryStatus = AX_FALSE;
    pstCmd->bGetGrpAtrr = AX_FALSE;
    pstCmd->bCheckFrmParam = AX_FALSE;
    pstCmd->bGetUserData = AX_FALSE;
    pstCmd->bGetRbInfo = AX_FALSE;
    pstCmd->bGetVuiParam = AX_FALSE;
    pstCmd->bGetDispMode = AX_FALSE;
    pstCmd->bGetGrpPrm = AX_FALSE;
    pstCmd->bRepeatTest = AX_FALSE;
    pstCmd->bSleep = AX_FALSE;
    pstCmd->bSkipRelease = AX_FALSE;
    pstCmd->bSkipFrms = AX_FALSE;
    pstCmd->bCreatGrpEx = AX_FALSE;

    return 0;
}


/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_OPAL_API_H_
#define _AX_OPAL_API_H_

#include "ax_opal_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get opal version
///
/// @param NA
///
/// @return version info
//////////////////////////////////////////////////////////////////////////////////////
const AX_CHAR *AX_OPAL_GetVersion(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get chip type
///
/// @param NA
///
/// @return chip type
//////////////////////////////////////////////////////////////////////////////////////
AX_OPAL_CHIP_TYPE_E AX_OPAL_GetChipType(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief initialize opal sdk
///
/// @param pstAttr      [I]: opal attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Init(const AX_OPAL_ATTR_T * pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief uninitialize opal sdk
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Deinit(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief start opal
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Start(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief stop opal
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Stop(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get sensor attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [I]: sensor attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsAttr(AX_S32 nSnsId, AX_OPAL_VIDEO_SNS_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [I]: sensor attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsAttr(AX_S32 nSnsId, const AX_OPAL_VIDEO_SNS_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get sensor soft photo sensitivity attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [O]: sensor soft photo sensitivity attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsSoftPhotoSensitivityAttr(AX_S32 nSnsId, AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor soft photo sensitivity attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [O]: sensor soft photo sensitivity attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsSoftPhotoSensitivityAttr(AX_S32 nSnsId, const AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video sensor soft photo sensitivity callback
///
/// @param nSnsId       [I]: sensor id
///        callback     [I]: video soft photo sensitivity callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterSnsSoftPhotoSensitivityCallback(AX_S32 nSnsId,
                                                           const AX_OPAL_VIDEO_SNS_SOFTPHOTOSENSITIVITY_CALLBACK callback,
                                                           AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video sensor soft photo sensitivity callback
///
/// @param nSnsId       [I]: sensor id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterSnsSoftPhotoSensitivityCallback(AX_S32 nSnsId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get sensor hot noise balance attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [O]: sensor hot noise balance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsHotNoiseBalanceAttr(AX_S32 nSnsId, AX_OPAL_SNS_HOTNOISEBALANCE_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor hot noise balance attribute
///
/// @param nSnsId       [I]: sensor id
///        pstAttr      [O]: sensor hot noise balance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsHotNoiseBalanceAttr(AX_S32 nSnsId, const AX_OPAL_SNS_HOTNOISEBALANCE_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get video channel attribute
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstAttr      [O]: video channel attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetChnAttr(AX_S32 nSnsId, AX_S32 nChnId, AX_OPAL_VIDEO_CHN_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief set video channel attribute
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstAttr      [I]: video channel attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetChnAttr(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_VIDEO_CHN_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video packet callback
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        callback     [I]: video packet callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterPacketCallback(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_VIDEO_PKT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video packet callback
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterPacketCallback(AX_S32 nSnsId, AX_S32 nChnId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video request IDR
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RequestIDR(AX_S32 nSnsId, AX_S32 nChnId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video snapshot
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pImageBuf    [O]: image buffer
///        nImageBufSize[I]: image buffer size
///        pActSize     [O]: actual image size
///        nQpLevel     [I]: qp level
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_Snapshot(AX_S32 nSnsId, AX_S32 nChnId, AX_VOID *pImageBuf, AX_U32 nImageBufSize, AX_U32 *pActSize, AX_U32 nQpLevel);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video snapshot
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pImageBuf    [O]: frame buffer
///        nImageBufSize[I]: frame buffer size
///        pActSize     [O]: actual image size
///        nWidth       [I]: frame width
///        nHeight      [I]: frame height
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_CaptureFrame(AX_S32 nSnsId, AX_S32 nChnId, AX_VOID *pFrameBuf, AX_U32 nImageBufSize, AX_U32 *pActSize, AX_U32 nWidth, AX_U32 nHeight);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get svc param
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstParam     [O]: svc parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSvcParam(AX_S32 nSnsId, AX_S32 nChnId, AX_OPAL_VIDEO_SVC_PARAM_T* pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set svc param
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstParam     [I]: svc parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSvcParam(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_VIDEO_SVC_PARAM_T* pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set svc region
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstRegion    [O]: svc region
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSvcRegion(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_VIDEO_SVC_REGION_T* pstRegion);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get algorithm parameter
///
/// @param nSnsId       [I]: sensor id
///        pstParam     [O]: algorithm parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoGetParam(AX_S32 nSnsId, AX_OPAL_ALGO_PARAM_T* pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set algorithm parameter
///
/// @param nSnsId       [I]: sensor id
///        pstParam     [I]: algorithm parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoSetParam(AX_S32 nSnsId, const AX_OPAL_ALGO_PARAM_T* pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video algorithm callback
///
/// @param nSnsId       [I]: sensor id
///        callback     [I]: video algorithm callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterAlgoCallback(AX_S32 nSnsId, const AX_OPAL_VIDEO_ALGO_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video algorithm callback
///
/// @param nSnsId       [I]: sensor id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterAlgoCallback(AX_S32 nSnsId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video add one create
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        pstAttr      [I]: osd attribute
///        pOsdHandle   [O]: osd handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdCreate(AX_S32 nSnsId, AX_S32 nChnId, const AX_OPAL_OSD_ATTR_T* pstAttr, AX_OPAL_HANDLE *pOsdHandle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video update osd
///
/// @param nSnsId       [I]: sensor id
///        pOsdHandle   [I]: osd handle
///        pstAttr      [I]: osd attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdUpdate(AX_S32 nSnsId, AX_S32 nChnId, AX_OPAL_HANDLE OsdHandle, const AX_OPAL_OSD_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video destroy osd
///
/// @param pOsdHandle   [I]: osd handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDestroy(AX_S32 nSnsId, AX_S32 nChnId, AX_OPAL_HANDLE OsdHandle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video draw osd rect
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        nRectSize    [I]: rect size
///        pstRect      [I]: rect item
///        nLineWidth   [I]: line width
///        nARGB        [I]: alpha & color
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDrawRect(AX_S32 nSnsId, AX_S32 nChnId, AX_U32 nRectSize, const AX_OPAL_RECT_T* pstRects, AX_U32 nLineWidth, AX_U32 nARGB);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video osd rect clear
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdClearRect(AX_S32 nSnsId, AX_S32 nChnId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video draw osd polygon
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///        nPolygonSize [I]: polygon size
///        pstPolygon   [I]: polygon item
///        nLineWidth   [I]: line width
///        nARGB        [I]: alpha & color
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDrawPolygon(AX_S32 nSnsId, AX_S32 nChnId, AX_U32 nPolygonSize, const AX_OPAL_POLYGON_T* pstPolygons, AX_U32 nLineWidth, AX_U32 nARGB);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video osd polygon clear
///
/// @param nSnsId       [I]: sensor id
///        nChnId       [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdClearPolygon(AX_S32 nSnsId, AX_S32 nChnId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get attribute
///
/// @param pstAttr      [O]: audio attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetAttr(AX_OPAL_AUDIO_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set attribute
///
/// @param pstAttr      [I]: audio attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetAttr(const AX_OPAL_AUDIO_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio buffer play
///
/// @param nChnId       [I]: audio channel
///        audio type   [I]: audio type
///        pData        [I]: audio data
///        nDataSize    [I]: audio data size
///        nChnId       [I]: audio channel
///
/// @return 0 if success, otherwise failure
AX_S32 AX_OPAL_Audio_Play(AX_S32 nChnId, AX_PAYLOAD_TYPE_E eType, const AX_U8 *pData, AX_U32 nDataSize);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio file play
///
/// @param nChnId       [I]: audio channel
///        eType        [I]: audio type
///        pstrFileName [I]: audio filename
///        nLoop        [I]: audio play times
///        callback     [I]: audio play file callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
AX_S32 AX_OPAL_Audio_PlayFile(AX_S32 nChnId, AX_PAYLOAD_TYPE_E eType, const AX_CHAR *pstrFileName, AX_S32 nLoop,
                              AX_OPAL_AUDIO_PLAYFILERESULT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio play stop play
///
/// @param nChnId       [I]: audio channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_StopPlay(AX_S32 nChnId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get capture volume
///
/// @param pVol         [O]: audio capture volume
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetCapVolume(AX_F32 *pVol);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set capture volume
///
/// @param nVol         [I]: audio capture volume
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetCapVolume(AX_F32 fVol);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get play volume
///
/// @param pVol         [O]: audio play volume
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetPlayVolume(AX_F32 *pVol);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set Play volume
///
/// @param nVol         [I]: audio play volume
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetPlayVolume(AX_F32 fVol);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get capture pipe encoder attribute
///
/// @param nChnId       [I]: audio capture pipe channel
///        pstAttr      [O]: audio capture pipe encoder attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetEncoderAttr(AX_S32 nChnId, AX_OPAL_AUDIO_ENCODER_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get play pipe channel attribute
///
/// @param nChnId       [I]: audio play pipe channel
///        pstAttr      [O]: audio play pipe attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetPlayPipeAttr(AX_S32 nChnId, AX_OPAL_AUDIO_PLAY_CHN_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set play pipe channel attribute
///
/// @param nChnId       [I]: audio play pipe channel
///        pstAttr      [I]: audio play pipe attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetPlayPipeAttr(AX_S32 nChnId, const AX_OPAL_AUDIO_PLAY_CHN_ATTR_T* pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register audio encode pakcet out callback
///
/// @param nChnId       [I]: audio channel
///        callback     [I]: audio encode pakcet out callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_RegisterPacketCallback(AX_S32 nChnId, const AX_OPAL_AUDIO_PKT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister audio encode pakcet out callback
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_UnRegisterPacketCallback(AX_S32 nChnId);

#ifdef __cplusplus
}
#endif

#endif /* _AX_OPAL_API_H_ */

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

#include "ax_global_type.h"
#include "ax_opal_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////
/// @brief initialize opal sdk
///
/// @param pstAttr      [I]: opal attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Init(const AX_OPAL_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief uninitialize opal sdk
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Deinit(AX_VOID);

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
/// @brief get opal hotbalance attribute
///
/// @param pstAttr      [I]: hotbalance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_GetHotBalanceAttr(AX_OPAL_HOTBALANCE_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief set opal hotbalance attribute
///
/// @param pstAttr      [I]: hotbalance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_SetHotBalanceAttr(const AX_OPAL_HOTBALANCE_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register hot balance callback
///
/// @param callback     [I]: hotbalance callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_RegisterHotBalanceCallback(const AX_OPAL_HOTBALANCE_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister hot balance callback
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_UnRegisterHotBalanceCallback(AX_VOID);

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
/// @param eSnsId       [I]: sensor id
///        pstAttr      [I]: sensor attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsAttr(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_SNS_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor attribute
///
/// @param eSnsId       [I]: sensor id
///        pstAttr      [I]: sensor attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsAttr(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_VIDEO_SNS_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get sensor soft photo sensitivity attribute
///
/// @param eSnsId       [I]: sensor id
///        pstAttr      [O]: sensor soft photo sensitivity attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsSoftPhotoSensitivityAttr(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor soft photo sensitivity attribute
///
/// @param eSnsId       [I]: sensor id
///        pstAttr      [O]: sensor soft photo sensitivity attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsSoftPhotoSensitivityAttr(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_SNS_SOFT_PHOTOSENSITIVITY_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video sensor soft photo sensitivity callback
///
/// @param eSnsId       [I]: sensor id
///        callback     [I]: video soft photo sensitivity callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterSnsSoftPhotoSensitivityCallback(AX_OPAL_SNS_ID_E eSnsId,
                                                           const AX_OPAL_VIDEO_SNS_SOFTPHOTOSENSITIVITY_CALLBACK callback,
                                                           AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video sensor soft photo sensitivity callback
///
/// @param eSnsId       [I]: sensor id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterSnsSoftPhotoSensitivityCallback(AX_OPAL_SNS_ID_E eSnsId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get sensor hot noise balance attribute
///
/// @param eSnsId       [I]: sensor id
///        pstAttr      [O]: sensor hot noise balance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSnsHotNoiseBalanceAttr(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_SNS_HOTNOISEBALANCE_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set sensor hot noise balance attribute
///
/// @param eSnsId       [I]: sensor id
///        pstAttr      [O]: sensor hot noise balance attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSnsHotNoiseBalanceAttr(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_SNS_HOTNOISEBALANCE_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief get video channel attribute
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstAttr      [O]: video channel attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetChanAttr(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, AX_OPAL_VIDEO_CHAN_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief set video channel attribute
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstAttr      [I]: video channel attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetChanAttr(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_VIDEO_CHAN_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video packet callback
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        callback     [I]: video packet callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterPacketCallback(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_VIDEO_PKT_CALLBACK callback,
                                            AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video packet callback
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterPacketCallback(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video frame callback
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        callback     [I]: video frame callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterFrameCallback(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_VIDEO_FRAME_CALLBACK callback,
                                           AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video frame callback
///
/// @param eSnsId       [I]: sensor id
///        eChan         [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterFrameCallback(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video request IDR
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RequestIDR(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video snapshot
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pImageBuf    [O]: image buffer
///        nImageBufSize[I]: image buffer size
///        pActSize     [O]: actual image size
///        nQpLevel     [I]: qp level
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_Snapshot(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, AX_VOID *pImageBuf, AX_U32 nImageBufSize,
                              AX_U32 *pActSize, AX_U32 nQpLevel);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get svc param
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstParam     [O]: svc parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_GetSvcParam(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, AX_OPAL_VIDEO_SVC_PARAM_PTR pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set svc param
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstParam     [I]: svc parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSvcParam(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_VIDEO_SVC_PARAM_PTR pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set svc region
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstRegion    [O]: svc region
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_SetSvcRegion(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_VIDEO_SVC_REGION_PTR pstRegion);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video get algorithm parameter
///
/// @param eSnsId       [I]: sensor id
///        pstParam     [O]: algorithm parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoGetParam(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_ALGO_PARAM_PTR pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video set algorithm parameter
///
/// @param eSnsId       [I]: sensor id
///        pstParam     [I]: algorithm parameter
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoSetParam(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_ALGO_PARAM_PTR pstParam);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register video algorithm callback
///
/// @param eSnsId       [I]: sensor id
///        callback     [I]: video algorithm callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_RegisterAlgoCallback(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_VIDEO_ALGO_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister video algorithm callback
///
/// @param eSnsId       [I]: sensor id
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_UnRegisterAlgoCallback(AX_OPAL_SNS_ID_E eSnsId);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video algorithm capture face recognize infomation
///
/// @param eSnsId       [I]: sensor id
///        bCaptureJpg  [I]: whether capture face image
///        callback     [I]: callback function
///        pErrStatus   [O]: error status
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoCaptureFaceRecognizeInfo(AX_OPAL_SNS_ID_E eSnsId, AX_BOOL bCaptureJpg,
                                                  AX_OPAL_VIDEO_ALGO_CAPTUREFACERECOGNIZE_CALLBACK callback,
                                                  AX_OPAL_ALGO_FACE_RECOGNIZE_ERR_CODE_E *peErrCode, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video algorithm set face recognize info
///
/// @param eSnsId       [I]: sensor id
///        nId          [I]: recognize Id
///        pstOp        [I]: operation
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoSetFaceRecognizeInfo(AX_OPAL_SNS_ID_E eSnsId, AX_U32 nId, const AX_OPAL_ALGO_FACE_FEATRUE_OP_PTR pstOp);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video algorithm get face recognize list
///
/// @param eSnsId                [I]: sensor id
///        ppstFaceRecognizeList [O]: face recognize list
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoGetFaceRecognizeList(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_ALGO_FACE_RECOGNIZE_LIST_T **ppstFaceRecognizeList);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video algorithm release face recognize list
///
/// @param eSnsId                [I]: sensor id
///        pstFaceRecognizeList  [I]: face recognize list
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoReleaseFaceRecognizeList(AX_OPAL_SNS_ID_E eSnsId, const AX_OPAL_ALGO_FACE_RECOGNIZE_LIST_PTR pstFaceRecognizeList);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video algorithm delete face recognize info
///
/// @param eSnsId                [I]: sensor id
///        bCleanup              [I]: whether cleanup
///        pstrRecognizeName     [I]: recognize name
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_AlgoDeleteFaceRecognizeInfo(AX_OPAL_SNS_ID_E eSnsId, AX_BOOL bCleanup, const AX_CHAR *pstrRecognizeName);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video add one create
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        pstAttr      [I]: osd attribute
///        pOsdHandle   [O]: osd handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdCreate(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, const AX_OPAL_OSD_ATTR_PTR pstAttr,
                               AX_OPAL_HANDLE *pOsdHandle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video update osd
///
/// @param eSnsId       [I]: sensor id
///        pOsdHandle   [I]: osd handle
///        pstAttr      [I]: osd attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdUpdate(AX_OPAL_HANDLE OsdHandle, const AX_OPAL_OSD_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video destroy osd
///
/// @param pOsdHandle   [I]: osd handle
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDestroy(AX_OPAL_HANDLE OsdHandle);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video draw osd rect
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        nRectSize    [I]: rect size
///        pstRect      [I]: rect item
///        nLineWidth   [I]: line width
///        nARGB        [I]: alpha & color
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDrawRect(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, AX_U32 nRectSize, const AX_OPAL_RECT_PTR pstRects,
                                 AX_U32 nLineWidth, AX_U32 nARGB);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video osd rect clear
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdClearRect(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video draw osd polygon
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///        nPolygonSize [I]: polygon size
///        pstPolygon   [I]: polygon item
///        nLineWidth   [I]: line width
///        nARGB        [I]: alpha & color
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdDrawPolygon(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan, AX_U32 nPolygonSize,
                                    const AX_OPAL_POLYGON_PTR pstPolygons, AX_U32 nLineWidth, AX_U32 nARGB);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief video osd polygon clear
///
/// @param eSnsId       [I]: sensor id
///        eChan        [I]: video channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Video_OsdClearPolygon(AX_OPAL_SNS_ID_E eSnsId, AX_OPAL_VIDEO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get attribute
///
/// @param pstAttr      [O]: audio attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetAttr(AX_OPAL_AUDIO_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set attribute
///
/// @param pstAttr      [I]: audio attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetAttr(const AX_OPAL_AUDIO_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio buffer play
///
/// @param eChan        [I]: audio channel
///        audio type   [I]: audio type
///        pData        [I]: audio data
///        nDataSize    [I]: audio data size
///        eChan        [I]: audio channel
///
/// @return 0 if success, otherwise failure
AX_S32 AX_OPAL_Audio_Play(AX_OPAL_AUDIO_CHAN_E eChan, AX_PAYLOAD_TYPE_E eType, const AX_U8 *pData, AX_U32 nDataSize);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio file play
///
/// @param eChan        [I]: audio channel
///        eType        [I]: audio type
///        pstrFileName [I]: audio filename
///        nLoop        [I]: audio play times
///        callback     [I]: audio play file callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
AX_S32 AX_OPAL_Audio_PlayFile(AX_OPAL_AUDIO_CHAN_E eChan, AX_PAYLOAD_TYPE_E eType, const AX_CHAR *pstrFileName, AX_S32 nLoop,
                              AX_OPAL_AUDIO_PLAYFILERESULT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio play stop play
///
/// @param eChan        [I]: audio channel
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_StopPlay(AX_OPAL_AUDIO_CHAN_E eChan);

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
/// @param eChan        [I]: audio capture pipe channel
///        pstAttr      [O]: audio capture pipe encoder attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetEncoderAttr(AX_OPAL_AUDIO_CHAN_E eChan, AX_OPAL_AUDIO_ENCODER_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get play pipe channel attribute
///
/// @param eChan        [I]: audio play pipe channel
///        pstAttr      [O]: audio play pipe attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetPlayPipeAttr(AX_OPAL_AUDIO_CHAN_E eChan, AX_OPAL_AUDIO_PLAY_CHAN_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio set play pipe channel attribute
///
/// @param eChan        [I]: audio play pipe channel
///        pstAttr      [I]: audio play pipe attribute
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetPlayPipeAttr(AX_OPAL_AUDIO_CHAN_E eChan, const AX_OPAL_AUDIO_PLAY_CHAN_ATTR_PTR pstAttr);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get aac encoder config buffer
///
/// @param ppConfBuf    [O]: audio aac encoder config buffer
///        pDataSize    [I]: audio encoder config buffer size
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_GetAacEncoderConfigBuf(AX_OPAL_AUDIO_CHAN_E eChan, const AX_U8 **ppConfBuf, AX_U32 *pDataSize);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief audio get aac encoder config buffer
///
/// @param pConfBuf     [I]: audio encoder config buffer
///        nDataSize    [I]: audio encoder config buffer size
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_SetAacDecoderConfigBuf(AX_OPAL_AUDIO_CHAN_E eChan, const AX_U8 *pConfBuf, AX_U32 nDataSize);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register audio catpure in frame callback
///
/// @param callback     [I]: audio capture in frame callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_RegisterCapInFrameCallback(const AX_OPAL_AUDIO_FRAME_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister audio catpure in frame callback
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_UnRegisterCapInFrameCallback(AX_VOID);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register audio encode pakcet out callback
///
/// @param eChan        [I]: audio channel
///        callback     [I]: audio encode pakcet out callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_RegisterPacketCallback(AX_OPAL_AUDIO_CHAN_E eChan, const AX_OPAL_AUDIO_PKT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister audio encode pakcet out callback
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_UnRegisterPacketCallback(AX_OPAL_AUDIO_CHAN_E eChan);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief register audio detect result callback
///
/// @param callback     [I]: audio detect result callback
///        pUserData    [I]: user data
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_RegisterDetectResultCallback(const AX_OPAL_AUDIO_DETECTRESULT_CALLBACK callback, AX_VOID *pUserData);

//////////////////////////////////////////////////////////////////////////////////////
/// @brief unregister audio detect result callback
///
/// @param NA
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_OPAL_Audio_UnRegisterDetectResultCallback(AX_VOID);

#ifdef __cplusplus
}
#endif

#endif /* _AX_SKEL_API_H_ */

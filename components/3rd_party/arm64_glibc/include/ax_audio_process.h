/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef _AX_AUDIO_PROCESS_H__
#define _AX_AUDIO_PROCESS_H__

#include "ax_base_type.h"
#include "ax_global_type.h"

typedef enum axAEC_MODE_E {
    AX_AEC_MODE_DISABLE = 0,
    AX_AEC_MODE_FLOAT,
    AX_AEC_MODE_FIXED,
} AX_AEC_MODE_E;

typedef enum axSUPPRESSION_LEVEL_E {
    AX_SUPPRESSION_LEVEL_LOW = 0,
    AX_SUPPRESSION_LEVEL_MODERATE,
    AX_SUPPRESSION_LEVEL_HIGH
} AX_SUPPRESSION_LEVEL_E;

typedef struct axAEC_FLOAT_CONFIG_T {
    AX_SUPPRESSION_LEVEL_E enSuppressionLevel;
} AX_AEC_FLOAT_CONFIG_T;

// Recommended settings for particular audio routes. In general, the louder
// the echo is expected to be, the higher this value should be set. The
// preferred setting may vary from device to device.
typedef enum axROUTING_MODE_E {
    AX_ROUTING_MODE_QUITE_EARPIECE_OR_HEADSET = 0,
    AX_ROUTING_MODE_EARPIECE,
    AX_ROUTING_MODE_LOUD_EARPIECE,
    AX_ROUTING_MODE_SPEAKERPHONE,
    AX_ROUTING_MODE_LOUD_SPEAKERPHONE
} AX_ROUTING_MODE_E;

typedef struct axAEC_FIXED_CONFIG_T {
    AX_ROUTING_MODE_E eRoutingMode;
} AX_AEC_FIXED_CONFIG_T;

typedef struct axAEC_CONFIG_T {
    AX_AEC_MODE_E enAecMode;
    union {
        /*0 ~ 2 default 0*/
        AX_AEC_FLOAT_CONFIG_T stAecFloatCfg;
        /*0 ~ 4 default 3*/
        AX_AEC_FIXED_CONFIG_T stAecFixedCfg;
    };
} AX_AEC_CONFIG_T;

// Determines the aggressiveness of the suppression. Increasing the level
// will reduce the noise level at the expense of a higher speech distortion.
typedef enum axAGGRESSIVENESS_LEVEL {
    AX_AGGRESSIVENESS_LEVEL_LOW = 0,
    AX_AGGRESSIVENESS_LEVEL_MODERATE,
    AX_AGGRESSIVENESS_LEVEL_HIGH,
    AX_AGGRESSIVENESS_LEVEL_VERYHIGH
} AX_AGGRESSIVENESS_LEVEL_E;

typedef struct axNS_CONFIG_T {
    AX_BOOL bNsEnable;
    /*0 ~ 3 default 2*/
    AX_AGGRESSIVENESS_LEVEL_E enAggressivenessLevel;
} AX_NS_CONFIG_T;

typedef enum axAGC_MODE_E {
    // Adaptive mode intended for use if an analog volume control is available
    // on the capture device. It will require the user to provide coupling
    // between the OS mixer controls and AGC through the |stream_analog_level()|
    // functions.
    //
    // It consists of an analog gain prescription for the audio device and a
    // digital compression stage.
    AX_AGC_MODE_ADAPTIVE_ANALOG = 0,

    // Adaptive mode intended for situations in which an analog volume control
    // is unavailable. It operates in a similar fashion to the adaptive analog
    // mode, but with scaling instead applied in the digital domain. As with
    // the analog mode, it additionally uses a digital compression stage.
    AX_AGC_MODE_ADAPTIVE_DIGITAL,

    // Fixed mode which enables only the digital compression stage also used by
    // the two adaptive modes.
    //
    // It is distinguished from the adaptive modes by considering only a
    // short time-window of the input signal. It applies a fixed gain through
    // most of the input level range, and compresses (gradually reduces gain
    // with increasing level) the input signal at higher levels. This mode is
    // preferred on embedded devices where the capture signal level is
    // predictable, so that a known gain can be applied.
    AX_AGC_MODE_FIXED_DIGITAL
} AX_AGC_MODE_E;

typedef struct axAGC_CONFIG_T {
    AX_BOOL bAgcEnable;
    AX_AGC_MODE_E enAgcMode;
    /*-31 ~ 0 default -3*/
    AX_S16 s16TargetLevel;
    /*0 ~ 90 default 9*/
    AX_S16 s16Gain;
} AX_AGC_CONFIG_T;

typedef struct axAP_UPTALKVQE_ATTR_T {
    AX_S32          s32SampleRate;
    AX_U32          u32FrameSamples;

    AX_AEC_CONFIG_T    stAecCfg;
    AX_NS_CONFIG_T     stNsCfg;
    AX_AGC_CONFIG_T    stAgcCfg;
} AX_AP_UPTALKVQE_ATTR_T;

typedef struct axAP_DNVQE_ATTR_T {
    AX_S32          s32SampleRate;
    AX_U32          u32FrameSamples;

    AX_NS_CONFIG_T     stNsCfg;
    AX_AGC_CONFIG_T    stAgcCfg;
} AX_AP_DNVQE_ATTR_T;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _AX_AUDIO_PROCESS_H__ */

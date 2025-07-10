/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_MIPI_RX_API__
#define __AX_MIPI_RX_API__

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define AX_MIPI_CLK_LANE_MAX        (2)
#define AX_MIPI_LANE_NUM_MAX        (4)
#define AX_LVDS_LANE_NUM_MAX        (4)

typedef enum {
    AX_LANE_COMBO_MODE_0    = 0x0,    /* [mipi 4lane] | [vi only use gpio] */
    AX_LANE_COMBO_MODE_1    = 0x1,    /* [mipi 2lane|1lane + 2lane|1lane] | [vi only use gpio] */
    AX_LANE_COMBO_MODE_2    = 0x2,    /* [mipi 2lane|1lane rx0] | [vi use gpio & rx(rx1)] */
    AX_LANE_COMBO_MODE_3    = 0x3,    /* [mipi no use] | [vi use rx] */
    AX_LANE_COMBO_MODE_MAX
} AX_LANE_COMBO_MODE_E;

typedef enum {
    AX_INPUT_MODE_MIPI = 0,
    AX_INPUT_MODE_SUBLVDS = 1,
    AX_INPUT_MODE_LVDS = 2,
    AX_INPUT_MODE_HISPI = 3,
    AX_INPUT_MODE_SLVS = 4,
    AX_INPUT_MODE_BT601 = 5,
    AX_INPUT_MODE_BT656 = 6,
    AX_INPUT_MODE_BT1120 = 7,
    AX_INPUT_MODE_DVP = 8,
    AX_INPUT_MODE_MAX
} AX_INPUT_MODE_E;

typedef enum {
    AX_MIPI_PHY_TYPE_DPHY = 0,
    AX_MIPI_PHY_TYPE_MAX,
} AX_MIPI_PHY_TYPE_E;

typedef enum {
    AX_MIPI_DATA_LANE_1 = 1,
    AX_MIPI_DATA_LANE_2 = 2,
    AX_MIPI_DATA_LANE_4 = 4,
    AX_MIPI_DATA_LANE_MAX
} AX_MIPI_LANE_NUM_E;

typedef enum {
    AX_SLVDS_DATA_LANE_2 = 2,
    AX_SLVDS_DATA_LANE_4 = 4,
    AX_SLVDS_DATA_LANE_MAX
} AX_SLVDS_LANE_NUM_E;

typedef struct {
    AX_MIPI_PHY_TYPE_E              ePhyMode;
    AX_MIPI_LANE_NUM_E              eLaneNum;
    AX_U32                          nDataRate;
    AX_S8                           nDataLaneMap[AX_MIPI_LANE_NUM_MAX];
    AX_S8                           nClkLane[AX_MIPI_CLK_LANE_MAX];
} AX_MIPI_RX_ATTR_T;

typedef struct {
    AX_SLVDS_LANE_NUM_E             eLaneNum;
    AX_U32                          nDataRate;
    AX_S8                           nDataLaneMap[AX_LVDS_LANE_NUM_MAX];
    AX_S8                           nClkLane[AX_MIPI_CLK_LANE_MAX];
} AX_LVDS_ATTR_T;

typedef enum {
    AX_BT_CLK_MODE_SDR = 0, //Single edge mode
    AX_BT_CLK_MODE_DDR = 1, //Double edge mode
} AX_BT_CLK_MODE_E;

typedef struct {
    AX_INPUT_MODE_E         eInputMode;
    union {
        AX_MIPI_RX_ATTR_T   tMipiAttr;
        AX_LVDS_ATTR_T      tLvdsAttr;
        AX_BT_CLK_MODE_E    eBtClkMode;
    };
} AX_MIPI_RX_DEV_T;

AX_S32 AX_MIPI_RX_Init(AX_VOID);
AX_S32 AX_MIPI_RX_DeInit(AX_VOID);
AX_S32 AX_MIPI_RX_SetLaneCombo(AX_LANE_COMBO_MODE_E eMode);

AX_S32 AX_MIPI_RX_Reset(AX_U32 nDevId);
AX_S32 AX_MIPI_RX_SetAttr(AX_U32 nDevId, const AX_MIPI_RX_DEV_T *pDevAttr);
AX_S32 AX_MIPI_RX_GetAttr(AX_U32 nDevId, AX_MIPI_RX_DEV_T *pDevAttr);
AX_S32 AX_MIPI_RX_Start(AX_U32 nDevId);
AX_S32 AX_MIPI_RX_Stop(AX_U32 nDevId);

#ifdef __cplusplus
}
#endif

#endif // __AX_MIPI_RX_API__

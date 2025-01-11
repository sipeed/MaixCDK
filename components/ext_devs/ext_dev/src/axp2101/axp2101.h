#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define AXP2101_CHIP_ID                 (0x47)
#define AXP2101_CHIP_ID_B               (0x4a)

#define AXP2101_STATUS1                 (0x00)
#define AXP2101_STATUS2                 (0x01)
#define AXP2101_VERSION                 (0x03)
#define AXP2101_COM_CFG                 (0x10)
#define AXP2101_CHARGE_GAUGE_WDT_CTRL   (0x18)
#define AXP2101_PWROFF_EN               (0x22)
#define AXP2101_IRQ_OFF_ON_LEVEL_CTRL   (0x27)
#define AXP2101_ADC_DATA_RELUST0        (0x34)
#define AXP2101_ADC_DATA_RELUST1        (0x35)
#define AXP2101_INT_STATUS1             (0x48)
#define AXP2101_ICC_CHG_SET             (0x62)
#define AXP2101_DC_ONOFF_DVM_CTRL       (0x80)
#define AXP2101_DC_VOL0_CTRL            (0x82)
#define AXP2101_DC_VOL1_CTRL            (0x83)
#define AXP2101_DC_VOL2_CTRL            (0x84)
#define AXP2101_DC_VOL3_CTRL            (0x85)
#define AXP2101_DC_VOL4_CTRL            (0x86)
#define AXP2101_LDO_ONOFF_CTRL0         (0x90)
#define AXP2101_LDO_ONOFF_CTRL1         (0x91)
#define AXP2101_LDO_VOL0_CTRL           (0x92)
#define AXP2101_LDO_VOL1_CTRL           (0x93)
#define AXP2101_LDO_VOL2_CTRL           (0x94)
#define AXP2101_LDO_VOL3_CTRL           (0x95)
#define AXP2101_LDO_VOL3_CTRL           (0x95)
#define AXP2101_LDO_VOL4_CTRL           (0x96)
#define AXP2101_LDO_VOL5_CTRL           (0x97)
#define AXP2101_BAT_PERCENT_DATA        (0xA4)

#ifdef __cplusplus
}
#endif

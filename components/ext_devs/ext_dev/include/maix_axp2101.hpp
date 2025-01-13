/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.10.28: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"

namespace maix::ext_dev::axp2101 {

    /**
     * @brief charger status
     * @maixpy maix.ext_dev.axp2101.ChargerStatus
     */
    enum class ChargerStatus {
        CHG_TRI_STATE,   //tri_charge
        CHG_PRE_STATE,   //pre_charge
        CHG_CC_STATE,    //constant charge
        CHG_CV_STATE,    //constant voltage
        CHG_DONE_STATE,  //charge done
        CHG_STOP_STATE,  //not charge
    };

    /**
     * @brief charger current
     * @maixpy maix.ext_dev.axp2101.ChargerCurrent
     */
    enum class ChargerCurrent {
        CHG_CUR_0MA,
        CHG_CUR_100MA = 4,
        CHG_CUR_125MA,
        CHG_CUR_150MA,
        CHG_CUR_175MA,
        CHG_CUR_200MA,
        CHG_CUR_300MA,
        CHG_CUR_400MA,
        CHG_CUR_500MA,
        CHG_CUR_600MA,
        CHG_CUR_700MA,
        CHG_CUR_800MA,
        CHG_CUR_900MA,
        CHG_CUR_1000MA,
    };

    /**
     * @brief power channel
     * @maixpy maix.ext_dev.axp2101.PowerChannel
     */
    enum class PowerChannel {
        DCDC1,
        DCDC2,
        DCDC3,
        DCDC4,
        DCDC5,
        ALDO1,
        ALDO2,
        ALDO3,
        ALDO4,
        BLDO1,
        BLDO2,
        DLDO1,
        DLDO2,
        VBACKUP,
        CPULDO,
    };

    /**
     * @brief power off time
     * @maixpy maix.ext_dev.axp2101.PowerOffTime
     */
    enum class PowerOffTime {
        POWEROFF_4S,
        POWEROFF_6S,
        POWEROFF_8S,
        POWEROFF_10S,
        POWEROFF_DISABLE = 65535,
    };

    /**
     * @brief power on time
     * @maixpy maix.ext_dev.axp2101.PowerOnTime
     */
    enum class PowerOnTime {
        POWERON_128MS,
        POWERON_512MS,
        POWERON_1S,
        POWERON_2S,
    };

    /**
     * Peripheral AXP2101 class
     * @maixpy maix.ext_dev.axp2101.AXP2101
     */
    class AXP2101 {
    public:
        /**
         * @brief AXP2101 constructor.
         * @param i2c_bus i2c bus number.
         * @param addr pmu device addr.
         * @maixpy maix.ext_dev.axp2101.AXP2101.__init__
         */
        AXP2101(int i2c_bus = -1, uint8_t addr = 0x34);
        ~AXP2101();

        /**
         * @brief Initialise the AXP2101.
         * @return err::Err type, if init success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.init
         */
        err::Err init();

        /**
         * @brief Poweroff immediately.
         * @return err::Err type, if init success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.poweroff
         */
        err::Err poweroff();

        /**
         * @brief Is the battery connected.
         * @return bool type, if battery is connected, return true.
         * @maixpy maix.ext_dev.axp2101.AXP2101.is_bat_connect
         */
        bool is_bat_connect();

        /**
         * @brief Is the power adapter connected.
         * @return bool type, if power adapter is connected, return true.
         * @maixpy maix.ext_dev.axp2101.AXP2101.is_vbus_in
         */
        bool is_vbus_in();

        /**
         * @brief Is bat charging.
         * @return bool type, if bat is charging, return true.
         * @maixpy maix.ext_dev.axp2101.AXP2101.is_charging
         */
        bool is_charging();

        /**
         * @brief Get the battery percentage.
         * @return int type, return battery percentage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_bat_percent
         */
        int get_bat_percent();

        /**
         * @brief Get the battery charging status.
         * @return int type, return battery charging status.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_charger_status
         */
        ext_dev::axp2101::ChargerStatus get_charger_status();

        /**
         * @brief Get the battery voltage.
         * @return uint16_t type, return battery voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_bat_vol
         */
        uint16_t get_bat_vol();

        /**
         * @brief Clear interrupt flag.
         * @return err::Err type, if clean success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.clean_irq
         */
        err::Err clean_irq();

        /**
         * @brief Set the battery charging current.
         * @param current The current to be set.
         * The available values are 0mA, 100mA, 125mA, 150mA, 175mA,
         * 200mA, 300mA, 400mA, 500mA, 600mA, 700mA, 800mA, 900mA, and 1000mA.
         * @return err::Err type, if set success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.set_bat_charging_cur
         */
        err::Err set_bat_charging_cur(ext_dev::axp2101::ChargerCurrent current);

        /**
         * @brief Get the battery charging current.
         * @return ChargerCurrent, return the currently set charging current.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_bat_charging_cur
         */
        ext_dev::axp2101::ChargerCurrent get_bat_charging_cur();

        /**
         * @brief Set and get the PMU DCDC1 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 1500mV~3400mV(step 20mV).
         * @return int, return the PMU DCDC1 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.dcdc1
         */
        int dcdc1(int voltage = -1);

        /**
         * @brief Set and get the PMU DCDC2 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~1200mV(step 10mV) and 1220mV~1540mV(step 20mV).
         * @return int, return the PMU DCDC2 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.dcdc2
         */
        int dcdc2(int voltage = -1);

        /**
         * @brief Set and get the PMU DCDC3 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~1200mV(step 10mV) and 1220mV~1540mV(step 20mV).
         * @return int, return the PMU DCDC3 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.dcdc3
         */
        int dcdc3(int voltage = -1);

        /**
         * @brief Set and get the PMU DCDC4 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~1200mV(step 10mV) and 1220mV~1840mV(step 20mV).
         * @return int, return the PMU DCDC4 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.dcdc4
         */
        int dcdc4(int voltage = -1);

        /**
         * @brief Set and get the PMU DCDC5 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 1400mV~3700mV(step 100mV).
         * @return int, return the PMU DCDC5 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.dcdc5
         */
        int dcdc5(int voltage = -1);

        /**
         * @brief Set and get the PMU ALDO1 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU ALDO1 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.aldo1
         */
        int aldo1(int voltage = -1);

        /**
         * @brief Set and get the PMU ALDO2 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU ALDO2 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.aldo2
         */
        int aldo2(int voltage = -1);

        /**
         * @brief Set and get the PMU ALDO3 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU ALDO3 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.aldo3
         */
        int aldo3(int voltage = -1);

        /**
         * @brief Set and get the PMU ALDO4 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU ALDO4 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.aldo4
         */
        int aldo4(int voltage = -1);

        /**
         * @brief Set and get the PMU BLDO1 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU BLDO1 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.bldo1
         */
        int bldo1(int voltage = -1);

        /**
         * @brief Set and get the PMU BLDO2 voltage.
         * @param voltage The voltage to be set,
         * voltage range is 500mV~3500mV(step 100mV).
         * @return int, return the PMU BLDO2 voltage.
         * @maixpy maix.ext_dev.axp2101.AXP2101.bldo2
         */
        int bldo2(int voltage = -1);

        /**
         * @brief Set power-off time, The device will shut down
         * if the power button is held down longer than this time.
         * @param tm The time to be set, you can set it to 4s, 6s, 8s, or 10s.
         * @return err::Err type, if set success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.set_poweroff_time
         */
        err::Err set_poweroff_time(ext_dev::axp2101::PowerOffTime tm);

        /**
         * @brief Get power-off time.
         * @return PowerOffTime, return power-off time.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_poweroff_time
         */
        ext_dev::axp2101::PowerOffTime get_poweroff_time();

        /**
         * @brief Set power-on time, The device will power on 
         * if the power button is held down longer than this time.
         * @param tm The time to be set, you can set it to 128ms, 512ms, 1s, or 2s.
         * @return err::Err type, if set success, return err::ERR_NONE.
         * @maixpy maix.ext_dev.axp2101.AXP2101.set_poweron_time
         */
        err::Err set_poweron_time(ext_dev::axp2101::PowerOnTime tm);

        /**
         * @brief Get power-on time.
         * @return PowerOnTime, return power-on time.
         * @maixpy maix.ext_dev.axp2101.AXP2101.get_poweron_time
         */
        ext_dev::axp2101::PowerOnTime get_poweron_time();

    private:
        err::Err deinit();
        err::Err inline set_register_bit(uint8_t registers, uint8_t bit);
        err::Err inline clear_register_bit(uint8_t registers, uint8_t bit);
        bool inline get_register_bit(uint8_t registers, uint8_t bit);
        bool is_vbus_good(void);
        err::Err enable_power_channel(PowerChannel channel);
        err::Err disable_power_channel(PowerChannel channel);
        bool is_enable_channel(PowerChannel channel);
    };
}
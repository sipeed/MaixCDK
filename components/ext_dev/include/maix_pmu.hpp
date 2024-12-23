/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.10.28: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"

namespace maix::ext_dev::pmu {

/**
 * @brief charger status
 * @maixpy maix.ext_dev.pmu.ChargerStatus
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
 * @brief power channel
 * @maixpy maix.ext_dev.pmu.PowerChannel
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
 * PMU driver class
 * @maixpy maix.ext_dev.pmu.PMU
 */
class PMU {

public:
    /**
     * @brief Construct a new PMU object, will open PMU.
     *
     * @param driver driver name, only support "axp2101".
     * @param i2c_bus i2c bus number. Automatically selects the on-board pmu when -1 is passed in.
     * @param addr PMU i2c addr.
     *
     * @maixpy maix.ext_dev.pmu.PMU.__init__
     */
    PMU(std::string driver = "axp2101", int i2c_bus = -1, int addr = 0x34);
    ~PMU();

    /**
     * @brief Poweroff immediately.
     * @return err::Err type, if init success, return err::ERR_NONE.
     * @maixpy maix.ext_dev.pmu.PMU.poweroff
     */
    err::Err poweroff();

    /**
     * @brief Is the battery connected.
     * @return bool type, if battery is connected, return true.
     * @maixpy maix.ext_dev.pmu.PMU.is_bat_connect
     */
    bool is_bat_connect();

    /**
     * @brief Is the power adapter connected.
     * @return bool type, if power adapter is connected, return true.
     * @maixpy maix.ext_dev.pmu.PMU.is_vbus_in
     */
    bool is_vbus_in();

    /**
     * @brief Is bat charging.
     * @return bool type, if bat is charging, return true.
     * @maixpy maix.ext_dev.pmu.PMU.is_charging
     */
    bool is_charging();

    /**
     * @brief Get the battery percentage.
     * @return int type, return battery percentage.
     * @maixpy maix.ext_dev.pmu.PMU.get_bat_percent
     */
    int get_bat_percent();

    /**
     * @brief Get the battery charging status.
     * @return int type, return battery charging status.
     * @maixpy maix.ext_dev.pmu.PMU.get_charger_status
     */
    ext_dev::pmu::ChargerStatus get_charger_status();

    /**
     * @brief Get the battery voltage.
     * @return uint16_t type, return battery voltage.
     * @maixpy maix.ext_dev.pmu.PMU.get_bat_vol
     */
    uint16_t get_bat_vol();

    /**
     * @brief Clear interrupt flag.
     * @return err::Err type, if clean success, return err::ERR_NONE.
     * @maixpy maix.ext_dev.pmu.PMU.clean_irq
     */
    err::Err clean_irq();

    /**
     * @brief Set the battery charging current.
     * @param current The current to be set.
     * @return err::Err type, if set success, return err::ERR_NONE.
     * @maixpy maix.ext_dev.pmu.PMU.set_bat_charging_cur
     */
    err::Err set_bat_charging_cur(int current);

    /**
     * @brief Get the battery charging current.
     * @return int, return the currently set charging current.
     * @maixpy maix.ext_dev.pmu.PMU.get_bat_charging_cur
     */
    int get_bat_charging_cur();

    /**
     * @brief Set the PMU channel voltage.
     * You can retrieve the available channel from ext_dev.pmu.PowerChannel.
     * @param voltage The voltage to be set.
     * @return int, return the channel voltage.
     * @maixpy maix.ext_dev.pmu.PMU.set_vol
     */
    err::Err set_vol(ext_dev::pmu::PowerChannel channel, int voltage);

    /**
     * @brief Get the PMU channel voltage.
     * You can retrieve the available channel from ext_dev.pmu.PowerChannel.
     * @return err::Err type, if set success, return err::ERR_NONE.
     * @maixpy maix.ext_dev.pmu.PMU.get_vol
     */
    int get_vol(ext_dev::pmu::PowerChannel channel);

private:
    std::string _driver;
    void* _param;
    bool _is_opened;
};

}
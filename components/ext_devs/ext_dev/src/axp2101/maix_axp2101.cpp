/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.10.28: Add framework, create this file.
 */

#include "maix_i2c.hpp"
#include "maix_axp2101.hpp"
#include <mutex>
#include <iostream>
#include <unistd.h>
#include "axp2101.h"

#define _BV(b)                          (1ULL << (uint64_t)(b))

namespace maix::ext_dev::axp2101::priv {

static const int DEFAULT_I2C_BUS_NUM = 4;
static const char* TAG = "AXP2101";

static std::recursive_mutex mtx;
static ::maix::peripheral::i2c::I2C* i2cdev{nullptr};
static int32_t dev_num{0};
static uint8_t dev_addr{0};

static err::Err maix_i2c_read(uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size)
{
    ::maix::Bytes* res = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        i2cdev->writeto((int)address, &reg, 1);
        res = i2cdev->readfrom((int)address, (int)size);
    }

    if (res == nullptr) return err::Err::ERR_READ;

    std::copy(res->data, res->data+res->data_len, buffer);

    delete res;
    return err::Err::ERR_NONE;
}

static err::Err maix_i2c_write(uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size)
{
    auto buff = new uint8_t[size+1];
    buff[0] = reg;
    std::copy(buffer, buffer+size, buff+1);

    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        i2cdev->writeto((int)address, buff, size+1);
    }

    delete[] buff;
    return err::Err::ERR_NONE;
}

static void maix_i2c_init(int bus, uint8_t addr)
{
    if (i2cdev) {
        dev_num++;
        log::warn("[%s]: AXP2101 already init, finish...", TAG);
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (bus < 0)
        i2cdev = new ::maix::peripheral::i2c::I2C(DEFAULT_I2C_BUS_NUM, ::maix::peripheral::i2c::MASTER);
    else
        i2cdev = new ::maix::peripheral::i2c::I2C(bus, ::maix::peripheral::i2c::MASTER);
    dev_num ++;
    dev_addr = addr;
}

static err::Err maix_i2c_deinit()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    dev_num--;
    if (dev_num > 0) return err::Err::ERR_NONE;
    dev_addr = 0;
    dev_num = 0;
    delete i2cdev;
    i2cdev = nullptr;
    return err::Err::ERR_NONE;
}

}

namespace maix::ext_dev::axp2101 {

AXP2101::AXP2101(int i2c_bus, uint8_t addr)
{
    priv::maix_i2c_init(i2c_bus, addr);
    err::Err ret = this->init();
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: AXP2101 init failed. Error code:%d", priv::TAG, ret);
    } else {
        log::info("[%s]: AXP2101 init success.", priv::TAG);
    }
}

AXP2101::~AXP2101()
{
    err::Err ret = this->deinit();
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: ~AXP2101 failed. Error code:%d", priv::TAG, ret);
    }
}

err::Err inline AXP2101::set_register_bit(uint8_t registers, uint8_t bit)
{
    uint8_t buffer;
    err::Err ret = priv::maix_i2c_read(priv::dev_addr, registers, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        return ret;
    }
    buffer |= (_BV(bit));
    ret = priv::maix_i2c_write(priv::dev_addr, registers, &buffer, 1);
    return ret;
}

err::Err inline AXP2101::clear_register_bit(uint8_t registers, uint8_t bit)
{
    uint8_t buffer;
    err::Err ret = priv::maix_i2c_read(priv::dev_addr, registers, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        return ret;
    }
    buffer &= (~_BV(bit));
    ret = priv::maix_i2c_write(priv::dev_addr, registers, &buffer, 1);
    return ret;
}

bool inline AXP2101::get_register_bit(uint8_t registers, uint8_t bit)
{
    uint8_t buffer;
    err::Err ret = priv::maix_i2c_read(priv::dev_addr, registers, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        return false;
    }
    return buffer & _BV(bit);
}

err::Err AXP2101::init()
{
    uint8_t buffer;
    err::Err ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_VERSION, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    }

    buffer &= 0xCF;
    if (buffer == AXP2101_CHIP_ID || buffer == AXP2101_CHIP_ID_B) {
        log::info("[%s]: Find AXP2101 PMU, chip version: 0x%x.", priv::TAG, buffer);
        return err::Err::ERR_NONE;
    } else {
        log::error("[%s]: Don't find AXP2101 PMU. Error code:%d", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }
}

err::Err AXP2101::deinit()
{
    return priv::maix_i2c_deinit();
}

err::Err AXP2101::poweroff()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_COM_CFG, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    }
    buffer |= 0x01;
    sync();
    ret = priv::maix_i2c_write(priv::dev_addr, AXP2101_COM_CFG, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_write failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    } else {
        return err::Err::ERR_NONE;
    }
}

bool AXP2101::is_bat_connect()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_STATUS1, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    return buffer & _BV(3);
}

bool AXP2101::is_vbus_good()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_STATUS1, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    return buffer & _BV(5);
}

bool AXP2101::is_vbus_in()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_STATUS2, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    return (buffer & _BV(3)) == 0 && is_vbus_good();
}

bool AXP2101::is_charging()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_STATUS2, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    return (buffer >> 5) == 0x01;
}

int AXP2101::get_bat_percent()
{
    if (!is_bat_connect()) {
        return -1;
    }

    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_BAT_PERCENT_DATA, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    return buffer;
}

ext_dev::axp2101::ChargerStatus AXP2101::get_charger_status()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_STATUS2, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return ext_dev::axp2101::ChargerStatus::CHG_STOP_STATE;
    }
    buffer &= 0x07;
    return (ext_dev::axp2101::ChargerStatus)buffer;
}

uint16_t AXP2101::get_bat_vol()
{
    uint8_t val_h, val_l;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_ADC_DATA_RELUST0, &val_h, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_ADC_DATA_RELUST1, &val_l, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return false;
    }
    if (val_h == -1 || val_l == -1)
        return 0;
    return ((val_h & 0x1F) << 8) | val_l;
}

err::Err AXP2101::clean_irq()
{
    const uint8_t buffer = 0xFF;
    err::Err ret;
    for (int i = 0; i < 3; i++) {
        ret = priv::maix_i2c_write(priv::dev_addr, AXP2101_INT_STATUS1+i, &buffer, 1);
        if (ret != err::Err::ERR_NONE) {
            log::error("[%s]: maix_i2c_write failed. Error code:%d", priv::TAG, ret);
            return err::Err::ERR_RUNTIME;
        }
    }
    return err::Err::ERR_NONE;
}

err::Err AXP2101::set_bat_charging_cur(ext_dev::axp2101::ChargerCurrent current)
{
    if (((int)current < 4 || (int)current > 16) && (int)current != 0) {
        log::error("[%s]: The available values are 0mA, 100mA, 125mA, 150mA, 175mA, "
        "200mA, 300mA, 400mA, 500mA, 600mA, 700mA, 800mA, 900mA, and 1000mA.", priv::TAG);
        return err::Err::ERR_ARGS;
    }

    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_ICC_CHG_SET, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    }
    buffer &= 0xE0;
    buffer |= (int)current;
    ret = priv::maix_i2c_write(priv::dev_addr, AXP2101_ICC_CHG_SET, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_write failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    } else {
        return err::Err::ERR_NONE;
    }
}

ext_dev::axp2101::ChargerCurrent AXP2101::get_bat_charging_cur()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_ICC_CHG_SET, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return ext_dev::axp2101::ChargerCurrent::CHG_CUR_0MA;
    }
    return (ext_dev::axp2101::ChargerCurrent)(buffer & 0x1F);
}

err::Err AXP2101::enable_power_channel(PowerChannel channel) {
    switch (channel) {
        case PowerChannel::DCDC1:
            return set_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 0);
        case PowerChannel::DCDC2:
            return set_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 1);
        case PowerChannel::DCDC3:
            return set_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 2);
        case PowerChannel::DCDC4:
            return set_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 3);
        case PowerChannel::DCDC5:
            return set_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 4);
        case PowerChannel::ALDO1:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 0);
        case PowerChannel::ALDO2:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 1);
        case PowerChannel::ALDO3:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 2);
        case PowerChannel::ALDO4:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 3);
        case PowerChannel::BLDO1:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 4);
        case PowerChannel::BLDO2:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 5);
        case PowerChannel::DLDO1:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL0, 7);
        case PowerChannel::DLDO2:
            return set_register_bit(AXP2101_LDO_ONOFF_CTRL1, 0);
        case PowerChannel::VBACKUP:
            return set_register_bit(AXP2101_CHARGE_GAUGE_WDT_CTRL, 2);
        default:
            break;
        }
        return err::ERR_NONE;
}

err::Err AXP2101::disable_power_channel(PowerChannel channel) {
    switch (channel) {
        case PowerChannel::DCDC1:
            return clear_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 0);
        case PowerChannel::DCDC2:
            return clear_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 1);
        case PowerChannel::DCDC3:
            return clear_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 2);
        case PowerChannel::DCDC4:
            return clear_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 3);
        case PowerChannel::DCDC5:
            return clear_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 4);
        case PowerChannel::ALDO1:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 0);
        case PowerChannel::ALDO2:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 1);
        case PowerChannel::ALDO3:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 2);
        case PowerChannel::ALDO4:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 3);
        case PowerChannel::BLDO1:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 4);
        case PowerChannel::BLDO2:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 5);
        case PowerChannel::DLDO1:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL0, 7);
        case PowerChannel::DLDO2:
            return clear_register_bit(AXP2101_LDO_ONOFF_CTRL1, 0);
        case PowerChannel::VBACKUP:
            return clear_register_bit(AXP2101_CHARGE_GAUGE_WDT_CTRL, 2);
        default:
            break;
        }
        return err::ERR_NONE;
}

bool AXP2101::is_enable_channel(PowerChannel channel) {
    switch (channel) {
        case PowerChannel::DCDC1:
            return get_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 0);
        case PowerChannel::DCDC2:
            return get_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 1);
        case PowerChannel::DCDC3:
            return get_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 2);
        case PowerChannel::DCDC4:
            return get_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 3);
        case PowerChannel::DCDC5:
            return get_register_bit(AXP2101_DC_ONOFF_DVM_CTRL, 4);
        case PowerChannel::ALDO1:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 0);
        case PowerChannel::ALDO2:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 1);
        case PowerChannel::ALDO3:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 2);
        case PowerChannel::ALDO4:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 3);
        case PowerChannel::BLDO1:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 4);
        case PowerChannel::BLDO2:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 5);
        case PowerChannel::DLDO1:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL0, 7);
        case PowerChannel::DLDO2:
            return get_register_bit(AXP2101_LDO_ONOFF_CTRL1, 0);
        case PowerChannel::VBACKUP:
            return get_register_bit(AXP2101_CHARGE_GAUGE_WDT_CTRL, 2);
        default:
            break;
        }
        return false;
}

int AXP2101::dcdc1(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: DCDC1: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 1500) {
            log::error("[%s]: DCDC1: Minimum voltage is %d mV", priv::TAG, 1500);
            return -1;
        } else if (voltage > 3400) {
            log::error("[%s]: DCDC1: Maximum voltage is %d mV", priv::TAG, 3400);
            return -1;
        }

        buffer = (voltage - 1500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL0_CTRL, &buffer, 1)) {
            log::error("[%s]: DCDC1: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::DCDC1) != true) {
            if(enable_power_channel(PowerChannel::DCDC1)) {
                log::error("[%s]: DCDC1: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::DCDC1)) {
            log::error("[%s]: DCDC1: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::DCDC1) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL0_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 1500;
    }

    return voltage;
}

int AXP2101::dcdc2(int voltage)
{
    uint8_t buffer;
    if (priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL1_CTRL, &buffer, 1)) {
        return -1;
    }
    buffer &= 0x80;

    if (voltage > 0) {
        if (voltage >= 500 && voltage <= 1200) {
            if (voltage % 10) {
                log::error("[%s]: DCDC2: The steps is must %d mV", priv::TAG, 10);
                return -1;
            }
            buffer |= ((voltage - 500) / 10);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL1_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC2: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else if (voltage >= 1220 && voltage <= 1540) {
            if (voltage % 20) {
                log::error("[%s]: DCDC2: The steps is must %d mV", priv::TAG, 20);
                return -1;
            }
            buffer |= ((voltage - 1220) / 20 + 71);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL1_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC2: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else {
            log::error("[%s]: DCDC2: The voltage setting range is"
            "500mV~1200mV(step 10mV) and 1220mV~1540mV(step 20mV).", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::DCDC2) != true) {
            if(enable_power_channel(PowerChannel::DCDC2)) {
                log::error("[%s]: DCDC2: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::DCDC2)) {
            log::error("[%s]: DCDC2: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::DCDC2) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL1_CTRL, &buffer, 1);
        buffer &= 0x7F;
        if (buffer < 71) {
            voltage = buffer * 10 + 500;
        } else  {
            voltage = buffer * 20 - 200;
        }
    }

    return voltage;
}

int AXP2101::dcdc3(int voltage)
{
    uint8_t buffer;
    if (priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL2_CTRL, &buffer, 1)) {
        return -1;
    }
    buffer &= 0x80;

    if (voltage > 0) {
        if (voltage >= 500 && voltage <= 1200) {
            if (voltage % 10) {
                log::error("[%s]: DCDC3: The steps is must %d mV", priv::TAG, 10);
                return -1;
            }
            buffer |= ((voltage - 500) / 10);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL2_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC3: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else if (voltage >= 1220 && voltage <= 1540) {
            if (voltage % 20) {
                log::error("[%s]: DCDC3: The steps is must %d mV", priv::TAG, 20);
                return -1;
            }
            buffer |= ((voltage - 1220) / 20 + 71);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL2_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC3: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else {
            log::error("[%s]: DCDC3: The voltage setting range is"
            "500mV~1200mV(step 10mV) and 1220mV~1540mV(step 20mV).", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::DCDC3) != true) {
            if(enable_power_channel(PowerChannel::DCDC3)) {
                log::error("[%s]: DCDC3: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::DCDC3)) {
            log::error("[%s]: DCDC3: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::DCDC3) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL2_CTRL, &buffer, 1);
        buffer &= 0x7F;
        if (buffer < 71) {
            voltage = buffer * 10 + 500;
        } else  {
            voltage = buffer * 20 - 200;
        }
    }

    return voltage;
}

int AXP2101::dcdc4(int voltage)
{
    uint8_t buffer;
    if (priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL3_CTRL, &buffer, 1)) {
        return -1;
    }
    buffer &= 0x80;

    if (voltage > 0) {
        if (voltage >= 500 && voltage <= 1200) {
            if (voltage % 10) {
                log::error("[%s]: DCDC4: The steps is must %d mV", priv::TAG, 10);
                return -1;
            }
            buffer |= ((voltage - 500) / 10);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL3_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC4: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else if (voltage >= 1220 && voltage <= 1840) {
            if (voltage % 20) {
                log::error("[%s]: DCDC4: The steps is must %d mV", priv::TAG, 20);
                return -1;
            }
            buffer |= ((voltage - 1220) / 20 + 71);
            if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL3_CTRL, &buffer, 1)) {
                log::error("[%s]: DCDC4: Set voltage %d mV error", priv::TAG, voltage);
                return -1;
            }
        } else {
            log::error("[%s]: DCDC4: The voltage setting range is"
            "500mV~1200mV(step 10mV) and 1220mV~1840mV(step 20mV).", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::DCDC4) != true) {
            if(enable_power_channel(PowerChannel::DCDC4)) {
                log::error("[%s]: DCDC4: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::DCDC4)) {
            log::error("[%s]: DCDC4: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::DCDC4) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL3_CTRL, &buffer, 1);
        buffer &= 0x7F;
        if (buffer < 71) {
            voltage = buffer * 10 + 500;
        } else  {
            voltage = buffer * 20 - 200;
        }
    }

    return voltage;
}

int AXP2101::dcdc5(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: DCDC5: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage != 1200 && voltage < 1400) {
            log::error("[%s]: DCDC5: Minimum voltage is %d mV", priv::TAG, 1400);
            return -1;
        } else if (voltage > 3700) {
            log::error("[%s]: DCDC5: Maximum voltage is %d mV", priv::TAG, 3700);
            return -1;
        }

        if (priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL4_CTRL, &buffer, 1)) {
            return -1;
        }
        buffer &= 0xE0;
        if (voltage == 1200) {
            buffer |= 0x19;
        } else {
            buffer |= (voltage - 1400) / 100;
        }

        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_DC_VOL4_CTRL, &buffer, 1)) {
            log::error("[%s]: DCDC5: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::DCDC5) != true) {
            if(enable_power_channel(PowerChannel::DCDC5)) {
                log::error("[%s]: DCDC5: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::DCDC5)) {
            log::error("[%s]: DCDC5: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::DCDC5) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_DC_VOL4_CTRL, &buffer, 1);
        buffer &= 0x1F;
        if (buffer == 0x19) {
            voltage = 1200;
        } else {
            voltage = buffer * 100 + 1400;
        }
    }

    return voltage;
}

int AXP2101::aldo1(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: ALDO1: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: ALDO1: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: ALDO1: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL0_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL0_CTRL, &buffer, 1)) {
            log::error("[%s]: ALDO1: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::ALDO1) != true) {
            if(enable_power_channel(PowerChannel::ALDO1)) {
                log::error("[%s]: ALDO1: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::ALDO1)) {
            log::error("[%s]: ALDO1: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::ALDO1) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL0_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

int AXP2101::aldo2(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: ALDO2: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: ALDO2: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: ALDO2: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL1_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL1_CTRL, &buffer, 1)) {
            log::error("[%s]: ALDO2: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::ALDO2) != true) {
            if(enable_power_channel(PowerChannel::ALDO2)) {
                log::error("[%s]: ALDO2: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::ALDO2)) {
            log::error("[%s]: ALDO2: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::ALDO2) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL1_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

int AXP2101::aldo3(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: ALDO3: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: ALDO3: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: ALDO3: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL2_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL2_CTRL, &buffer, 1)) {
            log::error("[%s]: ALDO3: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::ALDO3) != true) {
            if(enable_power_channel(PowerChannel::ALDO3)) {
                log::error("[%s]: ALDO3: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::ALDO3)) {
            log::error("[%s]: ALDO3: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::ALDO3) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL2_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

int AXP2101::aldo4(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: ALDO4: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: ALDO4: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: ALDO4: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL3_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL3_CTRL, &buffer, 1)) {
            log::error("[%s]: ALDO4: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::ALDO4) != true) {
            if(enable_power_channel(PowerChannel::ALDO4)) {
                log::error("[%s]: ALDO4: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::ALDO4)) {
            log::error("[%s]: ALDO4: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::ALDO4) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL3_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

int AXP2101::bldo1(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: BLDO1: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: BLDO1: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: BLDO1: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL4_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL4_CTRL, &buffer, 1)) {
            log::error("[%s]: BLDO1: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::BLDO1) != true) {
            if(enable_power_channel(PowerChannel::BLDO1)) {
                log::error("[%s]: BLDO1: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::BLDO1)) {
            log::error("[%s]: BLDO1: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::BLDO1) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL4_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

int AXP2101::bldo2(int voltage)
{
    uint8_t buffer;
    if (voltage > 0) {
        if (voltage % 100) {
            log::error("[%s]: BLDO2: The steps is must %d mV", priv::TAG, 100);
            return -1;
        }
        if (voltage < 500) {
            log::error("[%s]: BLDO2: Minimum voltage is %d mV", priv::TAG, 500);
            return -1;
        } else if (voltage > 3500) {
            log::error("[%s]: BLDO2: Maximum voltage is %d mV", priv::TAG, 3500);
            return -1;
        }

        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL5_CTRL, &buffer, 1);
        buffer &= 0xE0;
        buffer = (voltage - 500) / 100;
        if(priv::maix_i2c_write(priv::dev_addr, AXP2101_LDO_VOL5_CTRL, &buffer, 1)) {
            log::error("[%s]: BLDO2: Set voltage %d mV error", priv::TAG, voltage);
            return -1;
        }

        if (is_enable_channel(PowerChannel::BLDO2) != true) {
            if(enable_power_channel(PowerChannel::BLDO2)) {
                log::error("[%s]: BLDO2: Enable dcdc channel error", priv::TAG);
                return -1;
            }
        }
    } else if (voltage == 0) {
        if(disable_power_channel(PowerChannel::BLDO2)) {
            log::error("[%s]: BLDO2: Disable dcdc channel error", priv::TAG);
            return -1;
        }
    }

    if (is_enable_channel(PowerChannel::BLDO2) != true) {
        return 0;
    } else {
        priv::maix_i2c_read(priv::dev_addr, AXP2101_LDO_VOL5_CTRL, &buffer, 1);
        voltage = (buffer & 0x1F) * 100 + 500;
    }

    return voltage;
}

err::Err AXP2101::set_poweroff_time(ext_dev::axp2101::PowerOffTime tm)
{
    if (tm == PowerOffTime::POWEROFF_DISABLE) {
        return clear_register_bit(AXP2101_PWROFF_EN, 1);
    } else {
        set_register_bit(AXP2101_PWROFF_EN, 1);
    }
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    }
    buffer &= 0xF3;
    buffer |= ((int)tm << 2);
    ret = priv::maix_i2c_write(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_write failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    } else {
        return err::Err::ERR_NONE;
    }
}

ext_dev::axp2101::PowerOffTime AXP2101::get_poweroff_time()
{
    if (get_register_bit(AXP2101_PWROFF_EN, 1) == false) {
        return PowerOffTime::POWEROFF_DISABLE;
    }

    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return PowerOffTime::POWEROFF_DISABLE;
    }
    return (PowerOffTime)((buffer & 0x0C) >> 2);
}

err::Err AXP2101::set_poweron_time(ext_dev::axp2101::PowerOnTime tm)
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    }
    buffer &= 0xFC;
    buffer |= (int)tm;
    ret = priv::maix_i2c_write(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_write failed. Error code:%d", priv::TAG, ret);
        return err::Err::ERR_RUNTIME;
    } else {
        return err::Err::ERR_NONE;
    }
}

ext_dev::axp2101::PowerOnTime AXP2101::get_poweron_time()
{
    uint8_t buffer;
    err::Err ret;
    ret = priv::maix_i2c_read(priv::dev_addr, AXP2101_IRQ_OFF_ON_LEVEL_CTRL, &buffer, 1);
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s]: maix_i2c_read failed. Error code:%d", priv::TAG, ret);
        return PowerOnTime::POWERON_128MS;
    }
    return (PowerOnTime)(buffer & 0x03);
}

}
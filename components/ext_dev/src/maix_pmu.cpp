
#include "maix_basic.hpp"
#include "maix_pmu.hpp"
#include "maix_axp2101.hpp"

namespace maix::ext_dev::pmu {

typedef struct {
    union {
        maix::ext_dev::axp2101::AXP2101 *axp2101;
    } driver;
} pmu_param_t;

PMU::PMU(std::string driver, int i2c_bus, int addr)
{
    err::check_bool_raise(driver == "axp2101", "Only support axp2101 now");
    pmu_param_t *param = (pmu_param_t *)malloc(sizeof(pmu_param_t));
    err::check_null_raise(param, "Failed to malloc param");

    param->driver.axp2101 = new maix::ext_dev::axp2101::AXP2101(i2c_bus, addr);
    _param = (void *)param;
    _driver = driver;
}

PMU::~PMU()
{
    if (_param) {
        pmu_param_t *param = (pmu_param_t *)_param;
        if (_driver == "axp2101") {
            delete param->driver.axp2101;
            param->driver.axp2101 = NULL;
        }
        free(_param);
        _param = NULL;
    }
}

err::Err PMU::poweroff()
{
    err::Err err = err::Err::ERR_NOT_IMPL;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        err = param->driver.axp2101->poweroff();
    }
    return err;
}

bool PMU::is_bat_connect()
{
    bool ret = false;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = param->driver.axp2101->is_bat_connect();
    }
    return ret;
}

bool PMU::is_vbus_in()
{
    bool ret = false;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = param->driver.axp2101->is_vbus_in();
    }
    return ret;
}

bool PMU::is_charging()
{
    bool ret = false;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = param->driver.axp2101->is_charging();
    }
    return ret;
}

int PMU::get_bat_percent()
{
    int ret = -1;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = param->driver.axp2101->get_bat_percent();
    }
    return ret;
}

ext_dev::pmu::ChargerStatus PMU::get_charger_status()
{
    ext_dev::pmu::ChargerStatus ret = ext_dev::pmu::ChargerStatus::CHG_STOP_STATE;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = (ext_dev::pmu::ChargerStatus)param->driver.axp2101->get_charger_status();
    }
    return ret;
}

uint16_t PMU::get_bat_vol()
{
    uint16_t ret = 0;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = param->driver.axp2101->get_bat_vol();
    }
    return ret;
}

err::Err PMU::clean_irq()
{
    err::Err err = err::Err::ERR_NOT_IMPL;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        err = param->driver.axp2101->clean_irq();
    }
    return err;
}

err::Err PMU::set_bat_charging_cur(int current)
{
    err::Err err = err::Err::ERR_NOT_IMPL;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        if (current <= 200) {
            current = current * 0.04;
        } else {
            current = (current - 200) * 0.01 + 8;
        }
        err = param->driver.axp2101->set_bat_charging_cur((axp2101::ChargerCurrent)current);
    }
    return err;
}

int PMU::get_bat_charging_cur()
{
    int ret = 0;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        ret = (int)param->driver.axp2101->get_bat_charging_cur();
        if (ret <= 8) {
            ret = ret * 25;
        } else {
            ret = (ret - 8) * 100 + 200;
        }
    }
    return ret;
}

err::Err PMU::set_vol(ext_dev::pmu::PowerChannel channel, int voltage)
{
    err::Err err = err::Err::ERR_NOT_IMPL;
    int ret = -1;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        switch (channel) {
            case PowerChannel::DCDC1: ret = param->driver.axp2101->dcdc1(voltage); break;
            case PowerChannel::DCDC2: ret = param->driver.axp2101->dcdc2(voltage); break;
            case PowerChannel::DCDC3: ret = param->driver.axp2101->dcdc3(voltage); break;
            case PowerChannel::DCDC4: ret = param->driver.axp2101->dcdc4(voltage); break;
            case PowerChannel::DCDC5: ret = param->driver.axp2101->dcdc5(voltage); break;
            case PowerChannel::ALDO1: ret = param->driver.axp2101->aldo1(voltage); break;
            case PowerChannel::ALDO2: ret = param->driver.axp2101->aldo2(voltage); break;
            case PowerChannel::ALDO3: ret = param->driver.axp2101->aldo3(voltage); break;
            case PowerChannel::ALDO4: ret = param->driver.axp2101->aldo4(voltage); break;
            case PowerChannel::BLDO1: ret = param->driver.axp2101->bldo1(voltage); break;
            case PowerChannel::BLDO2: ret = param->driver.axp2101->bldo2(voltage); break;
            default:
                log::error("[%s]: Channel not support.", _driver.c_str());
                return err::Err::ERR_NOT_IMPL;
            if (ret != voltage) {
                log::error("Set voltage error. ret=%d", ret);
                err = err::Err::ERR_RUNTIME;
            } else {
                err = err::Err::ERR_NONE;
            }
        }
    }
    return err;
}

int PMU::get_vol(ext_dev::pmu::PowerChannel channel)
{
    int ret = -1;
    pmu_param_t *param = (pmu_param_t *)_param;
    if (_driver == "axp2101") {
        switch (channel) {
            case PowerChannel::DCDC1: ret = param->driver.axp2101->dcdc1(); break;
            case PowerChannel::DCDC2: ret = param->driver.axp2101->dcdc2(); break;
            case PowerChannel::DCDC3: ret = param->driver.axp2101->dcdc3(); break;
            case PowerChannel::DCDC4: ret = param->driver.axp2101->dcdc4(); break;
            case PowerChannel::DCDC5: ret = param->driver.axp2101->dcdc5(); break;
            case PowerChannel::ALDO1: ret = param->driver.axp2101->aldo1(); break;
            case PowerChannel::ALDO2: ret = param->driver.axp2101->aldo2(); break;
            case PowerChannel::ALDO3: ret = param->driver.axp2101->aldo3(); break;
            case PowerChannel::ALDO4: ret = param->driver.axp2101->aldo4(); break;
            case PowerChannel::BLDO1: ret = param->driver.axp2101->bldo1(); break;
            case PowerChannel::BLDO2: ret = param->driver.axp2101->bldo2(); break;
            default:
                log::error("[%s]: Channel not support.", _driver.c_str());
                return -1;
        }
    }
    return ret;
}

}
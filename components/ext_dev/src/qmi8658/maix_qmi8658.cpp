/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.6: Add framework, create this file.
 */

#include "qmi8658c.h"
#include "maix_i2c.hpp"
#include "maix_qmi8658.hpp"
#include <memory>
#include <mutex>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <map>
namespace maix::ext_dev::qmi8658::priv {

static const int DEFALUT_I2C_BUS = 4;
// static const int QMI8658C_I2C_ADDRESS = 0x6B;
static const uint64_t RESET_WAIT_TIME_MS = 2000;
static const char* TAG = "MAIX QMI8658";

/********************************************
 *
 * !!! Normally you don't need to modify the following global variables.
 *
 ********************************************/
struct I2cInfo {
    ::maix::peripheral::i2c::I2C* i2c_bus{nullptr};
    int dev_num{0};
};
static std::recursive_mutex mtx;
static std::map<int, I2cInfo> manager;

::maix::peripheral::i2c::I2C* Qmi8658c::maix_qmi_init_i2c_bus(int bus, uint32_t deviceFrequency, bool& is_not_exist)
{
    int _bus = (bus < 0) ? DEFALUT_I2C_BUS : bus;
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = manager.find(_bus);
    if (it != manager.end()) {
        is_not_exist = false;
        it->second.dev_num++;
        return it->second.i2c_bus;
    }
    is_not_exist = true;
    auto i2cbus = new ::maix::peripheral::i2c::I2C(_bus, ::maix::peripheral::i2c::Mode::MASTER, (int)deviceFrequency);
    // I2cInfo tmp {
    //     .i2c_bus = i2cbus,
    //     .dev_num = 1,
    // };
    // maix::log::info("%d", __LINE__);
    i2cbus->scan();
    // maix::log::info("maix_qmi_init_i2c_bus i2cbus: %p", i2cbus);
    // maix::log::info("%d", __LINE__);
    I2cInfo tmp;
    tmp.i2c_bus = i2cbus;
    tmp.dev_num = 1;
    manager.insert(std::pair<int, I2cInfo>(_bus, tmp));
    return i2cbus;
}

void Qmi8658c::maix_qmi_deinit_i2c_bus(int bus)
{
    // maix::log::info("maix_qmi_deinit_i2c_bus");
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = manager.find(bus);
    if (it == manager.end())
        return;
    it->second.dev_num--;
    if (it->second.dev_num > 0)
        return;
    if (it->second.i2c_bus)
        delete it->second.i2c_bus;
    it->second.dev_num = 0;
    manager.erase(it->first);
}

}

namespace maix::ext_dev::qmi8658 {

static std::vector<float> make_read_result(const imu::Mode& m, const priv::qmi_data_t& data)
{
    if (m == imu::Mode::DUAL)
        return {data.acc_xyz.x, data.acc_xyz.y, data.acc_xyz.z, data.gyro_xyz.x, data.gyro_xyz.y, data.gyro_xyz.z, data.temperature};
    if (m == imu::Mode::ACC_ONLY)
        return {data.acc_xyz.x, data.acc_xyz.y, data.acc_xyz.z, data.temperature};
    if (m == imu::Mode::GYRO_ONLY)
        return {data.gyro_xyz.x, data.gyro_xyz.y, data.gyro_xyz.z, data.temperature};
    log::error("[%s] Unknown Mode, return empty", priv::TAG);
    return {};
}

QMI8658::QMI8658(int i2c_bus, int addr, int freq, imu::Mode mode, imu::AccScale acc_scale,
                imu::AccOdr acc_odr, imu::GyroScale gyro_scale, imu::GyroOdr gyro_odr, bool block)
{
    auto qmi8658c = new priv::Qmi8658c(i2c_bus, (uint8_t)addr, freq);
    this->_data = (void*)qmi8658c;

    qmi8658c->deviceID = 0x0;
    this->_mode = mode;
    priv::qmi8658_cfg_t cfg;
    cfg.qmi8658_mode = priv::qmi8658_mode_dual;
    maix::log::info("cfg.qmi8658_mode: 0x%x", cfg.qmi8658_mode);
    cfg.acc_scale = static_cast<priv::acc_scale_t>(acc_scale);
    cfg.acc_odr = static_cast<priv::acc_odr_t>(acc_odr);
    cfg.gyro_scale = static_cast<priv::gyro_scale_t>(gyro_scale);
    cfg.gyro_odr = static_cast<priv::gyro_odr_t>(gyro_odr);

    if (!qmi8658c->need_reset) {
        log::warn("qmi8658c in this bus is already init! All config args will be ignore!");
        qmi8658c->deviceID = 0x05;
        return;
    }
    qmi8658c->reset();
    uint64_t reset_start_time = maix::time::ticks_ms();

    maix::log::info("Start a task to open [%s]", priv::TAG);

    auto init_task = [reset_start_time, cfg, qmi8658c]() -> std::pair<int, std::string> {
        uint64_t start = reset_start_time;
        priv::qmi8658_cfg_t thread_cfg;
        ::memcpy(&thread_cfg, &cfg, sizeof(priv::qmi8658_cfg_t));
        while (maix::time::ticks_ms()-start < priv::RESET_WAIT_TIME_MS) {
            maix::time::sleep_ms(50);
        }
        // maix::log::info("mode: 0x%x", thread_cfg.qmi8658_mode);
        auto qmi8658_result = qmi8658c->open(&thread_cfg);

        if (qmi8658_result == priv::qmi8658_result_open_error) {
            log::error("[%s] Open IMU Failed! Function read() will return empty", priv::TAG);
            return std::pair<int, std::string>(-1, "Open IMU Failed!");
        }
        log::info("[%s] Open IMU Succ. Chip Name: QMI8658", priv::TAG);
        log::info("[%s] Device ID: 0x%x", priv::TAG, qmi8658c->deviceID);
        log::info("[%s] Device Revision ID: 0x%x", priv::TAG, qmi8658c->deviceRevisionID);
        return std::pair<int, std::string>(0, "");
    };

    this->open_future = std::async(std::launch::async, init_task);
    this->open_fut_need_get = true;

    if (block) {
        maix::log::info("[%s] Waiting for IMU open...", priv::TAG);
        auto future_ret = this->open_future.get();
        if (future_ret.first < 0) {
            maix::log::error("[%s] Failed: %s", priv::TAG, future_ret.second.c_str());
            return;
        }
        this->open_fut_need_get = false;
        return;
    }


    // std::thread([reset_start_time, cfg, qmi8658c](){
    //     uint64_t start = reset_start_time;
    //     priv::qmi8658_cfg_t thread_cfg;
    //     ::memcpy(&thread_cfg, &cfg, sizeof(priv::qmi8658_cfg_t));
    //     while (maix::time::ticks_ms()-start < priv::RESET_WAIT_TIME_MS) {
    //         maix::time::sleep_ms(50);
    //     }
    //     maix::log::info("thread mode: 0x%x", thread_cfg.qmi8658_mode);
    //     auto qmi8658_result = qmi8658c->open(&thread_cfg);

    //     if (qmi8658_result == priv::qmi8658_result_open_error) {
    //         log::error("[%s] Open IMU Failed! Function read() will return empty", priv::TAG);
    //         return;
    //     }
    //     log::info("[%s] Open IMU Succ. Chip Name: QMI8658", priv::TAG);
    //     log::info("[%s] Device ID: 0x%x", priv::TAG, qmi8658c->deviceID);
    //     log::info("[%s] Device Revision ID: 0x%x", priv::TAG, qmi8658c->deviceRevisionID);
    // }).detach();

}

QMI8658::~QMI8658()
{
    if (this->open_fut_need_get) {
        // maix::log::info("QMI8658::~QMI8658() waiting...");
        this->open_future.wait();
        this->open_fut_need_get = false;
    }
    delete (priv::Qmi8658c*)(this->_data);
}

std::vector<float> QMI8658::read()
{
    if (((priv::Qmi8658c*)this->_data)->deviceID != 0x5) {
        // log::warn("[%s] IMU is not open, check open()'s result", priv::TAG);
        return {};
    }
    priv::qmi_data_t data;
    ::memset(&data, 0x00, sizeof(data));
    ((priv::Qmi8658c*)this->_data)->read(&data);

    return make_read_result(this->_mode, data);
}

}
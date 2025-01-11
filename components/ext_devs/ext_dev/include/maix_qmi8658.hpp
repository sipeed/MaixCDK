/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.6: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_imu.hpp"
#include <atomic>
#include <future>

namespace maix::ext_dev::qmi8658 {

/**
 * QMI8656 driver class
 * @maixpy maix.ext_dev.qmi8658.QMI8658
 */
class QMI8658 {
public:
    /**
     * @brief Construct a new QMI8658 object, will open QMI8658
     *
     * @param i2c_bus i2c bus number. Automatically selects the on-board qmi8658 when -1 is passed in.
     * @param addr QMI8658 i2c addr.
     * @param freq QMI8658 freq
     * @param mode QMI8658 Mode: ACC_ONLY/GYRO_ONLY/DUAL
     * @param acc_scale acc scale, see @qmi8658::AccScale
     * @param acc_odr acc output data rate, see @qmi8658::AccOdr
     * @param gyro_scale gyro scale, see @qmi8658::GyroScale
     * @param gyro_odr gyro output data rate, see @qmi8658::GyroOdr
     * @param block block or non-block, defalut is true
     *
     * @maixpy maix.ext_dev.qmi8658.QMI8658.__init__
     */
    QMI8658(int i2c_bus=-1, int addr=0x6B, int freq=400000,
            maix::ext_dev::imu::Mode mode=maix::ext_dev::imu::Mode::DUAL,
            maix::ext_dev::imu::AccScale acc_scale=maix::ext_dev::imu::AccScale::ACC_SCALE_2G,
            maix::ext_dev::imu::AccOdr acc_odr=maix::ext_dev::imu::AccOdr::ACC_ODR_8000,
            maix::ext_dev::imu::GyroScale gyro_scale=maix::ext_dev::imu::GyroScale::GYRO_SCALE_16DPS,
            maix::ext_dev::imu::GyroOdr gyro_odr=maix::ext_dev::imu::GyroOdr::GYRO_ODR_8000,
            bool block=true);
    ~QMI8658();

    QMI8658(const QMI8658&)             = delete;
    QMI8658& operator=(const QMI8658&)  = delete;
    QMI8658(QMI8658&&)                  = delete;
    QMI8658& operator=(QMI8658&&)       = delete;

    /**
     * @brief Read data from QMI8658.
     *
     * @return list type. If only one of the outputs is initialized, only [x,y,z] of that output will be returned.
     *                    If all outputs are initialized, [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z] is returned.
     *
     * @maixpy maix.ext_dev.qmi8658.QMI8658.read
     */
    std::vector<float> read();
private:
    void* _data;
    imu::Mode _mode;
    std::atomic_bool reset_finished{false};
    std::future<std::pair<int, std::string>> open_future;
    bool open_fut_need_get{false};
};


}
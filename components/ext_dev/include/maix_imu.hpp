/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.6: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"

#include <atomic>
#include <future>

namespace maix::ext_dev::imu {

/**
 * @brief imu mode
 * @maixpy maix.ext_dev.imu.Mode
 */
enum Mode {
    ACC_ONLY = 0,
    GYRO_ONLY,
    DUAL
};

/**
 * @brief imu acc scale
 * @maixpy maix.ext_dev.imu.AccScale
 */
enum AccScale {
    ACC_SCALE_2G = 0,
    ACC_SCALE_4G,
    ACC_SCALE_8G,
    ACC_SCALE_16G
};

/**
 * @brief imu acc output data rate
 * @maixpy maix.ext_dev.imu.AccOdr
 */
enum AccOdr {
    ACC_ODR_8000,      // Accelerometer ODR set to 8000 Hz.
    ACC_ODR_4000,      // Accelerometer ODR set to 4000 Hz.
    ACC_ODR_2000,      // Accelerometer ODR set to 2000 Hz.
    ACC_ODR_1000,      // Accelerometer ODR set to 1000 Hz.
    ACC_ODR_500,       // Accelerometer ODR set to 500 Hz.
    ACC_ODR_250,       // Accelerometer ODR set to 250 Hz.
    ACC_ODR_125,       // Accelerometer ODR set to 125 Hz.
    ACC_ODR_62_5,      // Accelerometer ODR set to 62.5 Hz.
    ACC_ODR_31_25,     // Accelerometer ODR set to 31.25 Hz.
    ACC_ODR_128 = 12,  // Accelerometer ODR set to 128 Hz.
    ACC_ODR_21,        // Accelerometer ODR set to 21 Hz.
    ACC_ODR_11,        // Accelerometer ODR set to 11 Hz.
    ACC_ODR_3,         // Accelerometer ODR set to 3 Hz.
};

/**
 * @brief imu gyro scale
 * @maixpy maix.ext_dev.imu.GyroScale
 */
enum GyroScale {
    GYRO_SCALE_16DPS = 0,       // Gyroscope scale set to ±16 degrees per second.
    GYRO_SCALE_32DPS,            // Gyroscope scale set to ±32 degrees per second.
    GYRO_SCALE_64DPS,            // Gyroscope scale set to ±64 degrees per second.
    GYRO_SCALE_128DPS,           // Gyroscope scale set to ±128 degrees per second.
    GYRO_SCALE_256DPS,           // Gyroscope scale set to ±256 degrees per second.
    GYRO_SCALE_512DPS,           // Gyroscope scale set to ±512 degrees per second.
    GYRO_SCALE_1024DPS,          // Gyroscope scale set to ±1024 degrees per second.
    GYRO_SCALE_2048DPS,          // Gyroscope scale set to ±2048 degrees per second.
};

/**
 * @brief imu gyro output data rate
 * @maixpy maix.ext_dev.imu.GyroOdr
 */
enum GyroOdr {
    GYRO_ODR_8000,     // Gyroscope ODR set to 8000 Hz.
    GYRO_ODR_4000,     // Gyroscope ODR set to 4000 Hz.
    GYRO_ODR_2000,     // Gyroscope ODR set to 2000 Hz.
    GYRO_ODR_1000,     // Gyroscope ODR set to 1000 Hz.
    GYRO_ODR_500,      // Gyroscope ODR set to 500 Hz.
    GYRO_ODR_250,      // Gyroscope ODR set to 250 Hz.
    GYRO_ODR_125,      // Gyroscope ODR set to 125 Hz.
    GYRO_ODR_62_5,     // Gyroscope ODR set to 62.5 Hz.
    GYRO_ODR_31_25,    // Gyroscope ODR set to 31.25 Hz.
};

/**
 * QMI8656 driver class
 * @maixpy maix.ext_dev.imu.IMU
 */
class IMU {
public:
    /**
     * @brief Construct a new IMU object, will open IMU
     *
     * @param driver driver name, only support "qmi8656"
     * @param i2c_bus i2c bus number. Automatically selects the on-board imu when -1 is passed in.
     * @param addr IMU i2c addr.
     * @param freq IMU freq
     * @param mode IMU Mode: ACC_ONLY/GYRO_ONLY/DUAL
     * @param acc_scale acc scale, see @imu::AccScale
     * @param acc_odr acc output data rate, see @imu::AccOdr
     * @param gyro_scale gyro scale, see @imu::GyroScale
     * @param gyro_odr gyro output data rate, see @imu::GyroOdr
     * @param block block or non-block, defalut is true
     *
     * @maixpy maix.ext_dev.imu.IMU.__init__
     */
    IMU(std::string driver, int i2c_bus=-1, int addr=0x6B, int freq=400000,
            maix::ext_dev::imu::Mode mode=maix::ext_dev::imu::Mode::DUAL,
            maix::ext_dev::imu::AccScale acc_scale=maix::ext_dev::imu::AccScale::ACC_SCALE_2G,
            maix::ext_dev::imu::AccOdr acc_odr=maix::ext_dev::imu::AccOdr::ACC_ODR_8000,
            maix::ext_dev::imu::GyroScale gyro_scale=maix::ext_dev::imu::GyroScale::GYRO_SCALE_16DPS,
            maix::ext_dev::imu::GyroOdr gyro_odr=maix::ext_dev::imu::GyroOdr::GYRO_ODR_8000,
            bool block=true);
    ~IMU();

    IMU(const IMU&)             = delete;
    IMU& operator=(const IMU&)  = delete;
    IMU(IMU&&)                  = delete;
    IMU& operator=(IMU&&)       = delete;

    /**
     * @brief Read data from IMU.
     *
     * @return list type. If only one of the outputs is initialized, only [x,y,z] of that output will be returned.
     *                    If all outputs are initialized, [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z] is returned.
     *
     * @maixpy maix.ext_dev.imu.IMU.read
     */
    std::vector<float> read();
private:
    void* _param;
    std::string _driver;
};


}
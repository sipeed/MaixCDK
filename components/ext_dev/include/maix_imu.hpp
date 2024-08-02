/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include <vector>

namespace maix::peripheral::imu
{
    /**
     * Peripheral IMU class
     * @maixpy maix.peripheral.imu.IMU
     */
    class IMU
    {
    public:
        /**
         * @brief IMU constructor
         *
         * @param acc_scale Accelerometer Full-scale. Defalut is min.
         * @param acc_lpf Accelerometer Low-Pass Filter. When acc_scale and acc_lpf are both -1, read will not return the acc sensor value.
         * @param gyro_scale Gyroscope Full-scale. Defalut is min.
         * @param gyro_lpf Gyroscope Low-Pass Filter. When gyro_scale and gyro_lpf are both -1, read will not return the gyro sensor value.
         * @param output_data_rate Set Output Data Rate.
         *
         * @maixpy maix.peripheral.imu.IMU.__init__
         */
        IMU(int acc_scale=-1, int acc_lpf=-1, int gyro_scale=-1, int gyro_lpf=-1, int output_data_rate=-1);
        ~IMU();

        /**
         * @brief Open sensor
         *
         * @param acc_scale Accelerometer Full-scale. Defalut is min.
         * @param acc_lpf Accelerometer Low-Pass Filter. When acc_scale and acc_lpf are both -1, read will not return the acc sensor value.
         * @param gyro_scale Gyroscope Full-scale. Defalut is min.
         * @param gyro_lpf Gyroscope Low-Pass Filter. When gyro_scale and gyro_lpf are both -1, read will not return the gyro sensor value.
         * @param output_data_rate Set Output Data Rate.
         *
         * @return err::Err type, err::Err::ERR_NONE means none err.
         *
         * @maixpy maix.peripheral.imu.IMU.open
         */
        err::Err open(int acc_scale=-1, int acc_lpf=-1, int gyro_scale=-1, int gyro_lpf=-1, int output_data_rate=-1);

        /**
         * @brief Read data from sensor.
         *
         * @return list type. If only one of the outputs is initialized, only [x,y,z] of that output will be returned.
         *                    If all outputs are initialized, [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z] is returned.
         *
         * @maixpy maix.peripheral.imu.IMU.read
         */
        std::vector<float> read();
    };
}; // namespace maix::peripheral::wdt

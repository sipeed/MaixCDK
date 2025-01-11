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
enum class Mode {
    ACC_ONLY = 0,
    GYRO_ONLY,
    DUAL
};

/**
 * @brief imu acc scale
 * @maixpy maix.ext_dev.imu.AccScale
 */
enum class AccScale {
    ACC_SCALE_2G = 0,
    ACC_SCALE_4G,
    ACC_SCALE_8G,
    ACC_SCALE_16G
};

/**
 * @brief imu acc output data rate
 * @maixpy maix.ext_dev.imu.AccOdr
 */
enum class AccOdr {
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
enum class GyroScale {
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
enum class GyroOdr {
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

    /**
     * @brief Caculate calibration, save calibration data to /maixapp/shart/imu_calibration
     * @param time_ms caculate max time, unit:ms
     * @return err::Err
     *
     * @maixpy maix.ext_dev.imu.IMU.calculate_calibration
     */
    err::Err calculate_calibration(uint64_t time_ms = 30 * 1000);

    /**
     * @brief Get calibration data
     * @return return an array, format is [acc_x_bias, acc_y_bias, acc_z_bias, gyro_x_bias, gyro_y_bias, gyro_z_bias]
     * If the calibration file cannot be found, an empty array will be returned.
     * @maixpy maix.ext_dev.imu.IMU.get_calibration
    */
    std::vector<double> get_calibration();
private:
    void* _param;
    std::string _driver;
};

typedef struct {
    char version[16];
    char id[256];
    char orientation[8];
    double tscale;
    double gscale;
    double ascale;
    double mscale;
} gcsv_header_t;

typedef struct {
    uint64_t t;
    struct {
        int x;
        int y;
        int z;
    } gyro;
    struct {
        int x;
        int y;
        int z;
    } acc;
    struct {
        int x;
        int y;
        int z;
    } mag;
} gcsv_info_t;

typedef struct {
    FILE *f;
    gcsv_header_t header;
} gcsv_handle_t;

static int _gcsv_init(gcsv_handle_t *handle, char *filename, gcsv_header_t *header)
{
    if (!header || !handle) return -1;

    memset(handle, 0, sizeof(gcsv_handle_t));
    handle->f = fopen(filename, "w");
    if (!handle->f)
        return -1;
    memcpy(&handle->header, header, sizeof(gcsv_header_t));

    char header_str[512];
    snprintf(header_str, sizeof(header_str),
    "GYROFLOW IMU LOG\n"
    "version:%s\n"
    "id,%s\n"
    "orientation,%s\n"
    "tscale,%f\n"
    "gscale,%.11lf\n"
    "ascale,%.11lf\n"
    "t,gx,gy,gz,ax,ay,az\n",
    header->version, header->id, header->orientation,
    header->tscale, header->gscale, header->ascale);

    fwrite(header_str, strlen(header_str), 1, handle->f);
    return 0;
}

static int _gcsv_write(gcsv_handle_t *handle, gcsv_info_t *info)
{
    if (!info || !handle) return -1;

    char info_str[512];
    snprintf(info_str, sizeof(info_str),
    "%ld,%d,%d,%d,%d,%d,%d\n",
    info->t, info->gyro.x, info->gyro.y, info->gyro.z, info->acc.x, info->acc.y, info->acc.z);
    fwrite(info_str, (size_t)strlen(info_str), 1, handle->f);
    return 0;
}

static int _gcsv_deinit(gcsv_handle_t *handle)
{
    if (!handle) return -1;
    if (handle->f) {
        fclose(handle->f);
    }

    return 0;
}

/**
 * Gcsv class
 * @maixpy maix.ext_dev.imu.Gcsv
 */
class Gcsv {
public:
    /**
     * @brief Construct a new IMU object
     * @maixpy maix.ext_dev.imu.Gcsv.__init__
     */
    Gcsv()
    {
        _is_opened = false;
    }
    ~Gcsv()
    {
        if (_is_opened) {
            close();
        }
    }

    /**
     * @brief Open a file
     * @param path the path where data will be saved
     * @param tscale time scale, default is 0.001
     * @param gscale gyroscope scale factor, default is 1, unit:g
     * @param ascale accelerometer scale factor, default is 1, unit:radians/second
     * @param mscale magnetometer scale factor, default is 1(unused)
     * @param version version number, default is "1.3"
     * @param id identifier for the IMU, default is "imu"
     * @param orientation sensor orientation, default is "YxZ"
     * @return error code
     * @maixpy maix.ext_dev.imu.Gcsv.open
     */
    err::Err open(std::string path, double tscale = 0.001, double gscale = 1, double ascale = 1, double mscale = 1, std::string version = "1.3", std::string id = "imu", std::string orientation = "YxZ") {
        if (_is_opened) {
            return err::ERR_NONE;
        }
        _header.tscale = tscale;
        _header.gscale = gscale;
        _header.ascale = ascale;
        _header.mscale = mscale;
        strncpy(_header.version, version.c_str(), sizeof(_header.version));
        strncpy(_header.id, id.c_str(), sizeof(_header.id));
        strncpy(_header.orientation, orientation.c_str(), sizeof(_header.orientation));
        _gcsv_init(&_handle, (char *)path.c_str(), &_header);
        _is_opened = true;
        return err::ERR_NONE;
    }

    /**
     * @brief Close file
     * @return error code
     * @maixpy maix.ext_dev.imu.Gcsv.close
     */
    err::Err close() {
        if (!_is_opened) {
            return err::ERR_NONE;
        }
        _gcsv_deinit(&_handle);
        _is_opened = false;
        return err::ERR_NONE;
    }

    /**
     * @brief Check if the object is already open
     * @return true, opened; false, not opened
     * @maixpy maix.ext_dev.imu.Gcsv.is_opened
     */
    bool is_opened() {
        return _is_opened;
    }

    /**
     * @brief Write imu data to gcsv file
     * @param t Timestamp of the current data. The actual value is equal to t * tscale. unit:s
     * @param gyro Gyroscope data must be an array consisting of x, y, and z-axis data. The actual value is equal to gyro * gscale. unit:g
     * @param acc Acceleration data must be an array consisting of x, y, and z-axis data. The actual value is equal to acc * ascale.unit:radians/second
     * @param mag Magnetic data must be an array consisting of x, y, and z-axis data. Currently not supported.
     * @maixpy maix.ext_dev.imu.Gcsv.write
     */
    err::Err write(double timestamp, std::vector<double> gyro, std::vector<double> acc, std::vector<double> mag = std::vector<double>()) {
        gcsv_info_t info = {0};
        info.t = timestamp;
        if (gyro.size() == 3) {
            info.gyro.x = (int)(gyro[0] / _header.gscale);
            info.gyro.y = (int)(gyro[1] / _header.gscale);
            info.gyro.z = (int)(gyro[2] / _header.gscale);
        }

        if (acc.size() == 3) {
            info.acc.x = (int)(acc[0] / _header.ascale);
            info.acc.y = (int)(acc[1] / _header.ascale);
            info.acc.z = (int)(acc[2] / _header.ascale);
        }

        _gcsv_write(&_handle, &info);
        return err::ERR_NONE;
    }
private:
    gcsv_handle_t _handle;
    gcsv_header_t _header;
    bool _is_opened;
};

}
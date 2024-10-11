
#include "maix_basic.hpp"
#include "maix_imu.hpp"
#include "maix_qmi8658.hpp"


namespace maix::ext_dev::imu {

typedef struct {
    union {
        maix::ext_dev::qmi8658::QMI8658 *qmi8658;
    } driver;
} imu_param_t;

IMU::IMU(std::string driver, int i2c_bus, int addr, int freq, imu::Mode mode, imu::AccScale acc_scale,
                imu::AccOdr acc_odr, imu::GyroScale gyro_scale, imu::GyroOdr gyro_odr, bool block)
{
    err::check_bool_raise(driver == "qmi8658", "Only support qmi8658 now");
    imu_param_t *param = (imu_param_t *)malloc(sizeof(imu_param_t));
    err::check_null_raise(param, "Failed to malloc param");


    param->driver.qmi8658 = new maix::ext_dev::qmi8658::QMI8658(i2c_bus, addr, freq, mode, acc_scale, acc_odr, gyro_scale, gyro_odr, block);
    _param = (void *)param;
    _driver = driver;
}

IMU::~IMU()
{
    if (_param) {
        imu_param_t *param = (imu_param_t *)_param;
        if (_driver == "qmi8658") {
            delete param->driver.qmi8658;
            param->driver.qmi8658 = NULL;
        }
        free(_param);
        _param = NULL;
    }
}

std::vector<float> IMU::read()
{
    std::vector<float> out;
    imu_param_t *param = (imu_param_t *)_param;
    if (_driver == "qmi8658") {
        out = param->driver.qmi8658->read();
    }
    return out;
}

}

#include "maix_basic.hpp"
#include "maix_imu.hpp"
#include "maix_qmi8658.hpp"

#define CALIBRATION_DATA_PATH "/maixapp/share/imu_calibration"
namespace maix::ext_dev::imu {

typedef struct {
    union {
        maix::ext_dev::qmi8658::QMI8658 *qmi8658;
    } driver;
    double bias[6];
} imu_param_t;

IMU::IMU(std::string driver, int i2c_bus, int addr, int freq, imu::Mode mode, imu::AccScale acc_scale,
                imu::AccOdr acc_odr, imu::GyroScale gyro_scale, imu::GyroOdr gyro_odr, bool block)
{
    err::check_bool_raise(driver == "qmi8658", "Only support qmi8658 now");
    imu_param_t *param = (imu_param_t *)malloc(sizeof(imu_param_t));
    err::check_null_raise(param, "Failed to malloc param");

    memset(param->bias, 0, sizeof(param->bias));
    std::vector<double> calibration_data = get_calibration();
    for (size_t i = 0; i < calibration_data.size(); i ++) {
        param->bias[i] = calibration_data[i];
    }
    // log::info("load calibration data: {%f, %f, %f, %f, %f, %f}",
    //         param->bias[0], param->bias[1], param->bias[2], param->bias[3], param->bias[4], param->bias[5]);
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

err::Err IMU::calculate_calibration(uint64_t time_ms)
{
    imu_param_t *param = (imu_param_t *)_param;
    uint64_t start_ms = time::ticks_ms();
    uint64_t last_ms = start_ms;
    uint64_t caculate_total_time = time_ms;
    double bias[6] = {0};
    double total_bias[6] = {0};
    int count = 0;
    while (!app::need_exit() && time::ticks_ms() - start_ms <= caculate_total_time) {
        auto res = read();
        for (size_t i = 0; i < sizeof(total_bias) / sizeof(total_bias[0]); i ++) {
            total_bias[i] += res[i];
        }
        count ++;
        if (time::ticks_ms() - last_ms >= 1000) {
            log::info("caculate %d/%d", (time::ticks_ms() - start_ms) / 1000, caculate_total_time / 1000);
            last_ms = time::ticks_ms();
        }
    }

    for (size_t i = 0; i < sizeof(total_bias) / sizeof(total_bias[0]); i ++) {
        bias[i] = total_bias[i] / count;
    }

    log::info("calibration data:");
    log::info("acc bias x:%f y:%f z:%f", bias[0], bias[1], bias[2]);
    log::info("gyro bias x:%f y:%f z:%f", bias[3], bias[4], bias[5]);

    memcpy(param->bias, bias, sizeof(bias));

    std::string path = CALIBRATION_DATA_PATH;
    FILE *f = fopen(path.c_str(), "w");
    if (!f) {
        log::error("open %s failed!", path.c_str());
        return err::ERR_RUNTIME;
    }

    char str[128];
    for (size_t i = 0; i < sizeof(total_bias) / sizeof(total_bias[0]); i ++) {
        snprintf(str, sizeof(str), "%f\n", bias[i]);
        fwrite(str, strlen(str), 1, f);
    }

    fclose(f);

    return err::ERR_NONE;
}

std::vector<double> IMU::get_calibration()
{
    FILE *f = fopen(CALIBRATION_DATA_PATH, "r");
    if (!f) {
        return std::vector<double>();
    }

    char buffer[128];
    std::vector<double> bias(6);
    int count = 0;
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        log::info("%s", buffer);
        bias[count ++] = atof(buffer);
    }
    fclose(f);

    return bias;
}

}
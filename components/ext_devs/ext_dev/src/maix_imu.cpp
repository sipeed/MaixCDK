
#include "maix_basic.hpp"
#include "maix_imu.hpp"
#include "maix_qmi8658.hpp"
#include "maix_ahrs_type.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include "maix_i2c.hpp"

using json = nlohmann::json;


#define CALIBRATION_DATA_PATH "/maixapp/share/misc/imu_calibration"
#define CALIBRATION_DATA_PATH2 "/maixapp/share/misc/imu_calibration.json"
namespace maix::ext_dev::imu {

enum class driver_type
{
    qmi8658
};

typedef struct {
    union {
        maix::ext_dev::qmi8658::QMI8658 *qmi8658;
    } driver;
    double bias[6];
    driver_type type;
} imu_param_t;


std::vector<ext_dev::imu::IMUInfo> get_imu_info()
{
    std::vector<ext_dev::imu::IMUInfo> info;
#if PLATFORM_MAIXCAM
    int default_i2c = 4;
    auto bus = peripheral::i2c::I2C(default_i2c, peripheral::i2c::Mode::MASTER, 200000, peripheral::i2c::AddrSize::SEVEN_BIT);
    auto res = bus.scan();
    for(auto r : res)
    {
        if(r == 0x6B) // QMI8658 default i2c addr
        {
            ext_dev::imu::IMUInfo qmi8658_info;
            qmi8658_info.name = "QMI8658";
            qmi8658_info.driver = "qmi8658";
            qmi8658_info.i2c_bus = default_i2c;
            qmi8658_info.addr = 0x6B;
            qmi8658_info.have_mag = false; // QMI8658 does not have mag
            info.push_back(qmi8658_info);
        }
    }
#elif PLATFORM_MAIXCAM2
    int default_i2c = 1;
    auto iio_device_base_path = "/sys/bus/iio/devices/iio:device";
    auto collect_iio_device_names = std::vector<std::string>();
    int idx = 0;
    while (!app::need_exit()) {
        auto path = iio_device_base_path + std::to_string(idx) + "/name";
        FILE *f = fopen(path.c_str(), "r");
        if (f) {
            char name[256];
            if (fgets(name, sizeof(name), f) != NULL) {
                name[strcspn(name, "\r\n")] = 0;
                collect_iio_device_names.push_back(name);
            }
            fclose(f);
            idx ++;
        } else {
            break;
        }
    }

    if ((std::find(collect_iio_device_names.begin(), collect_iio_device_names.end(), "lsm6dsox_gyro") != collect_iio_device_names.end())) {
        ext_dev::imu::IMUInfo imu_info;
        imu_info.name = "LSM6DSOWTR";
        imu_info.driver = "lsm6dsowtr";
        imu_info.i2c_bus = default_i2c;
        imu_info.addr = 0x6B;
        imu_info.have_mag = false;
        info.push_back(imu_info);
    } else {
        return info;
    }
#endif
    return info;
}



IMU::IMU(std::string driver, int i2c_bus, int addr, int freq, imu::Mode mode, imu::AccScale acc_scale,
                imu::AccOdr acc_odr, imu::GyroScale gyro_scale, imu::GyroOdr gyro_odr, bool block)
{
    err::check_bool_raise(driver == "qmi8658", "Only support qmi8658 now");
    imu_param_t *param = (imu_param_t *)malloc(sizeof(imu_param_t));
    err::check_null_raise(param, "Failed to malloc param");

    if(driver == "qmi8658")
    {
        param->type = driver_type::qmi8658;
    }
    else
    {
        free(param);
        throw err::Exception(err::ERR_ARGS, "not support " + driver);
    }

    _param = (void *)param;
    _driver = driver;
    _mode = mode;

    memset(param->bias, 0, sizeof(param->bias));
    std::vector<double> calibration_data = get_calibration();
    for (size_t i = 0; i < calibration_data.size(); i ++) {
        param->bias[i] = calibration_data[i];
    }
    // log::info("load calibration data: {%f, %f, %f, %f, %f, %f}",
    //         param->bias[0], param->bias[1], param->bias[2], param->bias[3], param->bias[4], param->bias[5]);
    param->driver.qmi8658 = new maix::ext_dev::qmi8658::QMI8658(i2c_bus, addr, freq, mode, acc_scale, acc_odr, gyro_scale, gyro_odr, block);
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
    if (param->type == driver_type::qmi8658) {
        out = param->driver.qmi8658->read();
    }
    return out;
}

ext_dev::imu::IMUData IMU::read_all(bool calib_gryo, bool radian)
{
    ext_dev::imu::IMUData res;
    auto data = read();
    switch(_mode)
    {
        case Mode::ACC_ONLY:
            res.acc.x = data[0];
            res.acc.y = data[1];
            res.acc.z = data[2];
            res.temp = data[3];
            break;
        case Mode::GYRO_ONLY:
            res.gyro.x = data[0];
            res.gyro.y = data[1];
            res.gyro.z = data[2];
            res.temp = data[3];
            break;
        case Mode::DUAL:
            res.acc.x = data[0];
            res.acc.y = data[1];
            res.acc.z = data[2];
            res.gyro.x = data[3];
            res.gyro.y = data[4];
            res.gyro.z = data[5];
            res.temp = data[6];
            break;
        default:
            throw err::Exception(err::ERR_NOT_IMPL);
            break;
    }
    if(calib_gryo)
    {
        res.gyro.x -= calib_gyro_data.x;
        res.gyro.y -= calib_gyro_data.y;
        res.gyro.z -= calib_gyro_data.z;
    }
    if(radian)
    {
        res.gyro.x *= ahrs::DEG2RAD;
        res.gyro.y *= ahrs::DEG2RAD;
        res.gyro.z *= ahrs::DEG2RAD;
    }
    return res;
}

Vector3f IMU::calib_gyro(uint64_t time_ms, int interval_ms, const std::string &save_id)
{
    uint64_t start_ms = time::ticks_ms();
    uint64_t last_ms = 0;
    uint64_t last_print_ms = 0;
    double total_bias[3] = {0};
    int count = 0;

    log::info0("calib_gyro now for %ld ms, !! don't move !! ...", time_ms);
    while (!app::need_exit() && time::ticks_ms() - start_ms <= time_ms)
    {
        uint64_t now = time::ticks_ms();
        if ((int)(now - last_ms) >= interval_ms)
        {
            last_ms = now;
            IMUData data = read_all(false, false);

            total_bias[0] += data.gyro.x;
            total_bias[1] += data.gyro.y;
            total_bias[2] += data.gyro.z;

            count++;
        }
        if (now - last_print_ms >= 500)
        {
            last_print_ms = now;
            log::print(log::LogLevel::LEVEL_INFO, ".");
            log::flush();
        }
    }
    log::print(log::LogLevel::LEVEL_INFO, "\n");

    _calib_gyro_loaded = true;
    if (count > 0)
    {
        calib_gyro_data.x = static_cast<float>(total_bias[0] / count);
        calib_gyro_data.y = static_cast<float>(total_bias[1] / count);
        calib_gyro_data.z = static_cast<float>(total_bias[2] / count);
    }
    else
    {
        calib_gyro_data.x = 0;
        calib_gyro_data.y = 0;
        calib_gyro_data.z = 0;
    }
    log::info("calib_gyro done, x: %.2f, y: %.2f, z %.2f", calib_gyro_data.x, calib_gyro_data.y, calib_gyro_data.z);

    if(!save_id.empty())
    {
        log::info("calib_gyro save ...");
        save_calib_gyro(calib_gyro_data, save_id);
        log::info("calib_gyro save done");
    }

    return calib_gyro_data;
}

bool IMU::calib_gyro_exists(const std::string &save_id)
{
    if (fs::exists(CALIBRATION_DATA_PATH2))
    {
        std::ifstream f(CALIBRATION_DATA_PATH2);
        try
        {
            json data = json::parse(f);

            if (data.contains(save_id) && data[save_id].contains("gyro"))
            {
                return true;
            }
        }
        catch (const std::exception &e)
        {
            log::error("%s %s", "Failed to parse or load gyro calibration:", e.what());
        }
    }
    return false;
}

Vector3f IMU::load_calib_gyro(const std::string &save_id)
{
    // load CALIBRATION_DATA_PATH2 with json
    Vector3f calib;
    calib.x = 0;
    calib.y = 0;
    calib.z = 0;
    if (fs::exists(CALIBRATION_DATA_PATH2))
    {
        std::ifstream f(CALIBRATION_DATA_PATH2);
        try
        {
            json data = json::parse(f);

            if (data.contains(save_id) && data[save_id].contains("gyro"))
            {
                const auto &gyro = data[save_id]["gyro"];
                if (gyro.contains("x") && gyro.contains("y") && gyro.contains("z"))
                {
                    calib.x = gyro["x"].get<float>();
                    calib.y = gyro["y"].get<float>();
                    calib.z = gyro["z"].get<float>();
                }
            }
        }
        catch (const std::exception &e)
        {
            log::error("%s %s", "Failed to parse or load gyro calibration:", e.what());
        }
    }

    calib_gyro_data = calib;
    _calib_gyro_loaded = true;
    return calib;
}

err::Err IMU::save_calib_gyro(const Vector3f &calib, const std::string &save_id)
{
    json data;

    if(save_id.empty())
        return err::ERR_ARGS;

    // 先尝试读取已有文件内容
    if (fs::exists(CALIBRATION_DATA_PATH2))
    {
        std::ifstream f(CALIBRATION_DATA_PATH2);
        try
        {
            data = json::parse(f);
        }
        catch (const std::exception &e)
        {
            log::error("%s %s", "Failed to parse calibration JSON:", e.what());
            data = json::object();
        }
    }
    else
    {
        data = json::object();
    }

    // 更新或添加 save_id 下的 gyro 数据
    data[save_id]["gyro"]["x"] = calib.x;
    data[save_id]["gyro"]["y"] = calib.y;
    data[save_id]["gyro"]["z"] = calib.z;

    // 写回文件
    auto dir = fs::dirname(CALIBRATION_DATA_PATH2);
    if (!dir.empty() && !fs::exists(dir))
    {
        if (fs::mkdir(dir) != err::ERR_NONE)
        {
            log::error("%s %s","Failed to create directory:", dir.c_str());
            return err::ERR_IO;
        }
    }
    std::ofstream ofs(CALIBRATION_DATA_PATH2);
    if (!ofs.is_open())
    {
        log::error("%s %s", "Failed to open calibration file for writing:", CALIBRATION_DATA_PATH2);
        return err::ERR_IO;
    }
    ofs << data.dump(4);  // 4空格缩进，美化输出
    ofs.close();

    return err::ERR_NONE;
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
            log::info("calibrate %d/%d", (time::ticks_ms() - start_ms) / 1000, caculate_total_time / 1000);
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
        bias[count ++] = atof(buffer);
    }
    fclose(f);

    return bias;
}

}
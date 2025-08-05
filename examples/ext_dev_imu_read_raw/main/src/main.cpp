
#include "maix_basic.hpp"
#include "main.h"
#include "maix_imu.hpp"

using namespace maix;
using namespace ext_dev;

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 <path> : read imu data and save in gcsv format\r\n"
    "1 : caculate calibration\r\n"
    "\r\n"
    "Example: ./maix_imu 0 output.mp4\r\n"
    "Example: ./maix_imu 1\r\n"
    "==================================\r\n");
}

int _main(int argc, char* argv[])
{
    int cmd = 0;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            helper();
            return 0;
        } else {
            cmd = atoi(argv[1]);
        }
    } else {
        helper();
        return 0;
    }

    switch (cmd) {
    case 0:
    {
        imu::IMU imu("qmi8658");
        imu::Gcsv gcsv = imu::Gcsv();
        double t_scale = 0.001;
        double g_scale = ((double)1024 * M_PI / 180) / 32768;
        double a_scale = ((double)16 / 32768);
        gcsv.open("/root/output.gcsv", t_scale, g_scale, a_scale);
        uint64_t first_write_ms = time::ticks_ms();
        while (!app::need_exit()) {
            auto res = imu.read();
            printf("\n");
            log::info("------------------------");
            printf("acc x:  %f\t", res[0]);
            printf("acc y:  %f\t", res[1]);
            printf("acc z:  %f\n", res[2]);
            printf("gyro x: %f (%f)\t", res[3], res[3] * M_PI / 180);
            printf("gyro y: %f (%f)\t", res[4], res[4] * M_PI / 180);
            printf("gyro z: %f (%f)\n", res[5], res[5] * M_PI / 180);
            printf("temp:   %f\n", res[6]);
            log::info("------------------------\n");

            double t = (double)(time::ticks_ms() - first_write_ms);
            std::vector<double> g = {res[3] * M_PI / 180, res[4] * M_PI / 180, res[5] * M_PI / 180};
            std::vector<double> a = {res[0], res[1], res[2]};
            gcsv.write(t, g, a);
            time::sleep_ms(500);
        }
    }
    break;
    case 1:
    {
        imu::IMU imu("qmi8658");
        imu.calculate_calibration();
    }
    break;
    default:
        helper();
    break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}



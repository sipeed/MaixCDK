
#include "maix_basic.hpp"
#include "main.h"
#include "maix_imu.hpp"
#include "maix_ahrs_mahony.hpp"
#include "maix_touchscreen.hpp"

#include "maix_image.hpp"
#include "maix_image_cv.hpp"
#include "maix_display.hpp"
#include "opencv2/opencv.hpp"

using namespace maix;
using namespace ext_dev;

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Usage:\r\n"
    "\t imu_ahrs <calibrate>"
    "\t\tcalibrate:"
    "\t\t\t0 : only run\r\n"
    "\t\t\t1 : calibrate and run\r\n"
    "==================================\r\n");
}

bool is_in_button(int x, int y, std::vector<int> btn_pos)
{
    return x > btn_pos[0] && x < btn_pos[0] + btn_pos[2] && y > btn_pos[1] && y < btn_pos[1] + btn_pos[3];
}

std::vector<cv::Point2f> render_pose(float pitch, float roll, float yaw, bool radian)
{
    if(!radian)
    {
        pitch *= ahrs::DEG2RAD;
        roll *= ahrs::DEG2RAD;
        yaw *= ahrs::DEG2RAD;
    }
    // 构建绕 x 轴的旋转矩阵 (pitch)
    cv::Matx33f Rx(1, 0, 0,
                   0, cos(pitch), -sin(pitch),
                   0, sin(pitch),  cos(pitch));

    // 绕 y 轴的旋转矩阵 (roll)
    cv::Matx33f Ry(cos(roll), 0, sin(roll),
                   0,        1, 0,
                   -sin(roll), 0, cos(roll));

    // 绕 z 轴的旋转矩阵 (yaw)
    cv::Matx33f Rz(cos(yaw), -sin(yaw), 0,
                   sin(yaw),  cos(yaw), 0,
                   0,          0,         1);

    // 总旋转矩阵：Z-Y-X 顺序
    cv::Matx33f R = Rz * Ry * Rx;

    // 定义单位向量
    cv::Vec3f x_axis(1, 0, 0);
    cv::Vec3f y_axis(0, 1, 0);
    cv::Vec3f z_axis(0, 0, 1);

    // 旋转单位向量
    cv::Vec3f x_rot = R * x_axis;
    cv::Vec3f y_rot = R * y_axis;
    cv::Vec3f z_rot = R * z_axis;

    // 投影到 x-z 平面：只保留 x 和 z 分量
    std::vector<cv::Point2f> projections;
    projections.emplace_back(x_rot[0], x_rot[2]);
    projections.emplace_back(y_rot[0], y_rot[2]);
    projections.emplace_back(z_rot[0], z_rot[2]);

    return projections;
}

void draw_image(image::Image &img, tensor::Vector3f &angle, bool dir_cam)
{
    float len = 0.35;
    int min_edge = std::min(img.width(), img.height());
    float obj_len = min_edge * len;
    float offset_x = img.width() * 0.5;
    float offset_y = img.height() * 0.5;
    auto v = render_pose(angle.x, angle.y, angle.z, false);
    v[0].x *= obj_len;
    v[1].x *= obj_len;
    v[2].x *= obj_len;
    v[0].y *= obj_len;
    v[1].y *= obj_len;
    v[2].y *= obj_len;
    v[0].x += offset_x;
    v[1].x += offset_x;
    v[2].x += offset_x;
    v[0].y = img.height() - v[0].y - offset_y;
    v[1].y = img.height() - v[1].y - offset_y;
    v[2].y = img.height() - v[2].y - offset_y;
    if(dir_cam)
    {
        img.draw_line(offset_x, img.height() - offset_y, v[1].x, v[1].y, image::COLOR_RED, 5);
        img.draw_line(v[2].x, v[2].y, v[0].x, v[0].y, image::COLOR_GRAY, 1);
        img.draw_line(offset_x, img.height() - offset_y, v[0].x, v[0].y, image::COLOR_WHITE, 5);
        img.draw_line(offset_x, img.height() - offset_y, v[2].x, v[2].y, image::COLOR_GREEN, 5);
    }
    else
    {
        img.draw_line(v[1].x, v[1].y, v[0].x, v[0].y, image::COLOR_GRAY, 1);
        img.draw_line(offset_x, img.height() - offset_y, v[1].x, v[1].y, image::COLOR_RED, 5);
        img.draw_line(offset_x, img.height() - offset_y, v[0].x, v[0].y, image::COLOR_WHITE, 5);
        img.draw_line(offset_x, img.height() - offset_y, v[2].x, v[2].y, image::COLOR_GREEN, 5);
    }
    img.draw_string(v[0].x, v[0].y, "x", image::COLOR_WHITE, 1.5);
    img.draw_string(v[1].x, v[1].y, "y", image::COLOR_RED, 1.5);
    img.draw_string(v[2].x, v[2].y, "z", image::COLOR_GREEN, 1.5);
}

void show_msg(display::Display &disp, std::string &msg)
{
    auto img = image::Image(disp.width(), disp.height());
    auto size = image::string_size(msg, 1.5);
    img.draw_string((img.width() - size.width()) / 2, (img.height() - size.height()) / 2, msg, image::COLOR_WHITE, 1.5);
    disp.show(img);
}

int _main(int argc, char* argv[])
{
    int calibrate = 0;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            helper();
            return 0;
        } else {
            calibrate = atoi(argv[1]);
        }
    };

    float kp = 2;
    float ki = 0.01;
    int pitch_offset = 0;

    display::Display disp;
    touchscreen::TouchScreen ts;
    int ts_x = 0, ts_y = 0;
    bool ts_pressed = false;
    bool ts_last_pressed = false;
    std::string dir_btn_name = "x dir";
    std::string calib_btn_name = "Calibrate";
    auto dir_font_size = image::string_size(dir_btn_name);
    auto calib_font_size = image::string_size(calib_btn_name);
    std::vector<int> calib_btn_disp_pos = {disp.width() - 100, disp.height() - 50, 100, 50};
    std::vector<int> ret_btn_disp_pos = {0, disp.height() - 50, 100, 50};
    std::vector<int> dir_btn_disp_pos = {disp.width() / 2 - dir_font_size.width() / 2 - 10, disp.height() - 50, dir_font_size.width() + 20 , 50};

    imu::IMU *imu = nullptr;
    try
    {
        imu = new imu::IMU("default");
    }
    catch(...)
    {
        log::error("init IMU failed");
        return 1;
    }
    ahrs::MahonyAHRS ahrs(kp, ki);
    if(calibrate == 1)
    {
        log::info("now calibrate, please don't move device");
        imu->calib_gyro(10000);
    }
    else
    {
        imu->load_calib_gyro();
    }
    char temp_char[64];
    double last_time = time::ticks_s();
    while (!app::need_exit()) {
        auto data = imu->read_all(true, true); // use calibrate value and unit rad/s.
        double t = time::ticks_s();
        float dt = t - last_time;
        auto angle = ahrs.get_angle(data.acc, data.gyro, data.mag, dt);
        last_time = t;

        // make y axis same with camera(x rotate 90 degree)
        angle.x -= pitch_offset;

        // print
        //  ^z  / y(front)
        //  |  /
        //  | /
        //  . ————————> x(right)
        snprintf(temp_char, sizeof(temp_char), "pitch: %6.2f, roll: %6.2f, yaw: %6.2f", angle.x, angle.y, angle.z);
        // printf("%s\n", temp_char);

        // show on image
        auto img = image::Image(disp.width(), disp.height());
        img.draw_string(2, 4, temp_char, image::COLOR_WHITE, 1.5);
        snprintf(temp_char, sizeof(temp_char), "dt: %3dms, temp: %4.1f", (int)(dt * 1000), data.temp);
        img.draw_string(2, 4+32, temp_char, image::COLOR_WHITE, 1.5);
        draw_image(img, angle, pitch_offset == 90);

        // draw button
        img.draw_rect(calib_btn_disp_pos[0], calib_btn_disp_pos[1], calib_btn_disp_pos[2], calib_btn_disp_pos[3], image::COLOR_WHITE, 2);
        img.draw_string(calib_btn_disp_pos[0] + 10 , calib_btn_disp_pos[1] + (calib_btn_disp_pos[3] - calib_font_size.height()) / 2, calib_btn_name);
        img.draw_rect(ret_btn_disp_pos[0], ret_btn_disp_pos[1], ret_btn_disp_pos[2], ret_btn_disp_pos[3], image::COLOR_WHITE, 2);
        img.draw_string(ret_btn_disp_pos[0] + 10 , ret_btn_disp_pos[1] + (ret_btn_disp_pos[3] - calib_font_size.height()) / 2, "< Exit");
        img.draw_rect(dir_btn_disp_pos[0], dir_btn_disp_pos[1], dir_btn_disp_pos[2], dir_btn_disp_pos[3], image::COLOR_WHITE, 2);
        img.draw_string(dir_btn_disp_pos[0] + 10 , dir_btn_disp_pos[1] + (dir_btn_disp_pos[3] - calib_font_size.height()) / 2, dir_btn_name);

        disp.show(img);

        // check button event
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && is_in_button(ts_x, ts_y, ret_btn_disp_pos))
        {
            break;
        }
        else if (ts_pressed && !ts_last_pressed && is_in_button(ts_x, ts_y, dir_btn_disp_pos))
        {
            if(pitch_offset == 0)
                pitch_offset = 90;
            else
                pitch_offset = 0;
        }
        else if (ts_pressed && !ts_last_pressed && is_in_button(ts_x, ts_y, calib_btn_disp_pos))
        {
            int count = 6;
            while(count -- > 0)
            {
                std::string msg = "Place on desk, don't move.\nStart in " + std::to_string(count) + "s";
                show_msg(disp, msg);
                time::sleep(1);
            }
            log::info("now calibrate, please don't move device");
            std::string msg = "Calibrating, don't move, keep 10s";
            show_msg(disp, msg);
            imu->calib_gyro(10000);
            ahrs.reset();
            last_time = time::ticks_s();
        }
        ts_last_pressed = ts_pressed;

        // time::sleep_ms(1); //  release cpu for a while

        // make sure loop interval > 1ms
        while(time::ticks_s() - last_time < 0.001)
            time::sleep_us(100);
    }
    delete imu;
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



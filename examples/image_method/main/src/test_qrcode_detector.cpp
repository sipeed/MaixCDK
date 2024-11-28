#include "test_image.hpp"
#include "maix_image.hpp"
#include "maix_image_obj.hpp"
#include "maix_image_extra.hpp"

extern param_t g_param;

int test_qrcode_detector(image::Image *img) {
    static image::QRCodeDetector *qrcode_detector = nullptr;
    if (qrcode_detector == nullptr) {
        qrcode_detector = new image::QRCodeDetector();
        err::check_null_raise(qrcode_detector, "create qrcode detecetor failed!");
    }

    uint64_t t = time::ticks_ms(), t2 = 0;
    auto result = qrcode_detector->detect(img);
    t2 = time::ticks_ms(), log::info("test_qrcode_detector use %lld ms, fps:%f", t2 - t, 1000.0 / (t2 - t));
    for (auto &i : result)
    {
        log::info("result: %s", i.payload().c_str());
        std::vector<std::vector<int>> corners = i.corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
        }
    }

    // if (result.size() > 0)
    // {
    //     auto r = result[0];
    //     auto x = r.x();
    //     auto y = r.y();
    //     auto w = r.w();
    //     auto h = r.h();
    //     auto sensor_size = g_param.cam->get_sensor_size();
    //     float scale_x = (float)sensor_size[0] / img->width();
    //     float scale_y = (float)sensor_size[1] / img->height();

    //     float scale_valid = scale_x > scale_y ? scale_y : scale_x;
    //     int sensor_valid_w = img->width() * scale_valid;
    //     int sensor_valid_h = img->height() * scale_valid;
    //     log::info("scale valid:%f sensor valid solution:%dx%d", scale_valid, (int)sensor_valid_w, (int)sensor_valid_h);

    //     int sensor_oft_x = (sensor_size[0] - sensor_valid_w) / 2;
    //     int sensor_oft_y = (sensor_size[1] - sensor_valid_h) / 2;
    //     log::info("sensor oft_x:%d, oft_y:%d", sensor_oft_x, sensor_oft_y);

    //     auto dst_x = x - w;
    //     auto dst_y = y - h;
    //     auto dst_w = w * 2;
    //     auto dst_h = h * 2;

    //     auto dst_sensor_x = sensor_oft_x + dst_x * scale_x;
    //     auto dst_sensor_y = sensor_oft_y + dst_y * scale_y;
    //     auto dst_sensor_w = dst_w * scale_x;
    //     auto dst_sensor_h = dst_h * scale_y;
    //     log::info("sensor dst x:%d, y:%d, w:%d, h:%d", (int)dst_sensor_x, (int)dst_sensor_y, (int)dst_sensor_w, (int)dst_sensor_h);
    // }
    // static bool config_to_default = true;
    // static int qrcode_not_found_count = 0;
    // if (result.size() > 0) {
    //     auto r = result[0];
    //     auto x = r.x();
    //     auto y = r.y();
    //     auto w = r.w();
    //     auto h = r.h();
    //     auto sensor_size = g_param.cam->get_sensor_size();
    //     float scale_x = (float)sensor_size[0] / img->width();
    //     float scale_y = (float)sensor_size[1] / img->height();
    //     int sensor_x = (int)(x * scale_x + 63) & (~63);
    //     int sensor_y = (int)(y * scale_y + 63) & (~63);
    //     int sensor_w = (int)(w * scale_x + 63) & (~63);
    //     int sensor_h = (int)(h * scale_y + 63) & (~63);

    //     float scale_valid = scale_x > scale_y ? scale_y : scale_x;
    //     float sensor_valid_w = img->width() * scale_valid;
    //     float sensor_valid_h = img->height() * scale_valid;

    //     // sensor_x = sensor_x < 0 ? 0 : sensor_x;
    //     // sensor_y = sensor_y < 0 ? 0 : sensor_y;
    //     // sensor_w = sensor_w + sensor_x > input_size[0] ? input_size[0] - sensor_x : sensor_w;
    //     // sensor_h = sensor_h + sensor_y > input_size[1] ? input_size[1] - sensor_y : sensor_h;
    //     // log::info("sensor_size:%dx%d sensor windowing: %d, %d, %d, %d", sensor_size[0], sensor_size[1], sensor_x, sensor_y, sensor_w, sensor_h);
    //     // g_param.cam->set_windowing({sensor_x, sensor_y, sensor_w, sensor_h});

    //     // config_to_default = false;
    //     // qrcode_not_found_count = 0;
    // } else {
    //     // if (!config_to_default) {
    //     //     qrcode_not_found_count ++;
    //     //     if (qrcode_not_found_count > 5) {
    //     //         auto sensor_size = g_param.cam->get_sensor_size();
    //     //         g_param.cam->set_windowing({0, 0, sensor_size[0], sensor_size[1]});
    //     //         config_to_default = true;
    //     //     }
    //     // }
    // }

    return 0;
}
#ifndef __JASHGDCAJALP_CALIBRATION_HPP
#define __JASHGDCAJALP_CALIBRATION_HPP

#include <cstdlib>
#include <cmath>

struct CameraImageInfo {
    int w;      // 采集图像宽度
    int h;      // 采集图像高度
    int x1;     // 裁剪图像用的左上角x坐标
    int y1;     // 裁剪图像用的左上角y坐标
    int ww;     // 裁剪图像用的宽度
    int hh;     // 裁剪图像用的高度
};

inline static CameraImageInfo g_camera_img_info {
    -1,-1,-1,-1,-1,-1
};

struct CaliPoint {
    int x1;
    int y1;
    int x2;
    int y2;
};

void cali(int ow, int oh, CaliPoint cam_p, CaliPoint oth_p)
{
    if (ow <= 0 || oh <= 0 || cam_p.x2 <= cam_p.x1 || cam_p.y2 <= cam_p.y1 || oth_p.x2 <= oth_p.x1 || oth_p.y2 <= oth_p.y1) {
        printf("%s error args\n", __PRETTY_FUNCTION__);
        return;
    }
    // 放大 oth 图像
    int othr_w = oth_p.x2 - oth_p.x1;
    int othr_h = oth_p.y2 - oth_p.y1;
    int camr_w = cam_p.x2 - cam_p.x1;
    int camr_h = cam_p.y2 - cam_p.y1;

    float othr2camr_w = static_cast<float>(camr_w) / othr_w;
    float othr2camr_h = static_cast<float>(camr_h) / othr_h;

    int new_oth_w = static_cast<int>(ow * othr2camr_w);
    int new_oth_h = static_cast<int>(oh * othr2camr_h);
    new_oth_w += new_oth_w%2;
    new_oth_h += new_oth_h%2;

    // int new_oth_r_w = camr_w;
    // int new_oth_r_h = camr_h;
    int new_oth_x1 = static_cast<int>(oth_p.x1*othr2camr_w);
    int new_oth_y1 = static_cast<int>(oth_p.y1*othr2camr_h);

    int new_cam_w = cam_p.x1 + new_oth_w - new_oth_x1;
    int new_cam_h = cam_p.y1 + new_oth_h - new_oth_y1;
    int new_cam_x1 = cam_p.x1 - new_oth_x1;
    int new_cam_y1 = cam_p.y1 - new_oth_y1;
    int new_cam_ww = new_cam_w - new_cam_x1;
    int new_cam_hh = new_cam_h - new_cam_y1;

    g_camera_img_info.w = new_cam_w + new_cam_w%2;
    g_camera_img_info.h = new_cam_h + new_cam_h%2;
    g_camera_img_info.x1 = new_cam_x1 + new_cam_x1%2;
    g_camera_img_info.y1 = new_cam_y1 + new_cam_y1%2;
    g_camera_img_info.ww = new_cam_ww + new_cam_ww%2;
    g_camera_img_info.hh = new_cam_hh + new_cam_hh%2;
}

#endif // __JASHGDCAJALP_CALIBRATION_HPP

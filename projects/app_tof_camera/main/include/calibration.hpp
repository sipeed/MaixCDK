#ifndef __JASHGDCAJALP_CALIBRATION_HPP
#define __JASHGDCAJALP_CALIBRATION_HPP

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "dbg.hpp"


inline static std::string CFG_PATH("./assets/cfg.bin");

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
        eprintln("In function<%s>: error args!", __PRETTY_FUNCTION__);
        return;
    }
    // 放大 oth 图像
    int othr_w = oth_p.x2 - oth_p.x1;
    int othr_h = oth_p.y2 - oth_p.y1;
    int camr_w = cam_p.x2 - cam_p.x1;
    int camr_h = cam_p.y2 - cam_p.y1;
    println("othr_w:%dx%d:othr_h", othr_w, othr_h);
    println("camr_w:%dx%d:camr_h", camr_w, camr_h);

    float othr2camr_w = static_cast<float>(camr_w) / othr_w;
    float othr2camr_h = static_cast<float>(camr_h) / othr_h;
    println("othr2camr: %0.2f, othr2camr_h: %0.2f", othr2camr_w, othr2camr_h);

    int new_oth_w = static_cast<int>(ow * othr2camr_w);
    int new_oth_h = static_cast<int>(oh * othr2camr_h);
    new_oth_w += new_oth_w%2;
    new_oth_h += new_oth_h%2;
    println("new_oth_w:%dx%d:new_oth_h", new_oth_w, new_oth_h);

    // int new_oth_r_w = camr_w;
    // int new_oth_r_h = camr_h;
    int new_oth_x1 = static_cast<int>(oth_p.x1*othr2camr_w);
    int new_oth_y1 = static_cast<int>(oth_p.y1*othr2camr_h);
    println("new_oth_point:(%d, %d)", new_oth_x1, new_oth_y1);

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

    println("cali res: w:%d, h:%d, (%d,%d), %dx%d",
        g_camera_img_info.w, g_camera_img_info.h, g_camera_img_info.x1,
        g_camera_img_info.y1, g_camera_img_info.ww, g_camera_img_info.hh);

    if (g_camera_img_info.x1 < 0) {
        g_camera_img_info.w -= g_camera_img_info.x1;
        g_camera_img_info.x1 = 0;
    }
    if (g_camera_img_info.y1 < 0) {
        g_camera_img_info.h -= g_camera_img_info.y1;
        g_camera_img_info.y1 = 0;
    }

    g_camera_img_info.w = ow;
    g_camera_img_info.h = oh;
    g_camera_img_info.x1 = g_camera_img_info.y1 = 0;
    g_camera_img_info.hh = oh;
    g_camera_img_info.ww = ow;
}

void save_cali_cfg()
{
    // println("save cfg to ./cfg.bin");
    std::vector<int> cfg = {
        g_camera_img_info.w,
        g_camera_img_info.h,
        g_camera_img_info.x1,
        g_camera_img_info.y1,
        g_camera_img_info.ww,
        g_camera_img_info.hh
    };

    std::ofstream outfile(CFG_PATH, std::ios::binary);
    // std::ofstream outfile("./cfg.bin", std::ios::binary);
    if (!outfile.is_open()) {
        eprintln("save cannot open file: %s", CFG_PATH.c_str());
        return;
    }
    size_t size = cfg.size();
    outfile.write(reinterpret_cast<char*>(&size), sizeof(size));
    outfile.write(reinterpret_cast<char*>(cfg.data()), size*sizeof(int));
    outfile.flush();
    outfile.close();
    // println("save finish");
}

int load_cali_cfg()
{
    std::vector<int> cfg;
    std::ifstream infile(CFG_PATH, std::ios::binary);
    if (!infile.is_open()) return -1;
    size_t read_size;
    infile.read(reinterpret_cast<char*>(&read_size), sizeof(read_size));
    if (read_size != 6) {
        eprintln("cfg.bin len error");
        return -2;
    }
    cfg.resize(read_size);
    infile.read(reinterpret_cast<char*>(cfg.data()), read_size*sizeof(int));
    infile.close();

    if (cfg.size() != read_size) return -1;

    if (cfg[0] <= 0 || cfg[1] <= 0 || cfg[2] < 0 || cfg[3] < 0 || cfg[4] < 0 || cfg[5] < 0) {
        eprintln("cfg.bin data err");
        return -3;
    }
    g_camera_img_info.w = cfg[0];
    g_camera_img_info.h = cfg[1];
    g_camera_img_info.x1 = cfg[2];
    g_camera_img_info.y1 = cfg[3];
    g_camera_img_info.ww = cfg[4];
    g_camera_img_info.hh = cfg[5];
    return 0;
}

void clear_cali_cfg()
{
    std::filesystem::remove(CFG_PATH);
}

#endif // __JASHGDCAJALP_CALIBRATION_HPP

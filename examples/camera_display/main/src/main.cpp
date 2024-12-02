

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "maix_fp5510.hpp"
#include "main.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace maix;
using namespace maix::ext_dev;

static int cmd_init(void);
static int cmd_loop(camera::Camera *cam, display::Display *disp);

static bool camera2_enable = false;
static bool disp2_enable = false;
fp5510::FP5510 *g_fp5510e = nullptr;

int _main(int argc, char* argv[])
{
    uint64_t t1, t2, t3;
    float fps = 0;
    char buf[128] = {0};

    int cam_w = -1;
    int cam_h = -1;
    image::Format cam_fmt = image::Format::FMT_RGB888;
    int cam_fps = -1;
    int cam_buffer_num = 3;
    bool raw = true;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            log::info("./camera_display <width> <height> <format> <fps> <buff_num> <raw>");
            log::info("example: ./camera_display 640 480 0 60 2 1");
            exit(0);
        } else {
            cam_w = atoi(argv[1]);
        }
    }
    if (argc > 2) cam_h = atoi(argv[2]);
    if (argc > 3) cam_fmt = (image::Format)atoi(argv[3]);
    if (argc > 4) cam_fps = atoi(argv[4]);
    if (argc > 5) cam_buffer_num = atoi(argv[5]);
    if (argc > 6) raw = atoi(argv[6]) ? true : false;
    log::info("Camera(%s) width:%d height:%d format:%s fps:%d buffer_num:%d raw:%d", camera::get_device_name().c_str(),
        cam_w, cam_h, image::fmt_names[cam_fmt].c_str(), cam_fps, cam_buffer_num, raw);

    camera::Camera cam = camera::Camera(cam_w, cam_h, cam_fmt, "", cam_fps, cam_buffer_num, true, raw);
    camera::Camera *cam2 = NULL;
    display::Display disp = display::Display();
    display::Display *disp2 = NULL;
    log::info("camera and display open success");
    log::info("camera size: %dx%d", cam.width(), cam.height());
    log::info("disp size: %dx%d format:%s", disp.width(), disp.height(), image::fmt_names[disp.format()].c_str());
    cam.skip_frames(5);
    err::check_bool_raise(!cmd_init(), "cmd init failed!");
    uint64_t last_loop_ms = time::ticks_ms();

    while(!app::need_exit())
    {
        // cmd loop
        cmd_loop(&cam, &disp);

        // time when start read image from camera
        t1 = time::ticks_ms();

        // read image from camera
        image::Image *img = cam.read();

        if (camera2_enable) {
            if (!cam2) {
                cam2 = cam.add_channel(cam.width(), cam.height());
                err::check_null_raise(cam2, "camera add channel failed!");
            }
            image::Image *img2 = cam2->read();
            if (img2->format() == image::Format::FMT_RGB888) {
                image::Image *resize_img2 = img2->resize(img->width()/2, img->height()/2);
                img->draw_image(0, 0, *resize_img2);
                delete resize_img2;
            }
            delete img2;
        } else {
            if (cam2) {
                delete cam2;
                cam2 = NULL;
            }
        }

        if (img->format() == image::Format::FMT_RGB888) {
            img->draw_string(0, 10, buf, image::COLOR_GREEN, 1.5);
        }

        if (disp2_enable) {
            if (!disp2) {
                disp2 = disp.add_channel();
                err::check_null_raise(disp2, "display add channel failed!");
            }
            image::Image disp2_img = image::Image(disp2->width(), disp2->height(), disp2->format());
            disp2_img.draw_string(0, 10, "HELLO MAIXCAM", image::COLOR_GREEN, 1.5);
            disp2_img.draw_rect(0, 0, disp2->width(), disp2->height(), image::Color::from_bgra(0,0xff,0,0.5), -1);
            disp2_img.draw_rect(50, 50, disp2->width()/2, disp2->height()/2, image::Color::from_bgra(0,0xff,0,0.5), -1);
            disp2->show(disp2_img);
        } else {
            if (disp2) {
                delete disp2;
                disp2 = NULL;
            }
        }

        // time when read image finished
        t2 = time::ticks_ms();
        disp.show(*img, image::FIT_COVER);

        // free image data, important!
        delete img;

        // calculate fps
        t3 = time::ticks_ms();
        fps = 1000.0f/(t3-t1);
        snprintf(buf, sizeof(buf), "cam: %ld, disp: %ld, all: %ld (ms), fps: %.2f", t2-t1, t3-t2, t3-last_loop_ms, fps);
        last_loop_ms = time::ticks_ms();
    }

    if (cam2) {
        delete cam2;
        cam2 = NULL;
    }
    return 0;
}

static int cmd_init(void)
{
    int flag;
    if((flag = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
    {
        perror("fcntl");
        return -1;
    }

    flag = flag | O_NONBLOCK;
    if (0 < fcntl(STDIN_FILENO, F_SETFL, flag)) {
        perror("fcntl");
        return -1;
    }

    printf( "========================\r\n"
            "Intput param:\r\n"
            "0 <value> : set exposure, unit:us\r\n"
            "1 <value> : set gain\r\n"
            "2 <value> : set luma\r\n"
            "3 <value> : set constrast\r\n"
            "4 <value> : set saturation\r\n"
            "5 <value> : set white balance mode\r\n"
            "6 <value> : set exposure mode\r\n"
            "7 <value> : show colorbar, 1:enable 0:disable\r\n"
            "8 <width> <height>: set new resolution\r\n"
            "9 <enable>: set camera hmirror, 1:enable;0:disable\r\n"
            "10 <enable>: set camera vflip, 1:enable;0:disable\r\n"
            "11 <x> <y> <w> <h>: set windowing\r\n"
            "12 <enable>: use a new channel of camera, 1:use 0:unuse\r\n"
            "13 <enable>: use a new channel of display, 1:use 0:unuse\r\n"
            "14 <enable>: set disolay hmirror, 1:enable;0:disable\r\n"
            "15 <enable>: set disolay vflip, 1:enable;0:disable\r\n"
            "16 : dump raw data to file, default save in: /root/dump_bayer.raw\r\n"
            "17 <fps>: set fps\r\n"
            "18 <addr> <data>: set reg\r\n"
            "19 <addr>: get reg\r\n"
            "20 <pos>: set fp5510e pos\r\n"
            "========================\r\n");
    fflush(stdin);
    return 0;
}

static int cmd_loop(camera::Camera *cam, display::Display *disp)
{
    uint64_t t1;
    uint64_t value = -1, value2 = -1, value3 = -1, value4 = -1;
    double value_f = -1, value_f2 = -1, value_f3 = -1, value_f4 = -1;
    uint64_t value_h = -1, value_h2 = -1, value_h3 = -1, value_h4 = -1;
    int cmd = -1;

    if (!cam) {
        fflush(stdin);
        return -1;
    }

    char buf[256] = {0};
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        log::info("input:%s", buf);
        sscanf(buf, "%d %ld %ld %ld %ld\r\n", &cmd, &value, &value2, &value3, &value4);
        sscanf(buf, "%d %lf %lf %lf %lf", &cmd, &value_f, &value_f2, &value_f3, &value_f4);
        sscanf(buf, "%d %lx %lx %lx %lx", &cmd, &value_h, &value_h2, &value_h3, &value_h4);
        // log::info("int cmd:%d value:%ld %ld %ld %ld", cmd, value, value2, value3, value4);
        // log::info("double cmd:%d value:%f %f %f %f", cmd, value_f, value_f2, value_f3, value_f4);
        // log::info("hex cmd:%d value:%#x %#x %#x %#x", cmd, value_h, value_h2, value_h3, value_h4);
        fflush(stdin);
        t1 = time::ticks_ms();
        switch (cmd) {
        case 0:
        {
            uint64_t out = 0;
            out = cam->exposure(value);
            log::info("set exposure: %ld\r\n", out);
            out = cam->gain();
            log::info("get gain: %d\r\n", out);
            out = cam->exposure();
            log::info("get exposure: %ld\r\n", out);
        }
        break;
        case 1:
        {
            uint32_t out = 0;
            out = cam->gain(value);
            log::info("set gain: %ld\r\n", out);
            out = cam->gain();
            log::info("get gain: %d\r\n", out);
            out = cam->exposure();
            log::info("get exposure: %ld\r\n", out);
        }
        break;
        case 2:
        {
            uint32_t out = 0;
            out = cam->luma(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set luma: %ld\r\n", value);
            out = cam->luma();
            log::info("get luma: %d\r\n", out);
        }
        break;
        case 3:
        {
            uint32_t out = 0;
            out = cam->constrast(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set constrast: %ld\r\n", value);
            out = cam->constrast();
            log::info("get constrast: %d\r\n", out);
        }
        break;
        case 4:
        {
            uint32_t out = 0;
            out = cam->saturation(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set saturation: %ld\r\n", value);
            out = cam->saturation();
            log::info("get saturation: %d\r\n", out);
        }
        break;
        case 5:
        {
            uint32_t out = 0;
            out = cam->set_awb(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set white balance mode: %ld\r\n", value);
            out = cam->set_awb();
            log::info("get white balance mode: %d\r\n", out);
        }
        break;
        case 6:
        {
            uint32_t out = 0;
            out = cam->exp_mode(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set exposure mode: %ld\r\n", value);
            out = cam->exp_mode();
            log::info("get exposure mode: %d\r\n", out);
            break;
        }
        case 7:
        {
            err::check_raise(cam->show_colorbar(value), "set error");
            break;
        }
        case 8:
        {
            err::check_raise(cam->set_resolution(value, value2), "set error");
            break;
        }
        case 9:
        {
            cam->hmirror(value);
            log::info("set camera hmirror: %d", value);
            log::info("get camera hmirror: %d", cam->hmirror());
            break;
        }
        case 10:
        {
            cam->vflip(value);
            log::info("set camera vflip: %d", value);
            log::info("get camera vflip: %d", cam->vflip());
            break;
        }
        case 11:
        {
            std::vector<int> roi = {(int)value, (int)value2, (int)value3, (int)value4};
            err::check_raise(cam->set_windowing(roi), "set error");
            break;
        }
        case 12:
        {
            camera2_enable = value;
            break;
        }
        case 13:
        {
            disp2_enable = value;
            break;
        }
        case 14:
        {
            if (disp)
                disp->set_hmirror(value);
            log::info("set display hmirror: %d", value);
            break;
        }
        case 15:
        {
            if (disp)
                disp->set_vflip(value);
            log::info("set display  vflip: %d", value);
            break;
        }
        case 16:
        {
            std::string path = "/root/dump_bayer.raw";
            image::Image *img = cam->read_raw();
            if (img) {
                log::info("dump bayer data to %s, width:%d height:%d data size:%d format:%s",
                            path.c_str(), img->width(), img->height(), img->data_size(), image::fmt_names[img->format()].c_str());
                fs::File *f = fs::open(path, "w+");
                err::check_null_raise(f, "open file error");
                f->write(img->data(), img->data_size());
                f->close();
                delete img;
                delete f;
            } else {
                log::info("Can not read raw image");
            }

            break;
        }
        case 17:
        {
            double fps = value_f;
            log::info("set fps %f", fps);
            err::check_raise(cam->set_fps(fps), "set fps error");
            break;
        }
        case 18:
        {
            log::info("i2c write addr:%#x data:%#x", value_h, value_h2);
            cam->write_reg(value_h, value_h2);
            break;
        }
        case 19:
        {
            log::info("i2c read addr:%#x", value_h);
            int data = cam->read_reg(value_h);
            log::info("i2c read data:%#x", data);
            break;
        }
        case 20:
        {
            try {
                if (!g_fp5510e) {
                    g_fp5510e = new fp5510::FP5510();
                }
            } catch (exception &e) {
                log::info("Create fp5510 failed!");
            }

            if (g_fp5510e) {
                g_fp5510e->set_pos(value);
                log::info(" set fp5510 pos:%d", g_fp5510e->get_pos());
            }
            break;
        }
        default:printf("Find not cmd!\r\n"); break;
        }

        log::info("cmd use %ld ms\r\n", time::ticks_ms() - t1);
        fflush(stdin);
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


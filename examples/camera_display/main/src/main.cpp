

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "main.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace maix;

static int cmd_init(void);
static int cmd_loop(camera::Camera *cam);

static bool camera2_enable = false;

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
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            log::info("./camera_display <width> <height> <format> <fps> <buff_num>");
            log::info("example: ./camera_display 640 480 0 60 2");
            exit(0);
        } else {
            cam_w = atoi(argv[1]);
        }
    }
    if (argc > 2) cam_h = atoi(argv[2]);
    if (argc > 3) cam_fmt = (image::Format)atoi(argv[3]);
    if (argc > 4) cam_fps = atoi(argv[4]);
    if (argc > 5) cam_buffer_num = atoi(argv[5]);
    log::info("Camera width:%d height:%d format:%s fps:%d buffer_num:%d", cam_w, cam_h, image::fmt_names[cam_fmt].c_str(), cam_fps, cam_buffer_num);

    camera::Camera cam = camera::Camera(cam_w, cam_h, cam_fmt, "", cam_fps, cam_buffer_num);
    camera::Camera *cam2 = NULL;
    display::Display screen = display::Display();
    log::info("camera and display open success");
    log::info("camera size: %dx%d", cam.width(), cam.height());
    log::info("screen size: %dx%d", screen.width(), screen.height());
    cam.skip_frames(5);
    err::check_bool_raise(!cmd_init(), "cmd init failed!");

    while(!app::need_exit())
    {
        // cmd loop
        cmd_loop(&cam);

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

        // time when read image finished
        t2 = time::ticks_ms();
        screen.show(*img);

        // free image data, important!
        delete img;

        // calculate fps
        t3 = time::ticks_ms();
        fps = 1000.0f/(t3-t1);
        snprintf(buf, sizeof(buf), "cam: %ld, disp: %ld, all: %ld (ms), fps: %.2f", t2-t1, t3-t2, t3-t1, fps);
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
            "9 <enable>: set hmirror, 1:enable;0:disable\r\n"
            "10 <enable>: set vflip, 1:enable;0:disable\r\n"
            "11 <x> <y> <w> <h>: set windowing\r\n"
            "12 <enable>: use a new channel of camera, 1:use 0:unuse\r\n"
            "========================\r\n");
    fflush(stdin);
    return 0;
}

static int cmd_loop(camera::Camera *cam)
{
    uint64_t t1;
    uint64_t value = -1, value2 = -1, value3 = -1, value4 = -1;
    int cmd = -1;

    if (!cam) {
        fflush(stdin);
        return -1;
    }

    int len = scanf("%d %ld %ld %ld %ld\r\n", &cmd, &value, &value2, &value3, &value4);
    if (len > 0) {
        log::info("len:%d cmd:%d value:%ld %ld %ld %ld", len, cmd, value, value2, value3, value4);
        fflush(stdin);
        t1 = time::ticks_ms();
        switch (cmd) {
        case 0:
        {
            uint64_t out = 0;
            out = cam->exposure(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set exposure: %ld\r\n", value);
            out = cam->exposure();
            log::info("get exposure: %ld\r\n", out);
        }
        break;
        case 1:
        {
            uint32_t out = 0;
            out = cam->gain(value);
            err::check_bool_raise(out == value, "set error");
            log::info("set gain: %ld\r\n", value);
            out = cam->gain();
            log::info("get gain: %d\r\n", out);
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
            log::info("set hmirror: %d", value);
            log::info("get hmirror: %d", cam->hmirror());
            break;
        }
        case 10:
        {
            cam->vflip(value);
            log::info("set vflip: %d", value);
            log::info("get vflip: %d", cam->vflip());
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


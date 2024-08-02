

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "main.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test_image.hpp"

using namespace maix;

typedef struct {
    char name[50];
    int (*func)(image::Image *);
} image_method_t;

typedef struct {
    int cam_w;
    int cam_h;
    image::Format cam_fmt;
    int cam_fps;
    int cam_buffnum;
    int image_method_idx;
    std::vector<image_method_t> method_list;
} priv_t;

static priv_t priv;
static int cmd_init(int argc, char* argv[]);

int _main(int argc, char* argv[])
{
    err::check_bool_raise(!cmd_init(argc, argv), "cmd get init param failed!");

    camera::Camera cam = camera::Camera(priv.cam_w ,priv.cam_h, priv.cam_fmt, nullptr, priv.cam_fps, priv.cam_buffnum);
    display::Display disp = display::Display();
    log::info("camera and display open success\n");
    log::info("camera size: %dx%d\n", cam.width(), cam.height());
    log::info("disp size: %dx%d\n", disp.width(), disp.height());

    uint64_t last_ms = time::time_ms();
    uint64_t last_loop_used_ms = 0;
    while(!app::need_exit())
    {
        image::Image *img = cam.read();
        err::check_null_raise(img, "camera read failed");

        if (priv.method_list[priv.image_method_idx].func) {
            priv.method_list[priv.image_method_idx].func(img);
        }

        disp.show(*img);
        delete img;

        log::info("loop used: %ld (ms), fps: %.2f", last_loop_used_ms, 1000.0f / last_loop_used_ms);
        last_loop_used_ms = time::ticks_ms() - last_ms;
        last_ms = time::ticks_ms();
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

static void cmd_helper(void)
{
    log::info("Image method index:");
    for (size_t i = 0; i < priv.method_list.size(); i ++) {
        log::info("\t[%d] %s", i, priv.method_list[i].name);
    }

    log::info("Input <image method index> <camera width> <camera height> <camera format> <camera fps> <camera buffnum>");
    log::info("<camera format> = 0, measn image::FMT_RGB888");
    log::info("<camera format> = 12, measn image::FMT_GRAYSCALE");
    log::info("Example: ./image_method 0 320 240 0 60 2 // test gaussion");
}

static int cmd_init(int argc, char* argv[])
{
    priv.cam_w = 320;
    priv.cam_h = 240;
    priv.cam_fmt = image::FMT_RGB888;
    priv.cam_fps = 60;
    priv.cam_buffnum = 2;

    // Config image method
    priv.method_list.push_back(image_method_t{"no method(default)", NULL});
    priv.method_list.push_back(image_method_t{"gaussian", test_gaussion});
    priv.method_list.push_back(image_method_t{"find_blobs", test_find_blobs});

    // Get init param
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            cmd_helper();
            exit(0);
        } else {
            priv.image_method_idx = atoi(argv[1]);

        }

    }

    if (argc > 2) {
        priv.cam_w = atoi(argv[2]);
    }

    if (argc > 3) {
        priv.cam_h = atoi(argv[3]);
    }

    if (argc > 4) {
        priv.cam_fmt = image::Format(atoi(argv[4]));
    }

    if (argc > 5) {
        priv.cam_fps = atoi(argv[5]);
    }

    if (argc > 6) {
        priv.cam_buffnum = atoi(argv[6]);
    }

    if (priv.image_method_idx >= (int)priv.method_list.size()) {
        log::info("Method: %d not found", priv.image_method_idx);
        log::info("Try:");
        cmd_helper();
        exit(0);
    } else {
        log::info("Use method: %d %s", priv.image_method_idx, priv.method_list[priv.image_method_idx].name);
    }
    log::info("Use width:%d hieght:%d format:%d fps:%d buffer number:%d", priv.cam_w, priv.cam_h, priv.cam_fmt, priv.cam_fps, priv.cam_buffnum);
    return 0;
}

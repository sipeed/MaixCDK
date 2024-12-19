

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
    std::string picture_path;
    bool use_picture;
} priv_t;

static priv_t priv;
param_t g_param;
static int cmd_init(int argc, char* argv[]);

int _main(int argc, char* argv[])
{
    err::check_bool_raise(!cmd_init(argc, argv), "cmd get init param failed!");
    camera::Camera *cam = NULL;
    image::Image *picture = NULL;
    if (!priv.use_picture) {
        cam = new camera::Camera(priv.cam_w ,priv.cam_h, priv.cam_fmt, nullptr, priv.cam_fps, priv.cam_buffnum);
        log::info("camera size: %dx%d\n", cam->width(), cam->height());
        g_param.cam = cam;
    } else {
        picture = image::load(priv.picture_path.c_str(), priv.cam_fmt);
        auto new_width = priv.cam_w > 0 ? priv.cam_w : picture->width();
        new_width = new_width % 2 == 0 ? new_width : new_width+1;
        auto new_height = priv.cam_h > 0 ? priv.cam_h : picture->height();
        new_height = new_height % 2 == 0 ? new_height : new_height+1;
        image::Image *new_picture = picture->resize(new_width, new_height, image::FIT_COVER);
        delete picture;
        picture = new_picture;
        log::info("picture path:%s size: %dx%d format:%s\n", priv.picture_path.c_str(), picture->width(), picture->height(), image::fmt_names[picture->format()]);
    }
    display::Display disp = display::Display();
    log::info("disp size: %dx%d\n", disp.width(), disp.height());
    log::info("camera and display open success\n");

    uint64_t last_ms = time::time_ms();
    uint64_t last_loop_used_ms = 0;
    do {
        image::Image *img = NULL;
        if (!priv.use_picture) {
            img = cam->read();
        } else {
            img = picture;
        }
        err::check_null_raise(img, "camera read failed");

        if (priv.method_list[priv.image_method_idx].func) {
            priv.method_list[priv.image_method_idx].func(img);
        }

        disp.show(*img);
        delete img;

        log::info("loop used: %ld (ms), fps: %.2f", last_loop_used_ms, 1000.0f / last_loop_used_ms);
        last_loop_used_ms = time::ticks_ms() - last_ms;
        last_ms = time::ticks_ms();
    } while(!app::need_exit() && !priv.use_picture);

    if (priv.use_picture) {
        log::info("Press ctrl+c to exit");
        while (!app::need_exit()) {
            time::sleep_ms(100);
        }
        // delete picture; // deleted after delete img
    }

    if (cam) {
        delete cam;
        cam = NULL;
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
    log::info("Input <image method index> <camera width> <camera height> <camera format> <jpeg_path>");
    log::info("<camera format> = 0, measn image::FMT_RGB888");
    log::info("<camera format> = 12, measn image::FMT_GRAYSCALE");
    log::info("Example: ./image_method 0 320 240 0 60 3 // test gaussion");
}

static int cmd_init(int argc, char* argv[])
{
    priv.cam_w = 320;
    priv.cam_h = 240;
    priv.cam_fmt = image::FMT_RGB888;
    priv.cam_fps = 60;
    priv.cam_buffnum = 3;

    // Config image method
    priv.method_list.push_back(image_method_t{"no method(default)", NULL});
    priv.method_list.push_back(image_method_t{"gaussian", test_gaussion});
    priv.method_list.push_back(image_method_t{"find_blobs", test_find_blobs});
    priv.method_list.push_back(image_method_t{"find_qrcode", test_find_qrcode});
    priv.method_list.push_back(image_method_t{"qrcode_detector", test_qrcode_detector});
    priv.method_list.push_back(image_method_t{"find_lines", test_find_lines});
    priv.method_list.push_back(image_method_t{"ed lib", test_ed_lib});
    priv.method_list.push_back(image_method_t{"tracking line", test_tracking_line});
    priv.method_list.push_back(image_method_t{"find_barcode", test_find_barcode});

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
        if (priv.cam_fps == 0) {
            if (strlen(argv[5]) > 0) {
                priv.use_picture = true;
                priv.picture_path = argv[5];
                if (!fs::exists(priv.picture_path)) {
                    cmd_helper();
                    log::error("\r\npath %s does not exist!\r\n", priv.picture_path.c_str());
                    exit(0);
                }
            }
        }
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

    if (!priv.use_picture) {
        log::info("Use width:%d hieght:%d format:%d fps:%d buffer number:%d", priv.cam_w, priv.cam_h, priv.cam_fmt, priv.cam_fps, priv.cam_buffnum);
    } else {
        log::info("Use width:%d hieght:%d format:%d path:%s", priv.cam_w, priv.cam_h, priv.cam_fmt, priv.picture_path.c_str());
    }

    return 0;
}



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

int _main(int argc, char* argv[])
{
    uint64_t t1, t2, t3;
    float fps = 0;
    char buf[128] = {0};

    camera::Camera cam = camera::Camera();
    display::Display screen = display::Display();
    log::info("camera and display open success\n");
    log::info("camera size: %dx%d\n", cam.width(), cam.height());
    log::info("screen size: %dx%d\n", screen.width(), screen.height());

    err::check_bool_raise(!cmd_init(), "cmd init failed!\r\n");

    while(!app::need_exit())
    {
        // cmd loop
        cmd_loop(&cam);

        // time when start read image from camera
        t1 = time::time_ms();

        // read image from camera
        image::Image *img = cam.read();
        err::check_null_raise(img, "camera read failed");

        // time when read image finished
        t2 = time::time_ms();

        // draw fps on image
        img->draw_string(0, 10, buf, image::Color::from_rgb(255, 0, 0), 1.5);

        // check if screen is closed by user(mostly for PC), and show image on screen
        if(!screen.is_opened())
        {
            log::info("screen closed\n");
            break;
        }
        screen.show(*img);

        // free image data, important!
        delete img;

        // calculate fps
        t3 = time::time_ms();
        fps = 1000.0f/(t3-t1);
        snprintf(buf, sizeof(buf), "cam: %ld, disp: %ld, all: %ld (ms), fps: %.2f", t2-t1, t3-t2, t3-t1, fps);
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
            "0 <value> : set exposure\r\n"
            "1 <value> : set gain\r\n"
            "2 <value> : set luma\r\n"
            "3 <value> : set constrast\r\n"
            "4 <value> : set saturation\r\n"
            "========================\r\n");
    fflush(stdin);
    return 0;
}

static int cmd_loop(camera::Camera *cam)
{
    uint64_t t1;
    uint64_t value = -1;
    int cmd = -1;

    if (!cam) {
        fflush(stdin);
        return -1;
    }

    if (scanf("%d  %ld\r\n", &cmd, &value) > 0) {
        log::info("cmd:%d %ld\r\n", cmd, value);
        t1 = time::time_ms();
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
        default:printf("Find not cmd!\r\n"); break;
        }
        log::info("cmd use %ld ms\r\n", time::time_ms() - t1);
        fflush(stdin);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig){ app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


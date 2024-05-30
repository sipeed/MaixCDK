
#include "stdio.h"
#include "main.h"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_camera.hpp"
#include "maix_basic.hpp"
#include "csignal"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace maix;

int _main(int argc, char* argv[])
{
    camera::Camera cam = camera::Camera(2560, 1440, image::Format::FMT_YVU420SP);
    video::Video v = video::Video("output.mp4", true);

    v.bind_camera(&cam);
    v.record_start();

    int sleep_s = 5;
    while(!app::need_exit()) {
        if (sleep_s -- <= 0) {
            v.record_finish();
            log::info("record finished!\r\n");
        }
        time::sleep(1);
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

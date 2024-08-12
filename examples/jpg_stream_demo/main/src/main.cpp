
#include "maix_jpg_stream.hpp"
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "main.h"

using namespace maix;
extern std::string html;

int _main(int argc, char* argv[])
{
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
    display::Display disp = display::Display();
    log::info("camera and display open success\n");
    log::info("camera size: %dx%d\n", cam.width(), cam.height());
    log::info("disp size: %dx%d\n", disp.width(), disp.height());
    http::JpegStreamer stream = http::JpegStreamer("", 8000);
    stream.set_html(html);
    stream.start();

    log::info("http://%s:%d\r\n", stream.host().c_str(), stream.port());
    while(!app::need_exit())
    {
        // read image from camera
        image::Image *img = cam.read();
        err::check_null_raise(img, "camera read failed");

        image::Image *jpg = img->to_jpeg();
        stream.write(jpg);
        delete jpg;

        disp.show(*img);

        // free image data, important!
        delete img;
    }
    stream.stop();
    return 0;
}

std::string html =
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <title>JPG Stream</title>\n"
"</head>\n"
"<body>\n"
"    <h1>JPG Stream</h1>\n"
"    <img src=\"/stream\" alt=\"Stream\">\n"
"</body>\n"
"</html>\n";


int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}



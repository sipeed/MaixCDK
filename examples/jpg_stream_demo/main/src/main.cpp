
#include "maix_jpg_stream.hpp"
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "main.h"

using namespace maix;
extern std::string html;

int _main(int argc, char* argv[])
{
    camera::Camera cam = camera::Camera(320, 240);
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
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig){ app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}



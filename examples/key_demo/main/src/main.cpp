

#include "maix_basic.hpp"
#include "maix_display.hpp"
#include "maix_key.hpp"
#include "main.h"

using namespace maix;
using namespace maix::peripheral;

static int g_key = 0;
static int g_state = 0;

void on_key(int key, int state)
{
    log::info("key: %d, state: %d\n", key, state);
    g_key = key;
    g_state = state;
}

int _main(int argc, char* argv[])
{
    char buf[128] = {0};

    display::Display screen = display::Display();
    log::info("screen size: %dx%d", screen.width(), screen.height());

    key::Key key = key::Key(on_key);
    // If you want to exit app when press ok button, you can just call add_default_listener(), more see it's definition.
    // key::add_default_listener();
    log::info("Init key ok");

    while(!app::need_exit())
    {
        maix::image::Image *img = new maix::image::Image(screen.width(), screen.height(), maix::image::Format::FMT_RGB888);
        if(!img)
        {
            throw err::Exception(err::ERR_NO_MEM, "create image failed");
        }
        snprintf(buf, sizeof(buf), "key: %d, state: %d", g_key, g_state);
        img->draw_string(0, 10, buf, maix::image::Color::from_rgb(255, 255, 255), 1.5);

        screen.show(*img);

        // free image data, important!
        delete img;
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



#include "maix_basic.hpp"
#include "maix_pwm.hpp"
#include "main.hpp"

using namespace maix;
using namespace maix::peripheral;


int _main(int argc, char* argv[])
{
    // LCD backlight demo
#if PLATFORM_MAIXCAM
    int pwm_id = 10;
#else
    #error "No backlight on this platform"
#endif

    pwm::PWM backlight(pwm_id, 100000, 20);

    time::sleep(2);

    backlight.duty(10);
    // restart, this only for first version system driver, we will remove it later
    backlight.disable();
    backlight.enable();
    time::sleep(2);

    backlight.duty(30);
    // restart, this only for first version system driver, we will remove it later
    backlight.disable();
    backlight.enable();
    time::sleep(2);

    backlight.duty(20);
    // restart, this only for first version system driver, we will remove it later
    backlight.disable();
    backlight.enable();

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



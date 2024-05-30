
#include "maix_basic.hpp"
#include "maix_pwm.hpp"
#include "main.hpp"
#include "maix_pinmap.hpp"

using namespace maix;
using namespace maix::peripheral;


#define SERVO_PERIOD    50   // 20ms
#define SERVO_MIN_DUTY  2.5  // 2.5% -> 0.5ms
#define SERVO_MAX_DUTY  12.5 // 12.5% -> 2.5ms

float angle_to_duty(float percent)
{
    return (SERVO_MAX_DUTY - SERVO_MIN_DUTY) * percent / 100.0 + SERVO_MIN_DUTY;
}

int _main(int argc, char* argv[])
{
    int pwm_id = 7; // for MaixCAM PWM7 is pin A19, see https://wiki.sipeed.com/hardware/zh/maixcam/index.html

    // set pinmap
    pinmap::set_pin_function("A19", "PWM7");

    // init pwm
    pwm::PWM output(pwm_id, SERVO_PERIOD, angle_to_duty(0), true);

    for(int i=0; i<100; ++i)
    {
        output.duty(angle_to_duty(i));
        time::sleep_ms(100);
    }

    for(int i=100; i>=0; --i)
    {
        output.duty(angle_to_duty(i));
        time::sleep_ms(100);
    }

    // while(!app::need_exit())
    // {
    //     time::sleep(1);
    // }

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



# TMC2209 Stepper Motor Driver for MaixCDK

This component is based on [janelia-arduino/TMC2209](https://github.com/janelia-arduino/TMC2209), which is licensed under the [BSD-3 License](./janelia-arduino_TMC2209/LICENSE). We have modified some of the code to adapt it to the MaixCDK platform.


## Usage

### Example

Here is a simple example demonstrating how to initialize and control a TMC2209 stepper motor:

```cpp
#include "maix_basic.hpp"
#include "main.h"
#include "maix_tmc2209.hpp"
#include "maix_pinmap.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    std::string port = "/dev/ttyS1";
    uint8_t uart_addr = 0x00;
    long uart_baudrate = 115200;
    float step_angle = 18;
    uint16_t micro_step = 256;
    float screw_pitch = 3;
    float speed = 6;
    bool use_internal_sense_resistors = true;
    uint8_t run_current_per = 100;
    uint8_t hold_current_per = 100;


    if (port == "/dev/ttyS1") {
        auto ret = peripheral::pinmap::set_pin_function("A19", "UART1_TX");
        if (ret != err::Err::ERR_NONE) {
            maix::log::error("Failed in function pinmap...");
            return -1;
        }
        ret = peripheral::pinmap::set_pin_function("A18", "UART1_RX");
        if (ret != err::Err::ERR_NONE) {
            maix::log::error("Failed in function pinmap...");
            return -1;
        }
    }
    maix::ext_dev::tmc2209::ScrewSlide slide(port.c_str(), uart_addr, uart_baudrate,
                                            step_angle, micro_step, screw_pitch, speed,
                                            use_internal_sense_resistors, run_current_per, hold_current_per);

    auto move_callback = [](float per) -> bool {
        maix::log::info("Slide moving... %0.2f", per);
        if (per >= 50) { /* >=80% */
            maix::log::info("%0.2f >= 50%, stop.", per);
            return true;
        }
        return false;
    };
    slide.move(6, -1, move_callback);
    slide.move(-3);
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
```

## Acknowledgments

- Original library by [janelia-arduino](https://github.com/janelia-arduino)
- Inspiration and guidance from the open-source community
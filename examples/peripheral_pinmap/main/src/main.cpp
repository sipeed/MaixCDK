
#include "maix_basic.hpp"
#include "main.h"
#include "maix_pinmap.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    log::info("Program start");

    std::vector<std::string> pins = peripheral::pinmap::get_pins();
    log::info("All pins:");
    for (auto& item : pins) {
        printf("%s ", item.c_str());
    } printf("\n");

    std::vector<std::string> gpio_a28_pin_functions = 
        peripheral::pinmap::get_pin_functions("A28");
    log::info("GPIO A28 pin functions:");
    for (auto& item : gpio_a28_pin_functions) {
        printf("%s ", item.c_str());
    } printf("\n");

    if (err::ERR_NONE != peripheral::pinmap::set_pin_function("A28", gpio_a28_pin_functions[0])) {
        log::error("set pin function failed");
    } 

    log::info("Program exit");

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



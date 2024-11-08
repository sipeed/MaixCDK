
#include "maix_basic.hpp"
#include "main.h"
#include "maix_modbus.hpp"
#include "maix_pinmap.hpp"

using namespace maix;
using namespace maix::comm;

int _main(int argc, char* argv[])
{

    if(peripheral::pinmap::set_pin_function("A19", "UART1_TX") != err::Err::ERR_NONE) {
        log::error("init uart1 failed!");
        return -1;
    }
    if (peripheral::pinmap::set_pin_function("A18", "UART1_RX") != err::Err::ERR_NONE) {
        log::error("init uart failed!");
        return -1;
    }

    auto master = modbus::Master(
        modbus::Mode::RTU,
        "/dev/ttyS1",
        115200, 1,
        0,
        false
    );

    // auto master = modbus::Master(
    //     modbus::Mode::TCP,
    //     "192.168.1.168",    // slave ip
    //     0, 0,
    //     5020,               // slave port
    //     false
    // );

    while (!app::need_exit()) {
        auto hr = master.read_holding_registers(10, 0, 1000);
        if (hr.empty())
            continue;
        log::info0("Master read hr: ");
        for (const auto i : hr) {
            printf("0x%04x ", i);
        }printf("\n");
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



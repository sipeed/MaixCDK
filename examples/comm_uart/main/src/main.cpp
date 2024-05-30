
#include "maix_basic.hpp"
#include "maix_uart.hpp"
#include "main.h"

using namespace maix;
using namespace maix::peripheral;

int _main(int argc, char* argv[])
{
    int rx_len = 0;
    uint8_t rx_buff[128] = {0};

    std::vector<std::string> ports = uart::list_devices();
    for(auto &port : ports)
    {
        log::info("find uart port: %s\n", port.c_str());
    }

    log::info("open %s\n", ports[0].c_str());

    uart::UART serial = uart::UART(ports[0], 115200);

    std::string msg = "hello\r\n";
    serial.write(msg.c_str());
    log::info("sent %s\n", msg.c_str());

    while(!app::need_exit())
    {
        rx_len = serial.read(rx_buff, sizeof(rx_buff));
        if(rx_len > 0)
        {
            log::info("received %d: \"%s\"\n", rx_len, rx_buff);
            log::print("hex:\n");
            for(int i=0; i < rx_len; ++i)
            {
                log::print("%02x ", rx_buff[i]);
            }
            log::print("\n\n");
            serial.write(rx_buff, rx_len);
            log::info("sent %d: \"%s\"\n", rx_len, rx_buff);
        }
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



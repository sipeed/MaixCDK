
#include "maix_basic.hpp"
#include "main.h"

#include "maix_modbus.hpp"

using namespace maix;
using namespace maix::comm;

int _main(int argc, char* argv[])
{
    // auto Slave = modbus::Slave(
    //     modbus::Mode::RTU,      // mode
    //     "/dev/ttyS0",           // serial device
    //     0x00, 10,               // coils
    //     0x00, 10,               // discrete input
    //     0x00, 10,               // input registers
    //     0x00, 10,               // holding registers
    //     115200, 1,              // serial 115200-8N1, slave id: 1
    //     0,                      // tcp port
    //     true);                  // debug ON

    auto Slave = modbus::Slave(
        modbus::Mode::TCP,      // mode
        "",                     // ip, keep empty
        0x00, 10,               // coils
        0x00, 10,               // discrete input
        0x00, 10,               // input registers
        0x00, 10,               // holding registers
        0, 1,                   // serial, ignore
        502,                      // tcp port
        false);                  // debug OFF

    std::vector<uint16_t> data{0x22, 0x33, 0x44};
    Slave.input_registers(data, 3);

    {
        auto r = Slave.input_registers();
        for (auto i : r) {
            printf("0x%x ", i);
        } printf("\n");
    }

    while (!app::need_exit()) {

        auto rt = Slave.receive(2000);  // timeout: 2000ms
        if (rt != err::Err::ERR_NONE) {
            continue;
        }

        auto type = Slave.request_type();
        if (type == modbus::RequestType::READ_HOLDING_REGISTERS) {
            // Operating registers through the interface ensures safety but incurs copy overhead.
            log::info("client read hr");
            auto hr = Slave.holding_registers();
            log::info0("\tnow hr data:");
            for (auto i : hr) {
                printf("0x%x ", i);
            } printf("\n");
            log::info("\tnow we make hr+1");
            for (auto& i : hr) {
                ++i;
            }
            log::info("\tnow updata hr");
            Slave.holding_registers(hr);

            // Directly manipulate registers to avoid copy overhead.
            // for(int i = 0; i < Slave->nb_registers; ++i) {
            //     printf("0x%x ", Slave->tab_registers[i]);
            //     Slave->tab_registers[i]++;
            // } printf("\n");
        }

        Slave.reply();
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



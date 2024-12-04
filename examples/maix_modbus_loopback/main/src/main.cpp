
#include "maix_basic.hpp"
#include "main.h"
#include "maix_modbus.hpp"
#include "maix_pinmap.hpp"
#include <iomanip>  // std::setw and std::setfill
#include <sstream>  // std::stringstream
#include <thread>   // std::thread

using namespace maix;
using namespace maix::comm;

/** MODE: RTU/TCP
 *  NOTE: RTU UART0(Slave) <--> UART1(Master)
 */
constexpr modbus::Mode MODE = modbus::Mode::RTU;
// constexpr modbus::Mode MODE = modbus::Mode::TCP;

/* slave cfg */
constexpr uint32_t REGISTERS_START_ADDRESS = 0x00;
constexpr uint32_t REGISTERS_NUMBER = 10;

/* rtu cfg */
constexpr uint32_t RTU_SLAVE_ID = 1;
constexpr int RTU_BAUDRATE = 115200;

/* tcp cfg */
constexpr int TCP_PORT = 502;

int master_rtu_thread()
{
    if(peripheral::pinmap::set_pin_function("A19", "UART1_TX") != err::Err::ERR_NONE) {
        log::error("init uart1 failed!");
        return -1;
    }
    if (peripheral::pinmap::set_pin_function("A18", "UART1_RX") != err::Err::ERR_NONE) {
        log::error("init uart failed!");
        return -1;
    }

    // modbus::set_master_debug(true);

    modbus::MasterRTU master("/dev/ttyS1", RTU_BAUDRATE);

    while (!app::need_exit()) {
        // log::info("master thread running...");

        std::vector<uint16_t> rt = master.read_holding_registers(RTU_SLAVE_ID, REGISTERS_START_ADDRESS, REGISTERS_NUMBER, 2000);
        if (rt.empty())
            continue;

        std::stringstream ss;
        ss << "Master read: ";
        for (const auto& value : rt) {
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << value << " ";
        }
        log::info(ss.str().c_str());
        time::sleep(1);
    }

    return 0;
}

int slave_rtu_thread()
{
    modbus::Registers cfg;
    cfg.coils.start_address             = REGISTERS_START_ADDRESS;
    cfg.coils.size                      = REGISTERS_NUMBER;
    cfg.discrete_inputs.start_address   = REGISTERS_START_ADDRESS;
    cfg.discrete_inputs.size            = REGISTERS_NUMBER;
    cfg.holding_registers.start_address = REGISTERS_START_ADDRESS;
    cfg.holding_registers.size          = REGISTERS_NUMBER;
    cfg.input_registers.start_address   = REGISTERS_START_ADDRESS;
    cfg.input_registers.size            = REGISTERS_NUMBER;

    modbus::Slave slave(
        modbus::Mode::RTU,
        "/dev/ttyS0",
        cfg,
        RTU_BAUDRATE,
        RTU_SLAVE_ID
    );

    while (!app::need_exit()) {
        // log::info("slave thread running...");

        auto rt = slave.receive(2000);
        if (rt != err::Err::ERR_NONE)
            continue;

        modbus::RequestType type = slave.request_type();
        if (type == modbus::RequestType::READ_HOLDING_REGISTERS) {
            std::stringstream ss;
            ss << "Slave update: ";
            for (int i = 0; i < slave->nb_registers; ++i) {
                slave->tab_registers[i]++;
                ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << slave->tab_registers[i] << " ";
            }
            log::info(ss.str().c_str());
        }

        slave.reply();
    }

    return 0;
}

void rtu_loopback()
{
    std::thread slave_th(slave_rtu_thread);
    time::sleep_ms(500);
    std::thread master_th(master_rtu_thread);

    slave_th.join();
    master_th.join();
}

int slave_tcp_thread()
{
    modbus::Registers cfg;
    cfg.coils.start_address             = REGISTERS_START_ADDRESS;
    cfg.coils.size                      = REGISTERS_NUMBER;
    cfg.discrete_inputs.start_address   = REGISTERS_START_ADDRESS;
    cfg.discrete_inputs.size            = REGISTERS_NUMBER;
    cfg.holding_registers.start_address = REGISTERS_START_ADDRESS;
    cfg.holding_registers.size          = REGISTERS_NUMBER;
    cfg.input_registers.start_address   = REGISTERS_START_ADDRESS;
    cfg.input_registers.size            = REGISTERS_NUMBER;

    modbus::Slave slave(
        modbus::Mode::TCP,
        "",
        cfg,
        0,
        0,
        TCP_PORT
    );

    while (!app::need_exit()) {
        // log::info("slave thread running...");

        auto rt = slave.receive(2000);
        if (rt != err::Err::ERR_NONE)
            continue;

        modbus::RequestType type = slave.request_type();
        if (type == modbus::RequestType::READ_HOLDING_REGISTERS) {
            std::stringstream ss;
            ss << "Slave update: ";
            for (int i = 0; i < slave->nb_registers; ++i) {
                slave->tab_registers[i]++;
                ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << slave->tab_registers[i] << " ";
            }
            log::info(ss.str().c_str());
        }

        slave.reply();
    }

    return 0;
}

int master_tcp_thread()
{
    // modbus::set_master_debug(true);

    modbus::MasterTCP master(TCP_PORT);

    while (!app::need_exit()) {
        // log::info("master thread running...");

        std::vector<uint16_t> rt = master.read_holding_registers("127.0.0.1", REGISTERS_START_ADDRESS, REGISTERS_NUMBER, 2000);
        if (rt.empty())
            continue;

        std::stringstream ss;
        ss << "Master read: ";
        for (const auto& value : rt) {
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << value << " ";
        }
        log::info(ss.str().c_str());
        time::sleep(1);
    }

    return 0;
}


void tcp_loopback()
{
    std::thread slave_th(slave_tcp_thread);
    time::sleep_ms(500);
    std::thread master_th(master_tcp_thread);

    slave_th.join();
    master_th.join();
}

int _main(int argc, char* argv[])
{

    if constexpr (MODE == modbus::Mode::RTU) {
        rtu_loopback();
    } else {
        tcp_loopback();
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



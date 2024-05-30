
#include "maix_basic.hpp"
#include "maix_i2c.hpp"
#include "main.hpp"

using namespace maix;
using namespace maix::peripheral;

// int _main(int argc, char* argv[])
// {
//     i2c::I2C i2c_obj(1, i2c::Mode::MASTER);
//     std::vector<int> addr_list = i2c_obj.scan();
//     // SP_TOF_ADDR
//     if (std::find(addr_list.begin(), addr_list.end(), SP_TOF_ADDR) == addr_list.end())
//     {
//         log::error("SP TOF not found\n");
//         return -1;
//     }

//     log::info("SP TOF found, addr: 0x%02x\n", SP_TOF_ADDR);

//     SP_TOF tof(i2c_obj);

//     while(!app::need_exit())
//     {
//         tof.measure();
//         log::info("distance: %d mm\n", tof.get_distance());
//         time::sleep_ms(300);
//     }
//     return 0;
// }


int _main(int argc, char* argv[])
{
    // write read eeprom
    i2c::I2C i2c_obj(1, i2c::Mode::MASTER);
    std::vector<int> addr_list = i2c_obj.scan();
    // print all i2c device address
    for(auto addr : addr_list)
    {
        log::info("i2c device addr: 0x%02x", addr);
    }
    #define EEPROM_ADDR 0x50
    // EEPROM_ADDR
    if (std::find(addr_list.begin(), addr_list.end(), EEPROM_ADDR) == addr_list.end())
    {
        log::error("EEPROM not found\n");
        return -1;
    }

    log::info("EEPROM found, addr: 0x%02x\n", EEPROM_ADDR);

    // write data
    uint8_t data[16] = {0};
    for(int i = 0; i < 16; i++)
    {
        data[i] = i * 2;
    }
    int written = i2c_obj.writeto_mem(EEPROM_ADDR, 0, data, 16, 8);
    if(written != 16)
    {
        log::error("write data failed, code: %d", written);
        return -1;
    }

    // read data
    Bytes *read_data = i2c_obj.readfrom_mem(EEPROM_ADDR, 0, 16, 8);
    if(read_data == nullptr)
    {
        log::error("read data failed");
        return -1;
    }
    for(int i = 0; i < 16; i++)
    {
        log::info("read data[%d]: %d", i, (*read_data)[i]);
    }
    delete read_data;

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



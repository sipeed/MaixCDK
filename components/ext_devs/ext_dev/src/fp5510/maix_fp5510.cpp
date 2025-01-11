#include "maix_fp5510.hpp"
#include "maix_i2c.hpp"

using namespace maix;
using namespace maix::peripheral;

namespace maix::ext_dev::fp5510 {
    typedef struct {
        i2c::I2C *i2c = nullptr;
        int i2c_id = 0;
        int slave_addr = 0x0c;
        uint8_t esc = 0;
        uint8_t tsc = 0;
        uint8_t mclk = 0;
        uint8_t t_src = 0;
        uint8_t step_mode = 0;
    } param_t;

    static void read_msg(param_t *param, uint8_t *msb, uint8_t *lsb) {
        i2c::I2C *i2c = param->i2c;
        int slave_addr = param->slave_addr;
        if (!i2c) return;

        int i = 0;
        for (i = 0; i < 5; i ++) {
            Bytes *read_data = i2c->readfrom(slave_addr, 2);
            if (read_data == nullptr) {
                log::warn("i2c read failed, retry ..");
                continue;
            }

            if (msb && read_data->data_len > 0) {
                *msb = read_data->data[0];
            }

            if (lsb && read_data->data_len > 1) {
                *lsb = read_data->data[1];
            }

            delete read_data;
            break;
        }

        if (i >= 5) {
            log::error("i2c read message failed");
        }
    }
    static void write_msg(param_t *param, uint8_t msb, uint8_t lsb) {
        i2c::I2C *i2c = param->i2c;
        int slave_addr = param->slave_addr;
        if (!i2c) return;
        uint8_t write_data[2] = {msb, lsb};
        int i = 0;
        for (i = 0; i < 5; i ++) {
            int written = i2c->writeto(slave_addr, write_data, sizeof(write_data));
            if(written != sizeof(write_data))
            {
                log::warn("i2c write failed, retry ..");
                time::sleep_ms(20);
                continue;
            }
            break;
        }

        if (i >= 5) {
            log::error("i2c write message failed");
        }
    }

    static void protection_off(param_t *param) {
        write_msg(param, 0xEC, 0xA3);
    }

    static void protection_on(param_t *param) {
        write_msg(param, 0xDC, 0x51);
    }

    static void tsc_esc_mc_setting(param_t *param, uint8_t esc, uint8_t tsc, uint8_t mc) {
        uint8_t data = (mc & 0x3)  |
                        0x04            |
                        (tsc << 0x03)   |
                        (esc << 0x04)   |
                        0x18;
        write_msg(param, 0xA1, data);
    }

    static void st_setting(param_t *param, uint8_t st) {
        uint8_t data = (st << 0x3) & 0xf8;
        write_msg(param, 0xA1, data);
    }

    FP5510::FP5510(int id, int slave_addr, int freq) {
        param_t *param = new param_t();
        _param = param;
        param->i2c_id = id;
        param->slave_addr = slave_addr;
        param->step_mode = 0x0d;
        param->esc = 0;
        param->tsc = 0;
        param->mclk = 0;
        param->t_src = 0;

        param->i2c = new i2c::I2C(id, i2c::Mode::MASTER, freq, i2c::AddrSize::SEVEN_BIT);
        err::check_null_raise(param->i2c, "create i2c error!");

        if (param->i2c->scan(param->slave_addr).empty()) {
            log::error("i2c address %#x not found", param->slave_addr);
            err::check_raise(err::ERR_RUNTIME, "i2c address not found!");
        }

        protection_off(param);
        tsc_esc_mc_setting(param, param->esc, param->tsc, param->mclk);
        st_setting(param, param->t_src);
        protection_on(param);
    }

    FP5510::~FP5510() {
        param_t *param = (param_t *)_param;
        if (param) {
            protection_off(param);

            i2c::I2C *i2c = param->i2c;
            if (i2c) {
                delete i2c;
                i2c = nullptr;
            }

            delete param;
            _param = nullptr;
        }
    }

    void FP5510::set_pos(uint32_t pos) {
        param_t *param = (param_t *)_param;
        pos = pos > 0x3ff ? 0x3ff : pos;
        uint8_t msb = (0x00U | ((pos & 0x3F0U) >> 4U));
	    uint8_t lsb = (((pos & 0x0FU) << 4U) | param->step_mode);
        write_msg(param, msb, lsb);
    }

    uint32_t FP5510::get_pos(void) {
        param_t *param = (param_t *)_param;
        uint8_t msb = 0, lsb = 0;
        read_msg(param, &msb, &lsb);
        uint32_t pos = (((uint16_t)msb << 4) & 0x3F0) | ((uint16_t)lsb & 0xF);
        return pos;
    }
}


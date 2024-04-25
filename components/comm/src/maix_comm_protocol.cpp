/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include <string.h>
#include <assert.h>
#include <stdexcept>
#include "maix_basic.hpp"
#include "maix_uart.hpp"
#include "maix_comm.hpp"

using namespace maix::peripheral;

namespace maix::comm
{

    CommBase *CommProtocol::_get_comm_obj(const std::string &method)
    {
        if(method == "uart")
        {
            std::vector<std::string> ports = uart::list_devices();
            if(ports.size() == 0)
            {
                log::warn("not found uart port, will use /dev/ttyS0");
                return new uart::UART("/dev/ttyS0", 115200);
            }
            return new uart::UART(ports[ports.size() - 1], 115200);
        }
        else
        {
            log::error("not support comm method: %s\n", method.c_str());
            return nullptr;
        }
    }

    CommProtocol::CommProtocol(int buff_size)
    {
        _tmp_buff_len = 128;
        _tmp_buff = new uint8_t[_tmp_buff_len];
        if(!_tmp_buff)
        {
            throw std::bad_alloc();
        }
        _p = new protocol::Protocol(buff_size);
        _comm_method = app::get_sys_config_kv("comm", "method", "uart");
        _comm = _get_comm_obj(_comm_method);
        if(!_comm)
        {
            log::error("get comm object %d failed\n", _comm_method.c_str());
            throw std::runtime_error("get comm object failed");
        }
        err::Err e = _comm->open();
        if(e != err::ERR_NONE)
        {
            log::error("open comm object %d failed: %d\n", _comm_method.c_str(), e);
            throw std::runtime_error("open comm object failed");
        }
    }

    CommProtocol::~CommProtocol()
    {
        if(_comm)
        {
            _comm->close();
            delete _comm;
            _comm = nullptr;
        }
        if(_tmp_buff)
        {
            delete _tmp_buff;
            _tmp_buff = nullptr;
        }
    }

    protocol::MSG *CommProtocol::get_msg()
    {
        protocol::MSG *msg = nullptr;
        int rx_len = 0;
        while(1)
        {
            rx_len = _comm->read(_tmp_buff, _tmp_buff_len, -1, 0);
            if(rx_len == 0)
            {
                break;
            }
            else if(rx_len < 0)
            {
                log::error("read error: %d, %s\n", -rx_len, err::to_str((err::Err)-rx_len).c_str());
                time::sleep_ms(10);
                break;
            }
            _p->push_data(_tmp_buff, rx_len);
        }
        msg = _p->decode(nullptr, 0);
        return msg;
    }

    err::Err CommProtocol::resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        int len = _p->encode_resp_ok(buff, buff_len, cmd, body, body_len);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_ok(uint8_t cmd, uint8_t *body, int body_len)
    {
        Bytes *buff = _p->encode_resp_ok(cmd, body, body_len);
        if(!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_ok(uint8_t cmd, Bytes *body)
    {
        Bytes *buff = _p->encode_resp_ok(cmd, body);
        if(!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        int len = _p->encode_report(buff, buff_len, cmd, body, body_len);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t cmd, uint8_t *body, int body_len)
    {
        Bytes *buff = _p->encode_report(cmd, body, body_len);
        if(!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t cmd, Bytes *body)
    {
        Bytes *buff = _p->encode_report(cmd, body);
        if(!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg)
    {
        int len = _p->encode_resp_err(buff, buff_len, cmd, code, msg);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_err(uint8_t cmd, err::Err code, const std::string &msg)
    {
        Bytes *buff = _p->encode_resp_err(cmd, code, msg);
        if(!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if(len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

} // namespace maix::comm

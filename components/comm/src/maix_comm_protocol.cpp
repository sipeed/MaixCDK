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

#include <vector>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <libgen.h>
#include <fstream>
#include <utility>
#include <tuple>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <thread>
#include <unordered_set>
#include <climits>
#include "maix_fs.hpp"

namespace maix::comm
{
    static CommProtocol *_comm_protocol = nullptr;
    static std::thread *_comm_th = nullptr;
    static volatile bool _comm_loop_need_exit = false;
    static volatile bool _comm_loop_exit = true;

    static void _cancel_comm_uart(uart::UART *obj)
    {
        if(_comm_protocol && obj)
        {
            rm_default_comm_listener();
            maix::log::info("[Maix Comm Protocol] UART %s ready to init", obj->get_port().c_str());
        }
    }

    CommBase *CommProtocol::_get_comm_obj(const std::string &method, err::Err &error)
    {
        error = err::Err::ERR_NONE;
        if (method == "uart")
        {
            std::vector<std::string> ports = uart::list_devices();
            if (ports.size() == 0)
            {
                // log::warn("not found uart port, will use /dev/ttyS0");
                // std::string uart_port("/dev/ttyS0");
                // listener_priv::CommFileHandle::write_comm_info(uart_port);
                // return new uart::UART(uart_port, 115200);
                log::error("No uart port found");
                return nullptr;
            }
            uart::UART *obj = nullptr;
            try
            {
                obj = new uart::UART(ports[ports.size() - 1], 115200);
                if (!obj)
                {
                    log::error("[Maix Comm Protocol] No Memory");
                }
            }catch(...)
            {
                log::error("[Maix Comm Protocol] Create uart obj failed");
            }
            if (!obj)
            {
                return nullptr;
            }
            // register uart
            err::Err e = uart::register_comm_callback(obj, _cancel_comm_uart);
            if (e != err::Err::ERR_NONE)
            {
                log::error("[Maix Comm Protocol] Register uart comm obj failed: %s", err::to_str(e).c_str());
                delete obj;
                return nullptr;
            }
            log::info("[Maix Comm Protocol] listening on uart port: %s", ports[ports.size() - 1].c_str());
            return obj;
        }
        else if (method == "none")
        {
            return nullptr;
        }
        error = err::Err::ERR_ARGS;
        log::error("not support comm method: %s\n", method.c_str());
        return nullptr;
    }

    CommProtocol::CommProtocol(int buff_size, uint32_t header, bool method_none_raise)
    {
        _tmp_buff_len = 128;
        _tmp_buff = new uint8_t[_tmp_buff_len];
        if (!_tmp_buff)
        {
            throw err::Exception(err::ERR_NO_MEM);
        }
        _p = new protocol::Protocol(buff_size, header);
        _comm_method = CommProtocol::get_method();
        _valid = false;
        err::Err e;
        _comm = _get_comm_obj(_comm_method, e);
        if (!_comm)
        {
            if(e != err::ERR_NONE)
            {
                std::string msg = "get comm " + _comm_method + " obj failed";
                log::error(msg.c_str());
                throw err::Exception(err::ERR_RUNTIME, msg);
            }
            log::info("comm protocol disabled");
            if (method_none_raise)
                throw err::Exception(err::ERR_ARGS, "comm protocol disabled");
            return;
        }
        e = _comm->open();
        if (e != err::ERR_NONE)
        {
            std::string msg = "open comm " + _comm_method + " obj failed: " + err::to_str(e);
            log::error(msg.c_str());
            throw err::Exception(err::ERR_RUNTIME, msg);
        }
        _valid = true;
    }

    CommProtocol::~CommProtocol()
    {
        if (_comm)
        {
            _comm->close();
            delete _comm;
            _comm = nullptr;
        }
        if (_tmp_buff)
        {
            delete _tmp_buff;
            _tmp_buff = nullptr;
        }
    }

    err::Err CommProtocol::set_method(const std::string &method)
    {
        if(method != "uart" && method != "none")
            return err::ERR_ARGS;
        return app::set_sys_config_kv("comm", "method", method);
    }

    std::string CommProtocol::get_method()
    {
        return app::get_sys_config_kv("comm", "method", "uart");
    }

    static std::vector<std::string> find_string(char *data, uint32_t data_len, uint32_t try_find_cnt = 0)
    {
        if (data_len <= 1)
            return {""};
        std::vector<std::string> str;
        uint32_t f_cnt = 0;
        uint32_t left = 0;
        for (uint32_t right = 0; right < data_len; ++right)
        {
            if (data[right] == '\0')
            {
                str.push_back(std::string(data + left, right - left));
                left = right + 1;
                if (try_find_cnt != 0 && ++f_cnt >= try_find_cnt)
                    break;
            }
        }
        return str;
    }

    static uint32_t find_idx(std::string &id, bool override_id = false)
    {
        // log::info("[%s] Need to find %s, len: %u", __PRETTY_FUNCTION__, id.c_str(), id.size());
        uint32_t idx = 0;
        auto apps_info = app::get_apps_info();
        for (const auto &info : apps_info)
        {
            // log::info("[%s] %s, len: %u", __PRETTY_FUNCTION__, info.id.c_str(), info.id.size());
            if (id == info.id)
            {
                if (override_id)
                {
                    id = app::get_app_path(id);
                }
                return idx;
            }
            ++idx;
        }
        return UINT32_MAX;
    }

    static void debug_show_strings(char *data, uint32_t data_len, uint32_t try_find_cnt = 0)
    {
        (void)data;
        (void)data_len;
        (void)try_find_cnt;
        // auto res = find_string(data, data_len, try_find_cnt);
        // for (auto& str : res) {
        //     printf("%s ", str.c_str());
        // } printf("\n");
    }

    void CommProtocol::execute_cmd(protocol::MSG *msg)
    {
        // log::info("[%s:%d] Start...", __PRETTY_FUNCTION__, __LINE__);
        switch (msg->cmd)
        {
        case maix::protocol::CMD_APP_LIST:
        {
            auto apps_info = app::get_apps_info();
            uint32_t size = 1;
            for (const auto &app_info : apps_info)
            {
                size += static_cast<uint32_t>(app_info.id.size());
                ++size;
            }
            auto buff = new uint8_t[size];
            buff[0] = static_cast<uint8_t>(apps_info.size() & 0xFF);
            uint32_t index = 1;
            for (const auto &app_info : apps_info)
            {
                // log::info("[%s:%d] CMD_APP_LIST find app: %s",
                //             __PRETTY_FUNCTION__, __LINE__, app_info.id.c_str());
                std::copy(app_info.id.begin(), app_info.id.end(), buff + index);
                index += app_info.id.size();
                buff[index++] = '\0';
            }
            // debug_show_strings((char*)buff+1, size-1);
            auto resp_ret = this->resp_ok(maix::protocol::CMD_APP_LIST, buff, size);
            if (resp_ret != err::Err::ERR_NONE)
            {
                log::error("[%s:%d] resp_ok failed, code = %u",
                           __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
            }
            msg->has_been_replied = true;
            delete[] buff;
            break;
        }
        case maix::protocol::CMD_START_APP:
        {
            uint8_t idx = msg->body[0];
            std::vector<std::string> res = find_string((char *)msg->body + 1, msg->body_len - 1, 2);
            if (res.size() <= 0 || res.size() > 2)
            {
                log::error("Unsupport CMD body");
                auto resp_ret = this->resp_err(maix::protocol::CMD_START_APP, err::Err::ERR_ARGS, "Unsupport CMD body");
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
                break;
            }
            // for (auto& arg:res) {
            //     log::info("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, arg.c_str());
            // }
            /*  If you need to execute an executable other than the /maixapp rule,
                uncomment this and modify app::switch_app to support launching external executables. */
            // if (UINT32_MAX == find_idx(res[0], true) && idx != 0xFF) {
            //     log::info("[%s:%d] Cannot find %s in APP LIST.Try to verify that the executable"
            //                 " file {%s} exists...", __PRETTY_FUNCTION__, __LINE__, res[0].c_str());
            //     if (!fs::exists(res[0])) {
            //         log::error("The executable file does not exist.");
            //         auto resp_ret = this->resp_err(CMD_START_APP, err::Err::ERR_ARGS, "Unsupport CMD body");
            //         if (resp_ret != err::Err::ERR_NONE) {
            //             log::error("[%s:%d] resp_ok failed, code = %u",
            //                 __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
            //         }
            //         break;
            //     }
            // }
            if (res.size() == 1 && idx == 0xFF)
            {
                /* NO ARG */
                // log::info("[%s:%d] START APP {%s}<%u> without arg", __PRETTY_FUNCTION__, __LINE__, res[0].c_str(), res[0].size());
                app::switch_app(res[0]);
                auto resp_ret = this->resp_ok(maix::protocol::CMD_START_APP, nullptr, 0);
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
            }
            else if (res.size() == 1 && idx != 0xFF)
            {
                /* idx WITH ARG */
                // log::info("[%s:%d] START APP idx {%u} with arg {%s}", __PRETTY_FUNCTION__, __LINE__, idx, res[0].c_str());
                auto app_list = app::get_apps_info();
                if (idx >= app_list.size())
                {
                    auto resp_ret = this->resp_err(maix::protocol::CMD_START_APP,
                                                   err::Err::ERR_NOT_FOUND, "app not found with this idx");
                    if (resp_ret != err::Err::ERR_NONE)
                    {
                        log::error("[%s:%d] resp_ok failed, code = %u",
                                   __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                    }
                    msg->has_been_replied = true;
                }
                app::switch_app("", idx, res[0].c_str());
                auto resp_ret = this->resp_ok(maix::protocol::CMD_START_APP, nullptr, 0);
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
            }
            else if (res.size() == 2)
            {
                /* WITH ARG */
                // log::info("[CommProtocol] START APP {%s} with arg {%s}", res[0].c_str(), res[1].c_str());
                app::switch_app(res[0], -1, res[1]);
                auto resp_ret = this->resp_ok(maix::protocol::CMD_START_APP, nullptr, 0);
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
            }
            else
            {
                log::error("[%s:%d] Unsupport body...", __PRETTY_FUNCTION__, __LINE__);
                auto resp_ret = this->resp_err(maix::protocol::CMD_START_APP, err::Err::ERR_ARGS, "Unsupport body!");
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
            }
            break;
        }
        case maix::protocol::CMD_EXIT_APP:
        {
            // log::info("[%s:%d] Need to EXIT...", __PRETTY_FUNCTION__, __LINE__);
            auto err_code = app::set_exit_msg(err::Err::ERR_NONE, "exited by CommProtocol");
            if (err_code != err::Err::ERR_NONE)
            {
                auto resp_ret = this->resp_err(maix::protocol::CMD_EXIT_APP, err_code, "Exit app failed!");
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                break;
            }
            auto resp_ret = this->resp_ok(maix::protocol::CMD_EXIT_APP, nullptr, 0);
            if (resp_ret != err::Err::ERR_NONE)
            {
                log::error("[%s:%d] resp_ok failed, code = %u",
                           __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
            }
            msg->has_been_replied = true;
            app::set_exit_flag(true);
            break;
        }
        case maix::protocol::CMD_CUR_APP_INFO:
        {
            auto res = app::app_id();
            auto idx = find_idx(res);
            int len = static_cast<int>(res.size() + 2);
            auto buff = new uint8_t[len];
            if (idx >= app::get_apps_info().size())
                buff[0] = 0xFF; /* app::app_idx() */
            else
                buff[0] = static_cast<uint8_t>(idx & 0xFF);
            std::copy(res.begin(), res.end(), buff + 1);
            buff[len - 1] = '\0';
            debug_show_strings((char *)buff + 1, len - 1);
            auto resp_ret = this->resp_ok(maix::protocol::CMD_CUR_APP_INFO, buff, len);
            if (resp_ret != err::Err::ERR_NONE)
            {
                log::error("[%s:%d] resp_ok failed, code = %u",
                           __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
            }
            msg->has_been_replied = true;
            delete[] buff;
            break;
        }
        case maix::protocol::CMD_APP_INFO:
        {
            uint8_t idx = msg->body[0];
            auto res = find_string((char *)msg->body + 1, msg->body_len - 1, 1);
            if (res.size() != 1 && idx == 0xFF)
            {
                auto resp_ret = this->resp_err(maix::protocol::CMD_APP_INFO, err::Err::ERR_ARGS, "ERROR ARGS");
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
                break;
            }
            auto apps_info = app::get_apps_info();
            if (idx != 0xFF && idx >= apps_info.size())
            {
                auto resp_ret = this->resp_err(maix::protocol::CMD_APP_INFO, err::Err::ERR_ARGS, "ERROR ARGS");
                if (resp_ret != err::Err::ERR_NONE)
                {
                    log::error("[%s:%d] resp_ok failed, code = %u",
                               __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
                }
                msg->has_been_replied = true;
                break;
            }
            if (idx == 0xFF)
            {
                /* find app with id */
                idx = static_cast<uint8_t>(find_idx(res[0]));
            }
            /* find app with idx */
            auto &app_info = apps_info.at(idx);
            uint32_t size = 1 + app_info.id.size() + 1 + app_info.name.size() + 1 + app_info.desc.size() + 1;
            auto buff = new uint8_t[size];
            buff[0] = idx;
            uint32_t index = 1;
            std::copy(app_info.id.begin(), app_info.id.end(), buff + index);
            index += app_info.id.size();
            buff[index++] = '\0';
            std::copy(app_info.name.begin(), app_info.name.end(), buff + index);
            index += app_info.name.size();
            buff[index++] = '\0';
            std::copy(app_info.desc.begin(), app_info.desc.end(), buff + index);
            index += app_info.desc.size();
            buff[index++] = '\0';
            auto resp_ret = this->resp_ok(maix::protocol::CMD_APP_INFO, buff, size);
            if (resp_ret != err::Err::ERR_NONE)
            {
                log::error("[%s:%d] resp_ok failed, code = %u",
                           __PRETTY_FUNCTION__, __LINE__, (uint8_t)resp_ret);
            }
            msg->has_been_replied = true;
            delete[] buff;
            break;
        }
        case maix::protocol::CMD_KEY:
        case maix::protocol::CMD_TOUCH:
        case maix::protocol::CMD_SET_REPORT:
            msg->has_been_replied = false;
            // log::warn("[%s] Not impl...", __PRETTY_FUNCTION__);
            break;
        default:
            msg->has_been_replied = false;
            break;
        }
        // log::info("[%s:%d] Finish...", __PRETTY_FUNCTION__, __LINE__);
    }

    protocol::MSG *CommProtocol::get_msg(int timeout)
    {
        protocol::MSG *msg = nullptr;
        if(!_valid)
            return msg;
        uint64_t t = time::ticks_ms();
        while (1)
        {
            int rx_len = 0;
            while (1)
            {
                rx_len = _comm->read(_tmp_buff, _tmp_buff_len, -1, timeout);
                if (rx_len == 0)
                {
                    break;
                }
                else if (rx_len < 0)
                {
                    log::error("read error: %d, %s\n", -rx_len, err::to_str((err::Err)-rx_len).c_str());
                    time::sleep_ms(10);
                    break;
                }
                _p->push_data(_tmp_buff, rx_len);
            }
            msg = _p->decode(nullptr, 0);
            if (msg || timeout == 0)
                break;
            if (timeout > 0 && (time::ticks_ms() - t > (uint64_t)timeout))
                break;
        }
        if (msg)
            this->execute_cmd(msg);
        return msg;
    }

    err::Err CommProtocol::resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        int len = _p->encode_resp_ok(buff, buff_len, cmd, body, body_len);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_ok(uint8_t cmd, uint8_t *body, int body_len)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        Bytes *buff = _p->encode_resp_ok(cmd, body, body_len);
        if (!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_ok(uint8_t cmd, Bytes *body)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        Bytes *buff = _p->encode_resp_ok(cmd, body);
        if (!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        int len = _p->encode_report(buff, buff_len, cmd, body, body_len);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t cmd, uint8_t *body, int body_len)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        Bytes *buff = _p->encode_report(cmd, body, body_len);
        if (!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::report(uint8_t cmd, Bytes *body)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        Bytes *buff = _p->encode_report(cmd, body);
        if (!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        int len = _p->encode_resp_err(buff, buff_len, cmd, code, msg);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        len = _comm->write(buff, len);
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    err::Err CommProtocol::resp_err(uint8_t cmd, err::Err code, const std::string &msg)
    {
        if(!_valid)
            return err::ERR_NOT_PERMIT;
        Bytes *buff = _p->encode_resp_err(cmd, code, msg);
        if (!buff)
        {
            return err::ERR_RUNTIME;
        }
        int len = _comm->write(buff->data, buff->size());
        delete buff;
        if (len < 0)
        {
            return (err::Err)-len;
        }
        return err::ERR_NONE;
    }

    void _comm_loop()
    {
        int timeout = 50; // 50 ms for fast exit at program start if user want comm to exit. after 3s, timeout will be 500ms to reduce cpu usage.
        uint64_t _t0 = time::ticks_ms();
        bool flag = false;
        while (!maix::app::need_exit() && !_comm_loop_need_exit)
        {
            try
            {
                if(!flag && (time::ticks_ms() - _t0) > 3000)
                {
                    flag = true;
                    timeout = 500;
                }
                auto msg = _comm_protocol->get_msg(timeout);
                if (!msg)
                    continue;
                if (msg->is_resp || msg->has_been_replied)
                {
                    delete msg;
                    continue;
                }
                _comm_protocol->resp_err(msg->cmd, err::Err::ERR_ARGS, "Unsupport CMD");
                delete msg;
            }
            catch (const std::exception &e)
            {
                maix::log::debug("[Default CommListener] %s", e.what());
                break;
            }
        }
        maix::log::info("[Maix Comm Protocol] exit success");
        _comm_loop_exit = true;
    }

    void add_default_comm_listener()
    {
        if(!_comm_protocol)
        {
            _comm_protocol = new maix::comm::CommProtocol();
            _comm_loop_exit = false;
            _comm_loop_need_exit = false;
            if(_comm_protocol->valid())
            {
                _comm_th = new std::thread([]()
                {
                    _comm_loop();
                });
            }
            else
            {
                _comm_loop_exit = true;
            }
        }
    }

    bool rm_default_comm_listener()
    {
        if(_comm_protocol)
        {
            log::info("[Maix Comm Protocol] exit...");
            _comm_loop_need_exit = true;
            while(!_comm_loop_exit)
            {
                time::sleep_ms(10);
            }
            if(_comm_th)
            {
                _comm_th->join();
                delete _comm_th;
                _comm_th = nullptr;
            }
            delete _comm_protocol;
            _comm_protocol = nullptr;
        }
        return true;
    }
} // namespace maix::comm


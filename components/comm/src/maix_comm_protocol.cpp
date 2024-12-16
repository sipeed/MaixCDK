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

namespace maix::comm::listener_priv
{

    class CommFileHandle
    {
    public:
        CommFileHandle() = delete;
        CommFileHandle(CommFileHandle &) = delete;
        ~CommFileHandle() = delete;
        CommFileHandle operator=(const CommFileHandle &) = delete;

        static std::string get_process_name();
        static std::pair<std::string, std::string> _get_file_path();
        static int write_comm_info(const std::string &info);
        static std::string read_comm_info();
        static bool rm_comm_info();
        static std::string read_symlink(const std::string &&path);
        static std::string read_symlink_recursive(const std::string &path, std::unordered_set<std::string> &visited);
    };

    class CommListener
    {
    public:
        CommListener(const CommListener &) = delete;
        CommListener &operator=(const CommListener &) = delete;

        static CommListener &get_instance();
        static void rm_instance();
        void start_listen();
        bool stop();

    private:
        CommListener();
        ~CommListener();

        void run() noexcept;
        static void clear_instance() noexcept;
        void loop() noexcept;

        bool check_device_occupy(const uint64_t interval_ms = 0) noexcept;
        // bool check_device_occupy_reset_timer()
        // {
        //     _last_check_time = time::ticks_ms();
        // }

        bool keep_join();

    private:
        static std::atomic<bool> initialized;
        static CommListener *instance;

        int value{0};
        maix::comm::CommProtocol *protocol{nullptr};
        std::string device{""};

        std::thread *th{nullptr};
        bool _need_exit{false};
        uint64_t _last_check_time;
    };

}

namespace maix::comm
{
    static std::fstream testfile_s;
    static std::thread *_stop_th = nullptr;
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
            maix::log::debug("Comm uart: %s", ports[ports.size() - 1].c_str());
            listener_priv::CommFileHandle::write_comm_info(ports[ports.size() - 1]);
            return new uart::UART(ports[ports.size() - 1], 115200);
        }
        else if (method == "none")
        {
            return nullptr;
        }
        error = err::Err::ERR_ARGS;
        log::error("not support comm method: %s\n", method.c_str());
        return nullptr;
        /* i2c device: /dev/i2c-x
            listener_priv::CommFileHandle::write_comm_info("/dev/i2c-x"); */
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

    void add_default_comm_listener()
    {
        comm::listener_priv::CommListener &listener = comm::listener_priv::CommListener::get_instance();
        listener.start_listen();
    }

    bool rm_default_comm_listener()
    {
        comm::listener_priv::CommListener::rm_instance();
        return true;
    }
} // namespace maix::comm

namespace maix::comm::listener_priv
{
    static const char *COMM_LISTENER_CACHE_PATH = "/tmp/maixapp-cache/";

    std::string CommFileHandle::get_process_name()
    {
        char buffer[1024];
        ::memset(buffer, 0x00, sizeof(buffer));
        ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len == -1)
            return {};
        buffer[len] = '\0';
        return {::basename(buffer)};
    }

    std::pair<std::string, std::string> CommFileHandle::_get_file_path()
    {
        std::string path(COMM_LISTENER_CACHE_PATH);
        if (!maix::fs::exists(path))
        {
            // maix::log::info("%s not exists, creating...", path.c_str());
            maix::fs::mkdir(path);
        }
        pid_t pid = ::getpid();
        std::string filepath(path + get_process_name() + '-' + std::to_string(pid));

        return std::make_pair(path, filepath);
    }

    int CommFileHandle::write_comm_info(const std::string &info)
    {
        std::string path, filepath;
        std::tie(path, filepath) = _get_file_path();

        std::fstream file(filepath, std::ios::out);
        if (!file.is_open())
        {
            maix::log::error("open file %s failed!", filepath.c_str());
            return -1;
        }
        file << info << std::endl;
        file.close();
        return 0;
    }

    std::string CommFileHandle::read_comm_info()
    {
        std::string path, filepath;
        std::tie(path, filepath) = _get_file_path();

        if (!maix::fs::exists(filepath))
        {
            maix::log::error("Comm config file %s does not exit!", filepath.c_str());
            return {};
        }

        std::fstream file(filepath, std::ios::in);
        if (!file.is_open())
        {
            maix::log::error("open file %s failed!", filepath.c_str());
            return {};
        }

        std::string info;
        std::getline(file, info);
        return info;
    }

    bool CommFileHandle::rm_comm_info()
    {
        std::string path, filepath;
        std::tie(path, filepath) = _get_file_path();

        if (!maix::fs::exists(filepath))
            return true;
        return (0 == std::remove(filepath.c_str()));
    }

    std::string CommFileHandle::read_symlink_recursive(const std::string &path, std::unordered_set<std::string> &visited)
    {
        if (visited.find(path) != visited.end())
        {
            maix::log::error("Detected loop in symbolic links");
            return {};
        }
        visited.insert(path);

        char buffer[1024];
        std::fill_n(buffer, sizeof(buffer), 0x00);
        ssize_t len = ::readlink(path.c_str(), buffer, sizeof(buffer) - 1);
        if (len == -1)
        {
            maix::log::error("readlink failed!!!");
            return {};
        }
        buffer[len] = '\0';
        std::string target(buffer);

        char resolved_path[PATH_MAX];
        if (::realpath(target.c_str(), resolved_path) == nullptr)
        {
            maix::log::error("realpath failed!!!");
            return {};
        }

        std::string resolved_target(resolved_path);
        if (fs::islink(std::string(resolved_target)))
        {
            return read_symlink_recursive(resolved_target, visited);
        }
        // maix::log::info("read_symlink_recursive target: %s", resolved_target.c_str());
        return resolved_target;
    }

    std::string CommFileHandle::read_symlink(const std::string &&path)
    {
        std::unordered_set<std::string> visited;
        return read_symlink_recursive(path, visited);
    }

    static uint32_t list_open_files(const char *target, uint32_t str_len)
    {
        char path[256];
        DIR *dir;
        struct dirent *entry;
        uint32_t cnt = 0;

        pid_t pid = ::getpid();

        ::snprintf(path, sizeof(path), "/proc/%d/fd", pid);

        if ((dir = ::opendir(path)) == nullptr)
        {
            maix::log::error("[%s] opendir failed!", __PRETTY_FUNCTION__);
            return cnt;
        }

        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (entry->d_type != DT_LNK)
                continue;

            size_t path_len = ::strlen(path);
            size_t name_len = ::strlen(entry->d_name);
            size_t buf_len = path_len + 1 + name_len + 1;

            char *buf = new char[buf_len];
            if (buf == nullptr)
            {
                maix::log::error("[%s] malloc failed!", __PRETTY_FUNCTION__);
                ::closedir(dir);
                return cnt;
            }

            ::snprintf(buf, buf_len, "%s/%s", path, entry->d_name);

            char link[256];
            ssize_t len = ::readlink(buf, link, sizeof(link) - 1);
            if (len == str_len && std::equal(link, link + len, target))
                ++cnt;

            delete[] buf;
        }

        ::closedir(dir);
        return cnt;
    }

    static uint32_t get_open_file_number(const std::string &filename)
    {
        // uint32_t cnt = list_open_files(filename.c_str(), filename.size());
        // maix::log::info("===========================%s cnt: %u", filename.c_str(), cnt);
        // return cnt;
        return list_open_files(filename.c_str(), filename.size());
    }

    static std::string analyze_device(const std::string &&device)
    {
        if (!maix::fs::exists(device))
        {
            maix::log::error("Device/File %s does not exists!", device.c_str());
            return {};
        }
        if (!fs::islink(std::move(device)))
        {
            // maix::log::info("%s no a symlink", device.c_str());
            return device;
        }
        return CommFileHandle::read_symlink(std::move(device));
    }

    std::atomic<bool> CommListener::initialized{false};
    CommListener *CommListener::instance = nullptr;

    CommListener::CommListener()
    {
        try
        {
            this->protocol = new maix::comm::CommProtocol();
        }
        catch (const std::exception &e)
        {
            this->protocol = nullptr;
        }
        if(this->protocol)
        {
            this->device = analyze_device(CommFileHandle::read_comm_info());
            log::debug("[Default CommListener] Start listening on port %s", this->device.c_str());
        }
    }

    CommListener::~CommListener()
    {
        // maix::log::info("CommListener destroyed");
        // this->stop();
        if (this->protocol)
        {
            delete this->protocol;
        }
        maix::log::debug("[Default CommListener] Stop listening on port %s", this->device.c_str());
        this->device.clear();
    }

    CommListener &CommListener::get_instance()
    {
        static std::mutex init_mutex;

        if (!initialized.load(std::memory_order_relaxed))
        {
            std::lock_guard<std::mutex> lock(init_mutex);
            if (!instance)
            {
                instance = new CommListener();
            }
            initialized.store(true, std::memory_order_relaxed);
        }
        return *instance;
    }

    bool CommListener::keep_join()
    {
        if (this->th == nullptr)
            return true;

        this->th->join();
        delete this->th;
        this->th = nullptr;

        return true;
    }

    bool CommListener::stop()
    {
        this->_need_exit = true;
        return keep_join();
    }

    void CommListener::rm_instance()
    {
        static std::mutex init_mutex;
        std::lock_guard<std::mutex> lock(init_mutex);

        if (initialized.load(std::memory_order_relaxed))
        {
            if (instance)
            {
                instance->stop();
                delete instance;
                instance = nullptr;
                CommFileHandle::rm_comm_info();
            }
            initialized.store(false, std::memory_order_relaxed);
        }
    }

    void CommListener::start_listen()
    {
        if (this->th != nullptr)
        {
            maix::log::warn("Default CommListener thread already running!!! IGNORE.");
            return;
        }
        if (this->protocol && this->protocol->valid())
        {
            this->th = new std::thread([this]()
                                       { this->run(); });
        }
    }

    void CommListener::run() noexcept
    {
        static std::atomic<bool> running_flag{false};
        if (true == running_flag.load(std::memory_order_relaxed))
        {
            maix::log::error("Default CommListener::run() already running!!!");
            return;
        }
        running_flag.store(true, std::memory_order_relaxed);
        this->loop();
        running_flag.store(false, std::memory_order_relaxed);
    }

    bool CommListener::check_device_occupy(const uint64_t interval_ms) noexcept
    {
        if (interval_ms == 0 || time::ticks_ms() - _last_check_time > interval_ms)
        {
            _last_check_time = time::ticks_ms();
            if (!this->device.empty() && get_open_file_number(this->device) > 1)
            {
                return true;
            }
        }
        return false;
    }

    void CommListener::loop() noexcept
    {
        int interval = 0;
        uint64_t start_ticks = time::ticks_s();
        int sleep_time = 10;
        bool flag = false;
        while (!maix::app::need_exit() && !this->_need_exit)
        {
            if(!flag)
            {
                if(time::ticks_s() - start_ticks > 2)
                {// check faster when program startup, then only check in 3s interval
                    interval = 3000;
                    sleep_time = 100;
                    flag = true;
                }
            }
            if (check_device_occupy(interval))
            {
                maix::log::info("[Default CommListener] Release device %s for program use",
                                this->device.c_str(), this->device.c_str());
                if (_stop_th)
                {
                    delete _stop_th;
                }
                _stop_th = new std::thread([]()
                                           { rm_default_comm_listener(); });
                break;
            }
            auto msg = this->protocol->get_msg(0);
            if (!msg)
                continue;
            if (msg->is_resp || msg->has_been_replied)
            {
                delete msg;
                continue;
            }
            this->protocol->resp_err(msg->cmd, err::Err::ERR_ARGS, "Unsupport CMD");
            delete msg;
            time::sleep_ms(sleep_time); // default listenner don't need fast react, to release CPU usage, so sleep here.
        }
        maix::log::info("[Default CommListener] exit success");
    }
}

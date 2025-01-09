/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_basic.hpp"
#include "maix_key.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "maix_app.hpp"
#include "maix_log.hpp"

#define KEY_DEVICE "/dev/input/event_keys"
#define KEY_DEVICE2 "/dev/input/event0"

static bool _key_defult_listener = false;
static maix::peripheral::key::Key *_default_key = nullptr;

static void on_key(int key, int state)
{
    // send INT signal to process
    if(state == maix::peripheral::key::KEY_PRESSED)
    {
        maix::log::info("exit app by KEY_OK");
        maix::app::set_exit_flag(true);
        raise(SIGINT);
    }
}

namespace maix::peripheral::key
{

    void add_default_listener()
    {
        if(_default_key)
            return;
    }

    void rm_default_listener()
    {
        if(_default_key)
        {
            delete _default_key;
            _default_key = nullptr;
        }
        _key_defult_listener = false;
    }

    class Port_Data
    {
    public:
        thread::Thread *thread;
        int fd;
        int long_press_time;
        bool read_thread_exit;
        bool read_thread_need_exit;
        std::vector<int> fds;
        Key *key;
        std::function<void(int, int)> callback;
    };


    static void _read_process(void *args)
    {
        Port_Data *data = (Port_Data *)args;
        int key = 0;
        int value = 0;
        uint64_t tick = 0;
        // epoll wait key event
        struct epoll_event ev;
        int epoll_fd = epoll_create(1);
        if (epoll_fd < 0)
        {
            data->read_thread_exit = true;
            log::error("create epoll failed: %s", strerror(errno));
            return;
        }
        ev.events = EPOLLIN;
        for (int fd : data->fds) {
            if (fd > 0) {
                ev.data.fd = fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
                    log::error("epoll_ctl add failed for fd %d: %s", fd, strerror(errno));
                    data->read_thread_exit = true;
                    close(epoll_fd);
                    return;
                }
            }
        }
        int fail_count = 0;
        while (!data->read_thread_need_exit && !app::need_exit())
        {
            int nfds = epoll_wait(epoll_fd, &ev, 1, 200);
            if (nfds > 0)
            {
                data->fd = ev.data.fd;
                err::Err e = data->key->read(key, value);
                if (e == err::Err::ERR_NONE)
                {
                    data->callback(key, value);
                    // get ticks_ms when the key is pressed.
                    if (key != 0 && value == 1) {
                        tick = time::ticks_ms();
                    } else if (key != 0 && value == 0) {
                        tick = 0;
                    }
                }
                else if (e != err::Err::ERR_NOT_READY)
                {
                    if(++fail_count > 10)
                    {
                        log::error("read key thread read failed: %s", err::to_str(e).c_str());
                        break;
                    }
                    time::sleep_ms(300);
                }
                continue;
            }
            // if long press event occurs, send KEY_LONG_PRESSED.
            if (tick != 0 && data->long_press_time !=0) {
                if ((int)(time::ticks_ms()- tick) >= data->long_press_time) {
                    data->callback(key, State::KEY_LONG_PRESSED);
                    tick = 0;
                }
            }
            time::sleep_ms(1);
        }
        log::info("read key thread exit");
        data->read_thread_exit = true;
    }

    Key::Key(std::function<void(int, int)> callback, bool open, const string &device, int long_press_time)
    {
        if(_key_defult_listener)
            rm_default_listener();
        this->_callback = callback;
        this->_data = nullptr;
        this->_device = "";
        this->_device_list = {};
        Port_Data *data = new Port_Data();
        this->_data = data;
        if (!this->_data)
        {
            throw err::Exception(err::ERR_NO_MEM, "create key data failed");
        }
        data->thread = nullptr;
        data->fd = -1;
        data->long_press_time = long_press_time;
        data->read_thread_exit = false;
        data->read_thread_need_exit = false;
        data->key = this;
        data->callback = callback;

        if (open)
        {
            err::Err e = this->open();
            if (e != err::ERR_NONE)
            {
                throw err::Exception(err::ERR_NOT_FOUND, std::string("Key device") + KEY_DEVICE + " not found");
            }
        }
    }

    Key::~Key()
    {
        close();
        if (this->_data)
        {
            Port_Data *data = (Port_Data *)this->_data;
            if(data->thread)
            {
                data->read_thread_need_exit = true;
                log::info("wait read key thread exit");
                while (!data->read_thread_exit)
                {
                    time::sleep_ms(1);
                }
                delete data->thread;
                data->thread = nullptr;
            }
            delete data;
            this->_data = nullptr;
        }
    }

    err::Err Key::open()
    {
        if (this->_fds.size())
        {
            this->close();
        }
        bool is_open = false;
        if(!this->_device.empty())
        {
            int fd = ::open(_device.c_str(), O_RDONLY);
            if (fd > 0) {
                int flags = fcntl(fd, F_GETFL, 0);
                fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                this->_fds.push_back(fd);
                is_open = true;
            }
        }
        if(!is_open)
        {
            int fd = ::open(KEY_DEVICE, O_RDONLY);
            if (fd < 0)
            {
                std::vector<std::string> key_devices = {{KEY_DEVICE2}};
                for (const auto& device : key_devices) {
                    int fd = ::open(device.c_str(), O_RDONLY);
                    if (fd == -1) {
                        log::error(("Failed to open device: " + device).c_str());
                        this->_device = device;
                        return err::Err::ERR_NOT_FOUND;
                    }
                    int flags = fcntl(fd, F_GETFL, 0);
                    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                    this->_fds.push_back(fd);
                }
            } else {
                int flags = fcntl(fd, F_GETFL, 0);
                fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                this->_fds.push_back(fd);
            }
        }
        if (this->_callback)
        {
            // new thread to read key
            Port_Data *data = (Port_Data *)this->_data;
            data->fds = this->_fds;
            data->read_thread_exit = false;
            data->read_thread_need_exit = false;
            data->thread = new thread::Thread(_read_process, this->_data);
            data->thread->detach();
        }
        return err::Err::ERR_NONE;
    }

    err::Err Key::read(int &key, int &value)
    {
        Port_Data *data = (Port_Data *)this->_data;
        if (this->_fds.size() == 0)
        {
            return err::Err::ERR_NOT_OPEN;
        }
        struct input_event ev;
        int ret;
        while (1)
        {
            if (this->_callback == nullptr) {
                for (int fd : this->_fds)
                {
                    ret = ::read(fd, &ev, sizeof(struct input_event));
                    if (ret > 0 && ev.code != 0) 
                        break;
                }
            } else {
                ret = ::read(data->fd, &ev, sizeof(struct input_event));
            }
            if (ret < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return err::Err::ERR_NOT_READY;
                }
                log::error("read key failed: %s", strerror(errno));
                return err::Err::ERR_READ;
            }
            if (ret != sizeof(struct input_event))
            {
                return err::Err::ERR_NOT_READY;
            }
            if (ev.type == EV_KEY)
            {
                key = ev.code;
                value = ev.value;
                break;
            }
        }
        return err::Err::ERR_NONE;
    }

    std::pair<int, int> Key::read()
    {
        int key = 0;
        int value = 0;
        err::Err e = this->read(key, value);
        if (e == err::Err::ERR_NOT_READY)
        {
            return std::make_pair(0, 0);
        }
        else if (e != err::Err::ERR_NONE)
        {
            throw err::Exception(e, "Key read failed");
        }
        return std::make_pair(key, value);
    }

    err::Err Key::close()
    {
        Port_Data *data = (Port_Data *)this->_data;
        data->read_thread_need_exit = true;

        bool err = false;
        for (int &fd : this->_fds)
        {
            if (fd > 0)
            {
                int ret = ::close(fd);
                if (ret < 0)
                {
                    log::error("Failed to close fd %d: %s", fd, strerror(errno));
                    err = true;
                }
                fd = -1;
            }
        }

        this->_fds.clear();
        return err ? err::Err::ERR_IO : err::Err::ERR_NONE;
    }

    bool Key::is_opened()
    {
        return this->_fds.size() > 0;
    }

    int Key::long_press_time(int press_time)
    {
        Port_Data *data = (Port_Data *)this->_data;
        if (press_time < 0) {
            return data->long_press_time;
        } else {
            return data->long_press_time = press_time;
        }
    }

    err::Err Key::get_key_devices()
    {
        return err::Err::ERR_NOT_IMPL;
    }

} // namespace maix::peripheral::key

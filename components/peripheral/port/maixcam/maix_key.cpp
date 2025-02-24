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
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include "maix_app.hpp"
#include "maix_log.hpp"
#include "maix_i2c.hpp"

#define KEY_DEVICE "/dev/input/event_keys"
#define KEY_DEVICE0 "/dev/input/event0"
#define KEY_DEVICE1 "/dev/input/powerkey"

static bool _key_defult_listener = false;
static maix::peripheral::key::Key *_default_key = nullptr;
static maix::peripheral::i2c::I2C *i2c_dev = nullptr;

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
        _default_key = new Key(on_key, true, KEY_DEVICE0);
        _key_defult_listener = true;
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
        thread::Thread *powerkey_thread;
        int fd, io_fd, uinput_fd;
        int long_press_time;
        bool read_thread_exit;
        bool read_thread_need_exit;
        bool powerkey_thread_exit;
        bool powerkey_thread_need_exit;
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

    static void _powerkey_process(void *args)
    {
        Port_Data *data = (Port_Data *)args;
        data->io_fd = open("/sys/class/gpio/gpio448/value", O_RDONLY);
        if (data->io_fd < 0) {
            data->powerkey_thread_exit = true;
            log::error("open gpio failed: %s", strerror(errno));
            return;
        }

        struct epoll_event ev;
        int epoll_fd = epoll_create1(0);
        if (epoll_fd < 0)
        {
            data->powerkey_thread_exit = true;
            log::error("create epoll failed: %s", strerror(errno));
            return;
        }
        ev.events = EPOLLPRI;
        ev.data.fd = data->io_fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->io_fd, &ev) < 0) {
            log::error("epoll_ctl add failed: %s", strerror(errno));
            data->powerkey_thread_exit = true;
            close(epoll_fd);
            return;
        }

        char buf[32];
        uint8_t i2c_data = 0xFF;
        struct input_event uinput_ev;
        static bool is_pressed = false;
        read(data->io_fd, buf, sizeof(buf));
        while (!data->powerkey_thread_need_exit && !app::need_exit())
        {
            int nfds = epoll_wait(epoll_fd, &ev, 1, 200);
            if (nfds > 0) {
                if (ev.events & EPOLLPRI) {
                    lseek(data->io_fd, 0, SEEK_SET);
                    read(data->io_fd, buf, sizeof(buf));

                    if (buf[0] == '0' && !is_pressed) {
                        is_pressed = true;
                        uinput_ev.type = EV_KEY;
                        uinput_ev.code = KEY_POWER;
                        uinput_ev.value = 1;
                        gettimeofday(&uinput_ev.time, NULL);
                        write(data->uinput_fd, &uinput_ev, sizeof(uinput_ev));
                        log::debug("Key pressed.\n");
                    } else if (buf[0] == '0' && is_pressed) {
                        is_pressed = false;
                        uinput_ev.type = EV_KEY;
                        uinput_ev.code = KEY_POWER;
                        uinput_ev.value = 0;
                        gettimeofday(&uinput_ev.time, NULL);
                        write(data->uinput_fd, &uinput_ev, sizeof(uinput_ev));
                        log::debug("Key press detected.\n");
                    }
                    uinput_ev.type = EV_SYN;
                    uinput_ev.code = SYN_REPORT;
                    uinput_ev.value = 0;
                    gettimeofday(&uinput_ev.time, NULL);
                    write(data->uinput_fd, &uinput_ev, sizeof(uinput_ev));

                    if (i2c_dev->writeto_mem(0x34, 0x49, &i2c_data, 1) != 1) {
                        log::error("clean pmu irq failed");
                        return;
                    }
                }
            }
            time::sleep_ms(20);
        }
        log::info("powerkey thread exit");
        data->powerkey_thread_exit = true;
    }

    static void _init_power_key(void *args)
    {
        uint8_t i2c_data = 0xFF;
        i2c_dev = new peripheral::i2c::I2C(4, i2c::Mode::MASTER);
        if (i2c_dev->writeto_mem(0x34, 0x49, &i2c_data, 1) != 1) {
            log::error("clean pmu irq failed");
            return;
        }

        if (!fs::exists("/sys/class/gpio/gpio448")) {
            system("echo 448 > /sys/class/gpio/export");
            system("echo \"in\" > /sys/class/gpio/gpio448/direction");
            system("echo \"falling\" > /sys/class/gpio/gpio448/edge");
            log::info("export axp2101 irq gpio success");
        }

        Port_Data *data = (Port_Data *)args;
        data->uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (data->uinput_fd < 0) {
            log::error("open /dev/uinput error");
            return;
        }

        ioctl(data->uinput_fd, UI_SET_EVBIT, EV_KEY);
        ioctl(data->uinput_fd, UI_SET_KEYBIT, KEY_POWER);
        struct uinput_user_dev uidev;
        memset(&uidev, 0, sizeof(uidev));
        strncpy(uidev.name, "powerkey", UINPUT_MAX_NAME_SIZE);
        uidev.id.bustype = BUS_USB;
        uidev.id.vendor = 0x3346;
        uidev.id.product = 0x2333;
        uidev.id.version = 1;
        write(data->uinput_fd, &uidev, sizeof(uidev));
        ioctl(data->uinput_fd, UI_DEV_CREATE);

        int max_event = -1;
        if (fs::exists(KEY_DEVICE1)) {
            fs::remove(KEY_DEVICE1);
        }

        DIR* dir = opendir("/dev/input/");
        if (!dir) {
            log::error("failed to open /dev/input");
            return;
        }

        try {
            struct dirent* ent;
            while ((ent = readdir(dir)) != nullptr) {
                std::string name(ent->d_name);
                if (name.find("event") == 0) {
                    int eventNumber = std::stoi(name.substr(5));
                    max_event = std::max(max_event, eventNumber);
                }
            }
            closedir(dir);
            std::string targetPath = "/dev/input/event" + std::to_string(max_event);
            if (fs::symlink(targetPath.c_str(), KEY_DEVICE1) == -1) {
                log::error("%s symlink error.", KEY_DEVICE1);
            } else {
                log::info("%s symlink to %s.", KEY_DEVICE1, targetPath.c_str());
            }
        } catch (const std::exception& e) {
            closedir(dir);
            log::error("Exception occurred: %s", e.what());
        }

        data->powerkey_thread = new thread::Thread(_powerkey_process, data);
        data->powerkey_thread->detach();
    }

    static void _deinit_power_key(void *args)
    {
        Port_Data *data = (Port_Data *)args;
        ioctl(data->uinput_fd, UI_DEV_DESTROY);
        close(data->uinput_fd);
        if (data->io_fd > 0) {
            close(data->io_fd);
        }
        ::unlink(KEY_DEVICE1);
    }

    Key::Key(std::function<void(int, int)> callback, bool open, const string &device, int long_press_time)
    {
        if(_key_defult_listener)
            rm_default_listener();
        this->_callback = callback;
        this->_data = nullptr;
        this->_device = device;
        this->_device_list = {};
        Port_Data *data = new Port_Data();
        this->_data = data;
        if (!this->_data)
        {
            throw err::Exception(err::ERR_NO_MEM, "create key data failed");
        }
        data->thread = nullptr;
        data->powerkey_thread = nullptr;
        data->fd = -1;
        data->io_fd = -1;
        data->uinput_fd = -1;
        data->long_press_time = long_press_time;
        data->read_thread_exit = false;
        data->read_thread_need_exit = false;
        data->powerkey_thread_exit = false;
        data->powerkey_thread_need_exit = false;
        data->key = this;
        data->callback = callback;

        err::Err e = get_key_devices();
        if (e != err::Err::ERR_NONE || _device_list.empty()) {
            // log::warn("Failed to get key devices, use default device: %s.", KEY_DEVICE0);
            _device_list.push_back(KEY_DEVICE0);
        } else {
            log::debug("Get key devices success.");
        }

        if (std::find(_device_list.begin(), _device_list.end(), KEY_DEVICE1) != _device_list.end() &&
            !fs::exists(KEY_DEVICE1) &&
            (device == "" || device == KEY_DEVICE1))
        {
            log::info("%s: Init pmu power key.", sys::device_name().c_str());
            _init_power_key((void*)data);
        }

        if (open)
        {
            err::Err e = this->open();
            if (e != err::ERR_NONE)
            {
                throw err::Exception(err::ERR_NOT_FOUND, std::string("Key device") + this->_device + " not found");
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
            if(data->powerkey_thread)
            {
                data->powerkey_thread_need_exit = true;
                log::info("wait powerkey thread exit");
                while (!data->powerkey_thread_exit)
                {
                    time::sleep_ms(1);
                }
                delete data->powerkey_thread;
                data->powerkey_thread = nullptr;
            }
            if (fs::exists(KEY_DEVICE1)) {
                _deinit_power_key((void*)data);
            }
            delete data;
            this->_data = nullptr;
        }
        if (i2c_dev != nullptr) {
            delete i2c_dev;
            i2c_dev = nullptr;
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
                for (const auto& device : _device_list) {
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
                // fix key code
                if(ev.code == KEY_DISPLAYTOGGLE)
                    ev.code = KEY_OK;
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
        std::ifstream file("/boot/board");
        if (!file.is_open()) {
            return err::Err::ERR_NOT_FOUND;
        }

        auto device_configs = sys::device_configs();
        auto it = device_configs.find("key_devices");

        if (it == device_configs.end()) {
            return err::Err::ERR_NOT_FOUND;
        }

        std::stringstream ss(it->second);
        std::string device;

        _device_list.clear();
        while (std::getline(ss, device, ',')) {
            device.erase(device.find_last_not_of(" \n\r\t")+1);
            device.erase(0, device.find_first_not_of(" \n\r\t"));
            if (!device.empty()) {
                _device_list.push_back(device);
            }
        }

        return err::Err::ERR_NONE;
    }

} // namespace maix::peripheral::key

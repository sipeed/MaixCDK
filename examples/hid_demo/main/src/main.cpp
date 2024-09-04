

#include "maix_basic.hpp"
#include "maix_hid.hpp"
#include "main.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace maix;
using namespace maix::peripheral;

static int cmd_init(void);
static int cmd_loop(hid::Hid &mouse, hid::Hid &keyboard, hid::Hid &touchpad);

int _main(int argc, char* argv[])
{
    hid::Hid mouse = hid::Hid(hid::DEVICE_MOUSE);
    hid::Hid keyboard = hid::Hid(hid::DEVICE_KEYBOARD);
    hid::Hid touchpad = hid::Hid(hid::DEVICE_TOUCHPAD);
    cmd_init();
    while(!app::need_exit())
    {
        // cmd loop
        cmd_loop(mouse, keyboard, touchpad);
        time::sleep_ms(100);
    }
    return 0;
}

static int cmd_init(void)
{
    int flag;
    if((flag = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
    {
        perror("fcntl");
        return -1;
    }

    flag = flag | O_NONBLOCK;
    if (0 < fcntl(STDIN_FILENO, F_SETFL, flag)) {
        perror("fcntl");
        return -1;
    }

    printf( "========================\r\n"
            "Key value. Refer to the \"Universal Serial Bus HID Usage Tables\" section of the official documentation(https://www.usb.org)."
            "Intput param:\r\n"
            "0 <key_value> : click key\r\n"
            "1 <press> <offset_x> <offset_y> <wheel movement>: mouse move\r\n"
            "2 <press> <x> <y> <wheel movement>: touchpad move\r\n"
            "3 <type>: read mouse:0/keyboard:1/touchpad:2\r\n"
            "========================\r\n");
    ::fflush(stdin);
    return 0;
}

static int cmd_loop(hid::Hid &mouse, hid::Hid &keyboard, hid::Hid &touchpad)
{
    uint64_t t1;
    uint64_t value = -1, value2 = -1, value3 = -1, value4 = -1;
    int cmd = -1;

    int len = scanf("%d %ld %ld %ld %ld\r\n", &cmd, &value, &value2, &value3, &value4);
    if (len > 0) {
        log::info("len:%d cmd:%d value:%ld %ld %ld %ld", len, cmd, value, value2, value3, value4);
        fflush(stdin);
        t1 = time::ticks_ms();
        switch (cmd) {
        case 0:
        {
            uint8_t key = value;
            std::vector<int> buff = std::vector<int>(8);
            buff[2] = key;
            keyboard.write(buff);

            time::sleep_ms(50);
            buff = std::vector<int>(8);
            keyboard.write(buff);
            break;
        }
        case 1:
        {
            uint8_t press = value;
            int8_t offset_x = value2;
            int8_t offset_y = value3;
            int8_t wheel_movement = value4;
            std::vector<int> buff = std::vector<int>(4);
            buff[0] = press;
            buff[1] = offset_x;
            buff[2] = offset_y;
            buff[3] = wheel_movement;
            mouse.write(buff);
            break;
        }
        case 2:
        {
            uint8_t press = value;
            uint16_t offset_x = value2;
            uint16_t offset_y = value3;
            int8_t wheel_movement = value4;
            std::vector<int> buff = std::vector<int>(6);
            buff[0] = press;
            buff[1] = offset_x & 0xff;
            buff[2] = (offset_x >> 8) & 0xff;
            buff[3] = offset_y & 0xff;
            buff[4] = (offset_y >> 8) & 0xff;
            buff[5] = wheel_movement;
            touchpad.write(buff);
            break;
        }
        default:printf("Find not cmd!\r\n"); break;
        }

        log::info("cmd use %ld ms\r\n", time::ticks_ms() - t1);
        fflush(stdin);
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


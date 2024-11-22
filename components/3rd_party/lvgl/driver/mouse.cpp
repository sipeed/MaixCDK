/**
 * @file mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mouse.h"
#include "maix_lvgl.hpp"
#include "maix_basic.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "monitor.h"

/*********************
 *      DEFINES
 *********************/
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#if CONFIG_LVGL_USE_MOUSE

static int mouse_fd = -1;
// static lv_obj_t * mouse_cursor = nullptr;

bool is_usb_mouse(const std::string& device) {
    std::string command = "udevadm info --name=" + device + " 2>/dev/null";
    FILE* pipe = ::popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }

    char buffer[128];
    std::ostringstream result;
    while (::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result << buffer;
    }

    ::pclose(pipe);

    std::string output = result.str();
    if (output.find("ID_INPUT_MOUSE=1") != std::string::npos) {
        maix::log::info("found usb mouse: %s", device.c_str());
        return true;
    }
    return false;
}

std::string find_usb_mice() {
    maix::log::info("%s, %d", __PRETTY_FUNCTION__, __LINE__);
    DIR* dir;
    struct dirent* ent;

    if ((dir = ::opendir("/dev/input")) == nullptr) {
        maix::log::error("Could not open /dev/input");
        return "";
    }

    int cnt = 0;
    while ((ent = readdir(dir)) != nullptr) {
        if (::strncmp(ent->d_name, "event", 5) != 0 && strcmp(ent->d_name, "mice") != 0)
            continue;
        std::string device_path = "/dev/input/" + std::string(ent->d_name);
        // maix::log::info("device: %s", device_path.c_str());
        if (is_usb_mouse(device_path))
            return device_path;
        ++cnt;
    }
    closedir(dir);

    maix::log::info("device cnt: %d", cnt);

    return "";
}

static void linux_mouse_init(const std::string& device)
{
    maix::log::info("linux mouse init");
    mouse_fd = ::open(device.c_str(), O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) {
        maix::log::error("Unable to open mouse device: %s, use touchscreen", device.c_str());
        return;
    }
}

static void linux_mouse_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    (void) indev_drv; // Unused

    int w, h;
    monitor_rect(&w, &h);

    static int x = 0, y = 0;
    static bool pressed = false;

    struct input_event ie;
    ssize_t n = read(mouse_fd, &ie, sizeof(struct input_event));

    if (n != (ssize_t)sizeof(struct input_event)) {
        data->point.x = (lv_coord_t)x;
        data->point.y = (lv_coord_t)y;
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->continue_reading = false;
        return;
    }

    if (ie.type == EV_REL) {
        if (ie.code == REL_X) {
            x += ie.value;
            x = (x<0) ? 0 : x;
            x = (x>w) ? w : x;
        } else if (ie.code == REL_Y) {
            y += ie.value;
            y = (y<0) ? 0 : y;
            y = (y>h) ? h : y;
        }
        // } else if (ie.code == REL_WHEEL) {
        //     const int wheel_data = ie.value * (h/10);
        //     lv_obj_t* child = lv_obj_get_child(lv_scr_act(), 0);
        //     lv_obj_scroll_by(child, 0, wheel_data, LV_ANIM_ON);
        // }
    } else if (ie.type == EV_KEY) {
        if (ie.code == BTN_LEFT){
            pressed = (ie.value == 1);
        }
    }

    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->continue_reading = true;

    // maix::log::info("x:%d, y:%d, s: %u", x, y, pressed);
}

static void touch_screen_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

    static int x, y;
    static bool pressed, continue_reading;
    if(maix::maix_touchscreen->read0(x, y, pressed) == maix::err::ERR_NOT_READY) {
        data->point.x = (lv_coord_t)x;
        data->point.y = (lv_coord_t)y;
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        return;
    }
    continue_reading = maix::maix_touchscreen->available();

    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->continue_reading = continue_reading;
}

/**
 * Initialize the mouse
 */
MouseInputDevice mouse_init(lv_indev_t * indev_drv)
{
    // maix::thread::Thread t = maix::thread::Thread([](void *args){
    //     lv_indev_t *indev_drv = (lv_indev_t *)args;
    //     while (!maix::app::need_exit())
    //     {
    //         maix::maix_touchscreen->available(-1);
    //         printf("mouse thread read\n");
    //         // lv_indev_read(indev_drv);
    //     }
    // }, indev_drv);
    // t.detach();
    const auto usb_mouse_device = find_usb_mice();
    if (usb_mouse_device.empty()) {
        maix::log::info("type: touchscreen");
        return MouseInputDevice::TOUCHSCREEN;
    }
    maix::log::info("type: usb mouse");
    linux_mouse_init(usb_mouse_device);
    return MouseInputDevice::USB_MOUSE;
}

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 */
void mouse_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    if (mouse_fd > 0)
        linux_mouse_read(indev_drv, data);
    else
        touch_screen_read(indev_drv, data);
}
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/

// #endif

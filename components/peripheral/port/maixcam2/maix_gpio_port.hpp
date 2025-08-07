#pragma once

#include <string>

#define HAVE_GPIO_STATE_LED 1
#define GPIO_STATE_LED_CHIP_ID 0  // GPIO0_A6
#define GPIO_STATE_LED_OFFSET 6   // GPIO0_A6
#define GPIO_STATE_LED_PATH "/sys/class/leds/sys-heartbeat"


namespace maix::peripheral::gpio
{
    bool maix_gpio_port_parse_pin(const std::string& pin_str, int& group, int& index);
}


#pragma once

#include <string>

#define HAVE_GPIO_STATE_LED 0
#define GPIO_STATE_LED_CHIP_ID 1  // GPIOB6 --> 1, 6
#define GPIO_STATE_LED_OFFSET 6   // GPIOB6 --> 1, 6
#define GPIO_STATE_LED_PATH "/sys/class/leds/led-user"


namespace maix::peripheral::gpio
{
    bool maix_gpio_port_parse_pin(const std::string& pin_str, int& group, int& index);
}



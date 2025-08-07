/**
 * @author neucrack@sipeed, iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.5.13: update this file.
 *         2025.8.7: add HAVE_GPIO_STATE_LED definition.
 */

#include "maix_gpio.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/gpio.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <errno.h>
#include "maix_gpio_port.hpp"


namespace maix::peripheral::gpio
{

#if HAVE_GPIO_STATE_LED
	static int led_init(int default_value)
	{
		int fd;
		const char *brightness_path = GPIO_STATE_LED_PATH "/brightness";
		const char *trigger_path = GPIO_STATE_LED_PATH "/trigger";
		char value_str[2] = {default_value == 0 ? '0' : '1', '\0'};
		char none_str[] = "none";

		// Step 1: Open brightness file and write default value
		fd = open(brightness_path, O_RDWR);
		if (fd < 0)
		{
			perror("Failed to open brightness file");
			return -1;
		}

		if (write(fd, value_str, sizeof(value_str)) < 0)
		{
			perror("Failed to write to brightness file");
			close(fd);
			return -1;
		}

		// Step 2: Write "none" to trigger file
		int trigger_fd = open(trigger_path, O_WRONLY);
		if (trigger_fd < 0)
		{
			perror("Failed to open trigger file");
			close(fd);
			return -1;
		}

		if (write(trigger_fd, none_str, strlen(none_str)) < 0)
		{
			perror("Failed to write to trigger file");
			close(fd);
			close(trigger_fd);
			return -1;
		}

		close(trigger_fd);

		// Step 3: Return fd (keep brightness file open)
		return fd;
	}

	static int led_set(int fd, int value)
	{
		char value_str[2] = {value == 0 ? '0' : '1', '\0'};

		// Write value to the open file descriptor
		if (write(fd, value_str, sizeof(value_str)) < 0)
		{
			perror("Failed to write to brightness file descriptor");
			return -1;
		}

		return 0;
	}
	static int led_get(int fd)
	{
		const char *brightness_path = GPIO_STATE_LED_PATH "/brightness";
		char value_str[2] = {0};

		// Open the brightness file for reading
		int brightness_fd = open(brightness_path, O_RDONLY);
		if (brightness_fd < 0)
		{
			perror("Failed to open brightness file");
			return -1;
		}

		// Read the current value from the brightness file
		if (read(brightness_fd, value_str, sizeof(value_str) - 1) < 0)
		{
			perror("Failed to read from brightness file");
			close(brightness_fd);
			return -1;
		}

		// Close the brightness file
		close(brightness_fd);

		// Convert the read value to an integer (assuming '0' or '1')
		if (value_str[0] == '0')
		{
			return 0;
		}
		else if (value_str[0] == '1')
		{
			return 1;
		}
		log::error("Unexpected value read from brightness file: %s\n", value_str);
		return -1;
	}

	static int led_deinit(int fd)
	{
		const char *trigger_path = GPIO_STATE_LED_PATH "/trigger";
		const char activity_str[] = "activity";
		int trigger_fd;

		// Step 1: Close the file descriptor, ignoring any error
		close(fd);

		// Step 2: Write "activity" to the trigger file
		trigger_fd = open(trigger_path, O_WRONLY);
		if (trigger_fd < 0)
		{
			perror("Failed to open trigger file");
			return -1;
		}

		if (write(trigger_fd, activity_str, strlen(activity_str)) < 0)
		{
			perror("Failed to write to trigger file");
			close(trigger_fd);
			return -1;
		}

		close(trigger_fd);
		return 0;
	}
#endif // HAVE_GPIO_STATE_LED

	GPIO::GPIO(std::string pin, gpio::Mode mode, gpio::Pull pull)
	{
		this->_pull = pull;
		this->_mode = mode;
		this->_fd = 0;
		this->_line = 0;

		// to upper case first
		// convert B14/GPIOB14 to chip_id and offset, B can be any letter
		// if pin is number, throw not implemented error
		int chip_id = 0;
		_offset = 0;
		std::transform(pin.begin(), pin.end(), pin.begin(), ::toupper);

		// parse gpio or pin name to chip id and offset
		if(!maix_gpio_port_parse_pin(pin, chip_id, _offset))
		{
			throw err::Exception(err::ERR_ARGS, "Invalid pin format: " + pin);
		}

#if HAVE_GPIO_STATE_LED
		// special for A14
		if (chip_id == GPIO_STATE_LED_CHIP_ID && _offset == GPIO_STATE_LED_OFFSET)
		{
			this->_fd = led_init(pull == gpio::Pull::PULL_UP ? 1 : 0);
			if (this->_fd <= 0)
			{
				throw err::Exception(err::Err::ERR_IO, "open A14 failed");
			}
			_special = true;
			return;
		}
#endif
		_special = false;
		// open gpiochip
		std::string chip_path = "/dev/gpiochip" + std::to_string(chip_id);
		int fd = open(chip_path.c_str(), O_RDWR);
		if (fd < 0)
		{
			throw err::Exception(err::Err::ERR_IO, "open " + chip_path + " failed");
		}
		// get gpio line
		struct gpiohandle_request req;
		memset(&req, 0, sizeof(req));
		req.lineoffsets[0] = _offset;
		req.lines = 1;
		if (mode == gpio::Mode::IN)
			req.flags = GPIOHANDLE_REQUEST_INPUT;
		else if (mode == gpio::Mode::OUT)
			req.flags = GPIOHANDLE_REQUEST_OUTPUT;
		else if (mode == gpio::Mode::OUT_OD)
			req.flags = GPIOHANDLE_REQUEST_OUTPUT | GPIOHANDLE_REQUEST_OPEN_DRAIN;
		req.default_values[0] = pull == gpio::Pull::PULL_UP ? 1 : 0;
		strncpy(req.consumer_label, "maix_gpio", sizeof(req.consumer_label));
		if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
		{
			close(fd);
			throw err::Exception(err::Err::ERR_IO, "get gpio line failed");
		}
		// save gpio line
		this->_fd = fd;
		this->_line = req.fd;
	}

	GPIO::~GPIO()
	{
#if HAVE_GPIO_STATE_LED
		if (_special)
		{
			led_deinit(_fd);
			return;
		}
#endif
		if (this->_line > 0)
			close(this->_line);
		if (this->_fd > 0)
			close(this->_fd);
	}

	int GPIO::value(int value)
	{
#if HAVE_GPIO_STATE_LED
		if (_special)
		{
			if (value >= 0)
				led_set(_fd, value);
			else
				return led_get(_fd);
			return value;
		}
#endif
		struct gpiohandle_data data;
		memset(&data, 0, sizeof(data));
		if (value >= 0)
		{
			data.values[0] = value;
			if (ioctl(this->_line, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
			{
				return (int)(-err::Err::ERR_IO);
			}
		}
		memset(&data, 0, sizeof(data));
		if (ioctl(this->_line, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
		{
			return (int)(-err::Err::ERR_IO);
		}
		return data.values[0];
	}

	void GPIO::high()
	{
		value(1);
	}

	void GPIO::low()
	{
		value(0);
	}

	void GPIO::toggle()
	{
		if (value() == 0)
			high();
		else
			low();
	}

	gpio::Mode GPIO::get_mode()
	{
		return _mode;
	}

	gpio::Pull GPIO::get_pull()
	{
		return _pull;
	}

	err::Err GPIO::reset(gpio::Mode mode, gpio::Pull pull)
	{
		if (mode == _mode && pull == _pull)
			return err::ERR_NONE;

		if (this->_line > 0) {
			::close(this->_line);
			this->_line = -1;
		}

		struct gpiohandle_request req;
		::memset(&req, 0, sizeof(req));
		req.lineoffsets[0] = _offset;
		req.lines = 1;
		if (mode == gpio::Mode::IN)
			req.flags = GPIOHANDLE_REQUEST_INPUT;
		else if (mode == gpio::Mode::OUT)
			req.flags = GPIOHANDLE_REQUEST_OUTPUT;
		else if (mode == gpio::Mode::OUT_OD)
			req.flags = GPIOHANDLE_REQUEST_OUTPUT | GPIOHANDLE_REQUEST_OPEN_DRAIN;
		req.default_values[0] = (pull == gpio::Pull::PULL_UP ? 1 : 0);
		::strncpy(req.consumer_label, "maix_gpio", sizeof(req.consumer_label));
		if (::ioctl(_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
			log::error("gpio set mode err: %s", ::strerror(errno));
			return err::ERR_IO;
		}

		this->_line = req.fd;
		this->_mode = mode;
		this->_pull = pull;
		return err::ERR_NONE;
	}

}; // namespace maix::peripheral::gpio

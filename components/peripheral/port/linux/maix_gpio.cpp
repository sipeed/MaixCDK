/**
 * @author neucrack@sipeed, iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.5.13: update this file.
 */


#include "maix_gpio.hpp"
#include "maix_basic.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/gpio.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <errno.h>

namespace maix::peripheral::gpio
{
	static int led_init(int default_value)
	{
		return -1;
	}

	static int led_set(int fd, int value)
	{
		return -1;
	}
	static int led_get(int fd)
	{
		return -1;
	}

	static int led_deinit(int fd)
	{
		return -1;
	}

	GPIO::GPIO(std::string pin, gpio::Mode mode, gpio::Pull pull)
	{
		this->_mode = mode;
		this->_pull = pull;
		this->_fd = 0;
		this->_line = 0;

		// to upper case first
		// convert B14/GPIOB14 to chip_id and offset, B can be any letter
		// if pin is number, throw not implemented error
		int chip_id = 0;
		int offset = 0;
		std::transform(pin.begin(), pin.end(), pin.begin(), ::toupper);
		if (pin.find("GPIO") != std::string::npos)
		{
			pin = pin.substr(4);
		}
		if (pin[0] >= 'A' && pin[0] <= 'Z')
		{
			chip_id = pin[0] - 'A';
			try
			{
				offset = std::stoi(pin.substr(1));
			}
			catch (const std::exception &e)
			{
				throw err::Exception(err::Err::ERR_NOT_IMPL, "pin format error");
			}
		}
		else
		{
			throw err::Exception(err::Err::ERR_NOT_IMPL, "GPIO pin only number not implemented in this platform");
		}
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
		req.lineoffsets[0] = offset;
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
		if (_special)
		{
			led_deinit(_fd);
			return;
		}
		if (this->_line > 0)
			close(this->_line);
		if (this->_fd > 0)
			close(this->_fd);
	}

	int GPIO::value(int value)
	{
		if (_special)
		{
			if (value >= 0)
				led_set(_fd, value);
			else
				return led_get(_fd);
			return value;
		}

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

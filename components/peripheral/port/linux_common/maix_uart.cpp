/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_uart.hpp"
#include "maix_log.hpp"
#include "maix_time.hpp"
#include "maix_fs.hpp"
#include "maix_thread.hpp"
#include "maix_app.hpp"
#include <linux/types.h>
#include <linux/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/mman.h>

namespace maix::peripheral::uart
{
	static UART* _comm_uart = nullptr;
	static std::function<void(uart::UART*)> _comm_callback = nullptr;
	static int _get_baudrate(int baud)
	{
		switch (baud)
		{
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;
		case 460800:
			return B460800;
		case 500000:
			return B500000;
		case 576000:
			return B576000;
		case 921600:
			return B921600;
#ifdef B1000000
		case 1000000:
			return B1000000;
#endif
#ifdef B1152000
		case 1152000:
			return B1152000;
#endif
#ifdef B1500000
		case 1500000:
			return B1500000;
#endif
#ifdef B2000000
		case 2000000:
			return B2000000;
#endif
#ifdef B2500000
		case 2500000:
			return B2500000;
#endif
#ifdef B3000000
		case 3000000:
			return B3000000;
#endif
#ifdef B3500000
		case 3500000:
			return B3500000;
#endif
#ifdef B4000000
		case 4000000:
			return B4000000;
#endif
		default:
			return -1;
		}
	}

	static int _uart_init(const char *id, int baudrate, int data_bits, int parity, int stop_bits, int flow_control)
	{
		int fd;
		struct termios opt;

		fd = ::open(id, O_RDWR | O_NONBLOCK | O_NOCTTY);
		if (fd < 0)
		{
			log::error("open %s failed\r\n", id);
			return -1;
		}

		int baud_actual = _get_baudrate(baudrate);
		if (baud_actual < 0)
		{
			log::error("uart baud %d rate not support\n", baudrate);
			::close(fd);
			return -1;
		}

		memset(&opt, 0, sizeof(opt));
		opt.c_cflag |= CLOCAL | CREAD;
		opt.c_cflag |= baud_actual;

		switch (data_bits)
		{
		case uart::BITS_5:
			opt.c_cflag |= CS5;
			break;
		case uart::BITS_6:
			opt.c_cflag |= CS6;
			break;
		case uart::BITS_7:
			opt.c_cflag |= CS7;
			break;
		case uart::BITS_8:
			opt.c_cflag |= CS8;
			break;
		default:
			log::error("uart data bits error\r\n");
			::close(fd);
			return -1;
		}

		switch (parity)
		{
		case uart::PARITY_NONE:
			opt.c_iflag &= ~INPCK;
			opt.c_cflag &= ~PARENB;
			break;
		case uart::PARITY_ODD:
			opt.c_iflag |= (INPCK | ISTRIP);
			opt.c_cflag |= (PARODD | PARENB);
			break;
		case uart::PARITY_EVEN:
			opt.c_iflag |= (INPCK | ISTRIP);
			opt.c_cflag |= PARENB;
			opt.c_cflag &= ~PARODD;
			break;
		default:
			log::error("uart parity error\r\n");
			::close(fd);
			return -1;
		}

		switch (stop_bits)
		{
		case STOP_1:
			opt.c_cflag &= ~CSTOPB;
			break;
		case STOP_2:
			opt.c_cflag |= CSTOPB;
			break;
		default:
			log::error("uart stop bits error\r\n");
			::close(fd);
			return -1;
		}

		switch (flow_control)
		{
		case uart::FLOW_CTRL_NONE:
			opt.c_cflag &= ~CRTSCTS;
			break;
		case uart::FLOW_CTRL_HW:
			opt.c_cflag |= CRTSCTS;
			break;
		default:
			log::error("uart flow control error\r\n");
			::close(fd);
			return -1;
		}

		opt.c_cc[VMIN] = 1;
		opt.c_cc[VTIME] = 0;

		tcflush(fd, TCIFLUSH);
		if (0 != tcsetattr(fd, TCSANOW, &opt))
		{
			log::error("uart set attr failed\r\n");
			::close(fd);
			return -1;
		}

		return fd;
	}

	static int _uart_deinit(int fd)
	{
		if (fd > 2)
		{
			close(fd);
		}
		return 0;
	}

	static int _uart_read(int fd, void *buff, int len)
	{
		len = read(fd, buff, len);
		return len;
	}

	static int _uart_write(int fd, const void *buff, int len)
	{
		ssize_t bytes_written = write(fd, buff, len);
		if (bytes_written < 0) {
			log::error("uart write failed: %d", bytes_written);
			return bytes_written;
		}

		// wait for data transmit completion
		// if (tcdrain(fd) != 0) {
		// 	log::error("uart wait write failed: %d", bytes_written);
		// 	return -1;
		// }
		return bytes_written;
	}

	UART::UART(const std::string &port, int baudrate, uart::BITS databits,
			   uart::PARITY parity, uart::STOP stopbits,
			   uart::FLOW_CTRL flow_ctrl)
	{
		_fd = -1;
		_uart_port = port;
		_baudrate = baudrate;
		_databits = databits;
		_parity = parity;
		_stopbits = stopbits;
		_flow_ctrl = flow_ctrl;
		_read_thread = nullptr;
		if (!port.empty())
		{
			err::Err e = this->open();
			if (e != err::ERR_NONE)
			{
				throw err::Exception(e, "open uart " + _uart_port + " failed");
			}
		}
	}

	UART::~UART()
	{
		this->close();
	}

    static int set_pinmux(uint64_t addr, uint32_t value)
    {
// 假设我们的系统页大小为4KB
#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)
        int fd;
        void *map_base, *virt_addr;

        /* 打开 /dev/mem 文件 */
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        {
            perror("Error opening /dev/mem");
            return -1;
        }

        /* 映射需要访问的物理内存页到进程空间 */
        map_base = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~PAGE_MASK);
        if (map_base == (void *)-1)
        {
            perror("Error mapping memory");
            close(fd);
            return -1;
        }

        /* 计算目标寄存器的虚拟地址 */
        virt_addr = (char *)map_base + (addr & PAGE_MASK);

        /* 写入值到目标寄存器 */
        *((uint32_t *)virt_addr) = value;

        /* 取消映射并关闭文件描述符 */
        if (munmap(map_base, PAGE_SIZE) == -1)
        {
            perror("Error unmapping memory");
        }

        close(fd);

        return 0;
    }

	err::Err UART::open()
	{
		if (_fd > 0)
		{
			return err::ERR_NONE;
		}
		if (_uart_port.empty())
		{
			log::error("uart port is not set\r\n");
			return err::ERR_ARGS;
		}

		// release comm protocol uart if exits
		if(_comm_uart && fs::realpath(_comm_uart->get_port()) == fs::realpath(_uart_port))
		{
			_comm_callback(_comm_uart);
			_comm_uart = nullptr;
		}

		// initialize uart
		_fd = _uart_init(_uart_port.c_str(), _baudrate, _databits, _parity, _stopbits, _flow_ctrl);
		if (_fd < 0)
		{
			log::error("open uart %s failed\r\n", _uart_port.c_str());
			return err::ERR_IO;
		}

		// FIXME: Check the pins of tx
		if (_uart_port == "/dev/ttyS0" || _uart_port == "/dev/serial0") {
			set_pinmux(0x0300190c, 0x84);		// set A16 drive capability
		}

		// self.oneByteTime = 1 / (self.com.baudrate / (self.com.bytesize + 2 + self.com.stopbits)) # 1 byte use time
		_one_byte_time_us = 1000000.0 / (_baudrate / (_databits + 2 + (_stopbits == STOP_1_5 ? 1.5 : _stopbits)));
		log::debug("one byte time: %d", _one_byte_time_us);
		return err::ERR_NONE;
	}

	bool UART::is_open()
	{
		return _fd > 0;
	}

	err::Err UART::close()
	{
		if (_fd <= 0)
			return err::ERR_NONE;
		int ret = _uart_deinit(_fd);
		_fd = -1;
		if(_read_thread)
		{
			_read_thread_need_exit = true;
			uint64_t t = time::ticks_ms();
			while(!_read_thread_exit)
			{
				time::sleep_ms(10);
				if(time::ticks_ms() - t > 5000)
				{
					log::error("waiting uart read thread exit");
					t = time::ticks_ms();
				}
			}
			delete _read_thread;
			_read_thread = nullptr;
		}
		if (ret != 0)
		{
			log::error("uart close failed\r\n");
			return err::ERR_IO;
		}
		return err::ERR_NONE;
	}

	void UART::set_received_callback(std::function<void(uart::UART&, Bytes&)> callback)
	{
		if(!_read_thread)
		{
			_read_thread_need_exit = false;
			_read_thread_exit = false;
			_read_thread = new thread::Thread([callback](void *args){
				UART *uart = (UART*)args;
				while(!app::need_exit() && !uart->_read_thread_need_exit)
				{
					Bytes *data = NULL;
					try
					{
						data = uart->read(-1, -1);
					}
					catch(const err::Exception& e)
					{
						log::error("read file failed");
						break;
					}
					if(!data)
					{
						log::error("uart read data is null");
						break;
					}
					callback(*uart, *data);
					delete data;
				}
				uart->_read_thread_exit = true;
			}, this);
			_read_thread->detach();
		}
		this->callback = callback;
	}

	int UART::write(const uint8_t *buff, int len)
	{
		if (!is_open())
			return err::ERR_NOT_OPEN;
		int remain_len = len;
		while (remain_len != 0)
		{
			const uint8_t *p = buff + len - remain_len;
			int len = _uart_write(_fd, p, remain_len);
			if (len < 0)
			{
				log::error("uart write failed, fd: %d, ret: %d\r\n", _fd, len);
				return err::ERR_IO;
			}
			else if (len == 0)
			{
				break;
			}
			remain_len -= len;
			if (remain_len <= 0)
			{
				break;
			}
		}
		return len - remain_len;
	}

	int UART::write(const char *buff, int len)
	{
		if (len < 0)
		{
			len = strlen(buff);
		}
		return write((const uint8_t *)buff, len);
	}

	int UART::write(const std::string &str)
	{
		return write(str.c_str(), str.size());
	}

	int UART::write_str(const std::string &str)
	{
		return write(str.c_str(), str.size());
	}

	int UART::write(Bytes &data)
	{
		return write(data.data, data.size());
	}

	int UART::available(int timeout)
	{
		if (!is_open())
			return -err::ERR_NOT_OPEN;
		// TIOCINQ
		int bytes = 0;
		if (timeout == 0)
		{
			if (ioctl(_fd, TIOCINQ, &bytes) < 0)
			{
				log::error("uart ioctl failed: %d\r\n", errno);
				return -err::ERR_IO;
			}
			return bytes;
		}
		timespec timeout_ts;
		timespec *p_timout = NULL;
		if (timeout > 0)
		{
			timeout_ts.tv_sec = timeout / 1000;
			timeout_ts.tv_nsec = (timeout % 1000) * 1000000;
			p_timout = &timeout_ts;
		}
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(_fd, &rfds);
		int ret = pselect(_fd + 1, &rfds, NULL, NULL, p_timout, NULL);
		if (ret < 0)
		{
			if (errno == EINTR)
				return -err::ERR_CANCEL;
			else
			{
				log::error("uart select failed: %d\r\n", errno);
				return -err::ERR_IO;
			}
		}
		else if (ret == 0)
		{
			return 0;
		}
		if (!FD_ISSET(_fd, &rfds)) // should not happen
		{
			throw err::Exception(err::ERR_IO, "uart fd not in select fds");
		}
		if (ioctl(_fd, TIOCINQ, &bytes) < 0)
		{
			log::error("uart ioctl failed: %d\r\n", errno);
			return -err::ERR_IO;
		}
		return bytes;
	}

	int UART::read(uint8_t *buff, int buff_len, int recv_len, int timeout)
	{
		if (!is_open())
			return -err::ERR_NOT_OPEN;
		uint64_t t = time::ticks_ms();
		int read_len = 0;
		if (recv_len == -1)
		{
			while (1)
			{
				int left_len = buff_len - read_len;
				if(left_len <= 0)
					break;
				int len = _uart_read(_fd, buff + read_len, left_len);
				if(len < 0)
				{
					if(errno != EAGAIN)
					{
						log::error("uart read failed: %d, %d\r\n", len, errno);
						return -err::ERR_IO;
					}
				}
				else
				{
					read_len += len;
				}
				if (timeout > 0 && (time::ticks_ms() - t) < (uint64_t)timeout)
					time::sleep_ms(1);
				else if (available(0) > 0)
					continue;
				else
				{
					int wait_time = _one_byte_time_us * 30; // system maybe use some time
					time::sleep_us(wait_time > 50000 ? 50000: wait_time);
					if (available(0) > 0)
						continue;
					if (timeout < 0 && read_len == 0) // block read
					{
						available(-1);
						continue;
					}
					break;
				}
			}
			return read_len;
		}
		if (recv_len > 0)
		{
			int t_last = 0;
			bool _available = false;
			while(1)
			{
				if(timeout > 0)
				{
					t_last = timeout - (time::ticks_ms() - t);
					if(t_last < 0)
						t_last = 0;
					_available = available(t_last);
				}
				else
					_available = available(timeout);
				if (_available)
				{
					int len = _uart_read(_fd, buff + read_len, recv_len - read_len);
					if (len < 0)
					{
						log::error("uart read failed\r\n");
						return -err::ERR_IO;
					}
					read_len += len;
				}
				if(read_len >= recv_len || (timeout > 0 && (time::ticks_ms() - t) > (uint64_t)timeout))
					break;
				int wait_time = _one_byte_time_us * 30; // system maybe use some time
				time::sleep_us(wait_time > 50000 ? 50000: wait_time);
				if(available(0) > 0)
					continue;
				if(timeout == 0)
					break;
			}
			return read_len;
		}
		throw err::Exception(err::ERR_ARGS, "recv_len must be -1 or > 0");
	}

	Bytes *UART::read(int len, int timeout)
	{
		int read_len = 0;
		int buff_len = len > 0 ? len : 512;
		Bytes *data = new Bytes(NULL, buff_len);
		int received = 0;
		uint64_t t = time::ticks_ms();
		int t2;
		while(1)
		{
			t2 = (int)(time::ticks_ms() - t);
			if(timeout > 0 && t2 >= timeout)
				break;
			read_len = read(data->data + received, buff_len - received, len > 0 ? len - received : len, timeout > 0 ? t2 : timeout);
			if (read_len < 0)
			{
				delete data;
				throw err::Exception(err::Err(-read_len), "read failed");
			}
			received += read_len;
			data->data_len = received;
			if(len > 0 && received == len)
			{
				break;
			}
			if(timeout > 0 && (int)(time::ticks_ms() - t) >= timeout)
				break;
			else if(received == buff_len)
			{
				buff_len = data->data_len * 2;
				Bytes *data2 = new Bytes(data->data, buff_len, true, true);
				delete data;
				data = data2;
				continue;
			}
			break;
		}
		return data;
	}

	Bytes *UART::readline(int timeout)
	{
		int read_len = 0;
		int buff_len = 128;
		if(timeout == 0)
		{
			throw err::Exception(err::ERR_ARGS, "timeout must be -1 or > 0");
		}
		Bytes *data = new Bytes(NULL, buff_len);
		uint64_t t = time::ticks_ms();
		do
		{
			uint8_t chr;
			int len = this->read(&chr, 1, 1, (timeout < 0 ? -1 : (int)(time::ticks_ms() - t)));
			if(len < 0)
			{
				log::error("uart read failed: %d\n", - len);
				break;
			}
			else if(len > 0)
			{
				data->data[read_len] = chr;
				read_len += len;
				if(chr == '\n')
					break;
			}
			else
			{
				time::sleep_ms(1);
			}
		} while (timeout < 0 || (timeout > 0 && (time::ticks_ms() - t) < (uint64_t)timeout));

		data->data_len = read_len;
		return data;
	}

	std::vector<std::string> list_devices()
	{
		std::vector<std::string> ports;
		// find /dev/ttyS* /dev/ttyUSB* /dev/ttyACM* /dev/ttyAMA* /dev/rfcomm* /dev/ttyAP*
		const std::string names[] = {"ttyS", "ttyUSB", "ttyACM", "ttyAMA*", "rfcomm", "ttyAP"};
		std::vector<std::string> *devs = fs::listdir("/dev");
		for (const std::string &dev : *devs)
		{
			// /dev/serial* is device
			if (dev.find("serial") != std::string::npos)
			{
				ports.push_back("/dev/" + dev);
				continue;
			}
			for (const std::string &name : names)
			{
				std::string device_path = "";
				std::string subsystem = "";
				if (dev.find(name) != std::string::npos)
				{
					// /sys/class/tty/{dev}/device exists
					if (fs::exists("/sys/class/tty/" + dev + "/device"))
					{
						device_path = fs::realpath("/sys/class/tty/" + dev + "/device");
						subsystem = fs::basename(fs::realpath(device_path + "/subsystem"));
					}
					if (subsystem != "platform")
						ports.push_back("/dev/" + dev);
				}
			}
		}
		delete devs;
		return ports;
	}

	err::Err register_comm_callback(uart::UART *obj, std::function<void(uart::UART*)> callback)
	{
		_comm_uart = obj;
		_comm_callback = callback;
		asm volatile ("" : : "g" (_comm_uart));
		asm volatile ("" : : "g" (_comm_callback));
		return err::Err::ERR_NONE;
	}
}; // namespace maix::peripheral::uart

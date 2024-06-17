/**
 * @author iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_spi.hpp"
#include "maix_log.hpp"
#include "cstdio"
#include <linux/types.h>
#include <linux/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/spi/spidev.h>
#include <errno.h>

namespace maix::peripheral::spi
{
    static inline void __close_fd(int* fd)
    {
        ::close(*fd);
        *fd = -1;
    }

    static inline std::string __get_errno_msg(void)
    {
        int errsv = errno;
        char buffer[64];
        ::strerror_r(errsv, buffer, sizeof(buffer));
        return std::string(buffer);
    }

    static inline err::Exception __ioctl_error(int* fd)
    {
        __close_fd(fd);
        return err::Exception(err::ERR_RUNTIME, __get_errno_msg());
    }

    static inline int __spi_transfer(int fd, unsigned char used_soft_cs,
                                        unsigned int speed_hz, unsigned char bits,
                                        const uint8_t *txbuf, uint8_t *rxbuf, size_t len) {
        struct spi_ioc_transfer transfer;

        ::memset(&transfer, 0, sizeof(struct spi_ioc_transfer));
        transfer.tx_buf = (uintptr_t)txbuf;
        transfer.rx_buf = (uintptr_t)rxbuf;
        transfer.len = len;
        transfer.delay_usecs = 0;
        transfer.speed_hz = speed_hz;
        transfer.bits_per_word = bits;
        transfer.cs_change = used_soft_cs;
        // log::info("[__spi_transfer] cs_change = %d", transfer.cs_change);

        if (::ioctl(fd, SPI_IOC_MESSAGE(1), &transfer) < 1)
            return -1;

        return 0;
    }

    SPI::SPI(int id, spi::Mode mode, int freq, int polarity, int phase,
            int bits, unsigned char cs_enable, bool soft_cs, std::string cs)
                :_used_soft_cs(soft_cs), _cs_enable(cs_enable), _bits(bits), _freq(freq)
    {
        {
            if (mode == spi::Mode::SLAVE)
                throw err::Exception(err::ERR_ARGS, "spi::Mode::SLAVE mode not implemented");

            uint32_t invalid_arguments = (
                (mode & ~0x01) | (polarity & ~0x01) | (phase & ~0x01));
            if (invalid_arguments)
                throw err::Exception(err::ERR_ARGS, "spi args error");

            if (soft_cs) {
                auto pull = cs_enable ? gpio::Pull::PULL_DOWN : gpio::Pull::PULL_UP;
                _cs = new gpio::GPIO(cs, gpio::Mode::OUT, pull);
                if (nullptr == _cs)
                    throw err::Exception(err::ERR_NO_MEM, "cannot alloc maix::peripheral::gpio");
            } else if (cs_enable != 0) {
                throw err::Exception(err::ERR_ARGS, "spi:switching the effective level of the default CS "
                                "is not supported for the time being, if you need a CS pin with an "
                                "effective level of high, please use the parameter 'cs' to select a "
                                "GPIO as the CS pin.");
            }

            std::string dev_name("/dev/spidev"+std::to_string(id)+".0");
            log::debug("try to open %s...", dev_name.c_str());
            _fd = ::open(dev_name.c_str(), O_RDWR);
            if (_fd < 0)
                throw err::Exception(err::ERR_IO, "cannot open "+dev_name);
        }

        uint32_t d8 = ((polarity << 1)|(phase));
        if (0 != cs_enable && !_used_soft_cs) {
            d8 |= SPI_CS_HIGH;
        }

        if (::ioctl(_fd, SPI_IOC_WR_MODE, &d8) < 0)
            throw __ioctl_error(&_fd);

        uint32_t rmode = d8;
        if (::ioctl(_fd, SPI_IOC_RD_MODE, &rmode) < 0)
            throw __ioctl_error(&_fd);

        if (d8 != rmode) {
            // log::error("SPI Write Mode: 0x%x", d8);
            // log::error("SPI Read Mode: 0x%x", rmode);
            // throw err::Exception(err::ERR_WRITE, "SPI write mode failed");
            log::debug("SPI write MODE: 0x%x, SPI read MODE: 0x%x", d8, rmode);
        }

        if (::ioctl(_fd, SPI_IOC_WR_MAX_SPEED_HZ, &freq) < 0)
            throw __ioctl_error(&_fd);

        if (::ioctl(_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            throw __ioctl_error(&_fd);
    }

    SPI::~SPI()
    {
        if (_used_soft_cs) {
            delete _cs;
        }
        __close_fd(&_fd);
    }

    Bytes *SPI::read(int length)
    {
        return this->write_read(nullptr, length);
    }

    std::vector<unsigned char> SPI::read(unsigned int length)
    {
        return this->write_read(std::vector<unsigned char>(), length);
    }

    int SPI::write(std::vector<unsigned char> data)
    {
        this->write_read(data, data.size());
        return data.size();
    }

    int SPI::write(Bytes *data)
    {
        this->write_read(data, data->data_len);
        return data->data_len;
    }

    std::vector<unsigned char> SPI::write_read(std::vector<unsigned char> data, int read_len)
    {
        if (read_len <= 0)
            return std::vector<unsigned char>();

        auto w_size = data.size();
        size_t len = std::max(w_size, static_cast<size_t>(read_len));
        if (len != w_size) {
            data.resize(len, 0x00);
        }

        auto wbuf = new unsigned char[len];
        if (nullptr == wbuf) {
            log::error("cannot alloc %d bytes for SPI::write_read", len);
            return std::vector<unsigned char>();
        }
        ::memset(wbuf, 0x00, len);
        ::memcpy(wbuf, data.data(), data.size());

        auto rbuf = new unsigned char[len];
        if (nullptr == rbuf) {
            log::error("cannot alloc %d bytes for SPI::write_read", len);
            delete[] wbuf;
            return std::vector<unsigned char>();
        }

        if (_used_soft_cs)
            this->enable_cs(true);
        if (__spi_transfer(_fd, _used_soft_cs, _freq, _bits, wbuf, rbuf, len) < 0) {
            delete[] wbuf;
            delete[] rbuf;
            return std::vector<unsigned char>();
        }
        if (_used_soft_cs)
            this->enable_cs(false);

        auto res = std::vector<unsigned char>(rbuf, rbuf+read_len);
        delete[] wbuf;
        delete[] rbuf;
        return res;
    }

    Bytes *SPI::write_read(Bytes *data, int read_len)
    {
        if (read_len <= 0)
            return nullptr;

        unsigned long w_size = 0;
        if (nullptr != data)
            w_size = data->size();
        size_t len = std::max(w_size, static_cast<size_t>(read_len));
        auto wbuf = new unsigned char[len];
        if (wbuf == nullptr) {
            log::error("cannot alloc %d bytes for SPI::write_read", len);
            return nullptr;
        }
        ::memset(wbuf, 0x00, len);
        if (nullptr != data && data->size() > 0) {
            std::copy(data->data, data->data+data->data_len, wbuf);
        }

        auto rbuf = new unsigned char[len];
        if (rbuf == nullptr) {
            log::error("cannot alloc %d bytes for SPI::write_read", len);
            delete[] wbuf;
            return nullptr;
        }

        if (_used_soft_cs)
            this->enable_cs(true);
        if (__spi_transfer(_fd, _used_soft_cs, _freq, _bits, wbuf, rbuf, len) < 0) {
            delete[] rbuf;
            delete[] wbuf;
            return nullptr;
        }
        if (_used_soft_cs)
            this->enable_cs(false);

        auto res = new Bytes(rbuf, read_len);
        delete[] wbuf;
        delete[] rbuf;
        return res;
    }

    bool SPI::is_busy()
    {
        return false;
    }

    void SPI::enable_cs(bool enable)
    {
        if (enable)
            _cs->value(_cs_enable);
        else
            _cs->value(!_cs_enable);
    }
}; // namespace maix

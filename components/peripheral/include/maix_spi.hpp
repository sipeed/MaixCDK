/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_basic.hpp"
#include "maix_gpio.hpp"
#include "vector"

namespace maix::peripheral::spi
{
    /**
     * SPI mode enum
     * @maixpy maix.peripheral.spi.Mode
     */
    enum Mode
    {
        MASTER = 0x0, // spi master mode
        SLAVE = 0x1,  // spi slave mode
    };

    /**
     * Peripheral spi class
     * @maixpy maix.peripheral.spi.SPI
     */
    class SPI
    {
    public:
        /**
         * @brief SPI constructor
         *
         * @param[in] id spi bus id, int type
         * @param[in] mode mode of spi, spi.Mode type, spi.Mode.MASTER or spi.Mode.SLAVE.
         * @param[in] freq freq of spi, int type
         * @param[in] polarity polarity of spi, 0 means idle level of clock is low, 1 means high, int type, default is 0.
         * @param[in] phase phase of spi, 0 means data is captured on the first edge of the SPI clock cycle, 1 means second, int type, default is 0.
         * @param[in] bits bits of spi, int type, default is 8.
         * @param[in] cs_enable cs pin active level, default is 0(low)
         * @param[in] soft_cs not use hardware cs, bool type, if set true, you can operate cs pin use gpio manually.
         * @param[in] cs soft cs pin number, std::string type, default is "GPIOA19", if SPI support multi hardware cs, you can set it to other value.
         * @maixpy maix.peripheral.spi.SPI.__init__
         */
        SPI(int id, spi::Mode mode, int freq, int polarity = 0, int phase = 0,
            int bits = 8, unsigned char cs_enable=0, bool soft_cs = false, std::string cs = "GPIOA19");
        ~SPI();

        /**
         * @brief read data from spi
         * @param[in] length read length, int type
         * @return bytes data, Bytes type in C++, bytes type in MaixPy. You need to delete it manually after use in C++.
         * @maixpy maix.peripheral.spi.SPI.read
         */
        Bytes *read(int length);

        /**
         * @brief read data from spi
         * @param[in] length read length, unsigned int type
         * @return bytes data, vector<unsigned char> type
         * @maixcdk maix.peripheral.spi.SPI.read
         */
        std::vector<unsigned char> read(unsigned int length);

        /**
         * @brief write data to spi
         * @param[in] data data to write, vector<unsigned char> type
         * the member range of the list is [0,255]
         * @return write length, int type, if write failed, return -err::Err code.
         * @maixcdk maix.peripheral.spi.SPI.write
         *
         */
        int write(std::vector<unsigned char> data);

        /**
         * @brief write data to spi
         * @param[in] data data to write, Bytes type in C++, bytes type in MaixPy
         * @return write length, int type, if write failed, return -err::Err code.
         * @maixpy maix.peripheral.spi.SPI.write
         */
        int write(Bytes *data);

        /**
         * @brief write data to spi and read data from spi at the same time.
         * @param[in] data data to write, vector<unsigned char> type.
         * @param[in] read_len read length, int type, should > 0.
         * @return read data, vector<unsigned char> type
         * @maixcdk maix.peripheral.spi.SPI.write_read
         */
        std::vector<unsigned char> write_read(std::vector<unsigned char> data, int read_len);

        /**
         * @brief write data to spi and read data from spi at the same time.
         * @param[in] data data to write, Bytes type in C++, bytes type in MaixPy
         * @param[in] read_len read length, int type, should > 0.
         * @return read data, Bytes type in C++, bytes type in MaixPy. You need to delete it manually after use in C++.
         * @maixpy maix.peripheral.spi.SPI.write_read
         */
        Bytes *write_read(Bytes *data, int read_len);

        /**
         * @brief get busy status of spi
         *
         * @return busy status, bool type
         * @maixpy maix.peripheral.spi.SPI.is_busy
         */
        bool is_busy();
    private:
        void enable_cs(bool enable);
    private:
        int _fd = -1;
        bool _used_soft_cs = false;
        gpio::GPIO *_cs = nullptr;
        unsigned char _cs_enable;

        int _bits;
        int _freq;
    };
}; // namespace maix::peripheral::spi

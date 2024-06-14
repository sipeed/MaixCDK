/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_basic.hpp"
#include "vector"

namespace maix::peripheral::i2c
{

    /**
     * @brief Address size enum
     * @maixpy maix.peripheral.i2c.AddrSize
     */
    enum AddrSize
    {
        SEVEN_BIT = 7,   // 7-bit address mode
        TEN_BIT   = 10   // 10-bit address mode
    };

    /**
     * @brief I2C mode enum
     * @maixpy maix.peripheral.i2c.Mode
     */
    enum Mode
    {
        MASTER = 0x00, // master mode
        SLAVE = 0x01   // slave mode
    };

    /**
     * Get supported i2c bus devices.
     * @return i2c bus devices list, int type, is the i2c bus id.
     * @maixpy maix.peripheral.i2c.list_devices
     */
    std::vector<int> list_devices();

    /**
     * Peripheral i2c class
     * @maixpy maix.peripheral.i2c.I2C
     */
    class I2C
    {
    public:
        /**
         * @brief I2C Device constructor
         *       this constructor will be export to MaixPy as _maix.example.Example.__init__
         *
         * @param[in] id i2c bus id, int type, e.g. 0, 1, 2
         * @param[in] freq i2c clock, int type, default is 100000(100kbit/s), will auto set fast mode if freq > 100000.
         * @param[in] mode mode of i2c, i2c.Mode.SLAVE or i2c.Mode.MASTER.
         * @param[in] addr_size address length of i2c, i2c.AddrSize.SEVEN_BIT or i2c.AddrSize.TEN_BIT.
         * @throw err::Exception if open i2c device failed.
         * @maixpy maix.peripheral.i2c.I2C.__init__
         */
        I2C(int id, i2c::Mode mode, int freq = 100000, i2c::AddrSize addr_size = i2c::AddrSize::SEVEN_BIT);
        ~I2C();

        /**
         * @brief scan all i2c salve address on the bus
         * @param addr If -1, only scan this addr, or scan from 0x08~0x77, default -1.
         * @return the list of i2c slave address, int list type.
         * @maixpy maix.peripheral.i2c.I2C.scan
         */
        std::vector<int> scan(int addr = -1);

        /**
         * @brief write data to i2c slave
         *
         * @param[in] addr i2c slave address, int type
         * @param[in] data data to write, vector<unsigned char> type in C++, int list in MaixPy.
         * Note: The range of value should be in [0,255].
         *
         * @return if success, return the length of written data, error occurred will return -err::Err。
         * @maixcdk maix.peripheral.i2c.I2C.writeto
         */
        int writeto(int addr, const std::vector<unsigned char> data);

        /**
         * @brief write data to i2c slave
         *
         * @param[in] addr i2c slave address, int type
         * @param[in] data data to write, bytes type.
         * Note: The range of value should be in [0,255].
         *
         * @return if success, return the length of written data, error occurred will return -err::Err.
         * @maixpy maix.peripheral.i2c.I2C.writeto
         */
        int writeto(int addr, const Bytes &data);

        /**
         * @brief write data to i2c slave
         * @param[in] addr i2c slave address, int type
         * @param[in] data data to write, uint8_t type.
         * @param[in] len data length to write, int type
         * @return if success, return the length of written data, error occurred will return -err::Err.
         * @maixcdk maix.peripheral.i2c.I2C.writeto
        */
        int writeto(int addr, const uint8_t *data, int len);

        /**
         * @brief read data from i2c slave
         *
         * @param[in] addr i2c slave address, int type
         * @param[in] len data length to read, int type
         *
         * @return the list of data read from i2c slave, bytes type, you should delete it after use in C++.
         *          If read failed, return nullptr in C++, None in MaixPy.
         * @maixpy maix.peripheral.i2c.I2C.readfrom
         */
        Bytes* readfrom(int addr, int len);

        /**
         * @brief write data to i2c slave's memory address
         * @param[in] addr i2c slave address, int type
         * @param[in] mem_addr memory address want to write, int type.
         * @param[in] data data to write, vector<unsigned char> type.
         * @param[in] mem_addr_size memory address size, default is 8.
         * @param[in] mem_addr_le memory address little endian, default is false, that is send high byte first.
         * @return data length written if success, error occurred will return -err::Err.
         * @maixcdk maix.peripheral.i2c.I2C.writeto_mem
         */
        int writeto_mem(int addr, int mem_addr, const std::vector<unsigned char> data, int mem_addr_size = 8, bool mem_addr_le = false);

        /**
         * @brief write data to i2c slave's memory address
         * @param[in] addr i2c slave address, int type
         * @param[in] mem_addr memory address want to write, int type.
         * @param[in] data data to write, bytes type.
         * @param[in] mem_addr_size memory address size, default is 8.
         * @param[in] mem_addr_le memory address little endian, default is false, that is send high byte first.
         * @return data length written if success, error occurred will return -err::Err.
         * @maixpy maix.peripheral.i2c.I2C.writeto_mem
         */
        int writeto_mem(int addr, int mem_addr, const Bytes &data, int mem_addr_size = 8, bool mem_addr_le = false);

        /**
         * @brief write data to i2c slave's memory address
         * @param[in] addr i2c slave address, int type
         * @param[in] mem_addr memory address want to write, int type.
         * @param[in] data data to write, uint8_t type.
         * @param[in] len data length to write, int type
         * @param[in] mem_addr_size memory address size, default is 8.
         * @param[in] mem_addr_le memory address little endian, default is false, that is send high byte first.
         * @return data length written if success, error occurred will return -err::Err.
         * @maixcdk maix.peripheral.i2c.I2C.writeto_mem
         */
        int writeto_mem(int addr, int mem_addr, const uint8_t *data, int len, int mem_addr_size = 8, bool mem_addr_le = false);

        /**
         * @brief read data from i2c slave
         * @param[in] addr i2c slave address, int type
         * @param[in] mem_addr memory address want to read, int type.
         * @param[in] len data length to read, int type
         * @param[in] mem_addr_size memory address size, default is 8.
         * @param[in] mem_addr_le memory address little endian, default is false, that is send high byte first.
         * @return the list of data read from i2c slave, bytes type, you should delete it after use in C++.
         *         If read failed, return nullptr in C++, None in MaixPy.
         * @maixpy maix.peripheral.i2c.I2C.readfrom_mem
         */
        Bytes* readfrom_mem(int addr, int mem_addr, int len, int mem_addr_size = 8, bool mem_addr_le = false);

    private:
        int _fd;
        int _freq;
        i2c::AddrSize _addr_size;
        i2c::Mode     _mode;
    };
} // namespace maix::peripheral::i2c
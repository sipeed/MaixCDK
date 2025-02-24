/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "vector"
#include "stdint.h"
#include "maix_comm_base.hpp"
#include "maix_type.hpp"
#include "maix_thread.hpp"
#include <functional>

/**
 * @brief maix uart peripheral driver
 * @maixpy maix.peripheral.uart
 */
namespace maix::peripheral::uart
{
    /**
     * @brief uart parity enum
     * @maixpy maix.peripheral.uart.PARITY
     */
    enum PARITY
    {
        PARITY_NONE = 0x00,  // no parity
        PARITY_ODD  = 0x01,  // odd parity
        PARITY_EVEN = 0x02,  // even parity
        PARITY_MAX
    };

    /**
     * @brief uart stop bits
     * @maixpy maix.peripheral.uart.STOP
     */
    enum STOP
    {
        STOP_1   = 0x01,  // 1 stop bit
        STOP_2   = 0x02,  // 2 stop bits
        STOP_1_5 = 0x03,  // 1.5 stop bits
        STOP_MAX
    };

    /**
     * uart stop bits
     * @maixpy maix.peripheral.uart.BITS
     */
    enum BITS
    {
        BITS_5 = 5,  // 5 data bits
        BITS_6 = 6,  // 6 data bits
        BITS_7 = 7,  // 7 data bits
        BITS_8 = 8,  // 8 data bits
        BITS_MAX
    };

    /**
     * @brief uart flow control
     * @maixpy maix.peripheral.uart.FLOW_CTRL
     */
    enum FLOW_CTRL
    {
        FLOW_CTRL_NONE = 0,  // no flow control
        FLOW_CTRL_HW   = 1,  // hardware flow control
        FLOW_CTRL_MAX
    };

    /**
     * Get supported uart ports.
     * @return uart ports list, string type.
     * @maixpy maix.peripheral.uart.list_devices
     */
    std::vector<std::string> list_devices();

    /**
     * @brief maix uart peripheral driver
     * @maixpy maix.peripheral.uart.UART
     */
    class UART : public comm::CommBase
    {
    public:
        /**
         * @brief UART constructor. You need to call open() to open the device.
         *
         * @param port uart port. string type, can get it by uart.list_devices().
         *             If empty, will not open device in constructor, default empty.
         *             if not empty, will auto open device in constructor, open fail will throw err.Exception.
         * @param baudrate baudrate of uart. int type, default 115200.
         * @param databits databits, values @see uart.DATA_BITS
         * @param parity parity, values @see uart.PARITY
         * @param stopbits stopbits, values @see uart.STOP_BITS
         * @param flow_control flow_control, values @see uart.FLOW_CTRL
         * @maixpy maix.peripheral.uart.UART.__init__
         */
        UART(const std::string &port = "", int baudrate = 115200, uart::BITS databits = uart::BITS_8,
            uart::PARITY parity = uart::PARITY_NONE, uart::STOP stopbits = uart::STOP_1,
            uart::FLOW_CTRL flow_ctrl = uart::FLOW_CTRL_NONE);

        ~UART();

        /**
         * Set port
         * @param port uart port. string type, can get it by uart.list_devices().
         * @return set port error code, err.Err type.
         * @maixpy maix.peripheral.uart.UART.set_port
         */
        err::Err set_port(const std::string &port) { _uart_port = port; return err::ERR_NONE; }

        /**
         * Get port
         * @return uart port, string type.
         * @maixpy maix.peripheral.uart.UART.get_port
         */
        std::string get_port() { return _uart_port; }

        /**
         * Set baud rate
         * @param baudrate baudrate of uart. int type, default 115200.
         * @return set baud rate error code, err.Err type.
         * @maixpy maix.peripheral.uart.UART.set_baudrate
         */
        err::Err set_baudrate(int baudrate) { _baudrate = baudrate; return err::ERR_NONE; }

        /**
         * Get baud rate
         * @return baud rate, int type.
         * @maixpy maix.peripheral.uart.UART.get_baudrate
         */
        int get_baudrate() { return _baudrate; }

        /**
         * Open uart device, before open, port must be set in constructor or by set_port().
         * If already opened, do nothing and return err.ERR_NONE.
         * @return open device error code, err.Err type.
         * @maixpy maix.peripheral.uart.UART.open
         */
        err::Err open();

        /**
         * Check if device is opened.
         * @return true if opened, false if not opened.
         * @maixpy maix.peripheral.uart.UART.is_open
        */
        bool is_open();

        /**
         * Close uart device, if already closed, do nothing and return err.ERR_NONE.
         * @return close device error code, err.Err type.
         * @maixpy maix.peripheral.uart.UART.close
         */
        err::Err close();

        /**
         * Set received callback function
         * @param callback function to call when received data
         * @maixpy maix.peripheral.uart.UART.set_received_callback
         */
        void set_received_callback(std::function<void(uart::UART&, Bytes&)> callback);

        /**
         * Send data to device
         * @param buff data buffer
         * @param len  data length need to send
         * @return sent data length, < 0 means error, value is -err.Err.
         * @maixcdk maix.peripheral.uart.UART.write
         */
        int write(const uint8_t *buff, int len);

        /**
         * Send data to device
         * @param buff data buffer
         * @param len  data length need to send, if len == -1, means buff is a string, send buff until '\0'.
         * @return sent data length, < 0 means error, value is -err.Err.
         * @maixcdk maix.peripheral.uart.UART.write
         */
        int write(const char *buff, int len = -1);

        /**
         * Send string data
         * @param str string data
         * @return sent data length, < 0 means error, value is -err.Err.
         * @maixcdk maix.peripheral.uart.UART.write
         */
        int write(const std::string &str);

        /**
         * Send string data
         * @param str string data
         * @return sent data length, < 0 means error, value is -err.Err.
         * @maixpy maix.peripheral.uart.UART.write_str
         */
        int write_str(const std::string &str);

        /**
         * @brief Send data to uart
         * @param[in] data data to send, bytes type. If you want to send str type, use str.encode() to convert.
         * @return sent length, int type, if < 0 means error, value is -err.Err.
         * @maixpy maix.peripheral.uart.UART.write
         */
        int write(Bytes &data);

        /**
         * Check if data available or wait data available.
         * @param timeout unit ms, timeout to wait data, default 0.
         *                  0 means check data available and return immediately,
         *                > 0 means wait until data available or timeout.
         *                - 1 means wait until data available.
         * @return available data number, 0 if timeout or no data, <0 if error, value is -err.Err, can be err::ERR_IO， err::ERR_CANCEL, err::ERR_NOT_OPEN.
         * @throw err.Exception if fatal error.
         * @maixpy maix.peripheral.uart.UART.available
         */
        int available(int timeout = 0);

        /**
         * Receive data
         * @param buff data buffer to store received data
         * @param buff_len data buffer length
         * @param recv_len max data length want to receive, default -1.
         *            -1 means read data in uart receive buffer.
         *            >0 means read recv_len data want to receive.
         *            other values is invalid.
         * @param timeout unit ms, timeout to receive data, default 0.
         *                 0 means read data in uart receive buffer and return immediately,
         *                -1 means block until read recv_len data,
         *                >0 means block until read recv_len data or timeout.
         * @return received data length, < 0 means error, value is -err.Err.
         * @maixcdk maix.peripheral.uart.UART.read
         */
        int read(uint8_t *buff, int buff_len, int recv_len = -1, int timeout = 0);

        /**
         * @brief Recv data from uart
         * @param len max data length want to receive, default -1.
         *            -1 means read data in uart receive buffer.
         *            >0 means read len data want to receive.
         *            other values is invalid.
         * @param timeout unit ms, timeout to receive data, default 0.
         *                 0 means read data in uart receive buffer and return immediately,
         *                -1 means block until read len data,
         *                >0 means block until read len data or timeout.
         * @return received data, bytes type.
         *         Attention, you need to delete the returned object yourself in C++.
         * @throw Read failed will raise err.Exception error.
         * @maixpy maix.peripheral.uart.UART.read
         */
        Bytes *read(int len = -1, int timeout = 0);

        /**
         * Read line from uart, that is read until '\n' or '\r\n'.
         * @param timeout unit ms, timeout to receive data, default -1 means block until read '\n' or '\r\n'.
         *                > 0 means block until read '\n' or '\r\n' or timeout.
         * @return received data, bytes type. If timeout will return the current received data despite not read '\n' or '\r\n'.
         *          e.g. If we want to read b'123\n', but when we only read b'12', timeout, then return b'12'.
         * @maixpy maix.peripheral.uart.UART.readline
        */
        Bytes *readline(int timeout = -1);

    private:
        int _fd;
        std::string      _uart_port;
        int              _baudrate;
        uart::BITS  _databits;
        uart::PARITY     _parity;
        uart::STOP  _stopbits;
        uart::FLOW_CTRL  _flow_ctrl;
        int         _one_byte_time_us;
        std::function<void(uart::UART&, Bytes&)> callback;
        thread::Thread *_read_thread;
        bool        _read_thread_need_exit;
        bool        _read_thread_exit;
    };

    err::Err register_comm_callback(uart::UART *obj, std::function<void(uart::UART*)> callback);

}; // namespace maix.peripheral.uart

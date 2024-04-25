/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <stdint.h>
#include <valarray>
#include "maix_err.hpp"
#include "maix_type.hpp"

namespace maix::comm
{
    /**
     * Communication base class, all communication methods should implement this interface
     * @maixcdk maix.comm.CommBase
     */
    class CommBase
    {
    public:
        virtual ~CommBase() {}

        /**
         * Open device, if already opened, do nothing and return err.ERR_NONE.
         * @return open device error code, err.Err type.
         * @maixcdk maix.comm.CommBase.open
         */
        virtual err::Err open() = 0;

        /**
         * Close device, if already closed, do nothing and return err.ERR_NONE.
         * @return close device error code, err.Err type.
         * @maixcdk maix.comm.CommBase.close
         */
        virtual err::Err close() = 0;

        /**
         * Check if opened
         * @return true if opened, else false.
         * @maixcdk maix.comm.CommBase.is_open
         */
        virtual bool is_open() = 0;

        /**
         * Send data to device
         * @param buff data buffer
         * @param len  data length need to send, the len must <= buff length.
         * @return sent data length, < 0 means error, value is -err.Err.
         * @maixcdk maix.comm.CommBase.write
         */
        virtual int write(const uint8_t *buff, int len) = 0;

        /**
         * @brief Send data to uart
         * @param[in] data data to send, bytes type. If you want to send str type, use str.encode() to convert.
         * @return sent length, int type, if < 0 means error, value is -err.Err.
         * @maixcdk maix.comm.CommBase.write
         */
        virtual int write(Bytes &data) = 0;

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
         * @maixcdk maix.comm.CommBase.read
         */
        virtual int read(uint8_t *buff, int buff_len, int recv_len, int timeout) = 0;

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
         * @maixcdk maix.comm.CommBase.read
         */
        virtual Bytes *read(int len, int timeout) = 0;
    };
}

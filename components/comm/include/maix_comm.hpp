/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#pragma once

#include <stdint.h>
#include <tuple>
#include <string>
#include "maix_type.hpp"
#include "maix_err.hpp"
#include "maix_protocol.hpp"
#include "maix_comm_base.hpp"

namespace maix
{
    /**
     * Communication protocol module
     */
    namespace comm
    {
        /**
         * Comm cmd
         * @maixpy maix.comm.CommCMD
         */
        enum CommCMD {
            CMD_APP_MAX = 0xC8,
            CMD_SET_REPORT = 0xF8,
            CMD_APP_LIST = 0xF9,
            CMD_START_APP = 0xFA,
            CMD_EXIT_APP = 0xFB,
            CMD_CUR_APP_INFO = 0xFC,
            CMD_APP_INFO = 0xFD,
            CMD_KEY = 0xFE,
            CMD_TOUCH = 0xFF
        };
        /**
         * Class for communication protocol
         * @maixpy maix.comm.CommProtocol
         */
        class CommProtocol
        {
        public:
            /**
             * Construct a new CommProtocol object
             * @param buff_size buffer size, default to 1024 bytes
             * @maixpy maix.comm.CommProtocol.__init__
             * @maixcdk maix.comm.CommProtocol.CommProtocol
             */
            CommProtocol(int buff_size = 1024, uint32_t header=protocol::HEADER);
            ~CommProtocol();

            /**
             * Read data to buffer, and try to decode it as maix.protocol.MSG object
             * @return decoded data, if nullptr, means no valid frame found.
             *         Attentioin, delete it after use in C++.
             * @maixpy maix.comm.CommProtocol.get_msg
             */
            protocol::MSG *get_msg();

            /**
             * Send response ok(success) message
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param body response body, can be null
             * @param body_len response body length, can be 0
             * @return send response error code, maix.err.Err type
             * @maixcdk maix.comm.CommProtocol.resp_ok
             */
            err::Err resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Send response ok(success) message
             * @param cmd CMD value
             * @param body response body, can be null
             * @param body_len response body length, can be 0
             * @return encoded data, if nullptr, means error, and the error code is -err.Err.
             *         Attentioin, delete it after use in C++.
             * @maixcdk maix.comm.CommProtocol.resp_ok
             */
            err::Err resp_ok(uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Send response ok(success) message
             * @param cmd CMD value
             * @param body response body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err.
             *         Attentioin, delete it after use in C++.
             * @maixpy maix.comm.CommProtocol.resp_ok
             */
            err::Err resp_ok(uint8_t cmd, Bytes *body = nullptr);

            /**
             * Send report message
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param body report body, can be null
             * @param body_len report body length, can be 0
             * @return send report error code, maix.err.Err type
             * @maixcdk maix.comm.CommProtocol.report
             */
            err::Err report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Send report message
             * @param cmd CMD value
             * @param body report body, can be null
             * @param body_len report body length, can be 0
             * @return encoded data, if nullptr, means error, and the error code is -err.Err.
             *         Attentioin, delete it after use in C++.
             * @maixcdk maix.comm.CommProtocol.report
             */
            err::Err report(uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Send report message
             * @param cmd CMD value
             * @param body report body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err.
             *         Attentioin, delete it after use in C++.
             * @maixpy maix.comm.CommProtocol.report
             */
            err::Err report(uint8_t cmd, Bytes *body = nullptr);

            /**
             * Encode response error message to buffer
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param code error code
             * @param msg error message
             * @return send response error code, maix.err.Err type
             * @maixcdk maix.comm.CommProtocol.resp_err
             */
            err::Err resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg);

            /**
             * Encode response error message to buffer
             * @param cmd CMD value
             * @param code error code
             * @param msg error message
             * @return encoded data, if nullptr, means error, and the error code is -err.Err.
             *         Attentioin, delete it after use in C++.
             * @maixpy maix.comm.CommProtocol.resp_err
             */
            err::Err resp_err(uint8_t cmd, err::Err code, const std::string &msg);

        private:
            void execute_cmd(protocol::MSG* msg);

        private:
            protocol::Protocol *_p;
            std::string _comm_method;
            CommBase *_comm;
            CommBase *_get_comm_obj(const std::string &method);
            uint8_t  *_tmp_buff;
            int       _tmp_buff_len;
        };
    } // namespace comm
} // namespace maix

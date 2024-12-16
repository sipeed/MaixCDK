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
         * @brief Add default CommProtocol listener.
         *
         * When the application uses this port, the listening thread will immediately
         * release the port resources and exit. If you need to start the default listening thread again,
         * please release the default port resources and then call this function.
         *
         * @maixpy maix.comm.add_default_comm_listener
         */
        void add_default_comm_listener();

        /**
         * @brief Remove default CommProtocol listener.
         *
         * @return bool type.
         *
         * @maixpy maix.comm.rm_default_comm_listener
         */
        bool rm_default_comm_listener();

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
             * @param header Customize header, default is maix.protocol.HEADER
             * @param method_none_raise If method set to "none", raise err.Exception() if method_none_raise is true. Default false,
             * if method is "none" and this arg is false, valid() function will return false and get_msg() always return none.
             * @throw Initialize failed will raise err::Exception()
             * @maixpy maix.comm.CommProtocol.__init__
             * @maixcdk maix.comm.CommProtocol.CommProtocol
             */
            CommProtocol(int buff_size = 1024, uint32_t header=maix::protocol::HEADER, bool method_none_raise = false);
            ~CommProtocol();

            /**
             * Read data to buffer, and try to decode it as maix.protocol.MSG object
             * @param timeout unit ms, 0 means return immediatly, -1 means block util have msg, >0 means block until have msg or timeout.
             * @return decoded data, if nullptr, means no valid frame found.
             *         Attentioin, delete it after use in C++.
             * @maixpy maix.comm.CommProtocol.get_msg
             */
            protocol::MSG *get_msg(int timeout = 0);

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


            /**
             * Is CommProtocol valid, only not valid when method not set to "none".
             * @return false if commprotocol method is "none".
             * @maixpy maix.comm.CommProtocol.valid
             */
            bool valid() { return _valid; }

            /**
             * Set CommProtocol method
             * @param method Can be "uart" or "none", "none" means not use CommProtocol.
             * @maixpy maix.comm.CommProtocol.set_method
             */
            static err::Err set_method(const std::string &method);

            /**
             * Get CommProtocol method
             * @return method Can be "uart" or "none", "none" means not use CommProtocol.
             * @maixpy maix.comm.CommProtocol.get_method
             */
            static std::string get_method();

        private:
            void execute_cmd(protocol::MSG* msg);

        private:
            protocol::Protocol *_p;
            std::string _comm_method;
            CommBase *_comm;
            CommBase *_get_comm_obj(const std::string &method, err::Err &error);
            uint8_t  *_tmp_buff;
            int       _tmp_buff_len;
            bool      _valid;
        };
    } // namespace comm
} // namespace maix

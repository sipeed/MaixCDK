/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <stdint.h>
#include <tuple>
#include <valarray>
#include <string>
#include "maix_err.hpp"
#include "maix_type.hpp"

namespace maix
{
    namespace protocol
    {
        /**
         * @brief protocol version
         * @maixpy maix.protocol.VERSION
        */
        const uint8_t VERSION = 1;

        /**
         * @brief protocol header
         * @maixpy maix.protocol.HEADER
        */
        extern uint32_t HEADER;

        /**
         * @brief protocol cmd, more doc see MaixCDK document's convention doc
         * @note max app custom CMD value should < CMD_APP_MAX
         * @maixpy maix.protocol.CMD
        */
        enum CMD
        {
            CMD_APP_MAX = 0xC8,     //  200, max app custom CMD value should < CMD_APP_MAX

            CMD_SET_REPORT   = 0xF8, // set auto upload data mode
            CMD_APP_LIST     = 0xF9,
            CMD_START_APP    = 0xFA,
            CMD_EXIT_APP     = 0xFB,
            CMD_CUR_APP_INFO = 0xFC,
            CMD_APP_INFO     = 0xFD,
            CMD_KEY          = 0xFE,
            CMD_TOUCH        = 0xFF,
        };

        /**
         * @brief protocol flags, more doc see MaixCDK document's convention doc
         * @maixpy maix.protocol.FLAGS
        */
        enum FLAGS
        {
            FLAG_REQ = 0x00,
            FLAG_RESP = 0x80,
            FLAG_IS_RESP_MASK = 0x80,

            FLAG_RESP_OK = 0x40,
            FLAG_RESP_ERR = 0x00,
            FLAG_RESP_OK_MASK = 0x40,

            FLAG_REPORT = 0x20,
            FLAG_REPORT_MASK = 0x20,

            FLAG_VERSION_MASK = 0x03
        };

        /**
         * @brief protocol msg
         * @maixpy maix.protocol.MSG
        */
        class MSG
        {
        public:
            MSG();
            ~MSG();

            /**
             * @brief protocol version
             * @maixpy maix.protocol.MSG.version
            */
            uint8_t version;

            /**
             * @brief Indicate response message type, true means CMD valid and the CMD processed correctly, (only for response msg)
             * @maixpy maix.protocol.MSG.resp_ok
            */
            uint8_t resp_ok;

            /**
             * @brief Flag whether CMD has been processed and responded to CMD sender.
             *        E.g. CMD CMD_START_APP will be automatically processed in CommProtocol.get_msg function,
             *             so the return msg will set this flag to true.
             * @maixpy maix.protocol.MSG.has_been_replied
             */
            bool has_been_replied{false};

            /**
             * @brief CMD value
             * @maixpy maix.protocol.MSG.cmd
            */
            uint8_t cmd;

            /**
             * @brief message is response or not, contrast with is_req
             * @maixpy maix.protocol.MSG.is_resp
            */
            bool is_resp;

            /**
             * @brief message is request or not, contrast with is_resp
             * @maixpy maix.protocol.MSG.is_req
            */
            bool is_req;

            /**
             * @brief message is request or not, contrast with is_resp
             * @maixpy maix.protocol.MSG.is_report
            */
            bool is_report;

            /**
             * Message body, read only, use set_body() to update
             * @attention DO NOT manually change this value
             * @maixcdk maix.protocol.MSG.body
            */
            uint8_t *body;

            /**
             * Message body length, read only, use set_body() to update
             * @attention DO NOT manually change this value
             * @maixpy maix.protocol.MSG.body_len
            */
            int body_len;

            /**
             * Encode response ok(success) message
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param body response body, can be null
             * @param body_len response body length, can be 0
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.MSG.encode_resp_ok
            */
            int encode_resp_ok(uint8_t *buff, int buff_len, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode response ok(success) message
             * @param body response body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.MSG.encode_resp_ok
            */
            Bytes *encode_resp_ok(uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode response ok(success) message
             * @param body response body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.MSG.encode_resp_ok
            */
            Bytes *encode_resp_ok(Bytes *body = nullptr);

            /**
             * Encode proactively report message
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param body report body, can be null
             * @param body_len report body length, can be 0
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.MSG.encode_report
            */
            int encode_report(uint8_t *buff, int buff_len, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode proactively report message
             * @param body report body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.MSG.encode_report
            */
            Bytes *encode_report(uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode proactively report message
             * @param body report body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.MSG.encode_report
            */
            Bytes *encode_report(Bytes *body = nullptr);

            /**
             * Encode response error message
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param code error code
             * @param msg error message
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.MSG.encode_resp_err
            */
            int encode_resp_err(uint8_t *buff, int buff_len, err::Err code, const std::string &msg);

            /**
             * Encode response error message
             * @param code error code
             * @param msg error message
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.MSG.encode_resp_err
            */
            Bytes *encode_resp_err(err::Err code, const std::string &msg);

            /**
             * Update message body
             * @param body_new new body data
             * @param body_len new body data length
             * @maixcdk maix.protocol.MSG.set_body
            */
            void set_body(uint8_t *body_new, int body_len);

            /**
             * Update message body
             * @param body_new new body data
             * @maixpy maix.protocol.MSG.set_body
            */
            void set_body(Bytes *body_new)
            {
                set_body(body_new->data, body_new->size());
            }

            /**
             * Get message body
             * @return message body, bytes type
             * @maixpy maix.protocol.MSG.get_body
            */
            Bytes *get_body()
            {
                return new Bytes(body, body_len);
            }

        private:
            int _body_buff_len;
        };

        /**
         * @brief Communicate protocol
         * @maixpy maix.protocol.Protocol
        */
        class Protocol
        {
        public:
            /**
             * @brief Construct a new Protocol object
             * @param buff_size Data queue buffer size
             * @maixpy maix.protocol.Protocol.__init__
             * @maixcdk maix.protocol.Protocol.Protocol
            */
            Protocol(int buff_size = 1024, uint32_t header=maix::protocol::HEADER);
            ~Protocol();

            /**
             * @brief Data queue buffer size
             * @maixpy maix.protocol.Protocol.buff_size
            */
            int buff_size() { return _buff_size; }

            /**
             * Add data to data queue
             * @param new_data new data
             * @param len new data length
             * @return error code, maybe err.Err.ERR_BUFF_FULL
             * @maixcdk maix.protocol.Protocol.push_data
            */
            err::Err push_data(uint8_t *new_data, int len);

            /**
             * Add data to data queue
             * @param new_data new data
             * @return error code, maybe err.Err.ERR_BUFF_FULL
             * @maixpy maix.protocol.Protocol.push_data
            */
            err::Err push_data(const Bytes *new_data);

            /**
             * Decode data in data queue and return a message
             * @param new_data new data add to data queue, if null, only decode.
             * @param len new data length, can be 0.
             * @return decoded message, if nullptr, means no message decoded.
             * @maixcdk maix.protocol.Protocol.decode
            */
            protocol::MSG *decode(uint8_t *new_data = nullptr, size_t len = 0);

            /**
             * Decode data in data queue and return a message
             * @param new_data new data add to data queue, if null, only decode.
             * @return decoded message, if nullptr, means no message decoded.
             * @maixpy maix.protocol.Protocol.decode
            */
            protocol::MSG *decode(const Bytes *new_data = nullptr);

            /**
             * Encode response ok(success) message to buffer
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param body response body, can be null
             * @param body_len response body length, can be 0
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.Protocol.encode_resp_ok
            */
            int encode_resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode response ok(success) message to buffer
             * @param cmd CMD value
             * @param body response body, can be null
             * @param body_len response body length, can be 0
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.Protocol.encode_resp_ok
            */
            Bytes *encode_resp_ok(uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode response ok(success) message to buffer
             * @param cmd CMD value
             * @param body response body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.Protocol.encode_resp_ok
            */
            Bytes *encode_resp_ok(uint8_t cmd, Bytes *body = nullptr);

            /**
             * Encode proactively report message to buffer
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param body report body, can be null
             * @param body_len report body length, can be 0
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.Protocol.encode_report
            */
            int encode_report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode proactively report message to buffer
             * @param cmd CMD value
             * @param body report body, can be null
             * @param body_len report body length, can be 0
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.Protocol.encode_report
            */
            Bytes *encode_report(uint8_t cmd, uint8_t *body = nullptr, int body_len = 0);

            /**
             * Encode proactively report message to buffer
             * @param cmd CMD value
             * @param body report body, can be null
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.Protocol.encode_report
            */
            Bytes *encode_report(uint8_t cmd, Bytes *body = nullptr);

            /**
             * Encode response error message to buffer
             * @param buff output buffer
             * @param buff_len output buffer length
             * @param cmd CMD value
             * @param code error code
             * @param msg error message
             * @return encoded data length, if < 0, means error, and the error code is -err.Err
             * @maixcdk maix.protocol.Protocol.encode_resp_err
            */
            int encode_resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg);

            /**
             * Encode response error message to buffer
             * @param cmd CMD value
             * @param code error code
             * @param msg error message
             * @return encoded data, if nullptr, means error, and the error code is -err.Err
             * @maixpy maix.protocol.Protocol.encode_resp_err
            */
            Bytes *encode_resp_err(uint8_t cmd, err::Err code, const std::string &msg);

        private:
            int _buff_size;
            uint8_t *_buff;
            int _data_len;
            uint32_t _header;
        };

        /**
         * @brief CRC16-IBM
         * @param data data
         * @param len data length
         * @return CRC16-IBM value, uint16_t type.
         * @maixcdk maix.protocol.crc16_IBM
        */
        uint16_t crc16_IBM(unsigned char *data, size_t len);

        /**
         * @brief CRC16-IBM
         * @param data data, bytes type.
         * @return CRC16-IBM value, uint16_t type.
         * @maixpy maix.protocol.crc16_IBM
        */
        uint16_t crc16_IBM(const Bytes *data);

        /**
         * @brief Encode message to buffer
         * @param out_buff output buffer
         * @param out_buff_len output buffer length
         * @param cmd CMD value
         * @param flags FLAGS value, @see maix.protocol.FLAGS
         * @param body message body, can be null
         * @param body_len message body length, can be 0
         * @param code error code, only for error message, that is FLAGS.FLAG_ERR in flags
         * @param version protocol version
         * @return encoded data length, if < 0, means error, and the error code is -err.Err
         * @maixcdk maix.protocol.encode
        */
        int encode(uint8_t *out_buff, int out_buff_len, uint8_t cmd, uint8_t flags, uint8_t *body, int body_len, uint8_t code = 0xFF, const uint8_t version = VERSION);

    } // namespace protocol
} // namespace maix

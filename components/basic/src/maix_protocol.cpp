/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_protocol.hpp"
#include <string.h>
#include <assert.h>

namespace maix::protocol
{
    uint32_t HEADER = 0xBBACCAAA;
    uint16_t crc16_IBM(uint8_t *ptr, size_t len)
    {
        unsigned int i;
        uint16_t crc = 0x0000;

        while (len--)
        {
            crc ^= *ptr++;
            for (i = 0; i < 8; ++i)
            {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xA001;
                else
                    crc = (crc >> 1);
            }
        }

        return crc;
    }

    uint16_t crc16_IBM(const Bytes *bytes)
    {
        return crc16_IBM(bytes->data, bytes->size());
    }

    int encode(uint8_t *out_buff, int out_buff_len,
               uint8_t cmd, uint8_t flags, uint8_t *body, int body_len,
               uint8_t code,
               const uint8_t version)
    {
        assert((uint64_t)out_buff % 4 == 0);

        if (version != VERSION)
            return -err::ERR_ARGS;
        if (out_buff_len < body_len + 12)
            return -err::ERR_ARGS;
        ((uint32_t *)out_buff)[0] = HEADER;
        ((uint32_t *)out_buff)[1] = body_len + 4;
        out_buff[8] = flags | version;
        out_buff[9] = cmd;
        if (code != 0xFF)
        {
            out_buff[10] = code;
            memcpy(out_buff + 11, body, body_len);
            uint16_t crc16 = crc16_IBM(out_buff, body_len + 11);
            out_buff[11 + body_len] = crc16 & 0xFF;
            out_buff[12 + body_len] = crc16 >> 8 & 0xFF;
            return body_len + 13;
        }
        memcpy(out_buff + 10, body, body_len);
        uint16_t crc16 = crc16_IBM(out_buff, body_len + 10);
        out_buff[10 + body_len] = crc16 & 0xFF;
        out_buff[11 + body_len] = crc16 >> 8 & 0xFF;
        return body_len + 12;
    }

    Bytes *encode_resp_ok(uint8_t cmd, uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[12 + body_len];
        int len = encode(buff, 12 + body_len, cmd, FLAG_RESP | FLAG_RESP_OK, body, body_len);
        if (len < 0)
        {
            delete[] buff;
            return nullptr;
        }
        Bytes *ret = new Bytes(buff, len, true, false);
        return ret;
    }

    Bytes *encode_resp_ok(uint8_t cmd, Bytes *body)
    {
        int body_len = body->size();
        return protocol::encode_resp_ok(cmd, body->data, body_len);
    }

    Bytes *encode_resp_err(uint8_t cmd, err::Err code, const std::string &msg)
    {
        uint8_t *buff = new uint8_t[13 + msg.length()];
        int len = encode(buff, 13 + msg.length(), cmd, FLAG_RESP | FLAG_RESP_ERR, (uint8_t *)msg.c_str(), msg.length(), code);
        if (len < 0)
        {
            delete[] buff;
            return nullptr;
        }
        Bytes *ret = new Bytes(buff, len, true, false);
        return ret;
    }

    int encode_resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        return encode(buff, buff_len, cmd, FLAG_RESP | FLAG_RESP_OK, body, body_len);
    }

    int encode_resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg)
    {
        return encode(buff, buff_len, cmd, FLAG_RESP | FLAG_RESP_ERR, (uint8_t *)msg.c_str(), msg.length(), code);
    }

    bool get_msg(uint8_t *data, int len, MSG *frame, int *idx, const uint32_t header=HEADER)
    {
        size_t data_len = 0;

        *idx = 0;
        if (len < 12)
            return false;
        // find header
        uint32_t i = 0;
        bool found = false;
        for (; i < (uint32_t)len - 4; i++)
        {
            if (data[i] == (header & 0xFF) &&
                data[i + 1] == ((header >> 8) & 0xFF) &&
                data[i + 2] == ((header >> 16) & 0xFF) &&
                data[i + 3] == ((header >> 24) & 0xFF))
            {
                found = true;
                break;
            }
        }
        // len >= 12
        if (!found)
        {
            *idx = i;
            return false;
        }
        if (len - i < 12)
            return false;

        // get data_len, and check data length
        data_len = data[i + 4] | (data[i + 5] << 8) | (data[i + 6] << 16) | (data[i + 7] << 24);
        if (data_len > len - i - 8)
            return false;
        *idx = i + 8 + data_len;
        // check crc
        uint16_t crc16 = crc16_IBM(data + i, data_len + 6);
        if (data[i + 6 + data_len] != (crc16 & 0xFF) || data[i + 7 + data_len] != (crc16 >> 8 & 0xFF))
        {
            return false;
        }
        // parse data
        frame->version = data[i + 8] & FLAG_VERSION_MASK;
        frame->is_resp = data[i + 8] & FLAG_IS_RESP_MASK;
        frame->is_req = !frame->is_resp;
        frame->is_report = data[i + 8] & FLAG_REPORT_MASK;
        frame->resp_ok = data[i + 8] & FLAG_RESP_OK_MASK;
        frame->cmd = data[i + 9];
        frame->set_body(data + i + 10, data_len - 4);
        frame->body_len = data_len - 4;
        return true;
    }

    std::tuple<MSG *, int> get_msg(uint8_t *data, int len)
    {
        MSG *frame = new MSG();
        int idx = 0;
        if (get_msg(data, len, frame, &idx))
            return std::make_tuple(frame, idx);
        else
        {
            delete frame;
            return std::make_tuple(nullptr, idx);
        }
    }

    MSG::MSG()
    {
        version = VERSION;
        is_resp = 0;
        is_req = 1;
        is_report = 0;
        resp_ok = 0;
        cmd = 0;
        body = nullptr;
        body_len = 0;
        _body_buff_len = 0;
    }

    MSG::~MSG()
    {
        if (body)
        {
            delete[] body;
        }
    }

    int MSG::encode_resp_ok(uint8_t *buff, int buff_len, uint8_t *body, int body_len)
    {
        return protocol::encode_resp_ok(buff, buff_len, this->cmd, body, body_len);
    }

    Bytes *MSG::encode_resp_ok(uint8_t *body, int body_len)
    {
        return protocol::encode_resp_ok(this->cmd, body, body_len);
    }

    Bytes *MSG::encode_resp_ok(Bytes *body)
    {
        return protocol::encode_resp_ok(this->cmd, body);
    }

    int MSG::encode_report(uint8_t *buff, int buff_len, uint8_t *body, int body_len)
    {
        return protocol::encode(buff, buff_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
    }

    Bytes *MSG::encode_report(uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[12 + body_len];
        int len = protocol::encode(buff, 12 + body_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
        if (len < 0)
        {
            delete[] buff;
            return nullptr;
        }
        Bytes *ret = new Bytes(buff, len, true, false);
        return ret;
    }

    Bytes *MSG::encode_report(Bytes *body)
    {
        uint8_t *buff = new uint8_t[12 + body_len];
        int len = protocol::encode(buff, 12 + body_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body->data, (int)body->size());
        if (len < 0)
        {
            delete[] buff;
            return nullptr;
        }
        Bytes *ret = new Bytes(buff, len, true, false);
        return ret;
    }

    int MSG::encode_resp_err(uint8_t *buff, int buff_len, err::Err code, const std::string &msg)
    {
        return protocol::encode_resp_err(buff, buff_len, this->cmd, code, msg);
    }

    Bytes *MSG::encode_resp_err(err::Err code, const std::string &msg)
    {
        return protocol::encode_resp_err(this->cmd, code, msg);
    }

    void MSG::set_body(uint8_t *body_new, int body_len)
    {
        if ((body && _body_buff_len < body_len))
        {
            delete[] body;
            body = new uint8_t[body_len];
            _body_buff_len = body_len;
        }
        else if (!body)
        {
            body = new uint8_t[body_len];
            _body_buff_len = body_len;
        }
        memcpy(body, body_new, body_len);
        this->body_len = body_len;
    }

    int Protocol::encode_resp_ok(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        return protocol::encode_resp_ok(buff, buff_len, cmd, body, body_len);
    }

    Bytes *Protocol::encode_resp_ok(uint8_t cmd, uint8_t *body, int body_len)
    {
        return protocol::encode_resp_ok(cmd, body, body_len);
    }

    Bytes *Protocol::encode_resp_ok(uint8_t cmd, Bytes *body)
    {
        return protocol::encode_resp_ok(cmd, body);
    }

    int Protocol::encode_report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        return protocol::encode(buff, buff_len, cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
    }

    Bytes *Protocol::encode_report(uint8_t cmd, uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[12 + body_len];
        int len = protocol::encode(buff, 12 + body_len, cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
        if (len < 0)
        {
            delete[] buff;
            return nullptr;
        }
        Bytes *ret = new Bytes(buff, len, true, false);
        return ret;
    }

    Bytes *Protocol::encode_report(uint8_t cmd, Bytes *body)
    {
        return Protocol::encode_report(cmd, body->data, body->size());
    }

    int Protocol::encode_resp_err(uint8_t *buff, int buff_len, uint8_t cmd, err::Err code, const std::string &msg)
    {
        return protocol::encode_resp_err(buff, buff_len, cmd, code, msg);
    }

    Bytes *Protocol::encode_resp_err(uint8_t cmd, err::Err code, const std::string &msg)
    {
        return protocol::encode_resp_err(cmd, code, msg);
    }

    Protocol::Protocol(int buff_size, uint32_t header)
    {
        _buff_size = buff_size;
        _buff = new uint8_t[buff_size];
        _data_len = 0;
        _header = header;
        HEADER = header;
    }

    Protocol::~Protocol()
    {
        delete[] _buff;
    }

    err::Err Protocol::push_data(uint8_t *new_data, int len)
    {
        if (_data_len + len > _buff_size)
            return err::ERR_BUFF_FULL;
        memcpy(_buff + _data_len, new_data, len);
        _data_len += len;
        return err::ERR_NONE;
    }

    err::Err Protocol::push_data(const Bytes *new_data)
    {
        return push_data((uint8_t *)&new_data[0], new_data->size());
    }

    MSG *Protocol::decode(uint8_t *new_data, size_t len)
    {
        if (len > 0)
        {
            push_data(new_data, len);
        }
        MSG *frame = new MSG();
        int idx = 0;
        if (get_msg(_buff, _data_len, frame, &idx, _header))
        {
            memmove(_buff, _buff + idx, _data_len - idx);
            _data_len -= idx;
            return frame;
        }
        if (idx > 0)
        {
            memmove(_buff, _buff + idx, _data_len - idx);
            _data_len -= idx;
        }
        delete frame;
        return nullptr;
    }

    MSG *Protocol::decode(const Bytes *new_data)
    {
        return decode((uint8_t *)&new_data[0], new_data->size());
    }

} // namespace maix::protocol

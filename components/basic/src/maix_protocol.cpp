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

    namespace
    {
        // Frame layout:
        // [header][data_len][flags][cmd][body][crc16]
        // RESP_ERR stores the error code as the first byte of body.
        constexpr int HEADER_LEN = sizeof(uint32_t);
        constexpr int DATA_LEN_FIELD_LEN = sizeof(uint32_t);
        constexpr int FLAGS_LEN = sizeof(uint8_t);
        constexpr int CMD_LEN = sizeof(uint8_t);
        constexpr int CRC_LEN = sizeof(uint16_t);
        constexpr int ERROR_CODE_LEN = sizeof(uint8_t);
        constexpr int DATA_LEN_OFFSET = HEADER_LEN;
        constexpr int FLAGS_OFFSET = HEADER_LEN + DATA_LEN_FIELD_LEN;
        constexpr int CMD_OFFSET = FLAGS_OFFSET + FLAGS_LEN;
        constexpr int BODY_OFFSET = CMD_OFFSET + CMD_LEN;
        constexpr int FRAME_DATA_OFFSET = HEADER_LEN + DATA_LEN_FIELD_LEN;
        constexpr int FRAME_DATA_OVERHEAD_LEN = FLAGS_LEN + CMD_LEN + CRC_LEN;
        constexpr int FRAME_OVERHEAD_LEN = HEADER_LEN + DATA_LEN_FIELD_LEN + FRAME_DATA_OVERHEAD_LEN;
    }

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
        if (body_len < 0 || (body_len > 0 && !body))
            return -err::ERR_ARGS;
        const int code_len = code == 0xFF ? 0 : ERROR_CODE_LEN;
        const int frame_len = body_len + FRAME_OVERHEAD_LEN + code_len;
        const int data_len = body_len + FRAME_DATA_OVERHEAD_LEN + code_len;
        if (out_buff_len < frame_len)
            return -err::ERR_ARGS;
        ((uint32_t *)out_buff)[0] = HEADER;
        ((uint32_t *)out_buff)[1] = data_len;
        out_buff[FLAGS_OFFSET] = flags | version;
        out_buff[CMD_OFFSET] = cmd;
        const int crc_offset = BODY_OFFSET + code_len + body_len;
        if (code != 0xFF)
        {
            out_buff[BODY_OFFSET] = code;
            if (body_len > 0)
                memcpy(out_buff + BODY_OFFSET + ERROR_CODE_LEN, body, body_len);
            uint16_t crc16 = crc16_IBM(out_buff, crc_offset);
            out_buff[crc_offset] = crc16 & 0xFF;
            out_buff[crc_offset + 1] = crc16 >> 8 & 0xFF;
            return frame_len;
        }
        if (body_len > 0)
            memcpy(out_buff + BODY_OFFSET, body, body_len);
        uint16_t crc16 = crc16_IBM(out_buff, crc_offset);
        out_buff[crc_offset] = crc16 & 0xFF;
        out_buff[crc_offset + 1] = crc16 >> 8 & 0xFF;
        return frame_len;
    }

    Bytes *encode_resp_ok(uint8_t cmd, uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[FRAME_OVERHEAD_LEN + body_len];
        int len = encode(buff, FRAME_OVERHEAD_LEN + body_len, cmd, FLAG_RESP | FLAG_RESP_OK, body, body_len);
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
        if (!body)
            return protocol::encode_resp_ok(cmd, nullptr, 0);
        int body_len = body->size();
        return protocol::encode_resp_ok(cmd, body->data, body_len);
    }

    Bytes *encode_resp_err(uint8_t cmd, err::Err code, const std::string &msg)
    {
        uint8_t *buff = new uint8_t[FRAME_OVERHEAD_LEN + ERROR_CODE_LEN + msg.length()];
        int len = encode(buff, FRAME_OVERHEAD_LEN + ERROR_CODE_LEN + msg.length(), cmd, FLAG_RESP | FLAG_RESP_ERR, (uint8_t *)msg.c_str(), msg.length(), code);
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
        if (len < FRAME_OVERHEAD_LEN)
            return false;
        // find header
        uint32_t i = 0;
        bool found = false;
        for (; i < (uint32_t)len - HEADER_LEN; i++)
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
        // Have at least one full minimal frame after the header position.
        if (!found)
        {
            *idx = i;
            return false;
        }
        if (len - i < FRAME_OVERHEAD_LEN)
            return false;

        // get data_len, and check data length
        data_len = data[i + DATA_LEN_OFFSET] |
                   (data[i + DATA_LEN_OFFSET + 1] << 8) |
                   (data[i + DATA_LEN_OFFSET + 2] << 16) |
                   (data[i + DATA_LEN_OFFSET + 3] << 24);
        if (data_len < FRAME_DATA_OVERHEAD_LEN || data_len > len - i - FRAME_DATA_OFFSET)
            return false;
        *idx = i + FRAME_DATA_OFFSET + data_len;
        // check crc
        const int crc_offset = i + FRAME_DATA_OFFSET + data_len - CRC_LEN;
        uint16_t crc16 = crc16_IBM(data + i, FRAME_DATA_OFFSET + data_len - CRC_LEN);
        if (data[crc_offset] != (crc16 & 0xFF) || data[crc_offset + 1] != (crc16 >> 8 & 0xFF))
        {
            return false;
        }
        // parse data
        frame->version = data[i + FLAGS_OFFSET] & FLAG_VERSION_MASK;
        frame->is_resp = data[i + FLAGS_OFFSET] & FLAG_IS_RESP_MASK;
        frame->is_req = !frame->is_resp;
        frame->is_report = data[i + FLAGS_OFFSET] & FLAG_REPORT_MASK;
        frame->resp_ok = data[i + FLAGS_OFFSET] & FLAG_RESP_OK_MASK;
        frame->cmd = data[i + CMD_OFFSET];
        frame->set_body(data + i + BODY_OFFSET, data_len - FRAME_DATA_OVERHEAD_LEN);
        frame->body_len = data_len - FRAME_DATA_OVERHEAD_LEN;
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
        if (!body)
            return protocol::encode_resp_ok(this->cmd, nullptr, 0);
        return protocol::encode_resp_ok(this->cmd, body);
    }

    int MSG::encode_report(uint8_t *buff, int buff_len, uint8_t *body, int body_len)
    {
        return protocol::encode(buff, buff_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
    }

    Bytes *MSG::encode_report(uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[FRAME_OVERHEAD_LEN + body_len];
        int len = protocol::encode(buff, FRAME_OVERHEAD_LEN + body_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
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
        int body_len = body ? (int)body->size() : 0;
        uint8_t *body_data = body ? body->data : nullptr;
        uint8_t *buff = new uint8_t[FRAME_OVERHEAD_LEN + body_len];
        int len = protocol::encode(buff, FRAME_OVERHEAD_LEN + body_len, this->cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body_data, body_len);
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
        if (!body)
            return protocol::encode_resp_ok(cmd, nullptr, 0);
        return protocol::encode_resp_ok(cmd, body);
    }

    int Protocol::encode_report(uint8_t *buff, int buff_len, uint8_t cmd, uint8_t *body, int body_len)
    {
        return protocol::encode(buff, buff_len, cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
    }

    Bytes *Protocol::encode_report(uint8_t cmd, uint8_t *body, int body_len)
    {
        uint8_t *buff = new uint8_t[FRAME_OVERHEAD_LEN + body_len];
        int len = protocol::encode(buff, FRAME_OVERHEAD_LEN + body_len, cmd, FLAG_RESP | FLAG_RESP_OK | FLAG_REPORT, body, body_len);
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
        if (!body)
            return Protocol::encode_report(cmd, nullptr, 0);
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
        return push_data(new_data->data, new_data->size());
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
        if (!new_data)
            return decode(nullptr, 0);
        return decode(new_data->data, new_data->size());
    }

} // namespace maix::protocol
